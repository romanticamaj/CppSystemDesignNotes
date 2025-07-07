# C++11 原子操作 (Atomic Operations)

## 核心概念

C++11 引入的原子操作（Atomic Operations）是一種用於多線程編程的同步機制，它提供了對共享變量的無鎖（lock-free）訪問方式。原子操作保證在多線程環境中對變量的讀寫操作是不可分割的，從而避免了數據競爭（data race）問題。

原子操作的核心特點：

1. **原子性（Atomicity）**：操作要麼完全執行，要麼完全不執行，不會被其他線程觀察到中間狀態
2. **可見性（Visibility）**：一個線程對原子變量的修改對其他線程立即可見
3. **順序性（Ordering）**：可以控制內存操作的順序，實現不同級別的內存序（memory ordering）
4. **無鎖（Lock-free）**：大多數原子操作不需要使用互斥鎖，提高了並發性能

C++11 標準庫通過 `<atomic>` 頭文件提供了原子類型和原子操作，包括：

- 原子類型（如 `std::atomic<T>`）
- 原子操作函數（如 `std::atomic_store`、`std::atomic_load` 等）
- 內存序枚舉（如 `std::memory_order_relaxed`、`std::memory_order_acquire` 等）
- 原子標誌類型（如 `std::atomic_flag`）

## 語法

### 原子類型

```cpp
// 基本原子類型
std::atomic<int> atomic_int(0);
std::atomic<bool> atomic_bool(false);
std::atomic<char> atomic_char('a');
std::atomic<void*> atomic_ptr(nullptr);

// 特化的原子類型
std::atomic_int a_int(0);       // 等同於 std::atomic<int>
std::atomic_bool a_bool(false); // 等同於 std::atomic<bool>
std::atomic_char a_char('a');   // 等同於 std::atomic<char>

// 原子標誌（最簡單的原子類型，保證無鎖）
std::atomic_flag flag = ATOMIC_FLAG_INIT;  // 必須初始化為清除狀態
```

### 原子操作

```cpp
// 存儲操作
atomic_int.store(10);  // 存儲值
atomic_int.store(20, std::memory_order_release);  // 帶內存序的存儲

// 載入操作
int value = atomic_int.load();  // 載入值
int value_acq = atomic_int.load(std::memory_order_acquire);  // 帶內存序的載入

// 交換操作
int old_value = atomic_int.exchange(30);  // 將值設為 30，返回舊值

// 比較並交換 (CAS)
int expected = 30;
bool success = atomic_int.compare_exchange_strong(expected, 40);
// 如果 atomic_int == expected，則設為 40 並返回 true
// 否則，將 expected 設為 atomic_int 的實際值並返回 false

// 原子算術操作
int prev = atomic_int.fetch_add(5);  // 加 5 並返回舊值
int prev2 = atomic_int.fetch_sub(3);  // 減 3 並返回舊值
int prev3 = atomic_int.fetch_and(0x0F);  // 按位與
int prev4 = atomic_int.fetch_or(0xF0);   // 按位或
int prev5 = atomic_int.fetch_xor(0xFF);  // 按位異或

// 原子標誌操作
bool was_set = flag.test_and_set();  // 設置標誌並返回之前的狀態
flag.clear();  // 清除標誌
```

### 內存序

```cpp
// 可用的內存序選項
std::memory_order_relaxed;  // 最寬鬆，僅保證操作的原子性
std::memory_order_consume;  // 數據依賴順序（較少使用）
std::memory_order_acquire;  // 獲取操作，建立同步關係的一端
std::memory_order_release;  // 釋放操作，建立同步關係的另一端
std::memory_order_acq_rel;  // 獲取+釋放語義
std::memory_order_seq_cst;  // 最嚴格，順序一致性（默認）

// 使用內存序的例子
atomic_int.store(10, std::memory_order_release);  // 釋放語義的存儲
int val = atomic_int.load(std::memory_order_acquire);  // 獲取語義的載入

// 釋放-獲取同步模式
// 線程 A
atomic_int.store(10, std::memory_order_release);

// 線程 B
int val = atomic_int.load(std::memory_order_acquire);
// 如果 val == 10，則線程 A 中 store 之前的所有寫入對線程 B 可見
```

