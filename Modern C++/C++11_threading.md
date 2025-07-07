# C++11 多執行緒支援

## 核心概念

C++11 標準首次在語言層面引入了對多執行緒程式設計的原生支援，這是 C++ 語言的一個重大進步。在 C++11 之前，多執行緒程式設計需要依賴平台特定的 API（如 POSIX threads 或 Windows threads）或第三方函式庫，導致程式碼可移植性差、標準化程度低。

C++11 的多執行緒支援主要包含以下幾個核心組件：

1. **執行緒管理**：`std::thread` 類用於創建和管理執行緒
2. **互斥量與鎖**：`std::mutex`、`std::lock_guard`、`std::unique_lock` 等用於同步存取共享資源
3. **條件變數**：`std::condition_variable` 用於執行緒間的通知和等待
4. **原子操作**：`std::atomic` 模板類用於無鎖程式設計
5. **未來與承諾**：`std::future`、`std::promise` 用於異步任務結果的傳遞
6. **執行緒局部存儲**：`thread_local` 關鍵字用於創建執行緒特定的變數

這些組件共同提供了一個完整的多執行緒程式設計框架，使開發者能夠編寫高效、可移植的並行程式碼。C++11 的多執行緒支援不僅提高了程式碼的可移植性，還通過提供高層次的抽象，降低了多執行緒程式設計的複雜性和錯誤傾向性。

## 語法

```cpp
// 執行緒創建與管理
#include <thread>
std::thread t(function, args...);  // 創建執行緒
t.join();                          // 等待執行緒完成
t.detach();                        // 分離執行緒

// 互斥量與鎖
#include <mutex>
std::mutex mtx;
mtx.lock();                        // 手動鎖定
mtx.unlock();                      // 手動解鎖

std::lock_guard<std::mutex> lock(mtx);  // RAII 風格的鎖
std::unique_lock<std::mutex> ulock(mtx);  // 更靈活的 RAII 鎖

// 條件變數
#include <condition_variable>
std::condition_variable cv;
cv.wait(ulock);                    // 等待通知
cv.notify_one();                   // 通知一個等待的執行緒
cv.notify_all();                   // 通知所有等待的執行緒

// 原子操作
#include <atomic>
std::atomic<int> counter(0);
counter++;                         // 原子增加
int value = counter.load();        // 原子讀取

// 未來與承諾
#include <future>
std::promise<int> prom;
std::future<int> fut = prom.get_future();
prom.set_value(10);                // 設置值
int result = fut.get();            // 獲取結果

// 異步執行
auto fut = std::async(std::launch::async, function, args...);

// 執行緒局部存儲
thread_local int counter = 0;      // 每個執行緒有獨立的 counter
```

## 與相似概念的比較

| 特性 | C++11 執行緒 | POSIX Threads | Windows Threads | OpenMP | TBB |
|------|-------------|--------------|----------------|--------|-----|
| 標準化 | C++ 標準 | POSIX 標準 | Windows 專有 | 開放標準 | Intel 庫 |
| 可移植性 | 高 | 中（類 Unix） | 低（僅 Windows） | 高 | 高 |
| 抽象層級 | 中 | 低 | 低 | 高 | 高 |
| 細粒度控制 | 高 | 高 | 高 | 中 | 中 |
| 易用性 | 中 | 低 | 低 | 高 | 高 |
| 性能開銷 | 低 | 極低 | 極低 | 低 | 低 |
| 任務並行 | 手動 | 手動 | 手動 | 自動 | 自動 |
| 數據並行 | 手動 | 手動 | 手動 | 自動 | 自動 |
| 異常處理 | 支持 | 不支持 | 不支持 | 有限 | 支持 |
| 執行緒池 | 不內建 | 不內建 | 不內建 | 內建 | 內建 |

## 使用範例

### 基本執行緒創建與管理

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

// 執行緒函數
void worker(int id) {
    std::cout << "Worker " << id << " starting\n";
    
    // 模擬工作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Worker " << id << " finished\n";
}

