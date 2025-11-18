# Container Member Function vs `std::lower_bound`

## TL;DR

| 容器類型                   | 推薦做法                     | 複雜度   | 原因                           |
| -------------------------- | ---------------------------- | -------- | ------------------------------ |
| `set`/`map`                | ⭐ `container.lower_bound()` | O(log n) | 直接訪問紅黑樹                 |
| `vector`/`deque`           | ⭐ `std::lower_bound()`      | O(log n) | RandomAccessIterator           |
| `set` + `std::lower_bound` | ❌ 錯誤                      | **O(n)** | BidirectionalIterator 線性移動 |

---

## 1. 核心問題：為何需要兩個版本？

C++ STL 在關聯容器（`set`, `map`, `multiset`, `multimap`）中提供了 **成員函數版本** 的 `lower_bound`/`upper_bound`，而不僅僅依賴泛型算法 `std::lower_bound`。

**關鍵差異在於：**

- Iterator Category 限制
- 時間複雜度保證

---

## 2. Iterator Category 限制

### 2.1 API 簽章對比

```cpp
// std::lower_bound - 泛型算法
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value);

// set::lower_bound - 成員函數
class set<Key> {
    iterator lower_bound(const Key& key);
    const_iterator lower_bound(const Key& key) const;
};
```

### 2.2 Iterator 類別與複雜度

| Container                         | Iterator Category     | `std::lower_bound` 複雜度 |
| --------------------------------- | --------------------- | ------------------------- |
| `vector`/`deque`/`array`          | RandomAccessIterator  | ✅ O(log n)               |
| `set`/`map`/`multiset`/`multimap` | BidirectionalIterator | ❌ **O(n)**               |
| `list`                            | BidirectionalIterator | O(n)                      |
| `forward_list`                    | ForwardIterator       | O(n)                      |

---

## 3. 時間複雜度陷阱 ⚠️

### 3.1 錯誤示例

```cpp
#include <set>
#include <algorithm>
#include <iostream>

int main() {
    std::set<int> s = {1, 3, 5, 7, 9, 11, 13, 15};

    // ❌ 錯誤做法：O(n) - 因為 set iterator 不支援隨機存取
    auto it1 = std::lower_bound(s.begin(), s.end(), 7);

    // ⭐ 正確做法：O(log n) - 利用紅黑樹結構
    auto it2 = s.lower_bound(7);

    std::cout << "Both find: " << *it1 << " and " << *it2 << "\n";
    // 輸出相同，但效能差異巨大！
}
```

### 3.2 為何 `std::lower_bound` 在 `set` 上是 O(n)？

**泛型算法的實作限制：**

```cpp
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value) {
    auto count = std::distance(first, last);  // O(n) for BidirectionalIterator

    while (count > 0) {
        auto it = first;
        auto step = count / 2;
        std::advance(it, step);  // ⚠️ O(step) for BidirectionalIterator

        if (*it < value) {
            first = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}
```

**問題分析：**

1. `std::advance(it, step)` 在 BidirectionalIterator 上是 **O(step)** 而非 O(1)
2. 每次二分都需要線性移動 iterator
3. 即使邏輯上是二分搜尋，iterator 移動總成本為 O(n)

**複雜度計算：**

```
第 1 次：移動 n/2 步
第 2 次：移動 n/4 步
第 3 次：移動 n/8 步
...
總計：n/2 + n/4 + n/8 + ... ≈ n
```

---

## 4. `set::lower_bound` 的優化實作

### 4.1 成員函數利用紅黑樹結構

```cpp
// set/map 內部實作（概念性）
template<class Key>
class set {
    struct Node {
        Key key;
        Node* parent;
        Node* left;
        Node* right;
    };
    Node* root;

public:
    iterator lower_bound(const Key& key) {
        Node* result = nullptr;
        Node* current = root;

        // 直接在樹上二分，O(log n)
        while (current) {
            if (!(current->key < key)) {  // current->key >= key
                result = current;
                current = current->left;  // ⭐ O(1) 指標操作
            } else {
                current = current->right; // ⭐ O(1) 指標操作
            }
        }

        return iterator(result);
    }
};
```

**關鍵優勢：**

- ✅ 直接訪問樹節點，無需 iterator 移動
- ✅ 每次比較都是 O(1) 指標操作
- ✅ 總複雜度嚴格保證 O(log n)

### 4.2 與泛型算法的對比

| 實作方式           | Iterator 移動         | 每步成本 | 總複雜度     |
| ------------------ | --------------------- | -------- | ------------ |
| `std::lower_bound` | `std::advance(it, n)` | O(n)     | O(n)         |
| `set::lower_bound` | 直接指標跳轉          | **O(1)** | **O(log n)** |

---

## 5. API 設計決策總結

| 面向           | `std::lower_bound`                          | `container.lower_bound()`  |
| -------------- | ------------------------------------------- | -------------------------- |
| **適用容器**   | 任何 ForwardIterator                        | set/map/multiset/multimap  |
| **複雜度保證** | O(log n) for RandomAccess<br>O(n) otherwise | **O(log n)** 嚴格保證      |
| **實作方式**   | Iterator 線性移動                           | 直接訪問內部樹結構         |
| **使用建議**   | 用於 vector/deque/array                     | ⭐ **優先使用** 於關聯容器 |

---

## 6. LeetCode 實戰建議

### 6.1 使用規則

