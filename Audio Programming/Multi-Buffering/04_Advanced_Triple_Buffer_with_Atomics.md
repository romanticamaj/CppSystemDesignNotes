# 🎯 Advanced Triple Buffer with Atomics

這是一個 **Single Producer / Single Consumer Triple Buffer** 的進階實作範例，示範如何使用 `std::atomic` 同步**buffer 指標與 dirty flag**，並深入分析 memory ordering 的使用策略。

靈感來源：  
👉 [How I Learned to Stop Worrying and Love Juggling C++ Atomics](https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/)

---

## ✨ 核心特色

- **三個緩衝區**：Front (consumer)、Middle (交接)、Back (producer)
- **打包 Pointer + Dirty Flag**：用 `std::atomic<BufferSlot>` 保證交換操作的原子性
- **明確 Memory Order**：防止重排、確保資料可見性
- **避免偽共享**：使用 cache line 對齊優化性能

---

## 🟢 心法表：Memory Ordering

| 操作         | 角色     | 目的                                        | 建議 memory_order |
| ------------ | -------- | ------------------------------------------- | ----------------- |
| `load()`     | Consumer | 觀察 Producer 已寫好的資料                  | `acquire`         |
| `store()`    | Producer | 保證寫完成後通知 Consumer                   | `release`         |
| `exchange()` | Producer | 更新 pointer+flag，不需要看 Consumer 的資料 | `release`         |
| `exchange()` | Consumer | 同時看 Producer 寫+清除 dirty flag          | `acq_rel`         |

> 🟢 快速口訣
>
> - 看對方 → acquire
> - 通知對方 → release
> - 看+通知 → acq_rel
> - 什麼都不管 → relaxed（幾乎不用）

---

## 🟢 happens-before 鏈

這些 memory_order 保證：

1. **Producer `exchange(release)`**
   - 先寫 buffer，再發佈 `dirty=true`
2. **Consumer `load(acquire)`**
   - 保證看見 Producer 寫的 buffer pointer
3. **Consumer `exchange(acq_rel)`**
   - 同步取得 buffer+flag 並清除 dirty
   - 後續讀寫不會被提前

---

## 💻 優化後的實現

### 基礎版本

```cpp
#include <atomic>
#include <array>
#include <cstddef>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>

// Triple buffer example with atomic pointer+flag packing.
// Inspired by: https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/

template<typename T>
class TripleBuffer {
public:
    TripleBuffer() {
        // 初始化指針指向對應的緩衝區
        front_buffer_ = &buffers_[0].data;
        back_buffer_ = &buffers_[2].data;
        middle_.store({&buffers_[1].data, false}, std::memory_order_relaxed);
    }

    // Consumer side: 獲取最新提交的緩衝區（如果沒有新數據則返回舊的）
    T& Read() {
        // 方案 1: 語義清晰的命名
        BufferSlot available_data = middle_.load(std::memory_order_acquire);
        
        if (available_data.dirty) {
            // 有新數據：交換緩衝區並清除 dirty flag
            BufferSlot previous_middle = middle_.exchange(
                {front_buffer_, false},
                std::memory_order_acq_rel
            );
            front_buffer_ = previous_middle.ptr;
        }

        return *front_buffer_;
    }

    // 替代方案：更簡潔的寫法
    T& ReadAlternative() {
        // 方案 2: 直接檢查並交換
        if (auto slot = middle_.load(std::memory_order_acquire); slot.dirty) {
            front_buffer_ = middle_.exchange(
                {front_buffer_, false},
                std::memory_order_acq_rel
            ).ptr;
        }
        return *front_buffer_;
    }

    // 方案 3: 函數式風格
    T& ReadFunctional() {
        return *([this]() -> T* {
            auto slot = middle_.load(std::memory_order_acquire);
            if (slot.dirty) {
                auto old_slot = middle_.exchange(
                    {front_buffer_, false},
                    std::memory_order_acq_rel
                );
                front_buffer_ = old_slot.ptr;
            }
            return front_buffer_;
        })();
    }

    // Producer side: 獲取後端緩衝區進行寫入
    T& Write() {
        return *back_buffer_;
    }

    // Producer side: 提交寫入的數據並交換緩衝區
    void Commit() {
        // 原子性地交換 back 與 middle 並設置 dirty flag
        BufferSlot old_middle_slot = middle_.exchange(
            {back_buffer_, true},
            std::memory_order_release
        );
        back_buffer_ = old_middle_slot.ptr;
    }

    // 獲取統計信息（用於調試）
    bool HasNewData() const {
        return middle_.load(std::memory_order_acquire).dirty;
    }

private:
    // 避免偽共享的緩存行大小
    static constexpr size_t kCacheLineSize = 64;

    // 對齊到緩存行的緩衝區結構
    struct alignas(kCacheLineSize) Buffer {
        T data{};
    };

    // 封裝指針 + dirty flag
    struct BufferSlot {
        T* ptr;
        bool dirty;

        // 確保結構體大小適合原子操作
        BufferSlot() : ptr(nullptr), dirty(false) {}
        BufferSlot(T* p, bool d) : ptr(p), dirty(d) {}
    };

    // 三個緩衝區
    std::array<Buffer, 3> buffers_;

    // 原子的緩衝區指針 + dirty flag
    std::atomic<BufferSlot> middle_;

    // 僅 consumer 訪問，避免偽共享
    alignas(kCacheLineSize) T* front_buffer_;

    // 僅 producer 訪問，避免偽共享
    alignas(kCacheLineSize) T* back_buffer_;
};
```

