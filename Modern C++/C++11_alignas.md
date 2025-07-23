# alignas (C++11)

`alignas` 是 C++11 引入的一個關鍵字，用於指定變數或類型的記憶體對齊方式。它允許開發者手動控制資料在記憶體中的佈局，這對於性能優化至關重要，特別是在處理多執行緒和硬體交互的場景中。

透過強制特定變數（例如，在多執行緒間共享的資料）對齊到 CPU 的快取行（Cache Line）邊界，`alignas` 可以有效避免「偽共享（False Sharing）」這一性能陷阱。

---

## ⚡️ 性能議題：偽共享 (False Sharing)

### 什麼是偽共享？

在現代多核心 CPU 中，為了加速記憶體存取，資料並非逐一位元組（byte）讀取，而是以稱為「**快取行（Cache Line）**」的區塊（通常為 64 位元組）為單位在主記憶體和 CPU 快取之間傳輸。

**偽共享** 發生在以下情況：

1. 多個執行緒在不同的 CPU 核心上執行。
2. 這些執行緒存取不同的變數。
3. 然而，這些變數恰好位於 **同一條快取行** 上。

當一個核心修改了其快取行中的資料時，CPU 的快取一致性協議（Cache Coherency Protocol）會強制其他核心上對應的快取行失效，並從主記憶體重新載入。這種不必要的快取同步會導致嚴重的性能下降。

### 如何解決？

解決方法是確保不同執行緒頻繁存取的變數位於**不同的快取行**上。在 C++ 中，我們可以使用 `alignas()` 關鍵字來強制變數或結構體對齊到快取行的大小。

---

## 🎯 核心概念

1. **語法**:

   - `alignas(expression)`: `expression` 必須是整數常數表達式，代表期望的對齊位元組數。例如 `alignas(8)`。
   - `alignas(type)`: 使用 `type` 的對齊要求。例如 `alignas(double)`。

2. **對齊值**:

   - 指定的對齊值必須是 2 的冪次方（例如 1, 2, 4, 8, 16, 32, 64...）。
   - `alignas(0)` 是無效的。

3. **過度對齊與基礎對齊**:

   - 每個類型都有一個由編譯器決定的**基礎對齊（fundamental alignment）**。
   - `alignas` 用於指定**擴展對齊（extended alignment）**，也稱為**過度對齊（over-alignment）**。
   - 如果 `alignas` 指定的值小於類型的基礎對齊，它將被忽略。

4. **與 `alignof` 的關係**:
   - `alignof(type)` 運算子可以查詢一個類型的對齊要求（以位元組為單位）。
   - `alignas` 設定對齊，`alignof` 查詢對齊。

## 💡 使用範例

### 1. 對齊基本變數

```cpp
// 將一個 char (通常對齊到 1 byte) 強制對齊到 8 bytes
alignas(8) char c1;

// 將一個 int 陣列對齊到 32 bytes
alignas(32) int int_array[10];
```

### 2. 對齊結構體 (struct) 或類別 (class)

您可以對齊整個結構體，或對齊其內部成員。

```cpp
#include <iostream>

// 整個結構體對齊到 16 bytes
struct alignas(16) MyStruct {
    int a;      // 4 bytes
    char b;     // 1 byte
    double c;   // 8 bytes
};

// 結構體內部的成員對齊
struct MemberAlignedStruct {
    alignas(16) int a; // 'a' 被強制對齊到 16-byte 邊界
    char b;
};

int main() {
    std::cout << "sizeof(MyStruct): " << sizeof(MyStruct) << std::endl;
    std::cout << "alignof(MyStruct): " << alignof(MyStruct) << std::endl;
    // 輸出通常會是：
    // sizeof(MyStruct): 16 (或更大，取決於編譯器)
    // alignof(MyStruct): 16

    std::cout << "sizeof(MemberAlignedStruct): " << sizeof(MemberAlignedStruct) << std::endl;
    std::cout << "alignof(MemberAlignedStruct): " << alignof(MemberAlignedStruct) << std::endl;
    // 輸出通常會是：
    // sizeof(MemberAlignedStruct): 32 (因為 a 的對齊要求)
    // alignof(MemberAlignedStruct): 16
}
```

### 3. 避免偽共享的實際應用

這是在多執行緒程式設計中 `alignas` 最重要的用途之一。

```cpp
#include <thread>

// 假設快取行大小為 64 bytes
// 在 C++17 中，可以使用 std::hardware_destructive_interference_size
constexpr size_t CACHE_LINE_SIZE = 64;

struct ThreadData {
    // 使用 alignas 確保 counter1 和 counter2 位於不同的快取行
    alignas(CACHE_LINE_SIZE) int counter1 = 0;
    alignas(CACHE_LINE_SIZE) int counter2 = 0;
};

// 如果不使用 alignas，定義會是這樣：
// struct ThreadData {
//     int counter1 = 0; // counter1 和 counter2 很可能在同一個快取行
//     int counter2 = 0;
// };

ThreadData data;

void thread1_func() {
    for (int i = 0; i < 10000000; ++i) {
        data.counter1++;
    }
}

void thread2_func() {
    for (int i = 0; i < 10000000; ++i) {
        data.counter2++;
    }
}

int main() {
    std::thread t1(thread1_func);
    std::thread t2(thread2_func);
    t1.join();
    t2.join();
    return 0;
}
```