int main() {
    std::cout << "Main thread starting\n";
    
    // 創建多個執行緒
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i);
    }
    
    std::cout << "Main thread: created " << threads.size() << " workers\n";
    
    // 等待所有執行緒完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Main thread: all workers completed\n";
    
    return 0;
}
```

### 互斥量與資料保護

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

class Counter {
private:
    int value = 0;
    std::mutex mtx;  // 保護 value 的互斥量
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);  // RAII 風格的鎖
        ++value;
    }
    
    int get_value() {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

void worker(Counter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
    }
}

int main() {
    Counter counter;
    const int num_threads = 10;
    const int iterations_per_thread = 1000;
    
    std::vector<std::thread> threads;
    
    // 創建多個執行緒，共享同一個 counter
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, std::ref(counter), iterations_per_thread);
    }
    
    // 等待所有執行緒完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 驗證結果
    std::cout << "Expected value: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "Actual value: " << counter.get_value() << std::endl;
    
    return 0;
}
```

### 條件變數與生產者-消費者模式

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>

// 執行緒安全的隊列
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool finished = false;
    
public:
    // 添加元素到隊列
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(item);
        }
        cv.notify_one();  // 通知一個等待的消費者
    }
    
    // 從隊列取出元素
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // 等待直到隊列非空或已完成
        cv.wait(lock, [this] { return !queue.empty() || finished; });
        
        if (queue.empty() && finished) {
            return false;  // 隊列為空且已完成
        }
        
        item = queue.front();
        queue.pop();
        return true;
    }
    
    // 標記隊列已完成
    void finish() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            finished = true;
        }
        cv.notify_all();  // 通知所有等待的消費者
    }
    
    // 檢查隊列是否為空
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
};

// 生產者函數
void producer(ThreadSafeQueue<int>& queue, int id, int num_items) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 500);
    
    for (int i = 0; i < num_items; ++i) {
        // 模擬工作
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        
        int item = id * 1000 + i;
        queue.push(item);
        std::cout << "Producer " << id << " produced: " << item << std::endl;
    }
}

// 消費者函數
void consumer(ThreadSafeQueue<int>& queue, int id) {
    int item;
    while (queue.pop(item)) {
        std::cout << "Consumer " << id << " consumed: " << item << std::endl;
        
        // 模擬處理時間
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "Consumer " << id << " finished" << std::endl;
}

int main() {
    ThreadSafeQueue<int> queue;
    
    const int num_producers = 3;
    const int num_consumers = 2;
    const int items_per_producer = 5;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // 創建消費者
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer, std::ref(queue), i);
    }
    
    // 創建生產者
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, std::ref(queue), i, items_per_producer);
    }
    
    // 等待所有生產者完成
    for (auto& t : producers) {
        t.join();
    }
    
    // 標記隊列已完成
    queue.finish();
    
    // 等待所有消費者完成
    for (auto& t : consumers) {
        t.join();
    }
    
    std::cout << "All threads finished" << std::endl;
    
    return 0;
}
```

### 原子操作與無鎖程式設計

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

// 使用原子變數的計數器
class AtomicCounter {
private:
    std::atomic<int> value{0};
    
public:
    void increment() {
        ++value;  // 原子增加
    }
    
    int get_value() const {
        return value.load();  // 原子讀取
    }
};

// 使用互斥量的計數器（用於比較）
class MutexCounter {
private:
    int value = 0;
    mutable std::mutex mtx;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++value;
    }
    
    int get_value() const {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

template<typename Counter>
void worker(Counter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
    }
}

template<typename Counter>
void run_test(const std::string& name, int num_threads, int iterations_per_thread) {
    Counter counter;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 創建執行緒
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker<Counter>, std::ref(counter), iterations_per_thread);
    }
    
    // 等待所有執行緒完成
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << name << " results:" << std::endl;
    std::cout << "  Expected value: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "  Actual value: " << counter.get_value() << std::endl;
    std::cout << "  Time taken: " << duration.count() << " ms" << std::endl;
}

int main() {
    const int num_threads = 10;
    const int iterations_per_thread = 1000000;
    
    run_test<AtomicCounter>("Atomic counter", num_threads, iterations_per_thread);
    run_test<MutexCounter>("Mutex counter", num_threads, iterations_per_thread);
    
    return 0;
}
```

