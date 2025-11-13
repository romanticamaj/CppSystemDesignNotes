# C++ std::set 設計分析：Ordered Set 與自平衡 BST

從 API 設計與實現原理角度分析 C++ `std::set`，理解 Ordered Set 和 Self-Balancing BST 的設計決策。

## TL;DR - LeetCode 常用操作速查

### 核心操作（均為 O(log n)）

```cpp
set<int> s;
s.insert(x);              // 插入，返回 pair<iterator, bool>
s.erase(x);               // 刪除值，返回刪除數量
s.count(x);               // 存在性檢查，返回 0 或 1
s.find(x);                // 查找，返回 iterator 或 end()
```

### ⭐ 有序遍歷

```cpp
for (int x : s) { }       // 自動升序遍歷，O(n)
auto it = s.begin();      // 最小值
auto it = s.rbegin();     // 最大值（反向迭代器）
```

### ⭐ 範圍查詢（二分查找系列）

```cpp
auto it = s.lower_bound(x);  // >= x 的第一個，O(log n)
auto it = s.upper_bound(x);  // > x 的第一個，O(log n)
auto [l, r] = s.equal_range(x); // [lower_bound, upper_bound)
```

### 常見模式

```cpp
// ✅ 存在性檢查（推薦）
if (s.count(x)) { }       // 最直觀

// ✅ 需要迭代器時
auto it = s.find(x);
if (it != s.end()) { }

// ✅ 取最小/最大值並刪除
int min_val = *s.begin();
s.erase(s.begin());

// ✅ 範圍刪除
s.erase(s.lower_bound(lo), s.upper_bound(hi));
```

---

## Table of Contents

