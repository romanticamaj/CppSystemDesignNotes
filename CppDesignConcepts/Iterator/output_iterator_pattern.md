# Output Iterator 設計模式深度解析

## 1. C++ STL 中採用 Output Iterator Pattern 的算法

C++ 標準庫廣泛使用 Output Iterator Pattern，將「計算邏輯」與「結果儲存」分離。以下是常見的採用此模式的算法：

| 算法                       | 函數簽名 (簡化)                                        | 用途                             |
| -------------------------- | ------------------------------------------------------ | -------------------------------- |
| `std::copy`                | `copy(first, last, d_first)`                           | 複製範圍內的元素                 |
| `std::transform`           | `transform(first, last, d_first, op)`                  | 對範圍內的元素應用函數並儲存結果 |
| `std::partial_sum`         | `partial_sum(first, last, d_first)`                    | 計算前綴和 (Prefix Sum)          |
| `std::adjacent_difference` | `adjacent_difference(first, last, d_first)`            | 計算相鄰元素的差值               |
| `std::fill_n`              | `fill_n(d_first, count, value)`                        | 將值填入從起點開始的 n 個位置    |
| `std::generate_n`          | `generate_n(d_first, count, gen)`                      | 使用生成器填入 n 個值            |
| `std::unique_copy`         | `unique_copy(first, last, d_first)`                    | 複製並移除連續重複元素           |
| `std::replace_copy`        | `replace_copy(first, last, d_first, old_val, new_val)` | 複製並替換特定值                 |

---

## 2. 函數簽名分析

我們以 `std::partial_sum` 和 `std::transform` 為例，分析其參數設計如何體現此模式。

### 案例 A: `std::partial_sum`

```cpp
template< class InputIt, class OutputIt >
OutputIt partial_sum( InputIt first, InputIt last, OutputIt d_first );
//   ^^^                                     ^^^^
//  返回類型                              第三參數：輸出起點
```

- **`first`, `last` (InputIt)**: 定義輸入範圍。只需具備讀取 (`*it`) 和遞增 (`++it`) 能力。
- **`d_first` (OutputIt)**: 定義輸出起點。只需具備寫入 (`*it = val`) 和遞增 (`++it`) 能力。
- **返回值**: 指向最後被寫入元素的下一個位置的 Iterator，方便後續操作。

### 案例 B: `std::transform`

```cpp
template< class InputIt, class OutputIt, class UnaryOperation >
OutputIt transform( InputIt first1, InputIt last1, OutputIt d_first, UnaryOperation op );
```

- 同樣將輸入 (`first1`, `last1`) 與輸出 (`d_first`) 分離。
- 允許輸入和輸出是完全不同的容器，甚至是不同的數據類型（只要 `op` 能處理）。

---

## 3. 設計概念

### (1) 最小權限原則 (Principle of Least Privilege)

Output Iterator 的設計核心在於**只要求完成任務所需的最小能力**。

- **需要的操作**：
  - `*it = value` (寫入)：將計算結果存入。
  - `++it` (遞增)：移動到下一個寫入位置。
- **不需要的操作**：
  - `value = *it` (讀取)：算法不關心輸出位置原本的值。
  - `it == end` (比較)：算法通常由輸入範圍決定何時停止，或者由計數決定，不依賴輸出範圍的邊界檢查（這也是常見陷阱的來源）。

這種設計使得 Output Iterator 可以是：

- 原生指標 (`int*`)
- 容器的 iterator (`vector<int>::iterator`)
- 插入迭代器 (`back_insert_iterator`)
- 串流迭代器 (`ostream_iterator`)

### (2) 類型解耦 (Type Decoupling)

輸入 (`InputIt`) 和輸出 (`OutputIt`) 是獨立的模板參數，這意味著它們可以是完全不同的類型。

**優勢：**

- **跨容器操作**：從 `std::list` 讀取，寫入 `std::vector`。
- **類型轉換**：輸入 `int`，輸出 `long long` (防止溢位) 或 `string` (格式化輸出)。

```cpp
std::vector<int> input = {1, 2, 3};
std::vector<long long> output(3); // 不同類型

// InputIt 是 vector<int>::iterator
// OutputIt 是 vector<long long>::iterator
std::partial_sum(input.begin(), input.end(), output.begin());
```

### (3) 設計模式總結

- **單一職責原則 (Single Responsibility)**：
  - 輸入 Iterator 負責提供數據。
  - 算法負責計算邏輯。
  - 輸出 Iterator 負責處置結果（儲存、輸出、傳輸）。
- **開放封閉原則 (Open-Closed)**：
  - 算法對擴展開放：你可以撰寫新的 Output Iterator (例如寫入網路 socket)，而無需修改算法本身。
  - 算法對修改封閉：標準庫算法的實作不需要變動。
- **依賴倒置原則 (Dependency Inversion)**：
  - 算法依賴於抽象的 Iterator 概念 (Concept)，而非具體的容器實作。

### (4) 性能考量

- **零開銷抽象 (Zero-Overhead Abstraction)**：
  - Output Iterator 的操作 (`*it = val`, `++it`) 在編譯後通常會優化為直接的記憶體寫入和指標移動。
  - 使用 STL 算法的性能通常等同於手寫的優化迴圈。
- **O(1) 操作**：
  - 無論底層容器為何（Array, List, Vector），Output Iterator 的寫入和遞增操作保證是均攤 O(1) 的。算法本身不需要使用 `std::advance` 等可能為 O(N) 的操作來移動輸出位置。