```cpp
// ⭐ 關聯容器：使用成員函數
std::set<int> s;
auto it = s.lower_bound(target);  // O(log n)

// ⭐ 順序容器：使用泛型算法
std::vector<int> v;
auto it = std::lower_bound(v.begin(), v.end(), target);  // O(log n)

// ❌ 反例：效能陷阱
std::set<int> s;
auto it = std::lower_bound(s.begin(), s.end(), target);  // O(n) - 錯誤！
```

### 6.2 實戰案例：有序集合範圍查詢

```cpp
// LeetCode 常見場景：在有序集合中查詢範圍
std::set<int> nums = {1, 3, 5, 7, 9, 11, 13, 15};

// 查詢 [5, 10) 範圍內的元素
auto lower = nums.lower_bound(5);   // >= 5，O(log n)
auto upper = nums.lower_bound(10);  // >= 10（即 > 9），O(log n)

for (auto it = lower; it != upper; ++it) {
    std::cout << *it << " ";  // 輸出：5 7 9
}
```

### 6.3 常見陷阱

**❌ 錯誤模式：**

```cpp
std::set<int> s = {1, 2, 3, 4, 5};

// 陷阱 1：使用泛型算法
if (std::binary_search(s.begin(), s.end(), 3)) {  // O(n)
    // ...
}

// 陷阱 2：手動實作二分
// 因為 set iterator 不支援 +-，無法實作
```

**⭐ 正確模式：**

```cpp
std::set<int> s = {1, 2, 3, 4, 5};

// 正確 1：使用成員函數
if (s.find(3) != s.end()) {  // O(log n)
    // ...
}

// 正確 2：使用 count
if (s.count(3)) {  // O(log n)
    // ...
}

// 正確 3：lower_bound
auto it = s.lower_bound(3);  // O(log n)
if (it != s.end() && *it == 3) {
    // ...
}
```

---

## 7. 設計哲學：Why Member Functions Matter

### 7.1 C++ STL 核心設計原則

> **"When a container can provide a more efficient implementation than the generic algorithm, it should offer that operation as a member function."**

**三大原則：**

1. **Zero-overhead principle** - 不為未使用的功能付出代價
2. **最佳化機會** - 容器內部實作可利用資料結構特性
3. **API 一致性** - member function 優先於 algorithm（當效能有差異時）

### 7.2 其他類似案例

| 功能         | 容器成員函數         | 泛型算法             | 差異原因                 |
| ------------ | -------------------- | -------------------- | ------------------------ |
| **排序**     | `list::sort()`       | `std::sort()`        | list 不支援 RandomAccess |
| **移除**     | `list::remove()`     | `std::remove()`      | in-place vs erase-remove |
| **計數**     | `map::count()`       | `std::count()`       | O(log n) vs O(n)         |
| **查找**     | `set::find()`        | `std::find()`        | O(log n) vs O(n)         |
| **二分查找** | `set::lower_bound()` | `std::lower_bound()` | O(log n) vs O(n)         |

**共同模式：**

- 成員函數利用內部資料結構
- 提供比泛型算法更好的複雜度保證
- API 命名保持一致性

---

## 8. 效能基準測試

### 8.1 實驗設定

```cpp
#include <set>
#include <algorithm>
#include <chrono>
#include <iostream>

void benchmark() {
    std::set<int> s;
    for (int i = 0; i < 1000000; ++i) {
        s.insert(i);
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 測試 1：std::lower_bound
    for (int i = 0; i < 10000; ++i) {
        auto it = std::lower_bound(s.begin(), s.end(), i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();

    // 測試 2：set::lower_bound
    for (int i = 0; i < 10000; ++i) {
        auto it = s.lower_bound(i);
    }

    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "std::lower_bound: " << duration1.count() << " ms\n";
    std::cout << "set::lower_bound: " << duration2.count() << " ms\n";
    std::cout << "Speedup: " << (double)duration1.count() / duration2.count() << "x\n";
}
```

**預期結果（1M 元素，10K 查詢）：**

```
std::lower_bound: ~5000 ms  (O(n) × 10K queries)
set::lower_bound: ~1 ms     (O(log n) × 10K queries)
Speedup: ~5000x
```

---

## 9. 總結

### 9.1 決策樹

```
需要在容器中進行二分查找？
    │
    ├─ 容器類型是 set/map？
    │   └─ ⭐ 使用 container.lower_bound()  [O(log n)]
    │
    └─ 容器類型是 vector/deque/array？
        └─ ⭐ 使用 std::lower_bound()  [O(log n)]
```

### 9.2 核心要點

1. **Iterator Category 決定複雜度**

   - RandomAccessIterator：泛型算法 O(log n) ✅
   - BidirectionalIterator：泛型算法 O(n) ❌

2. **關聯容器優先成員函數**

   - 直接訪問內部樹結構
   - 嚴格 O(log n) 保證

3. **設計哲學：效能優先**
   - 當成員函數更高效時，STL 提供兩個版本
   - 保持 API 一致性，但允許最佳化實作

---

## 參考資料

- [cppreference: std::lower_bound](https://en.cppreference.com/w/cpp/algorithm/lower_bound)
- [StackOverflow: Prefer container.lower_bound to std::lower_bound](https://stackoverflow.com/questions/79734459/prefer-containerlower-bound-to-stdlower-bound)
- [Effective STL Item 44: Prefer member functions to algorithms with the same names](https://www.aristeia.com/effective-stl.html)