1. [什麼是 Ordered Set（有序集合）](#什麼是-ordered-set有序集合)
2. [Self-Balancing BST 原理](#self-balancing-bst-原理)
3. [C++ std::set 實現：紅黑樹](#c-stdset-實現紅黑樹)
4. [插入操作設計](#插入操作設計)
5. [查找操作設計](#查找操作設計)
6. [刪除操作設計](#刪除操作設計)
7. [範圍查詢設計](#範圍查詢設計)
8. [迭代器設計](#迭代器設計)
9. [API 設計原則總結](#api-設計原則總結)

---

## 什麼是 Ordered Set（有序集合）

### 核心特性

**Ordered Set** 是一種抽象資料結構，具備三大特性：

1. **唯一性（Uniqueness）**：每個元素只出現一次
2. **有序性（Ordering）**：元素按照比較函數排序
3. **對數時間（Logarithmic Time）**：插入、刪除、查找均為 O(log n)

### 為何需要「有序」？

| 場景            | 無序集合（unordered_set） | 有序集合（set）     |
| --------------- | ------------------------- | ------------------- |
| 查找第 k 小元素 | ❌ O(n)                   | ✅ O(log n) 遍歷    |
| 範圍查詢        | ❌ 不支持                 | ✅ O(log n + k)     |
| 中位數維護      | ❌ 每次 O(n)              | ✅ 迭代器定位       |
| 滑動窗口最值    | ❌ 需手動維護             | ✅ begin()/rbegin() |

**LeetCode 經典題：**

- 滑動窗口最大值（需維護有序）
- 數據流中的中位數（需快速定位中間值）
- 區間合併（需按起點排序）

---

## Self-Balancing BST 原理

### 為何需要「自平衡」？

**問題：** 普通 BST 最壞情況退化成鏈表 O(n)

```
插入順序：1, 2, 3, 4, 5          期望：平衡樹
    1                              3
     \                           /   \
      2        → 退化           2      4
       \                       /        \
        3                     1          5
         \
          4
           \
            5
```

**解決：** 自平衡通過旋轉保持 O(log n) 高度

### 主流實現對比

| 實現                     | 特性          | C++ 容器               | 最壞高度  |
| ------------------------ | ------------- | ---------------------- | --------- |
| 紅黑樹（Red-Black Tree） | 插入/刪除較快 | `std::set`             | 2log n    |
| AVL 樹                   | 查找最快      | -                      | 1.44log n |
| B 樹                     | 磁碟友好      | `std::map`（部分實現） | -         |

**C++ 選擇紅黑樹的原因：**

1. 插入/刪除僅需 **最多 3 次旋轉**（AVL 需 O(log n) 次）
2. 查找性能略差但可接受（2x vs 1.44x）
3. 工程上平衡性能與複雜度

---

## C++ std::set 實現：紅黑樹

### 紅黑樹五大性質

```cpp
enum Color { RED, BLACK };

struct Node {
    int val;
    Color color;
    Node *left, *right, *parent;
};
```

1. **節點顏色**：只有紅色或黑色
2. **根節點**：必須是黑色
3. **葉節點**：NIL 節點（虛擬）為黑色
4. **紅色限制**：紅色節點的子節點必須是黑色（無連續紅色）
5. **黑色深度**：任意節點到葉節點的所有路徑，黑色節點數量相同

### 為何這些性質保證 O(log n)？

**關鍵推導：** 性質 4 + 5 → 最長路徑 ≤ 2 × 最短路徑

```
最短路徑（全黑）：h
最長路徑（紅黑交替）：2h
包含 n 個節點的樹：h ≥ log(n+1) - 1
∴ 最壞高度 = 2log(n+1)
```

### 插入時的平衡維護

```cpp
// 簡化概念（實際實現更複雜）
void insert_fixup(Node* z) {
    while (z->parent->color == RED) {  // 違反性質 4
        if (z->parent == z->parent->parent->left) {
            Node* uncle = z->parent->parent->right;
            if (uncle->color == RED) {
                // Case 1: 叔叔紅色 → 重新著色
                recolor(z);
            } else {
                // Case 2/3: 叔叔黑色 → 旋轉
                rotate(z);
            }
        }
    }
    root->color = BLACK;  // 保證性質 2
}
```

**三種情況（均在 O(1) 內修復）：**

1. 叔叔紅色：重新著色，向上遞迴
2. 叔叔黑色 + 折線：旋轉成直線
3. 叔叔黑色 + 直線：旋轉 + 著色完成

**LeetCode 啟示：** 不需手寫紅黑樹，但理解平衡機制有助於估算性能

---

## 插入操作設計

### `insert(value)` - 插入元素

**介面：** `pair<iterator, bool> insert(const T& value);`

**設計重點：**

1. **返回類型：`pair<iterator, bool>`**

   ```cpp
   auto [it, inserted] = s.insert(42);
   // it: 元素位置的迭代器
   // inserted: true = 新插入, false = 已存在
   ```

   **為何不只返回 bool？**

   - 經常需要迭代器進行後續操作（erase, 範圍查詢）
   - 即使元素存在，也能定位到它

2. **性能保證：O(log n)**
   ```cpp
   s.insert(x);  // 最壞 2log(n) 比較 + 最多 3 次旋轉
   ```

**LeetCode 場景：**

```cpp
// ✅ 檢查是否新插入
if (s.insert(x).second) {
    // 首次出現，進行特殊處理
}

// ✅ 批量插入排序
vector<int> nums = {3, 1, 4, 1, 5};
set<int> s(nums.begin(), nums.end());  // 自動去重 + 排序
```

### `emplace(args...)` - 原地構造

```cpp
set<pair<int, int>> s;
s.emplace(1, 2);  // 直接構造 pair，避免臨時對象
```

**優勢：** 減少 copy/move，但對 `int` 等簡單類型無差異

---

## 查找操作設計

### `find(value)` vs `count(value)`

**介面：**

```cpp
iterator find(const T& value);     // 返回迭代器
size_t count(const T& value);      // 返回 0 或 1（set 唯一性）
```

**設計對比：**

| 場景       | `find()`     | `count()`      | 推薦                |
| ---------- | ------------ | -------------- | ------------------- |
| 存在性檢查 | `!= end()`   | `== 1`         | ⭐ `count()` 更直觀 |
| 需要迭代器 | 直接用       | 需二次查找     | ⭐ `find()`         |
| 後續刪除   | 避免重複查找 | 需再 `erase()` | ⭐ `find()`         |

**LeetCode 典型模式：**

```cpp
// ✅ 單純檢查存在
if (s.count(x)) { }

// ✅ 需操作元素
auto it = s.find(x);
if (it != s.end()) {
    s.erase(it);  // 避免重複查找
}

// ❌ 低效：連續兩次查找
if (s.count(x)) {
    s.erase(x);  // 又查找一次
}
```

---

## 刪除操作設計

### `erase()` 的三種重載

**介面：**

```cpp
size_t erase(const T& value);         // 按值刪除，返回刪除數量
iterator erase(iterator pos);         // 按位置刪除，返回下一個
iterator erase(iterator first, last); // 範圍刪除
```

**設計重點：**

1. **返回值差異**

   ```cpp
   s.erase(42);      // 返回 0 或 1
   s.erase(it);      // 返回下一個迭代器（C++11+）
   ```

   **為何迭代器版本返回迭代器？**

   - 允許連續刪除：`it = s.erase(it);`
   - 避免迭代器失效

2. **性能差異**
   ```cpp
   s.erase(value);   // O(log n) 查找 + O(log n) 刪除
   s.erase(it);      // O(log n) 刪除（已定位）
   ```

**LeetCode 高效模式：**

```cpp
// ✅ 迭代中刪除
for (auto it = s.begin(); it != s.end(); ) {
    if (*it % 2 == 0) {
        it = s.erase(it);  // 返回下一個
    } else {
        ++it;
    }
}

// ✅ 範圍刪除：刪除 [10, 20)
s.erase(s.lower_bound(10), s.lower_bound(20));
```

---

## 範圍查詢設計

### `lower_bound()` vs `upper_bound()`

**介面：**

```cpp
iterator lower_bound(const T& value);  // >= value 的第一個
iterator upper_bound(const T& value);  // > value 的第一個
```

**設計核心：** 二分查找的兩種邊界

```cpp
set<int> s = {1, 3, 3, 5, 7};

s.lower_bound(3);  // 指向第一個 3
s.upper_bound(3);  // 指向 5（第一個 > 3）

// 不存在時
s.lower_bound(4);  // 指向 5（第一個 >= 4）
s.upper_bound(4);  // 也指向 5（第一個 > 4）
```

**記憶技巧：**

- `lower_bound`：「下界包含」→ `>=`
- `upper_bound`：「上界不包含」→ `>`

### `equal_range()` - 一次取得兩個邊界

```cpp
auto [l, r] = s.equal_range(x);  // [lower_bound, upper_bound)
```

**優勢：** 單次二分查找完成（內部優化）

**LeetCode 經典場景：**

```cpp
// 場景 1：範圍計數
auto [l, r] = s.equal_range(x);
int count = distance(l, r);

// 場景 2：插入位置
auto pos = s.lower_bound(x);
// 插入後 x 會在 pos 之前

// 場景 3：查找最接近的值
auto it = s.lower_bound(x);
if (it != s.begin()) {
    auto prev = --it;
    // prev 是 < x 的最大值
}
```

---

## 迭代器設計

### 雙向迭代器（Bidirectional Iterator）

**特性：**

```cpp
auto it = s.begin();
++it;   // 前進（中序遍歷的下一個），O(1) 均攤
--it;   // 後退，O(1) 均攤
```

### 為何不是隨機訪問迭代器？

**C++ 迭代器類別對比：**

| 迭代器類別 | 支援操作 | 典型容器 | 跳躍 n 步 |
|-----------|---------|----------|-----------|
| 隨機訪問（Random Access） | `it + n`, `it[n]` | `vector`, `deque` | **O(1)** |
| 雙向（Bidirectional） | `++it`, `--it` | `set`, `map`, `list` | **O(n)** |
| 前向（Forward） | `++it` | `forward_list` | O(n) |

**核心差異：樹 vs 陣列的記憶體佈局**

```cpp
// vector：連續記憶體 → O(1) 跳躍
vector<int> v = {1, 2, 3, 4, 5};
auto it = v.begin();
it += 5;  // ✅ 直接指針算術：it = it + 5 * sizeof(int)

// set：樹結構 → O(n) 遍歷
set<int> s = {1, 2, 3, 4, 5};
auto it = s.begin();
it += 5;  // ❌ 編譯錯誤！雙向迭代器不支援
```

**為何 `set` 不能 O(1) 跳躍？**

樹節點在記憶體中**非連續**，前進必須遍歷：

```
     3
   /   \
  2     5      要從 1 跳到 5：
 /     /       1. 從 1 向上到 2
1     4        2. 從 2 向上到 3
               3. 從 3 向右到 5
               4. 從 5 向左到 4
               → 需要 5 步操作！
```

### 正確的跳躍方法：`std::advance()`

```cpp
set<int> s = {1, 2, 3, 4, 5};
auto it = s.begin();

// ✅ 使用 advance（會根據迭代器類別選擇最佳實現）
std::advance(it, 5);  // O(5)：內部調用 5 次 ++it

// 等價於手動：
for (int i = 0; i < 5; ++i) ++it;
```

**`std::advance()` 的設計智慧：**

```cpp
// 內部實現（簡化）
template<typename Iter>
void advance(Iter& it, int n) {
    if constexpr (is_random_access) {
        it += n;  // O(1)，vector 等
    } else {
        while (n--) ++it;  // O(n)，set 等
    }
}
```

### 中序遍歷保證有序

**實現概念：**

```cpp
void inorder(Node* node) {
    if (!node) return;
    inorder(node->left);   // 左子樹
    visit(node);           // 當前節點
    inorder(node->right);  // 右子樹
}
```

**迭代器的 `++` 操作：**

1. 有右子樹 → 右子樹的最左節點
2. 無右子樹 → 向上找到第一個「右轉」的祖先

**LeetCode 應用：**

```cpp
// ✅ 有序遍歷
for (auto it = s.begin(); it != s.end(); ++it) {
    // 保證升序
}

// ✅ 反向遍歷（降序）
for (auto it = s.rbegin(); it != s.rend(); ++it) {
    // 保證降序
}

// ✅ 取第 k 小
auto it = s.begin();
advance(it, k - 1);  // O(k) 時間
```

---

## API 設計原則總結

### LeetCode 場景快速決策表

| 場景        | 推薦 API          | 時間複雜度 | 說明           |
| ----------- | ----------------- | ---------- | -------------- |
| 檢查存在    | `count(x)`        | O(log n)   | 返回 0/1       |
| 插入去重    | `insert(x)`       | O(log n)   | 自動去重       |
| 刪除元素    | `erase(x)`        | O(log n)   | 返回刪除數量   |
| 最小值      | `*s.begin()`      | O(1)       | 紅黑樹最左節點 |
| 最大值      | `*s.rbegin()`     | O(1)       | 最右節點       |
| >= x 第一個 | `lower_bound(x)`  | O(log n)   | 二分查找       |
| > x 第一個  | `upper_bound(x)`  | O(log n)   | 二分查找       |
| 範圍刪除    | `erase(it1, it2)` | O(k log n) | k = 範圍大小   |

### 與其他容器對比

| 操作     | set      | multiset | unordered_set | priority_queue |
| -------- | -------- | -------- | ------------- | -------------- |
| 唯一性   | ✅       | ❌       | ✅            | ❌             |
| 有序性   | ✅       | ✅       | ❌            | 部分（堆頂）   |
| 插入     | O(log n) | O(log n) | O(1) 均攤     | O(log n)       |
| 查找     | O(log n) | O(log n) | O(1) 均攤     | ❌             |
| 刪除任意 | O(log n) | O(log n) | O(1) 均攤     | ❌             |
| 範圍查詢 | ✅       | ✅       | ❌            | ❌             |

**選擇指南：**

- 需要去重 + 有序 → `set`
- 允許重複 + 有序 → `multiset`
- 只需去重（無序） → `unordered_set`
- 只需最大/最小值 → `priority_queue`

### 核心設計理念

1. **時間保證：** 所有操作 O(log n)（紅黑樹）
2. **空間保證：** 樹結構開銷（每節點額外指針 + 顏色）
3. **介面一致性：** 返回迭代器便於組合操作
4. **STL 整合：** 支援標準算法（`equal_range`, `distance`）

---

## 常見錯誤與最佳實踐

### ❌ 錯誤模式

```cpp
// 錯誤 1：修改迭代器指向的值
auto it = s.find(x);
*it = new_value;  // ❌ set 元素是 const（破壞有序性）

// 錯誤 2：假設隨機訪問
auto it = s.begin();
it += 5;  // ❌ 編譯錯誤，非隨機訪問迭代器

// 錯誤 3：範圍查詢邊界錯誤
s.erase(s.lower_bound(x), s.upper_bound(x));  // 只刪除等於 x
s.erase(s.lower_bound(x), s.upper_bound(y));  // 刪除 [x, y]
```

### ✅ 正確模式

```cpp
// 模式 1：需要修改值 → 先刪後插
s.erase(it);
s.insert(new_value);

// 模式 2：遍歷 k 個元素
auto it = s.begin();
for (int i = 0; i < k && it != s.end(); ++i, ++it) { }

// 模式 3：滑動窗口維護有序
multiset<int> window;  // 允許重複
window.insert(nums[r]);
if (window.size() > k) {
    window.erase(window.find(nums[l++]));  // 只刪一個
}
int median = *next(window.begin(), k / 2);
```

---

## 延伸閱讀

- **Multi-set 應用：** 當需要允許重複元素時使用 `std::multiset`
- **自定義比較函數：** `set<T, Compare>` 實現自定義排序
- **C++20 改進：** `contains()` 方法簡化存在性檢查
- **性能分析：** [`container_size_api_analysis.md`](./container_size_api_analysis.md)