---

## 4. 三種使用模式

Output Iterator Pattern 的靈活性體現在它支持多種使用場景：

### 模式 A: Out-of-Place (寫入新容器)

保留原始數據，將結果存入新的容器。

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};
std::vector<int> result(nums.size()); // ⚠️ 必須預先分配空間

std::partial_sum(nums.begin(), nums.end(), result.begin());
```

### 模式 B: In-Place (原地修改)

直接修改輸入容器，節省記憶體。前提是算法支持輸入輸出重疊（大多數 STL 算法如 `transform`, `partial_sum` 都支持，但需查閱文檔確認）。

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};

// 輸出起點 = 輸入起點
std::partial_sum(nums.begin(), nums.end(), nums.begin());
// nums 變為 {1, 3, 6, 10, 15}
```

### 模式 C: Stream 輸出 / 動態插入

不預先分配空間，而是直接輸出到串流或動態插入容器。

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};

// 1. 輸出到 Console
std::partial_sum(nums.begin(), nums.end(),
                 std::ostream_iterator<int>(std::cout, " "));

// 2. 動態插入到空容器 (使用 back_inserter)
std::vector<int> result; // 空容器
std::partial_sum(nums.begin(), nums.end(),
                 std::back_inserter(result)); // 自動呼叫 push_back
```

---

## 5. 常見錯誤與陷阱

### ❌ 錯誤 1：目標容器空間不足

這是最常見的錯誤。Output Iterator 通常不檢查邊界（除了 Insert Iterator）。

```cpp
std::vector<int> src = {1, 2, 3};
std::vector<int> dst; // 空容器

// 💥 未定義行為！dst.begin() 無法寫入，且會越界
std::copy(src.begin(), src.end(), dst.begin());
```

**修正：**

1. 使用 `dst.resize(src.size())` 預先分配空間。
2. 使用 `std::back_inserter(dst)`。

### ❌ 錯誤 2：In-place 操作導致 Iterator 失效

如果在 In-place 操作過程中改變了容器大小（例如使用 `back_inserter` 插入同一個 vector），可能導致輸入 Iterator 失效。

```cpp
std::vector<int> v = {1, 2, 3};
// 💥 危險：push_back 可能導致重新分配，使 v.begin() 失效
std::copy(v.begin(), v.end(), std::back_inserter(v));
```

**修正：**
使用 `reserve` 預留空間，或避免對同一個容器同時進行讀取和插入操作。

### ❌ 錯誤 3：忽略溢位風險

當輸出類型與輸入類型相同，但計算結果可能變大時（如 `partial_sum`）。

```cpp
std::vector<int> nums = {INT_MAX, 1};
std::vector<int> res(2);
std::partial_sum(nums.begin(), nums.end(), res.begin()); // 💥 溢位
```

**修正：**
使用更大的輸出類型：`std::vector<long long> res(2);`

---

## 6. 進階技巧

### 自定義 Output Iterator

你可以建立一個 class 滿足 Output Iterator 的需求，實現特殊的輸出邏輯（例如丟棄數據、寫入資料庫、發送網路封包）。

```cpp
struct DiscardIterator {
    // 滿足 Output Iterator 需求
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    DiscardIterator& operator*() { return *this; }
    DiscardIterator& operator++() { return *this; }
    DiscardIterator& operator++(int) { return *this; }

    // 賦值運算符負責 "處理" 數據 (這裡是丟棄)
    template<typename T>
    DiscardIterator& operator=(const T&) { return *this; }
};

// 使用：執行算法但忽略結果 (例如只為了副作用或測試性能)
std::transform(v.begin(), v.end(), DiscardIterator(), op);
```

### 組合算法

利用 Output Iterator 的特性，將多個算法串接。以下展示如何計算 **移動平均 (Moving Average)**：

概念：`移動平均[i] = (前綴和[i] - 前綴和[i-k]) / k`

```cpp
// 原始數據
std::vector<double> data = {1, 2, 3, 4, 5, 6};
int k = 3; // 窗口大小

// 1. 計算前綴和
std::vector<double> sums;
sums.reserve(data.size());
std::partial_sum(data.begin(), data.end(), std::back_inserter(sums));
// sums: {1, 3, 6, 10, 15, 21}

// 2. 計算區間和 (利用 adjacent_difference 的變體)
// 我們需要 sums[i] - sums[i-k]，這可以視為一種廣義的差分
std::vector<double> window_sums;
// 前 k-1 個元素無法構成完整窗口，這裡簡化處理，只計算完整的
for (size_t i = k - 1; i < sums.size(); ++i) {
    double prev_sum = (i == k - 1) ? 0.0 : sums[i - k];
    window_sums.push_back(sums[i] - prev_sum);
}
// window_sums: {6, 9, 12, 15} (即 1+2+3, 2+3+4, ...)

// 3. 計算平均值 (transform)
std::vector<double> avgs;
std::transform(window_sums.begin(), window_sums.end(),
                std::back_inserter(avgs),
                [k](double sum) { return sum / k; });

// avgs: {2, 3, 4, 5}
```

### 反向操作

結合 `reverse_iterator` 可以從後往前處理。

```cpp
// 計算後綴和 (Suffix Sum)
std::vector<int> nums = {1, 2, 3};
std::vector<int> suffix(3);

// 使用 rbegin(), rend()
std::partial_sum(nums.rbegin(), nums.rend(), suffix.rbegin());
// suffix: {6, 5, 3}
```
