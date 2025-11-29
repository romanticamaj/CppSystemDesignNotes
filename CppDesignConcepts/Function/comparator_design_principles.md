# C++ Comparator 設計完整心法

### —— C++ 技術顧問暨資深開發者報告

---

## 1. 為何 Comparator 在 C++ 中這麼重要？

C++ 中 comparator 不是「比較大小」而已，而是決定排序語義的核心機制。

它必須滿足 **Strict Weak Ordering（嚴格弱序）**：

- **非自反性 (Irreflexivity)**：`comp(a,a)` 必為 false
- **反對稱性 (Asymmetry)**：若 `comp(a,b)` 為 true，則 `comp(b,a)` 必為 false
- **可傳遞性 (Transitivity)**：`comp(a,b)` 且 `comp(b,c)` → `comp(a,c)`
- **等價類一致性 (Transitivity of Equivalence)**：若 `a` 與 `b` 等價（即 `!comp(a,b) && !comp(b,a)`），且 `b` 與 `c` 等價，則 `a` 與 `c` 必等價

違反這些將導致：

- `std::sort` **未定義行為 (UB)**
- `std::map` / `std::set` 錯誤行為
- `priority_queue` 比較順序錯誤

**Comparator 的核心使命：為資料建立一個穩定、一致的排序關係。**

---

## 2. priority_queue 的比較語義（與 sort/map 相反）

這是許多人最常搞錯的概念。在 `priority_queue` 裡

```
comp(a, b) == true  →  a 的優先度比 b 低
```

因此：

| 想要的行為           | comparator 寫法 |
| -------------------- | --------------- |
| max-heap（大者優先） | `return a < b;` |
| min-heap（小者優先） | `return a > b;` |

---

## 3. Comparator 的實作方式

### 3.1 Functor（struct operator()）→ 最推薦、企業級常用

```cpp
struct Cmp {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};
```

- 可 inline
- 無捕捉（符合 Compare 概念）
- 適合放入 STL 容器

### 3.2 Lambda（用 decltype）

```cpp
auto cmp = [](const T& a, const T& b){ return a < b; };
priority_queue<T, vector<T>, decltype(cmp)> pq(cmp);
```

- 需 `decltype(cmp)`
- 若是 class-level，需要 `static inline` 成員變數：
  ```cpp
  class Solution {
      static inline auto cmp = [](int a, int b) { return a > b; };

      // 1. Template 參數必須傳入型別：decltype(cmp)
      // 2. 建構子參數 {cmp}：
      //    - C++20 前：必須傳入 (lambda 預設無法 default construct)
      //    - C++20 起：無捕捉 lambda 可 default construct，故可省略 {cmp}
      std::priority_queue<int, std::vector<int>, decltype(cmp)> pq{cmp};
  };
  ```
- 有 capture 時無法用於容器

---

## 4. Comparator 設計心法（最重要章節）

### 心法 1：比較的是「優先度」，不是「大小」

特別是 priority_queue。

- max-heap → `return a < b;`
- min-heap → `return a > b;`

### 心法 2：不要比較等於

錯誤：

```cpp
if (a == b) return false;
```

會破壞 Strict Weak Ordering。

### 心法 3：多欄位比較必須建立唯一順序

錯誤：

```cpp
return a.first > b.first && a.second > b.second;
```

這無法形成線性順序。

正確：

```cpp
if (a.first != b.first)
    return a.first > b.first;
return a.second < b.second;
```

### 心法 4：operator() 必須是 const

容器中的 comparator 是 const 物件。

### 心法 5：map/set/sort 的比較語義與 priority_queue 不同

- `sort/map/set`: comp(a,b)==true → a 排前面
- `priority_queue`: comp(a,b)==true → a 優先度低

---

## 5. 多欄位比較標準模板（建議背起來）

以下範例：

- `int` 小 → 大（min-heap）
- `string` 大 → 小（max-heap）

```cpp
using Item = std::pair<int, std::string>;

struct Cmp {
    bool operator()(const Item& a, const Item& b) const {
        if (a.first != b.first)
            return a.first > b.first;     // int: 小者優先

        return a.second < b.second;       // string: 大者優先
    }
};
```

完全符合 Strict Weak Ordering。

---

## 6. Template 化

```cpp
template <typename Key, typename Value>
struct PairCmp {
    bool operator()(const std::pair<Key,Value>& a,
                    const std::pair<Key,Value>& b) const
    {
        if (a.first != b.first)
            return a.first > b.first;
        return a.second < b.second;
    }
};
```

---

## 7. 實務場景的 Comparator 常用模式

| 場景       | comparator 設計               |
| ---------- | ----------------------------- |
| Dijkstra   | min-heap on distance          |
| A\*        | min-heap on f-score           |
| Scheduler  | min deadline                  |
| Networking | min timestamp                 |
| LRU        | min last-use time             |
| Top-K      | max/min on frequency or score |

---

## 8. 總結

C++ 的比較器本質上是在定義一個「排序語義」。
要寫對 comparator，必須理解：

- Strict Weak Ordering（排序數學基礎）
- priority_queue 的反向語義
- 多欄位比較的正規寫法
- functor/lambda 的使用規範
