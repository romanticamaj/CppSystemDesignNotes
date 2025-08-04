#include <iostream>
#include <thread>
#include <atomic>
#include <array>
#include <vector>
#include <chrono>
#include <cstddef>
#include <iomanip>

// 一個無鎖、單生產者、單消費者的三緩衝區。
template<typename T>
class TripleBuffer {
private:
    static constexpr size_t kCacheLineSize = 64;

    struct alignas(kCacheLineSize) AlignedBuffer {
        T data{};
    };

public:
    explicit TripleBuffer() noexcept {
        // 初始狀態：
        // - 消費者擁有 buffers_[0] (front_buffer_)
        // - 生產者擁有 buffers_[1] (back_buffer_)
        // - buffers_[2] 在 ready_ptr_ 中等待
        front_buffer_ = &buffers_[0].data;
        back_buffer_ = &buffers_[1].data;
        ready_ptr_.store(&buffers_[2].data, std::memory_order_relaxed);
        has_new_data_.store(false, std::memory_order_relaxed);
    }

    // 生產者：寫入資料。
    [[nodiscard]] T& Write() noexcept {
        return *back_buffer_;
    }

    // 生產者：提交資料。
    void Commit() noexcept {
        // 原子性地交換 back_buffer_ 與 ready_ptr_
        T* old_ready = ready_ptr_.exchange(back_buffer_, std::memory_order_release);
        
        // 將舊的 ready buffer 作為新的 back buffer
        back_buffer_ = old_ready;
        
        // 設置新資料標記
        has_new_data_.store(true, std::memory_order_release);
    }

    // 消費者：讀取最新資料。
    [[nodiscard]] T& Read() noexcept {
        // 檢查是否有新資料
        if (has_new_data_.load(std::memory_order_acquire)) {
            // 獲取最新的緩衝區
            T* new_front = ready_ptr_.load(std::memory_order_acquire);
            
            // 如果確實有新的緩衝區且與當前不同
            if (new_front && new_front != front_buffer_) {
                // 交換緩衝區
                T* old_front = front_buffer_;
                front_buffer_ = new_front;
                
                // 將舊的 front buffer 放回 ready_ptr_（供生產者使用）
                ready_ptr_.store(old_front, std::memory_order_release);
                
                // 清除新資料標記
                has_new_data_.store(false, std::memory_order_release);
            }
        }
        return *front_buffer_;
    }

private:
    std::array<AlignedBuffer, 3> buffers_;
    std::atomic<T*> ready_ptr_;        // 指向準備好的緩衝區
    std::atomic<bool> has_new_data_;   // 是否有新資料的標記

    alignas(kCacheLineSize) T* front_buffer_;  // 消費者當前讀取的緩衝區
    alignas(kCacheLineSize) T* back_buffer_;   // 生產者當前寫入的緩衝區
};

// --- 使用範例 ---

struct AudioFrame {
    std::array<float, 512> samples;
    int frame_id = -1;
};

int main() {
    TripleBuffer<AudioFrame> buffer;
    std::atomic<bool> stop_flag{false};

    // 生產者執行緒（模擬音訊處理）
    std::thread producer([&]() {
        for (int id = 0; !stop_flag.load(std::memory_order_relaxed); ++id) {
            AudioFrame& frame = buffer.Write();

            frame.frame_id = id;
            for (size_t i = 0; i < frame.samples.size(); ++i) {
                frame.samples[i] = static_cast<float>(id % 100) * 0.01f;
            }

            buffer.Commit();
            std::cout << "生產者：已提交資料幀 " << frame.frame_id << std::endl;

            std::this_thread::sleep_for(std::chrono::microseconds(10600)); // 模擬 512 frame @ 48kHz
        }
    });

    // 消費者執行緒（模擬播放）
    std::thread consumer([&]() {
        int last_frame_id = -1;
    
        while (!stop_flag.load(std::memory_order_relaxed)) {
            const AudioFrame& frame = buffer.Read();

            if (frame.frame_id > last_frame_id) {
                last_frame_id = frame.frame_id;
                std::cout << "消費者：正在播放資料幀 " << frame.frame_id << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_flag.store(true, std::memory_order_relaxed);

    producer.join();
    consumer.join();

    return 0;
}