### 進階版本：帶版本號和統計

```cpp
template<typename T>
class AdvancedTripleBuffer {
public:
    AdvancedTripleBuffer() {
        front_buffer_ = &buffers_[0].data;
        back_buffer_ = &buffers_[2].data;
        middle_.store({&buffers_[1].data, false, 0}, std::memory_order_relaxed);
    }

    // Consumer side: 讀取最新數據
    std::pair<T&, uint64_t> Read() {
        BufferSlot pending_slot = middle_.load(std::memory_order_acquire);

        if (pending_slot.dirty && pending_slot.version > last_read_version_) {
            // 有新數據且版本號更新：執行緩衝區交換
            BufferSlot consumed_slot = middle_.exchange(
                {front_buffer_, false, pending_slot.version},
                std::memory_order_acq_rel
            );
            
            front_buffer_ = consumed_slot.ptr;
            last_read_version_ = pending_slot.version;
            stats_.reads_completed.fetch_add(1, std::memory_order_relaxed);
            
        } else if (pending_slot.dirty) {
            // 有新數據但版本號相同（重複讀取）
            stats_.duplicate_reads.fetch_add(1, std::memory_order_relaxed);
        } else {
            // 沒有新數據
            stats_.no_new_data.fetch_add(1, std::memory_order_relaxed);
        }

        return {*front_buffer_, last_read_version_};
    }

    // Producer side: 寫入數據
    T& Write() {
        return *back_buffer_;
    }

    // Producer side: 提交數據
    void Commit() {
        current_write_version_++;

        auto [old_middle, old_dirty, old_version] = middle_.exchange(
            {back_buffer_, true, current_write_version_},
            std::memory_order_release
        );

        back_buffer_ = old_middle;
        stats_.writes_completed.fetch_add(1, std::memory_order_relaxed);

        if (old_dirty) {
            // 上一個數據還沒被讀取就被覆蓋了
            stats_.data_overwritten.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 獲取統計信息
    struct Stats {
        std::atomic<uint64_t> writes_completed{0};
        std::atomic<uint64_t> reads_completed{0};
        std::atomic<uint64_t> duplicate_reads{0};
        std::atomic<uint64_t> no_new_data{0};
        std::atomic<uint64_t> data_overwritten{0};
    };

    const Stats& GetStats() const { return stats_; }

    void ResetStats() {
        stats_.writes_completed.store(0, std::memory_order_relaxed);
        stats_.reads_completed.store(0, std::memory_order_relaxed);
        stats_.duplicate_reads.store(0, std::memory_order_relaxed);
        stats_.no_new_data.store(0, std::memory_order_relaxed);
        stats_.data_overwritten.store(0, std::memory_order_relaxed);
    }

private:
    static constexpr size_t kCacheLineSize = 64;

    struct alignas(kCacheLineSize) Buffer {
        T data{};
    };

    struct BufferSlot {
        T* ptr;
        bool dirty;
        uint64_t version;

        BufferSlot() : ptr(nullptr), dirty(false), version(0) {}
        BufferSlot(T* p, bool d, uint64_t v) : ptr(p), dirty(d), version(v) {}
    };

    std::array<Buffer, 3> buffers_;
    std::atomic<BufferSlot> middle_;

    alignas(kCacheLineSize) T* front_buffer_;
    alignas(kCacheLineSize) T* back_buffer_;

    alignas(kCacheLineSize) uint64_t current_write_version_{0};
    alignas(kCacheLineSize) uint64_t last_read_version_{0};

    alignas(kCacheLineSize) Stats stats_;
};
```