### 原子操作函數

```cpp
// 非成員函數版本的原子操作
std::atomic<int> atomic_int(0);

// 存儲和載入
std::atomic_store(&atomic_int, 10);
int value = std::atomic_load(&atomic_int);

// 帶內存序的版本
std::atomic_store_explicit(&atomic_int, 20, std::memory_order_release);
int value_acq = std::atomic_load_explicit(&atomic_int, std::memory_order_acquire);

// 交換和 CAS
int old_value = std::atomic_exchange(&atomic_int, 30);
int expected = 30;
bool success = std::atomic_compare_exchange_strong(&atomic_int, &expected, 40);

// 算術操作
int prev = std::atomic_fetch_add(&atomic_int, 5);
int prev2 = std::atomic_fetch_sub(&atomic_int, 3);
```

## 與相似概念的比較

| 特性 | 原子操作 | 互斥鎖 | 信號量 | 條件變量 |
|------|---------|--------|--------|---------|
| 同步機制 | 無鎖 | 鎖定 | 鎖定 | 鎖定 |
| 適用場景 | 簡單共享變量 | 複雜共享資源 | 資源計數 | 條件等待 |
| 性能開銷 | 低 | 中到高 | 中 | 高 |
| 死鎖風險 | 無 | 有 | 有 | 有 |
| 實現複雜度 | 簡單 | 中等 | 中等 | 複雜 |
| 可擴展性 | 高 | 中 | 中 | 中到低 |
| 適合操作 | 單一變量讀寫 | 複雜數據結構 | 資源限制 | 線程協調 |
| 硬件支持 | 需要 | 不需要 | 不需要 | 不需要 |

## 使用範例

### 基本用法：原子計數器

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

std::atomic<int> counter(0);  // 原子計數器

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter++;  // 原子自增操作
    }
}

int main() {
    const int num_threads = 10;
    const int iterations_per_thread = 10000;
    
    std::vector<std::thread> threads;
    
    // 創建多個線程，每個線程增加計數器
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(increment_counter, iterations_per_thread));
    }
    
    // 等待所有線程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 輸出最終計數器值
    std::cout << "預期計數器值: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "實際計數器值: " << counter << std::endl;
    
    return 0;
}
```

### 使用不同內存序的原子操作

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>

std::atomic<bool> x(false);
std::atomic<bool> y(false);
std::atomic<int> z(0);

void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);  // 寬鬆序，僅保證原子性
    y.store(true, std::memory_order_release);  // 釋放序，建立同步點
}

void read_y_then_x() {
    // 獲取序，與對應的釋放操作同步
    while (!y.load(std::memory_order_acquire)) {
        // 自旋等待 y 變為 true
        std::this_thread::yield();
    }
    
    // 此時 x 必定為 true，因為 y 的釋放-獲取同步保證了 x 的可見性
    if (x.load(std::memory_order_relaxed)) {
        ++z;
    }
}

int main() {
    std::thread t1(write_x_then_y);
    std::thread t2(read_y_then_x);
    
    t1.join();
    t2.join();
    
    assert(z.load() == 1);  // 這個斷言永遠不會失敗
    std::cout << "z = " << z.load() << std::endl;
    
    return 0;
}
```

### 實現無鎖數據結構：原子堆疊

