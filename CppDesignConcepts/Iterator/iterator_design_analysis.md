# C++ Iterator 設計分析：核心概念與實戰

## TL;DR 速查表

| Iterator Category     | 支援操作            | 典型容器                      | Range-for | 複雜度    |
| --------------------- | ------------------- | ----------------------------- | --------- | --------- |
| InputIterator         | `++`, `*`, `==`     | `istream_iterator`            | ✅        | 單次掃描  |
| ForwardIterator       | `++`, `*`, `==`     | `forward_list`, `unordered_*` | ✅        | 多次掃描  |
| BidirectionalIterator | `++`, `--`, `*`     | `list`, `set`, `map`          | ✅        | 雙向      |
| RandomAccessIterator  | `+`, `-`, `[]`, `<` | `vector`, `deque`, `array`    | ✅        | O(1) 隨機 |

**Container Adapter 無 Iterator：** `stack` ❌, `queue` ❌, `priority_queue` ❌

---

## 1. Iterator Category 核心差異

### 能力遞增層級

```
InputIterator/OutputIterator
        ↓
  ForwardIterator
        ↓
BidirectionalIterator
        ↓
RandomAccessIterator
        ↓
ContiguousIterator (C++20)
```

### 關鍵操作對比

```cpp
// ForwardIterator - 只能向前
std::forward_list<int> flist = {1, 2, 3};
auto it = flist.begin();
++it;     // ✅
// --it;  // ❌ 編譯錯誤

// BidirectionalIterator - 可雙向
std::list<int> lst = {1, 2, 3};
auto it2 = lst.begin();
++it2;  // ✅
--it2;  // ✅

// RandomAccessIterator - 隨機存取
std::vector<int> v = {1, 2, 3, 4, 5};
auto it3 = v.begin();
it3 += 3;     // ✅ O(1)
auto x = it3[2];  // ✅ O(1)
```

---

## 2. Container 支援情況

### Sequential Containers

| Container        | Iterator Category | `begin()`/`end()` | Random `[]` |
| ---------------- | ----------------- | ----------------- | ----------- |
| `array`/`vector` | RandomAccess      | ✅                | ✅          |
| `deque`          | RandomAccess      | ✅                | ✅          |
| `list`           | Bidirectional     | ✅                | ❌          |
| `forward_list`   | Forward           | ✅                | ❌          |

### Associative Containers

| Container                            | Iterator Category | 內部結構                |
| :----------------------------------- | :---------------- | :---------------------- |
| `set`, `map`, `multiset`, `multimap` | Bidirectional     | 紅黑樹 (Red-Black Tree) |
| `unordered_set`, `unordered_map`     | Forward           | 雜湊表 (Hash Table)     |

### Container Adapters ❌

```cpp
std::stack<int> s;     // ❌ 無 begin()/end()
std::queue<int> q;     // ❌ 無 begin()/end()
std::priority_queue<int> pq; // ❌ 無 begin()/end()
```

**設計原因：強制語義完整性 (Enforcing Semantic Integrity)**

1. **封裝性 (Encapsulation)**：

   - `stack` (LIFO) 只能存取頂端 (`top`)。
   - `queue` (FIFO) 只能存取頭尾 (`front`/`back`)。
   - `priority_queue` 只能存取最大/最小值 (`top`)。
   - 如果提供 Iterator，使用者就能遍歷甚至修改中間的元素，這會**破壞這些資料結構的定義與限制**。

2. **防止誤用 (Prevention of Misuse)**：
   - 不提供 Iterator 可以防止開發者寫出低效或錯誤的代碼（例如在 stack 中間搜尋元素）。
   - 這符合 C++ 的 **"Make interfaces easy to use correctly and hard to use incorrectly"** 原則。

---

## 3. Range-Based For 支援

### 展開機制

```cpp
// 你寫的
for (auto x : container) { }

// 編譯器轉換
auto __begin = container.begin();  // 需要 begin()
auto __end = container.end();      // 需要 end()
for (; __begin != __end; ++__begin) { auto x = *__begin; }
```

### 支援清單