## 🎨 命名風格建議

### 變數命名的語義化改進

在原子操作中，好的變數命名能大幅提升程式碼可讀性。以下是幾種推薦的命名風格：

#### 1. 描述性命名（推薦）
```cpp
// ❌ 不好的命名
auto middle_status = middle_.load(std::memory_order_acquire);

// ✅ 好的命名 - 描述變數的用途
BufferSlot available_data = middle_.load(std::memory_order_acquire);
BufferSlot pending_update = middle_.load(std::memory_order_acquire);
BufferSlot shared_state = middle_.load(std::memory_order_acquire);
```

#### 2. 動作導向命名
```cpp
// 強調動作和結果
BufferSlot to_consume = middle_.load(std::memory_order_acquire);
BufferSlot ready_buffer = middle_.load(std::memory_order_acquire);
BufferSlot next_available = middle_.load(std::memory_order_acquire);
```

#### 3. 時間序列命名
```cpp
// 強調時間關係
BufferSlot current_snapshot = middle_.load(std::memory_order_acquire);
BufferSlot latest_commit = middle_.load(std::memory_order_acquire);
BufferSlot previous_state = middle_.exchange(...);
```

#### 4. 角色導向命名
```cpp
// 從 Consumer 角度命名
BufferSlot consumer_view = middle_.load(std::memory_order_acquire);
BufferSlot reader_snapshot = middle_.load(std::memory_order_acquire);

// 從 Producer 角度命名
BufferSlot producer_handoff = middle_.exchange(...);
BufferSlot writer_commit = middle_.exchange(...);
```

### 函數命名的改進建議

#### 1. 更具體的函數名
```cpp
// 基本版本
T& Read()                    // 通用但不夠具體
T& GetLatestData()          // 更明確的意圖
T& ConsumeBuffer()          // 強調消費動作
T& AcquireReadBuffer()      // 強調獲取動作

// 進階版本
std::pair<T&, uint64_t> Read()                    // 通用
std::pair<T&, uint64_t> ReadWithVersion()        // 明確返回版本
std::pair<T&, uint64_t> GetLatestWithMetadata()  // 強調元數據
```

#### 2. 狀態檢查函數
```cpp
// ❌ 模糊的命名
bool HasNewData() const

// ✅ 清晰的命名
bool IsDataAvailable() const
bool HasPendingUpdate() const
bool IsBufferReady() const
bool CanRead() const
```

### 完整的改進範例

```cpp
template<typename T>
class SemanticTripleBuffer {
public:
    // 使用語義化的函數和變數命名
    T& ConsumeLatestData() {
        BufferSlot available_content = shared_slot_.load(std::memory_order_acquire);
        
        if (available_content.dirty) {
            BufferSlot consumed_slot = shared_slot_.exchange(
                {consumer_buffer_, false},
                std::memory_order_acq_rel
            );
            consumer_buffer_ = consumed_slot.ptr;
        }
        
        return *consumer_buffer_;
    }
    
    void PublishData() {
        BufferSlot released_slot = shared_slot_.exchange(
            {producer_buffer_, true},
            std::memory_order_release
        );
        producer_buffer_ = released_slot.ptr;
    }
    
    bool IsNewDataAvailable() const {
        return shared_slot_.load(std::memory_order_acquire).dirty;
    }

private:
    std::atomic<BufferSlot> shared_slot_;      // 共享交換點
    T* consumer_buffer_;                       // 消費者緩衝區
    T* producer_buffer_;                       // 生產者緩衝區
};
```