在這個範例中，`alignas(CACHE_LINE_SIZE)` 確保 `counter1` 和 `counter2` 的記憶體位址至少相隔 64 位元組，因此它們不會共享同一個快取行。當 `thread1` 修改 `counter1` 時，不會導致 `thread2` 所在的 CPU 核心的快取行失效，從而避免了偽共享帶來的性能懲罰。

---

## 📊 視覺化記憶體佈局

讓我們用圖示來理解 `ThreadData` 結構體在記憶體中的佈局差異。假設快取行大小為 64 位元組。

### 情況一：不使用 `alignas`

```cpp
struct ThreadData {
    int counter1; // 4 bytes
    int counter2; // 4 bytes
};
```

記憶體佈局可能如下：

```
<----------------------------- Cache Line 1 (64 bytes) ----------------------------->
+----------+----------+--------------------------------------------------------------+
| counter1 | counter2 | 其他變數或未使用的空間...                                    |
| (4 bytes)| (4 bytes)|                                                              |
+----------+----------+--------------------------------------------------------------+
^          ^
|          |
Thread 1   Thread 2
存取此處   存取此處
```

**問題**：`counter1` 和 `counter2` 位於同一個快取行。當 Thread 1 修改 `counter1` 時，會導致整個 Cache Line 1 失效，即使 Thread 2 只關心 `counter2`，它也必須等待快取行從主記憶體重新載入，從而造成性能瓶頸。這就是**偽共享**。

### 情況二：使用 `alignas`

```cpp
struct ThreadData {
    alignas(64) int counter1;
    alignas(64) int counter2;
};
```

記憶體佈局變為：

```
<----------------------------- Cache Line 1 (64 bytes) ----------------------------->
+--------------------+---------------------------------------------------------------+
| counter1 (4 bytes) | PADDING (60 bytes of empty space)                             |
+--------------------+---------------------------------------------------------------+
^
|
Thread 1 存取此處

<----------------------------- Cache Line 2 (64 bytes) ----------------------------->
+--------------------+---------------------------------------------------------------+
| counter2 (4 bytes) | PADDING (60 bytes of empty space)                             |
+--------------------+---------------------------------------------------------------+
^
|
Thread 2 存取此處
```

**解決方案**：`alignas(64)` 強制 `counter1` 和 `counter2` 分別位於一個 64 位元組對齊的記憶體位址上。這意味著它們各自佔據一個獨立的快取行。現在，當 Thread 1 修改 `counter1` 時，只會影響 Cache Line 1，而不會干擾到 Thread 2 正在使用的 Cache Line 2。偽共享問題得以解決。

---

## ⚖️ 權衡與取捨 (Trade-offs)

正如您所觀察到的，`alignas` 並非沒有代價。它的主要權衡在於 **性能 vs. 記憶體用量**。

### 主要代價：增加記憶體消耗

- **填充 (Padding)**：為了滿足更嚴格的對齊要求，編譯器必須在變數之間或結構體末尾插入未使用的位元組，即「填充」。
- **空間浪費**：從上面的視覺化圖示可以看出，為了將 `counter1` (4 bytes) 和 `counter2` (4 bytes) 分開，我們浪費了 `(60 + 4) + (60) = 124` 個位元組的記憶體。原本只需要 8 個位元組的結構體，現在佔用了 128 個位元組。

### 何時該使用 `alignas`？

`alignas` 是一種**優化工具**，不應濫用。只有在特定情況下，用記憶體換取性能才是值得的：

1. **已確認的性能瓶頸**：

   - 只有當分析工具（Profiler）明確指出 **偽共享** 是造成性能問題的元兇時，才應考慮使用 `alignas` 來解決。
   - 過早優化是萬惡之源。不要在沒有證據的情況下隨意添加 `alignas`。

2. **高頻率、競爭性的寫入**：

   - 當多個執行緒**頻繁地寫入**位於同一個快取行的不同資料時，偽共享的影響最大。
   - 如果資料主要是讀取，或者寫入頻率很低，那麼偽共享的影響可能微不足道，不值得為此浪費記憶體。

3. **硬體交互**：
   - 與特定硬體（如 DMA 控制器、GPU）進行交互時，可能需要將資料結構對齊到特定的記憶體邊界（如分頁大小），這時 `alignas` 就非常必要。

### 總結

| 優點 (Pros)                                         | 缺點 (Cons)                                           |
| :-------------------------------------------------- | :---------------------------------------------------- |
| ✅ **提升性能**：有效避免偽共享，減少快取失效。     | ❌ **增加記憶體**：產生填充，導致空間浪費。           |
| ✅ **硬體兼容性**：滿足特定硬體的對齊要求。         | ❌ **增加複雜性**：需要理解底層硬體（快取行大小）。   |
| ✅ **原子操作優化**：某些架構上對齊的原子操作更快。 | ❌ **可能被濫用**：在非必要場景下使用會造成負面影響。 |

**經驗法則**：將 `alignas` 視為一把鋒利的手術刀，只在精準定位到問題（通常是偽共享）後才使用它，而不是當作一把可以隨意揮舞的錘子。
