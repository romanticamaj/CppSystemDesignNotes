# Algorithm: `std::binary_search` / Binary Search

## 1. Core Concept

Binary search 在已排序的容器中尋找目標值，透過不斷將搜尋區間對半分割來達成 O(log n) 的查找效率。

C++ STL 提供三個核心 API：

- `std::binary_search`: 回傳 `bool`，判斷元素是否存在
- `std::lower_bound`: 回傳 iterator，指向第一個 >= target 的位置
- `std::upper_bound`: 回傳 iterator，指向第一個 > target 的位置

## 2. C++ STL API 設計

### 介面簽章

```cpp
// 檢查元素是否存在
template<class ForwardIt, class T>
bool binary_search(ForwardIt first, ForwardIt last, const T& value);

// 找到第一個 >= value 的位置
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value);

// 找到第一個 > value 的位置
template<class ForwardIt, class T>
ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T& value);
```

### 關鍵設計：半開區間 `[first, last)`

C++ STL 採用 **半開區間** 設計：

- `first`: 包含在搜尋範圍內
- `last`: **不包含**在搜尋範圍內（one-past-the-end）

這與 C++ iterator 慣例一致（如 `vec.begin()` 到 `vec.end()`）。

## 3. C++ Example

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> nums = {1, 3, 5, 7, 9, 11, 13, 15};

    // std::binary_search - 回傳 bool
    bool found = std::binary_search(nums.begin(), nums.end(), 7);
    std::cout << "7 exists: " << (found ? "Yes" : "No") << "\n";

    // std::lower_bound - 第一個 >= target
    auto lower = std::lower_bound(nums.begin(), nums.end(), 7);
    if (lower != nums.end()) {
        std::cout << "lower_bound(7): " << *lower
                  << " at index " << (lower - nums.begin()) << "\n";
    }

    // std::upper_bound - 第一個 > target
    auto upper = std::upper_bound(nums.begin(), nums.end(), 7);
    if (upper != nums.end()) {
        std::cout << "upper_bound(7): " << *upper << "\n";
    }

    // 元素不存在時的行為
    auto lower_missing = std::lower_bound(nums.begin(), nums.end(), 8);
    std::cout << "lower_bound(8): " << *lower_missing << "\n"; // 9

    return 0;
}
```

## 4. 實作解析

### `std::lower_bound` 實作（半開區間風格）

```cpp
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value) {
    auto left = first;
    auto right = last;  // 注意：right 是 one-past-the-end

    while (left < right) {
        auto mid = left + (right - left) / 2;

        if (*mid >= value) {
            right = mid;     // 保留 mid（可能是答案）
        } else {
            left = mid + 1;  // 排除左半部
        }
    }

    return left;  // left == right 時為答案
}
```

**關鍵點：**

- `right = mid` 而非 `right = mid - 1`，因為 `[left, right)` 本身就不包含 `right`
- 循環條件是 `left < right`，而非 `left <= right`
- 結束時 `left == right`，指向答案位置

### `std::binary_search` 基於 `lower_bound` 實作

```cpp
template<class ForwardIt, class T>
bool binary_search(ForwardIt first, ForwardIt last, const T& value) {
    auto it = std::lower_bound(first, last, value);
    return (it != last) && !(*it < value) && !(value < *it);
    // 等價於：return (it != last) && (*it == value);
}
```

**為什麼基於 `lower_bound`？**

- `lower_bound` 找到第一個 >= value 的位置
- 如果該位置的值等於 value，表示找到
- 否則表示不存在

### 比較函數設計：為何用 `<` 而非 `==`

C++ STL 的比較函數採用 **strict weak ordering**（嚴格弱序），表達的是 **"小於"關係**，而非 "等於"。

#### 核心概念

**預設比較函數：**

```cpp
std::less<T>  // 等價於 operator<
```

**如何用 `<` 判斷相等？**

在 strict weak ordering 中，相等的定義是：

```cpp
// a == b 的定義
!(a < b) && !(b < a)