### 命名原則總結

1. **避免縮寫**：`middle_status` → `available_data`
2. **使用動詞**：`Read()` → `ConsumeBuffer()`
3. **描述用途**：`slot` → `shared_state`
4. **保持一致**：統一使用 `consumer_` 或 `reader_` 前綴
5. **避免歧義**：`current` 可能指當前時間或當前狀態，要明確

## 🧪 使用範例和測試

### 基本使用範例

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

struct AudioFrame {
    std::array<float, 1024> samples;
    uint64_t timestamp;
    int frame_id;

    AudioFrame() : timestamp(0), frame_id(0) {
        samples.fill(0.0f);
    }
};

void basic_example() {
    TripleBuffer<AudioFrame> buffer;
    std::atomic<bool> stop_flag{false};

    // Producer thread (音訊生成)
    std::thread producer([&buffer, &stop_flag]() {
        int frame_id = 0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

        while (!stop_flag.load(std::memory_order_relaxed)) {
            // 獲取寫入緩衝區
            AudioFrame& write_frame = buffer.Write();

            // 模擬音訊生成
            write_frame.frame_id = frame_id++;
            write_frame.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count();

            // 生成隨機音訊樣本
            for (auto& sample : write_frame.samples) {
                sample = dis(gen) * 0.1f; // 小音量
            }

            // 提交數據
            buffer.Commit();

            // 模擬 48kHz, 1024 樣本的處理時間
            std::this_thread::sleep_for(std::chrono::microseconds(21333)); // ~21.3ms
        }
    });

    // Consumer thread (音訊播放)
    std::thread consumer([&buffer, &stop_flag]() {
        int last_frame_id = -1;

        while (!stop_flag.load(std::memory_order_relaxed)) {
            // 讀取最新數據
            const AudioFrame& read_frame = buffer.Read();

            if (read_frame.frame_id > last_frame_id) {
                // 處理新的音訊幀
                std::cout << "播放幀 " << read_frame.frame_id
                         << ", 時間戳: " << read_frame.timestamp << std::endl;
                last_frame_id = read_frame.frame_id;

                // 模擬音訊播放處理
                std::this_thread::sleep_for(std::chrono::microseconds(20000)); // 20ms
            } else {
                // 沒有新數據，短暫等待
                std::this_thread::sleep_for(std::chrono::microseconds(1000)); // 1ms
            }
        }
    });

    // 運行 5 秒
    std::this_thread::sleep_for(std::chrono::seconds(5));
    stop_flag.store(true, std::memory_order_relaxed);

    producer.join();
    consumer.join();
}
```

### 進階測試：性能分析

```cpp
void performance_test() {
    AdvancedTripleBuffer<AudioFrame> buffer;
    std::atomic<bool> stop_flag{false};

    // 高頻生產者
    std::thread fast_producer([&buffer, &stop_flag]() {
        int frame_id = 0;
        while (!stop_flag.load(std::memory_order_relaxed)) {
            AudioFrame& frame = buffer.Write();
            frame.frame_id = frame_id++;
            frame.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count();

            buffer.Commit();

            // 高頻率：每 10ms 一幀
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // 低頻消費者
    std::thread slow_consumer([&buffer, &stop_flag]() {
        while (!stop_flag.load(std::memory_order_relaxed)) {
            auto [frame, version] = buffer.Read();

            std::cout << "讀取幀 " << frame.frame_id
                     << ", 版本: " << version << std::endl;

            // 低頻率：每 30ms 讀取一次
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    });

    // 統計線程
    std::thread stats_thread([&buffer, &stop_flag]() {
        while (!stop_flag.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            const auto& stats = buffer.GetStats();
            std::cout << "\n=== 統計信息 ===\n"
                     << "寫入完成: " << stats.writes_completed.load(std::memory_order_relaxed) << "\n"
                     << "讀取完成: " << stats.reads_completed.load(std::memory_order_relaxed) << "\n"
                     << "重複讀取: " << stats.duplicate_reads.load(std::memory_order_relaxed) << "\n"
                     << "無新數據: " << stats.no_new_data.load(std::memory_order_relaxed) << "\n"
                     << "數據覆蓋: " << stats.data_overwritten.load(std::memory_order_relaxed) << "\n"
                     << std::endl;
        }
    });

    // 運行 10 秒
    std::this_thread::sleep_for(std::chrono::seconds(10));
    stop_flag.store(true, std::memory_order_relaxed);

    fast_producer.join();
    slow_consumer.join();
    stats_thread.join();

    // 最終統計
    const auto& final_stats = buffer.GetStats();
    std::cout << "\n=== 最終統計 ===\n"
             << "總寫入: " << final_stats.writes_completed.load(std::memory_order_relaxed) << "\n"
             << "總讀取: " << final_stats.reads_completed.load(std::memory_order_relaxed) << "\n"
             << "數據利用率: " << (static_cast<double>(final_stats.reads_completed.load(std::memory_order_relaxed)) /
                                final_stats.writes_completed.load(std::memory_order_relaxed) * 100.0) << "%\n"
             << std::endl;
}

int main() {
    std::cout << "=== 基本範例 ===\n";
    basic_example();

    std::cout << "\n=== 性能測試 ===\n";
    performance_test();

    return 0;
}
```

## 🔧 編譯和運行

```bash
# 編譯（需要 C++17 或更高版本）
g++ -std=c++17 -O2 -pthread -o triple_buffer_test triple_buffer_test.cpp

# 運行
./triple_buffer_test
```

## 🧭 進階優化方向

### 1. 檢查原子操作是否真正無鎖

```cpp
template<typename T>
void check_lock_free() {
    using BufferSlot = typename TripleBuffer<T>::BufferSlot;

    std::cout << "BufferSlot 大小: " << sizeof(BufferSlot) << " 字節\n";
    std::cout << "std::atomic<BufferSlot> 是否無鎖: "
              << std::atomic<BufferSlot>{}.is_lock_free() << "\n";

    // 在某些平台上，如果結構體太大，可能會退化為使用鎖
    static_assert(std::atomic<BufferSlot>::is_always_lock_free,
                  "BufferSlot 必須始終無鎖");
}
```

### 2. 針對特定平台的優化

```cpp
// 針對 x86-64 的優化版本
#ifdef __x86_64__
template<typename T>
class OptimizedTripleBuffer {
    // 使用 128 位原子操作（如果支持）
    struct alignas(16) BufferSlot {
        T* ptr;
        uint64_t flags_and_version; // 打包 dirty flag 和版本號
    };

    std::atomic<BufferSlot> middle_;
    // ...
};
#endif
```

### 3. 多生產者/多消費者擴展

```cpp
// 注意：這需要額外的同步機制
template<typename T>
class MultiProducerTripleBuffer {
    // 需要使用 compare_exchange 循環來處理競爭
    bool TryCommit() {
        BufferSlot expected = middle_.load(std::memory_order_acquire);
        BufferSlot desired = {back_buffer_, true, ++current_version_};

        return middle_.compare_exchange_strong(
            expected, desired,
            std::memory_order_release,
            std::memory_order_relaxed
        );
    }
};
```

## 📊 性能特性

### 優勢

- **無鎖操作**：避免了互斥鎖的開銷
- **緩存友好**：避免偽共享，提高緩存效率
- **低延遲**：Producer 和 Consumer 可以並行工作
- **數據一致性**：保證讀取到的數據是完整的

### 限制

- **單生產者/單消費者**：不適用於多對多場景
- **內存使用**：需要三倍的緩衝區空間
- **平台依賴**：原子操作的性能因平台而異

## 📄 參考資料

- [How I Learned to Stop Worrying and Love Juggling C++ Atomics](https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/)
- [cppreference: std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [cppreference: std::memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order)
- [Herb Sutter: atomic Weapons](https://herbsutter.com/2013/02/11/atomic-weapons-the-c-memory-model-and-modern-hardware/)
