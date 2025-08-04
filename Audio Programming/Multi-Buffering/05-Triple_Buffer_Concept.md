# 🎯 三緩衝區（Triple Buffer）：使用 Atomics 實現無鎖同步

本文件闡述了實現一個無鎖、**單生產者／單消費者三緩衝區（Single-Producer, Single-Consumer Triple Buffer）** 的核心概念。它著重於如何使用 `std::atomic` 來安全地同步資料，而無需使用互斥鎖（mutex）。

完整的 C++ 實作範例請參考：[`06-Triple_Buffer_Implementation.cpp`](./06-Triple_Buffer_Implementation.cpp)

---

## ✨ 核心原則

- **三個緩衝區**：
  1. **前緩衝區（Front Buffer）**：專供消費者讀取。
  2. **後緩衝區（Back Buffer）**：專供生產者寫入。
  3. **中介緩衝區（Middle Buffer）**：作為交接點，存放生產者最新提交的資料。
- **原子性交接**：一個 `std::atomic<SharedState>` 同時持有指向中介緩衝區的指標和一個 `dirty` 標記。這使得生產者和消費者能透過單一原子操作交換緩衝區。
- **記憶體順序（Memory Ordering）**：我們使用 `acquire-release` 語義，確保對緩衝區的寫入 **發生在（happen-before）** 消費者看到 `dirty` 標記之前，從而保證資料的一致性。

---

## 🟢 記憶體順序規則

| 操作         | 執行緒 | 目的                            | `std::memory_order` |
| :----------- | :----- | :------------------------------ | :------------------ |
| `exchange()` | 生產者 | 發布新緩衝區並設定 dirty 標記。 | `release`           |
| `load()`     | 消費者 | 檢查是否有新緩衝區可用。        | `acquire`           |
| `exchange()` | 消費者 | 獲取新緩衝區並清除 dirty 標記。 | `acq_rel`           |

---

## 💻 核心操作偽代碼

以下是 `TripleBuffer` 關鍵操作的偽代碼，展示了其運作邏輯。

### Class Structure

```cpp
template<typename T>
class TripleBuffer {
private:
    // 包含指標和 dirty 旗標的原子結構體
    std::atomic<SharedState> middle_;

    // 指向各個緩衝區的非原子指標
    T* front_buffer_; // 消費者使用
    T* back_buffer_;  // 生產者使用

    // 實際儲存資料的三個緩衝區
    std::array<AlignedBuffer, 3> buffers_;
};
```

### Producer: Write

```cpp
T& Write() {
    // 直接返回後端緩衝區的引用，供生產者寫入。
    // 這個緩衝區在此期間由生產者獨佔。
    return *back_buffer_;
}
```

### Producer: Commit

```cpp
void Commit() {
    // 1. 準備要提交的新狀態：包含指向後端緩衝區的指標和 true 旗標
    SharedState new_state = { back_buffer_, true };

    // 2. 原子性地交換中介狀態
    //    - `release` 語義確保對 back_buffer_ 的所有寫入都已完成
    SharedState previous_state = middle_.exchange(new_state, std::memory_order_release);

    // 3. 生產者取得舊的中介緩衝區，作為下一次寫入的目標
    back_buffer_ = previous_state.ptr;
}
```

### Consumer: Read

```cpp
T& Read() {
    // 1. 檢查是否有新資料
    //    - `acquire` 語義確保能看到生產者 `release` 的最新狀態
    if (middle_.load(std::memory_order_acquire).dirty) {

        // 2. 準備一個「乾淨」的狀態，將我們用完的 front_buffer 交還
        SharedState clean_state = { front_buffer_, false };

        // 3. 原子性地交換中介狀態，以獲取新資料並歸還舊緩衝區
        //    - `acq_rel` 同時滿足讀取和寫入的記憶體順序需求
        SharedState consumed_state = middle_.exchange(clean_state, std::memory_order_acq_rel);

        // 4. 將消費者的 front_buffer 指向剛取得的新緩衝區
        front_buffer_ = consumed_state.ptr;
    }

    // 5. 返回當前的前端緩衝區
    return *front_buffer_;
}
```

---

## 📄 參考資料

- [How I Learned to Stop Worrying and Love Juggling C++ Atomics](https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/)
- [cppreference: std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [cppreference: std::memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order)
