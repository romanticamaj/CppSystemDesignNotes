# 為無鎖緩衝打造的原子積木 (Atomic Building Blocks)

在 `01_多重緩衝機制概述.md` 中，我們了解到一個天真的、僅使用 `std::mutex` 的雙緩衝實作會阻塞即時執行緒，而無鎖 (lock-free) 設計是高效能的關鍵。

本章節將深入探討實現無鎖緩衝區交換的**原子積木 (Atomic Building Blocks)**，聚焦於「為什麼需要它們」以及「如何正確使用它們」。

---

## 1. 問題的根源：為何 `ptr = new_ptr` 在多執行緒中是危險的？

在單執行緒中，`Buffer* g_ptr = new_buffer;` 是一個簡單的操作。但在多執行緒環境下，這行程式碼隱藏著兩大風險：

1.  **競態條件 (Race Condition) / 資料撕裂 (Data Tearing)**：在 64 位元系統上，一個指標的寫入通常是原子性的。但在 32 位元系統上，寫入一個 64 位元指標可能需要兩個指令。如果一個執行緒在寫入到一半時被中斷，另一個執行緒可能會讀到一個「撕裂」的、無效的指標位址，導致程式崩潰。

2.  **指令重排 (Instruction Reordering)**：為了極致的效能，編譯器和 CPU 會對指令進行重排。考慮以下程式碼：

    ```cpp
    // Producer Thread
    g_buffer->fillData(123); // 1. 填充資料
    g_data_is_ready = true;  // 2. 設置旗標
    ```

    編譯器或 CPU 可能會認為這兩個操作互不相關，並將它們重排：

    ```cpp
    // Reordered by compiler/CPU
    g_data_is_ready = true;  // 2. 旗標先被設置了！
    g_buffer->fillData(123); // 1. 資料才開始填充
    ```

    如果此時 Consumer 執行緒介入，它會看到 `g_data_is_ready` 為 `true`，並開始處理一個**尚未準備好**的緩衝區，導致讀取到垃圾資料或舊資料。

**`std::atomic`** 正是為了解決這兩個問題而生的。它不僅保證操作的**原子性**，還能透過**記憶體順序 (Memory Ordering)** 來限制指令重排，建立跨執行緒的同步規則。

---

## 2. 核心同步模式：Acquire-Release 語義

在所有記憶體順序中，**Acquire-Release** 是構建無鎖 Producer-Consumer 模式的基石。

可以把它想像成一個**跨執行緒的同步合約**：

- **`store` with `std::memory_order_release`** (釋放語義)

  - **作用**：對一個原子變數進行寫入。
  - **合約**：保證在這次 `store` **之前**的所有記憶體寫入（無論是否為原子操作），對於其他執行緒中匹配的 `acquire` 操作都是可見的。
  - **比喻**：「我已經把所有東西都準備好了，現在正式發布！任何看到這個發布信號的人，都能看到我之前準備的所有東西。」

- **`load` with `std::memory_order_acquire`** (獲取語義)
  - **作用**：從一個原子變數讀取。
  - **合約**：保證在這次 `load` **之後**的所有記憶體讀取，都能看到執行 `release` 操作的那個執行緒在 `store` 之前的所有寫入。
  - **比喻**：「我確認收到了發布信號。現在，我可以安全地查看對方之前準備的所有東西了。」

這個「先準備好，再發布」的模式是我們實現無鎖緩衝區交換的**核心思想**。

---

## 3. 實現緩衝區交換的關鍵 API

### A. `load` 與 `store`：最基本的讀寫

這是 Acquire-Release 模式最直接的體現，通常用於一個「資料指標」和一個「就緒旗標」的組合。

```cpp
#include <atomic>
#include <vector>

// 全局共享資源
std::atomic<std::vector<float>*> g_atomic_ptr { nullptr };
std::atomic<bool> g_data_ready { false };

// --- Producer Thread (e.g., Audio Processing Thread) ---
void produce_new_data() {
    auto new_buffer = new std::vector<float>(1024, 1.0f); // 準備新資料

    // 1. 先用 relaxed 順序儲存指標，因為同步點在後面的旗標
    g_atomic_ptr.store(new_buffer, std::memory_order_relaxed);

    // 2. 使用 release 語義設置旗標，這是一個「發布」操作
    //    這個 store 會確保上面的 store 指令不會被重排到它後面
    g_data_ready.store(true, std::memory_order_release);
}

// --- Consumer Thread (e.g., Audio Playback Thread) ---
void consume_data() {
    // 1. 使用 acquire 語義檢查旗標，這是一個「獲取」操作
    if (g_data_ready.load(std::memory_order_acquire)) {

        // 2. 如果旗標為 true，acquire 語義保證我們能安全地看到 g_atomic_ptr 的最新值
        std::vector<float>* buffer = g_atomic_ptr.load(std::memory_order_relaxed);

        // ... 使用 buffer ...

        // (可選) 重置旗標
        g_data_ready.store(false, std::memory_order_relaxed);
    }
}
```