### 未來與承諾

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>
#include <random>

// 計算向量中元素的和
int calculate_sum(const std::vector<int>& data) {
    std::cout << "Worker thread " << std::this_thread::get_id() 
              << " calculating sum of " << data.size() << " elements\n";
    
    // 模擬耗時計算
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return std::accumulate(data.begin(), data.end(), 0);
}

// 計算向量中元素的平均值
double calculate_average(const std::vector<int>& data) {
    std::cout << "Worker thread " << std::this_thread::get_id() 
              << " calculating average of " << data.size() << " elements\n";
    
    // 模擬耗時計算
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    int sum = std::accumulate(data.begin(), data.end(), 0);
    return static_cast<double>(sum) / data.size();
}

// 使用 promise 和 future 手動設置結果
void manual_task(std::promise<std::string>&& promise) {
    std::cout << "Manual task running in thread " << std::this_thread::get_id() << std::endl;
    
    try {
        // 模擬工作
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // 設置結果
        promise.set_value("Task completed successfully");
    }
    catch (...) {
        // 設置異常
        promise.set_exception(std::current_exception());
    }
}

int main() {
    // 生成隨機數據
    std::vector<int> data(10000000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (auto& item : data) {
        item = dis(gen);
    }
    
    std::cout << "Main thread " << std::this_thread::get_id() << " starting\n";
    
    // 使用 async 啟動異步任務
    std::future<int> sum_future = std::async(std::launch::async, calculate_sum, std::ref(data));
    std::future<double> avg_future = std::async(std::launch::async, calculate_average, std::ref(data));
    
    // 使用 promise 和 future
    std::promise<std::string> promise;
    std::future<std::string> manual_future = promise.get_future();
    std::thread manual_thread(manual_task, std::move(promise));
    
    std::cout << "Main thread: tasks started, waiting for results\n";
    
    // 主執行緒可以做其他工作
    std::cout << "Main thread: doing other work while waiting\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 獲取結果（如果尚未準備好，將阻塞）
    try {
        int sum = sum_future.get();
        std::cout << "Sum result: " << sum << std::endl;
        
        double average = avg_future.get();
        std::cout << "Average result: " << average << std::endl;
        
        std::string manual_result = manual_future.get();
        std::cout << "Manual task result: " << manual_result << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    
    // 等待手動執行緒完成
    manual_thread.join();
    
    std::cout << "Main thread: all tasks completed\n";
    
    return 0;
}
```

### 執行緒局部存儲

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <string>

// 執行緒局部變數
thread_local int counter = 0;
thread_local std::string thread_name = "Unknown";

void worker(int id) {
    // 設置執行緒名稱
    thread_name = "Thread-" + std::to_string(id);
    
    std::cout << thread_name << " starting with counter = " << counter << std::endl;
    
    // 修改執行緒局部變數
    for (int i = 0; i < 5; ++i) {
        ++counter;
        std::cout << thread_name << ": counter = " << counter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << thread_name << " finished with counter = " << counter << std::endl;
}

int main() {
    std::cout << "Main thread starting with counter = " << counter << std::endl;
    
    // 修改主執行緒的執行緒局部變數
    thread_name = "MainThread";
    counter = 100;
    
    std::cout << thread_name << ": counter set to " << counter << std::endl;
    
    // 創建工作執行緒
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker, i);
    }
    
    // 等待工作執行緒完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 主執行緒的執行緒局部變數不受影響
    std::cout << thread_name << ": counter = " << counter << " (unchanged)" << std::endl;
    
    return 0;
}
```

## 使用情境

### 適合使用多執行緒的情況

1. **I/O 綁定的操作**：當程式需要等待外部資源（如檔案、網路、資料庫）時：
   ```cpp
   std::vector<std::thread> threads;
   for (const auto& url : urls) {
       threads.emplace_back([url]() {
           download_file(url);  // I/O 綁定操作
       });
   }
   ```

2. **CPU 密集型計算**：將大型計算任務分解為可並行執行的子任務：
   ```cpp
   std::vector<std::future<Result>> futures;
   for (const auto& chunk : data_chunks) {
       futures.push_back(std::async(std::launch::async, process_chunk, chunk));
   }
   ```

3. **響應式 UI**：保持使用者介面響應，同時執行後台任務：
   ```cpp
   std::thread background_thread([this]() {
       // 執行耗時操作
       process_large_dataset();
       
       // 完成後通知 UI 執行緒
       ui_queue.push(UpdateUIMessage{});
   });
   background_thread.detach();  // UI 執行緒不等待
   ```

4. **服務器應用**：處理多個並發連接：
   ```cpp
   while (true) {
       Client client = accept_connection();
       std::thread client_thread(&Server::handle_client, this, client);
       client_thread.detach();
   }
   ```

5. **平行算法**：實現資料平行處理：
   ```cpp
   void parallel_for_each(std::vector<Data>& items, const std::function<void(Data&)>& func) {
       const size_t num_threads = std::thread::hardware_concurrency();
       std::vector<std::thread> threads;
       
       size_t items_per_thread = items.size() / num_threads;
       
       for (size_t i = 0; i < num_threads; ++i) {
           size_t start = i * items_per_thread;
           size_t end = (i == num_threads - 1) ? items.size() : (i + 1) * items_per_thread;
           
           threads.emplace_back([&items, &func, start, end]() {
               for (size_t j = start; j < end; ++j) {
                   func(items[j]);
               }
           });
       }
       
       for (auto& t : threads) {
           t.join();
       }
   }
   ```

### 不適合使用多執行緒的情況

1. **簡單、快速的操作**：執行緒創建和管理的開銷可能超過收益：
   ```cpp
   // 不值得為簡單操作創建執行緒
   std::thread t([]() { return a + b; });  // 開銷大於收益
   ```

2. **高度依賴共享狀態的任務**：需要頻繁同步的任務可能導致效能下降：
   ```cpp
   // 執行緒間頻繁同步可能導致效能瓶頸
   std::mutex mtx;
   for (int i = 0; i < 1000000; ++i) {
       std::lock_guard<std::mutex> lock(mtx);
       shared_data.update(i);  // 每次迭代都需要鎖
   }
   ```

3. **順序依賴的操作**：某些任務本質上是順序的，無法有效並行化：
   ```cpp
   // 每一步依賴前一步的結果
   result1 = step1();
   result2 = step2(result1);
   result3 = step3(result2);
   ```

4. **單核系統**：在只有一個 CPU 核心的系統上，多執行緒可能不會帶來效能提升：
   ```cpp
   // 在單核系統上，這可能只會增加上下文切換開銷
   if (std::thread::hardware_concurrency() <= 1) {
       // 考慮使用單執行緒方法
   }
   ```

5. **需要確定性執行順序的場景**：當結果必須完全可重現時：
   ```cpp
   // 多執行緒執行可能導致不確定的執行順序
   // 對於需要確定性結果的場景（如某些科學計算），可能不適合
   ```

### 最佳實踐

- 使用適當的同步機制保護共享資源，避免資料競爭
- 優先使用高層次的抽象（如 `std::async`、`std::future`）而非直接管理執行緒
- 避免過度使用執行緒，考慮系統的硬體並行能力（`std::thread::hardware_concurrency()`）
- 使用 RAII 風格的鎖（如 `std::lock_guard`、`std::unique_lock`）而非手動鎖定/解鎖
- 注意死鎖風險，使用 `std::lock` 或 `std::scoped_lock`（C++17）同時鎖定多個互斥量
- 盡量減少執行緒間的共享狀態，優先考慮消息傳遞或任務分解
- 使用條件變數而非輪詢等待，減少 CPU 資源浪費
- 考慮使用執行緒池而非為每個任務創建新執行緒
- 注意執行緒安全性，特別是在使用標準庫容器和算法時
- 使用原子操作處理簡單的共享狀態，避免不必要的鎖開銷
- 理解記憶體模型和記憶體順序，特別是在使用原子操作時
- 使用執行緒局部存儲減少共享狀態，提高並行性
- 考慮使用更高層次的並行程式設計模型，如任務並行（C++17 的 `std::execution`）