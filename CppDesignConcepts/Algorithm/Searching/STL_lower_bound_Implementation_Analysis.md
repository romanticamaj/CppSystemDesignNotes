# 深度解析：C++ STL `lower_bound` 實作

C++ 標準模板庫 (STL) 中的 `std::lower_bound` 是一個經過高度優化且泛化的二分搜尋實現。理解其源碼不僅能幫助我們寫出更穩健的程式碼，還能加深對二分搜尋本質的理解。

本文將深入分析一個典型的 `std::lower_bound` 實現，並將其與我們常用的基於索引的 `while (left < right)` 模板進行比較。

## `std::lower_bound` 典型實作

```cpp
template <class ForwardIterator, class T>
ForwardIterator lower_bound (ForwardIterator first, ForwardIterator last, const T& val)
{
  ForwardIterator it;
  // difference_type 用於表示兩個迭代器之間的距離
  typename iterator_traits<ForwardIterator>::difference_type count, step;

  // 1. 計算搜索區間內的元素數量
  count = distance(first, last);

  // 2. 只要區間內還有元素，就繼續搜索
  while (count > 0)
  {
    it = first;
    step = count / 2;

    // 3. 將迭代器 it 移動到區間的中點
    advance(it, step);

    // 4. 核心比較
    if (*it < val) {
      // 中點元素太小，答案肯定在右半邊
      // 將 first 移動到中點之後的位置，開始新的搜索
      first = ++it;
      // 更新剩餘的元素數量
      count -= step + 1;
    }
    else {
      // 中點元素 >= val，它可能是答案，或者答案在左半邊
      // 因此，我們將搜索範圍縮小到左半邊，但不排除中點
      // 新的元素數量就是左半邊的數量 step
      count = step;
    }
  }
  // 循環結束時，first 就是第一個不小於 val 的位置
  return first;
}
```

## 核心概念分析

這個實現雖然看起來複雜，但其核心邏輯與我們熟悉的索引版本是完全一致的。它只是將基於「索引」的操作，轉換為基於「迭代器」的泛型操作。

1. **泛型設計 (Iterators, not Indices)**

   - 它使用 `ForwardIterator` 而非 `int` 索引，這使得 `lower_bound` 可以應用於任何支援前向遍歷的容器（如 `std::vector`, `std::list`, `std::deque`）。

2. **區間表示 (`count = distance(first, last)`)**

   - 我們常用的 `left` 和 `right` 索引定義了一個閉區間 `[left, right]` 或半開區間 `[left, right)`。
   - STL 版本則使用 `first` 迭代器和 `count` 來定義搜索區間 `[first, first + count)`。`distance(first, last)` 的作用類似於 `right - left`。

3. **循環條件 (`while (count > 0)`)**

   - 這等價於 `while (left < right)`。只要搜索區間還未縮小到 0 個元素，循環就繼續。

4. **中點計算 (`advance(it, step)`)**

   - `step = count / 2;` 找到步長。
   - `it = first; advance(it, step);` 將迭代器 `it` 移動到區間的中點。這完全對應於 `mid = left + (right - left) / 2;`。

5. **區間收縮邏輯**
   - 這是最關鍵的部分，它完美地對應了我們在 `while (left < right)` 模板中的邏輯。
   - **`if (*it < val)`**:
     - **STL 版本**: `first = ++it; count -= step + 1;`
     - **索引版本**: `left = mid + 1;`
     - 兩者都表示：中點 `it` (或 `mid`) 的值太小，它和它左邊的所有元素都**不可能是答案**。因此，新的搜索起點是中點的**下一個**位置。
   - **`else (*it >= val)`**:
     - **STL 版本**: `count = step;`
     - **索引版本**: `right = mid;`
     - 兩者都表示：中點 `it` (或 `mid`) 的值**可能是答案**，或者答案在它左邊。因此，我們將搜索範圍的**結尾**縮小到中點 `it` (或 `mid`)，但**不能排除它**。新的搜索範圍大小就是 `step`。

### `upper_bound` 的關鍵差異

`upper_bound` 的實現與 `lower_bound` 幾乎完全相同，唯一的差別在於核心的比較邏輯。

```cpp
template <class ForwardIterator, class T>
ForwardIterator upper_bound (ForwardIterator first, ForwardIterator last, const T& val)
{
  // ... (與 lower_bound 相同的初始化)
  while (count > 0)
  {
    it = first; step = count / 2; std::advance(it, step);

    // 唯一的不同之處！
    if (!(val < *it)) { // 等價於 if (val >= *it) 或 if (*it <= val)
      // 中點元素 <= val，答案肯定在右半邊
      first = ++it;
      count -= step + 1;
    }
    else { // val < *it
      // 中點元素 > val，它可能是答案，或者答案在左半邊
      count = step;
    }
  }
  return first;
}
```

- **`if (!(val < *it))`**: 這個條件等同於 `if (*it <= val)`。
  - 如果中點元素**小於或等於** `val`，意味著我們要找的、第一個嚴格大於 `val` 的元素一定在右邊。因此，我們捨棄左半邊和中點 (`first = ++it`)。
  - 這對應到索引版本的 `if (nums[mid] <= target) { left = mid + 1; }`。

## 與索引版本的對應關係

| 概念           | `lower_bound` (STL)              | `upper_bound` (STL)              | `while (left < right)` (索引版本)                       |
| :------------- | :------------------------------- | :------------------------------- | :------------------------------------------------------ |
| **初始化**     | `count = distance(first, last)`  | `count = distance(first, last)`  | `right = nums.size()`                                   |
| **循環條件**   | `while (count > 0)`              | `while (count > 0)`              | `while (left < right)`                                  |
| **中點計算**   | `advance(it, step)`              | `advance(it, step)`              | `mid = left + (right-left)/2`                           |
| **核心比較**   | `if (*it < val)`                 | `if (!(val < *it))`              | `if (nums[mid] < target)` 或 `if (nums[mid] <= target)` |
| **捨棄左半邊** | `first = ++it; count -= step+1;` | `first = ++it; count -= step+1;` | `left = mid + 1;`                                       |
| **捨棄右半邊** | `count = step;`                  | `count = step;`                  | `right = mid;`                                          |
| **返回結果**   | `return first;`                  | `return first;`                  | `return left;`                                          |

## 結論

STL 的 `lower_bound` 實現是一個優雅的、泛型編程的典範。它透過迭代器和距離（`count`）來抽象化二分搜尋，使其能夠適用於多種容器。

儘管語法和操作方式不同，其底層的**算法思想**——如何根據比較結果來安全地、不遺漏地縮小搜索區間——與我們手寫的 `while (left < right)` 版本的 `lower_bound` 是完全一致的。理解這一點，有助於我們在需要時寫出同樣穩健的、針對特定問題的二分搜尋程式碼。