```cpp
#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <functional>

// 無鎖堆疊的節點
template<typename T>
class Node {
public:
    T data;
    Node* next;
    
    Node(const T& value) : data(value), next(nullptr) {}
};

// 無鎖堆疊實現
template<typename T>
class LockFreeStack {
private:
    std::atomic<Node<T>*> head;
    
public:
    LockFreeStack() : head(nullptr) {}
    
    ~LockFreeStack() {
        // 清理堆疊
        Node<T>* current = head.load();
        while (current) {
            Node<T>* next = current->next;
            delete current;
            current = next;
        }
    }
    
    // 入棧操作
    void push(const T& value) {
        Node<T>* new_node = new Node<T>(value);
        new_node->next = head.load(std::memory_order_relaxed);
        
        // 使用 CAS 操作確保原子性更新
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed)) {
            // 如果 CAS 失敗，new_node->next 已被更新為當前的 head
            // 所以我們只需要重試 CAS
        }
    }
    
    // 出棧操作
    bool pop(T& result) {
        Node<T>* current_head = head.load(std::memory_order_relaxed);
        
        // 如果堆疊為空，返回 false
        if (!current_head) {
            return false;
        }
        
        // 使用 CAS 操作確保原子性更新
        while (!head.compare_exchange_weak(current_head, current_head->next,
                                          std::memory_order_release,
                                          std::memory_order_relaxed)) {
            // 如果 CAS 失敗，current_head 已被更新為當前的 head
            // 檢查是否為空
            if (!current_head) {
                return false;
            }
        }
        
        // 獲取數據並刪除節點
        result = current_head->data;
        delete current_head;
        return true;
    }
    
    // 檢查堆疊是否為空
    bool empty() const {
        return head.load(std::memory_order_relaxed) == nullptr;
    }
};

// 測試無鎖堆疊
void test_lock_free_stack() {
    LockFreeStack<int> stack;
    const int num_threads = 4;
    const int ops_per_thread = 100000;
    
    // 計數器，用於驗證操作
    std::atomic<int> push_count(0);
    std::atomic<int> pop_count(0);
    std::atomic<int> sum_popped(0);
    
    // 創建生產者線程
    std::vector<std::thread> producers;
    for (int t = 0; t < num_threads; ++t) {
        producers.push_back(std::thread([&stack, &push_count, t, ops_per_thread]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                int value = t * ops_per_thread + i;
                stack.push(value);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        }));
    }
    
    // 創建消費者線程
    std::vector<std::thread> consumers;
    for (int t = 0; t < num_threads; ++t) {
        consumers.push_back(std::thread([&stack, &pop_count, &sum_popped]() {
            int value;
            while (pop_count.load(std::memory_order_relaxed) < push_count.load(std::memory_order_relaxed)) {
                if (stack.pop(value)) {
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                    sum_popped.fetch_add(value, std::memory_order_relaxed);
                } else {
                    // 如果堆疊暫時為空，讓出 CPU
                    std::this_thread::yield();
                }
            }
        }));
    }
    
    // 等待所有線程完成
    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }
    
    // 驗證結果
    std::cout << "總入棧操作: " << push_count << std::endl;
    std::cout << "總出棧操作: " << pop_count << std::endl;
    
    // 檢查是否所有元素都被處理
    if (stack.empty() && push_count == pop_count) {
        std::cout << "所有元素都被正確處理" << std::endl;
    } else {
        std::cout << "錯誤：有元素未被處理" << std::endl;
    }
}

int main() {
    test_lock_free_stack();
    return 0;
}
```

### 原子標誌實現自旋鎖

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

// 使用原子標誌實現的自旋鎖
class SpinLock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        // 自旋直到獲得鎖
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 自旋等待
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

// 使用 RAII 方式管理自旋鎖
class SpinLockGuard {
private:
    SpinLock& lock;
    
public:
    explicit SpinLockGuard(SpinLock& lock) : lock(lock) {
        lock.lock();
    }
    
    ~SpinLockGuard() {
        lock.unlock();
    }
    
    // 禁止拷貝和移動
    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;
};

// 共享資源
int shared_counter = 0;
SpinLock counter_lock;

void increment_with_spinlock(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        SpinLockGuard guard(counter_lock);
        ++shared_counter;
    }
}

int main() {
    const int num_threads = 10;
    const int iterations_per_thread = 10000;
    
    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 創建多個線程，每個線程增加計數器
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(increment_with_spinlock, iterations_per_thread));
    }
    
    // 等待所有線程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 輸出結果
    std::cout << "預期計數器值: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "實際計數器值: " << shared_counter << std::endl;
    std::cout << "執行時間: " << duration.count() << " 毫秒" << std::endl;
    
    return 0;
}
```

### 使用原子操作實現雙重檢查鎖定的單例模式

```cpp
#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class Singleton {
private:
    // 私有構造函數
    Singleton() {
        std::cout << "Singleton 實例被創建" << std::endl;
    }
    
    // 禁止拷貝和賦值
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    // 靜態原子指針，用於實例
    static std::atomic<Singleton*> instance;
    static std::mutex init_mutex;
    