| 容器                | Range-for | 原因                     |
| ------------------- | --------- | ------------------------ |
| 所有 STL Containers | ✅        | 有 `begin()`/`end()`     |
| 原生陣列            | ✅        | 編譯器特殊處理           |
| `stack`/`queue`     | ❌        | **無** `begin()`/`end()` |

```cpp
// ✅ 可用
std::vector<int> v = {1, 2, 3};
for (auto x : v) { }

std::set<int> s = {1, 2, 3};
for (auto x : s) { }  // 按順序遍歷

// ❌ 不可用
std::stack<int> st;
for (auto x : st) { }  // ❌❌❌ 編譯錯誤
```

---

## 4. Iterator 輔助函數

### `std::next()` / `std::prev()`

**關鍵特性：不修改原 iterator**

```cpp
std::vector<int> v = {10, 20, 30, 40, 50};
auto it = v.begin() + 2;  // 指向 30

auto next_it = std::next(it, 2);  // 指向 50，it 不變
auto prev_it = std::prev(it);     // 指向 20，it 不變
std::cout << *it;  // 仍然是 30 ✅
```

**複雜度差異：**

| Iterator Category     | `next(it, n)` 複雜度 |
| --------------------- | -------------------- |
| BidirectionalIterator | O(n)                 |
| RandomAccessIterator  | **O(1)** ⭐          |

### `std::advance()` / `std::distance()`

```cpp
// advance - 原地修改
std::list<int> lst = {1, 2, 3, 4, 5};
auto it = lst.begin();
std::advance(it, 3);  // it 被修改為指向 4

// distance - 計算距離
auto dist = std::distance(lst.begin(), it);  // 3（O(n) for list）
```

---

## 5. Iterator 失效規則 ⚠️

### 各容器失效情況

**`vector` - 最嚴格**

| 操作          | Iterator 失效範圍        |
| ------------- | ------------------------ |
| `push_back()` | 如重新分配，**全部失效** |
| `insert()`    | 插入點及之後**全部失效** |
| `erase()`     | 刪除點及之後**全部失效** |

**`list`/`set`/`map` - 較寬鬆**

| 操作        | Iterator 失效範圍 |
| ----------- | ----------------- |
| `insert()`  | **無失效** ✅     |
| `erase(it)` | 只有 `it` 失效    |

### 安全的刪除模式

```cpp
// ❌ 錯誤
for (auto it = v.begin(); it != v.end(); ++it) {
    if (*it % 2 == 0) v.erase(it);  // it 失效
}

// ✅ 正確：erase 返回值
for (auto it = v.begin(); it != v.end(); ) {
    if (*it % 2 == 0) {
        it = v.erase(it);  // 返回下一個有效 iterator
    } else {
        ++it;
    }
}

// ✅ 正確：erase-remove idiom
v.erase(std::remove_if(v.begin(), v.end(),
                       [](int x) { return x % 2 == 0; }),
        v.end());
```

---

## 6. LeetCode 實戰技巧

### 容器選擇決策

```
需要隨機存取？
    ✅ → vector (RandomAccess)

需要雙向遍歷？
    ✅ → list/set/map (Bidirectional)

需要有序 + 快速查找？
    ✅ → set/map (O(log n))

需要快速查找（無序可）？
    ✅ → unordered_set/map (O(1))
```

### 性能陷阱 ⚠️

**在 `set`/`map` 上用 `std::lower_bound`**

```cpp
std::set<int> s = {1, 2, 3, 4, 5};

// ❌ O(n) - BidirectionalIterator 限制
auto it1 = std::lower_bound(s.begin(), s.end(), 3);

// ⭐ O(log n) - 成員函數利用紅黑樹
auto it2 = s.lower_bound(3);
```

**詳細原因分析：**

1. **`std::lower_bound` (通用算法)**：

   - 它依賴 `std::advance` 來移動 Iterator 進行二分搜尋。
   - 對於 `RandomAccessIterator` (如 `vector`)，`std::advance` 是 O(1)，所以總複雜度是 O(log n)。
   - 對於 `BidirectionalIterator` (如 `set`/`list`)，`std::advance` 是 **O(k)** (線性移動)。
   - 雖然比較次數仍是 O(log n)，但**移動 Iterator 的總步數**會退化成 O(n)。