// 用 comp 表達
!comp(a, b) && !comp(b, a)
```

#### `binary_search` 實作細節

參考 cppreference 的標準實作：

```cpp
template<class ForwardIt, class T, class Compare>
bool binary_search(ForwardIt first, ForwardIt last, const T& value, Compare comp)
{
    first = std::lower_bound(first, last, value, comp);
    return (!(first == last) && !(comp(value, *first)));
}
```

**邏輯分析：**

1. `lower_bound` 回傳第一個 `>= value` 的位置

   - 保證：`!comp(*first, value)` 為真（即 `*first >= value`）

2. 檢查 `*first == value` 需同時滿足：

   - `*first >= value` ✓（已由 lower_bound 保證）
   - `*first <= value` ← 需檢查：`!comp(value, *first)`

3. 兩條件同時成立即為相等

**實例說明：**

```cpp
std::vector<int> v = {1, 3, 5, 7, 9};

// 情況 1: 找到
auto it = lower_bound(v.begin(), v.end(), 5);  // it 指向 5
// comp(5, 5) = (5 < 5) = false
// !comp(5, 5) = true  ✓ 相等

// 情況 2: 未找到
it = lower_bound(v.begin(), v.end(), 6);  // it 指向 7
// comp(6, 7) = (6 < 7) = true
// !comp(6, 7) = false  ✗ 不相等
```

#### 設計優勢

1. **最小化要求**：只需實作 `operator<`，無需 `operator==`
2. **靈活性**：支援自訂排序規則（如降序、多鍵比較）
3. **一致性**：所有 STL 算法使用相同的比較語義

### 對比：Closed Interval `[left, right]` 風格

傳統的閉區間實作（常見於 LeetCode）：

```cpp
int binary_search_closed(const std::vector<int>& arr, int target) {
    int left = 0, right = arr.size() - 1;  // right 指向最後一個元素

    while (left <= right) {  // 注意：left <= right
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;  // 必須手動 -1
        }
    }

    return -1;  // 不存在
}
```

## 5. 設計對比總結

| 設計面向   | Closed `[left, right]` | Half-Open `[left, right)` (STL) |
| ---------- | ---------------------- | ------------------------------- |
| 初始化     | `right = size() - 1`   | `right = size()`                |
| 循環條件   | `left <= right`        | `left < right`                  |
| 更新 right | `right = mid - 1`      | `right = mid`                   |
| 結束狀態   | `left > right`         | `left == right`                 |
| 適用場景   | 精確查找、LeetCode     | 範圍查詢、STL 風格              |

### Half-Open 優勢

1. **一致性**：與 C++ iterator 設計一致
2. **簡潔性**：`right = mid` 比 `right = mid - 1` 更直觀
3. **正確性**：減少 off-by-one 錯誤
4. **擴展性**：更容易實作 `lower_bound`/`upper_bound` 等變體

### 關於 `set`/`map` 的 `lower_bound`

關聯容器（`set`, `map`）提供成員函數版本的 [`lower_bound()`](OrderedSet_Binary_Search.md)，與泛型算法 [`std::lower_bound()`](OrderedSet_Binary_Search.md) 有重要差異。

**核心差異：**

- `std::lower_bound(set.begin(), set.end(), x)` - ❌ O(n) 複雜度
- `set.lower_bound(x)` - ⭐ O(log n) 複雜度

詳細分析請參閱 [OrderedSet Binary Search](OrderedSet_Binary_Search.md)。

## 6. Complexity Analysis

- **Time:** O(log n)
- **Space:** O(1)

## 7. Common Use Cases

- 判斷元素是否存在於已排序容器
- 找到插入位置以維持排序
- 搜尋旋轉排序陣列（rotated sorted array）
- Binary search on answer（如求平方根）
- 範圍查詢（配合 `lower_bound`/`upper_bound`）

## Related Algorithms

- `std::equal_range`: 同時回傳 lower_bound 和 upper_bound
- Exponential search: 適用於無界或極大範圍的搜尋