### B. `exchange`：簡潔的指針交換

當我們的邏輯是「用一個新指標替換舊指標，並拿回舊指標」時，`exchange` 提供了一個更簡潔、高效的單一操作。

`exchange` 將「讀取舊值」和「寫入新值」合併為一個不可分割的原子操作。

```cpp
#include <atomic>

// Producer 持有 write_buffer，Consumer 持有 read_buffer
// Producer 完成後，想和 Consumer 交換
std::atomic<Buffer*> g_shared_ptr;

// --- Producer Thread ---
// write_buffer 已經填滿資料
void swap_with_consumer(Buffer* write_buffer) {
    // 原子性地將 g_shared_ptr 設為 write_buffer，並返回它之前的值
    // acq_rel 意味著：
    // - acquire: 讀取 g_shared_ptr 的當前值
    // - release: 寫入 write_buffer，並將 write_buffer 的內容發布出去
    Buffer* old_ptr = g_shared_ptr.exchange(write_buffer, std::memory_order_acq_rel);

    // 現在 Producer 拿到了之前 Consumer 正在讀的 buffer (old_ptr)
    // Producer 可以開始向這個新的 write_buffer 寫入資料
}
```

`exchange` 是實現一個簡單雙緩衝交換機制的理想工具。

### C. `compare_exchange_strong` (CAS)：條件式更新的基石

有時候，我們的更新操作需要依賴當前的狀態。例如：「只有當緩-衝區處於『空閒』狀態時，我才要佔用它並設為『寫入中』」。這就是 **CAS (Compare-And-Swap)** 的用武之地。

`compare_exchange_strong` 會：

1. **比較**：原子變數的**當前值**是否與你提供的**期望值 (expected)** 相同。
2. **交換**：如果相同，就將其更新為**目標值 (desired)**，並返回 `true`。
3. **失敗**：如果不同（意味著被其他執行緒搶先修改了），則**不更新**，將**期望值 (expected)** 更新為原子變數的**實際值**，並返回 `false`。

```cpp
#include <atomic>

enum class State { Free, Writing, Full };
std::atomic<State> g_buffer_state { State::Free };

// --- Producer Thread ---
bool try_claim_buffer_for_writing() {
    State expected = State::Free; // 我期望緩衝區是空閒的

    // 嘗試將狀態從 Free 原子性地更新為 Writing
    // 如果成功，代表我成功佔領了緩衝區
    // 如果失敗，代表有其他執行緒已經修改了狀態 (例如，它變成了 Full)
    // 失敗時，expected 的值會被自動更新為當前的實際狀態
    return g_buffer_state.compare_exchange_strong(
        expected,
        State::Writing,
        std::memory_order_acq_rel
    );
}
```

CAS 是實現三緩衝區、環形緩衝區等更複雜無鎖資料結構的**核心**，因為它提供了解決多執行緒競爭的基礎機制。

---

## 4. 總結：如何選擇？

| 當你的意圖是...                                              | 優先選擇...                        | 核心思想                             |
| :----------------------------------------------------------- | :--------------------------------- | :----------------------------------- |
| **發布資料**：一個執行緒準備數據，另一個安全地讀取。         | `store(release)` / `load(acquire)` | 建立一個「發布-獲取」的同步點。      |
| **無條件交換**：立即交換兩個值（如指標），並拿回舊值。       | `exchange`                         | 將「讀-改-寫」合併為一個原子操作。   |
| **有條件更新**：操作依賴於當前狀態，且可能與其他執行緒競爭。 | `compare_exchange_strong`          | 「檢查再更新」，解決資源競爭的基礎。 |

掌握了這三組原子積木，你就擁有了構建高效、安全、無鎖的多重緩衝機制所需的最重要工具。