2. **`s.lower_bound` (成員函數)**：
   - 它直接利用 `set` 內部的**紅黑樹結構** (Tree Traversal)。
   - 從根節點開始往下搜尋，不需要線性移動 Iterator。
   - 因此保證 O(log n) 複雜度。

---

## 7. 設計原則總結

### C++ 設計哲學

1. **Zero-overhead abstraction** - Iterator 與原生指標效能相同
2. **Principle of Least Privilege** - `stack` 不提供 iterator 防止誤用
3. **統一介面** - 所有容器使用相同的遍歷方式

### Iterator 使用規則

**✅ 推薦：**

- 關聯容器用成員函數：`set.lower_bound()` 而非 `std::lower_bound()`
- 使用 `std::next()`/`prev()` 避免修改原 iterator
- 優先 erase-remove idiom 避免 iterator 失效

**❌ 避免：**

- 修改容器後繼續使用舊 iterator
- 在 `set`/`map` 上用 `std::lower_bound`（效能退化）
- 嘗試對 `stack`/`queue` 使用 range-for

---

## 8. OutputIterator 與算法設計模式

### 概念與職責

**OutputIterator** 是 C++ STL 中最基礎的寫入迭代器概念：

| 能力   | 說明             | 語法          |
| ------ | ---------------- | ------------- |
| 可寫入 | 將值寫入當前位置 | `*it = value` |
| 可遞增 | 移動到下一位置   | `++it`        |

**不需要的能力：**

- ❌ 讀取 (`value = *it`)
- ❌ 比較 (`it == end`)
- ❌ 隨機存取 (`it[n]`)

### 典型應用：`std::partial_sum`

```cpp
template< class InputIt, class OutputIt >
OutputIt partial_sum(InputIt first, InputIt last, OutputIt d_first);
//                                                ^^^^^^^^^
//                                              OutputIterator
```

**設計優勢：**

1. **輸入輸出分離** - 可選擇 in-place 或 out-of-place 操作
2. **類型解耦** - 輸入輸出可為不同容器類型
3. **Stream 支持** - 可直接輸出至 `ostream_iterator`

```cpp
std::vector<int> nums = {1, 2, 3};

// 模式 1: 寫入新容器
std::vector<long long> prefix(3);
std::partial_sum(nums.begin(), nums.end(), prefix.begin());

// 模式 2: In-place 修改
std::partial_sum(nums.begin(), nums.end(), nums.begin());

// 模式 3: Stream 輸出
std::partial_sum(nums.begin(), nums.end(),
                 std::ostream_iterator<int>(std::cout, " "));
```

### 其他使用 OutputIterator 的算法

| 算法                                                                                          | 用途     | 典型場景           |
| --------------------------------------------------------------------------------------------- | -------- | ------------------ |
| [`std::transform`](https://en.cppreference.com/w/cpp/algorithm/transform)                     | 映射轉換 | 元素逐一處理       |
| [`std::copy`](https://en.cppreference.com/w/cpp/algorithm/copy)                               | 複製元素 | 跨容器複製         |
| [`std::partial_sum`](https://en.cppreference.com/w/cpp/algorithm/partial_sum)                 | 前綴和   | LeetCode 常用      |
| [`std::adjacent_difference`](https://en.cppreference.com/w/cpp/algorithm/adjacent_difference) | 差分計算 | 對稱於 partial_sum |

**詳細分析：** 參見 [`output_iterator_pattern.md`](output_iterator_pattern.md)

---

## 9. 參考資料

- [cppreference: Iterator library](https://en.cppreference.com/w/cpp/iterator)
- [cppreference: OutputIterator](https://en.cppreference.com/w/cpp/named_req/OutputIterator)
- [Effective STL: Item 26](https://www.aristeia.com/effective-stl.html)
- [Output Iterator Pattern 深度分析](output_iterator_pattern.md)