public:
    // 獲取單例實例的靜態方法
    static Singleton* getInstance() {
        // 第一次檢查（無鎖）
        Singleton* p = instance.load(std::memory_order_acquire);
        if (p == nullptr) {
            // 如果實例不存在，獲取互斥鎖
            std::lock_guard<std::mutex> lock(init_mutex);
            // 第二次檢查（有鎖）
            p = instance.load(std::memory_order_relaxed);
            if (p == nullptr) {
                // 創建新實例
                p = new Singleton();
                // 使用 release 內存序確保所有初始化操作在 store 之前完成
                instance.store(p, std::memory_order_release);
            }
        }
        return p;
    }
    
    // 示例方法
    void doSomething() {
        std::cout << "Singleton 正在執行操作" << std::endl;
    }
    
    // 清理函數（實際應用中可能需要更複雜的資源管理）
    static void cleanup() {
        Singleton* p = instance.load(std::memory_order_acquire);
        if (p) {
            delete p;
            instance.store(nullptr, std::memory_order_release);
        }
    }
};

// 初始化靜態成員
std::atomic<Singleton*> Singleton::instance{nullptr};
std::mutex Singleton::init_mutex;

// 測試函數
void test_singleton() {
    Singleton* s = Singleton::getInstance();
    s->doSomething();
}

int main() {
    const int num_threads = 10;
    std::vector<std::thread> threads;
    
    // 創建多個線程，同時訪問單例
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(test_singleton));
    }
    
    // 等待所有線程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 清理
    Singleton::cleanup();
    
    return 0;
}
```

### 原子操作與內存屏障

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>

// 共享變量
std::atomic<bool> ready(false);
std::atomic<int> data(0);

void producer() {
    // 準備數據
    data.store(42, std::memory_order_relaxed);
    
    // 使用 release 內存序設置 ready 標誌
    // 這確保了 data 的存儲在 ready 的存儲之前完成
    ready.store(true, std::memory_order_release);
}

void consumer() {
    // 使用 acquire 內存序檢查 ready 標誌
    // 這確保了 ready 的載入在 data 的載入之前完成
    while (!ready.load(std::memory_order_acquire)) {
        // 自旋等待
        std::this_thread::yield();
    }
    
    // 此時，由於 release-acquire 同步，data 的值必定是 42
    assert(data.load(std::memory_order_relaxed) == 42);
    std::cout << "消費者讀取到數據: " << data.load(std::memory_order_relaxed) << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## 使用情境

### 適合使用原子操作的情況

1. **簡單共享變量**：當多個線程需要訪問和修改單個共享變量時
   ```cpp
   // 原子計數器
   std::atomic<int> counter(0);
   
   // 多線程增加計數器
   void increment() {
       counter.fetch_add(1, std::memory_order_relaxed);
   }
   ```

2. **標誌和狀態變量**：用於線程間的信號通知
   ```cpp
   // 使用原子布爾值作為完成標誌
   std::atomic<bool> task_completed(false);
   
   // 一個線程設置標誌
   void worker() {
       // 執行任務...
       task_completed.store(true, std::memory_order_release);
   }
   
   // 另一個線程檢查標誌
   void monitor() {
       while (!task_completed.load(std::memory_order_acquire)) {
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
       }
       // 任務已完成，進行後續處理...
   }
   ```

3. **無鎖數據結構**：實現高性能的並發數據結構
   ```cpp
   // 無鎖隊列的一部分實現
   template<typename T>
   class LockFreeQueue {
       // ...
       std::atomic<Node*> head;
       std::atomic<Node*> tail;
       
       void enqueue(T value) {
           Node* new_node = new Node(value);
           Node* old_tail = tail.exchange(new_node, std::memory_order_acq_rel);
           old_tail->next.store(new_node, std::memory_order_release);
       }
       // ...
   };
   ```

4. **自旋鎖和輕量級同步原語**：當需要短時間的互斥訪問時
   ```cpp
   // 使用原子標誌實現自旋鎖
   class SpinLock {
       std::atomic_flag flag = ATOMIC_FLAG_INIT;
   public:
       void lock() {
           while (flag.test_and_set(std::memory_order_acquire)) {}
       }
       void unlock() {
           flag.clear(std::memory_order_release);
       }
   };
   ```

5. **等待-通知機制**：實現簡單的線程協調
   ```cpp
   // 使用原子變量實現簡單的等待-通知機制
   std::atomic<bool> ready(false);
   
   // 通知線程
   void notify() {
       // 準備數據...
       ready.store(true, std::memory_order_release);
   }
   
   // 等待線程
   void wait_and_process() {
       while (!ready.load(std::memory_order_acquire)) {
           std::this_thread::yield();
       }
       // 處理數據...
   }
   ```

### 不適合使用原子操作的情況

1. **複雜的共享數據結構**：當需要對複雜數據結構進行原子更新時
   ```cpp
   // 不適合直接使用原子操作
   // 應該使用互斥鎖或讀寫鎖
   std::mutex data_mutex;
   std::map<int, std::string> shared_map;
   
   void update_map(int key, const std::string& value) {
       std::lock_guard<std::mutex> lock(data_mutex);
       shared_map[key] = value;
   }
   ```

2. **長時間操作**：當臨界區包含耗時操作時
   ```cpp
   // 不適合使用自旋鎖或原子操作
   // 應該使用互斥鎖
   std::mutex file_mutex;
   
   void write_to_file(const std::string& data) {
       std::lock_guard<std::mutex> lock(file_mutex);
       // 長時間的文件 I/O 操作...
   }
   ```

3. **複雜的線程協調**：當需要複雜的等待條件或多個線程協調時
   ```cpp
   // 不適合僅使用原子變量
   // 應該使用條件變量
   std::mutex mtx;
   std::condition_variable cv;
   bool ready = false;
   std::queue<int> data_queue;
   
   void producer() {
       // 生產數據...
       {
           std::lock_guard<std::mutex> lock(mtx);
           data_queue.push(42);
           ready = true;
       }
       cv.notify_one();
   }
   
   void consumer() {
       std::unique_lock<std::mutex> lock(mtx);
       cv.wait(lock, [] { return ready && !data_queue.empty(); });
       int data = data_queue.front();
       data_queue.pop();
       // 處理數據...
   }
   ```

4. **需要事務性操作**：當需要保證多個操作作為一個整體原子執行時
   ```cpp
   // 不適合使用分散的原子操作
   // 應該使用互斥鎖
   std::mutex account_mutex;
   int account_a = 1000;
   int account_b = 500;
   
   void transfer(int amount) {
       std::lock_guard<std::mutex> lock(account_mutex);
       if (account_a >= amount) {
           account_a -= amount;
           account_b += amount;
       }
   }
   ```

5. **硬件不支持**：當目標平台不支持某些原子操作時
   ```cpp
   // 在某些平台上，大型原子類型可能不支持真正的原子操作
   // 此時可能會退化為使用全局鎖
   std::atomic<LargeStruct> large_atomic;  // 可能不是真正無鎖的
   ```

### 最佳實踐

1. **選擇合適的內存序**：使用最寬鬆但仍滿足需求的內存序
   ```cpp
   // 對於獨立計數器，可以使用 relaxed 內存序
   counter.fetch_add(1, std::memory_order_relaxed);
   
   // 對於需要同步的操作，使用 acquire-release 內存序
   ready.store(true, std::memory_order_release);  // 生產者
   if (ready.load(std::memory_order_acquire)) {}  // 消費者
   ```

2. **避免過度使用**：原子操作不是萬能的，有時互斥鎖更合適
   ```cpp
   // 對於簡單操作，使用原子變量
   std::atomic<int> simple_counter;
   
   // 對於複雜操作，使用互斥鎖
   std::mutex complex_data_mutex;
   ```

3. **注意