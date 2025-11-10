# Container `size()` API：Unsigned 陷阱與 `ssize()` 解決方案

## TL;DR

| 場景         | ❌ 常見錯誤              | ⭐ 正確做法                              |
| ------------ | ------------------------ | ---------------------------------------- |
| 迴圈邊界計算 | `i < v.size() - k`       | `i + k < v.size()` 或 `i < ssize(v) - k` |
| `std::min`   | `min(len, v.size())`     | `min(len, ssize(v))`                     |
| 反向遍歷     | `i >= 0 && i < v.size()` | `i < ssize(v)` (signed 索引)             |
| 空容器檢查   | `v.size() == 0`          | `v.empty()`                              |

---

## 為什麼 `size()` 返回 `size_t` (unsigned)

```cpp
container::size_type size() const noexcept;  // size_type = size_t (unsigned)
```

**設計理由：**

1. **容量最大化**：64-bit 系統，signed 無法表達完整記憶體範圍（2^63 vs 2^64）
2. **系統一致性**：`malloc(size_t)`, `new(size_t)` 都用 unsigned
3. **語義正確**：容器大小概念上非負

---

## 五大 Unsigned 陷阱

### 1. 減法 Underflow

```cpp
// ❌ 當 nums.size() < k 時爆炸
for (int i = 0; i < nums.size() - k; ++i) {
    // size = 2, k = 3 → 2 - 3 = 18446744073709551615 (underflow!)
}

// ⭐ 解法：移項避免減法
for (int i = 0; i + k < nums.size(); ++i) {}

// ⭐ 或用 ssize() (C++20)
for (int i = 0; i < ssize(nums) - k; ++i) {}
```

### 2. `std::min/max` 型別衝突

```cpp
int windowSize = 5;

// ❌ Compile Error: 無法推導 int vs size_t
int n = std::min(windowSize, nums.size());

// ⭐ 解法
int n = std::min(windowSize, ssize(nums));  // C++20
int n = std::min<int>(windowSize, nums.size());  // 明確型別
```

### 3. 與負數比較

```cpp
int delta = -5;

// ❌ 隱式轉換陷阱
if (v.size() > delta) {  // delta 變成 18446744073709551611
    // 永遠 false！
}

// ⭐ 解法
if (ssize(v) > delta) {}  // 正確：3 > -5
```

### 4. 反向遍歷無窮迴圈

```cpp
// ❌ unsigned 永遠 >= 0，--i 從 0 變成 MAX
for (size_t i = v.size()-1; i >= 0; --i) {}

// ⭐ 解法
for (int i = ssize(v)-1; i >= 0; --i) {}

// ⭐ 或用反向迭代器（最佳）
for (auto it = v.rbegin(); it != v.rend(); ++it) {}
```

### 5. 空容器邊界

```cpp
// ❌ v.size()-1 在空容器時 underflow
if (v.size() - 1 >= 0) {}  // 永遠 true

// ⭐ 解法
if (!v.empty()) {
    int last = v.back();  // 安全 + 語義清晰
}
```

---

## C++20 `ssize()` 解決方案

```cpp
#include <iterator>

template<class C>
constexpr auto ssize(const C& c) -> /* ptrdiff_t (signed) */;

// 使用範例
vector<int> v{1,2,3,4,5};
int k = 3;

// ⭐ 滑動窗口安全
for (int i = 0; i < ssize(v) - k; ++i) {}

// ⭐ std::min 無衝突
int n = std::min(windowSize, ssize(v));

// ⭐ C 陣列也支援
int arr[] = {1,2,3};
static_assert(ssize(arr) == 3);
```

**支援情況：**

- 所有標準容器（除 `forward_list`）：✅
- C 陣列：✅ (C++20)
- `forward_list`：❌

**為何 `forward_list` 無 `size()`？**

```cpp
forward_list<int> fl{1, 2, 3};
// fl.size();  // ❌ 編譯錯誤

auto sz = std::distance(fl.begin(), fl.end());  // ⭐ O(n) 顯式操作
```

設計理由（零成本抽象原則）：

1. **維護 size 需額外記憶體**：單向鏈表每個節點已存 data + next，加 size 成員違反最小化設計
2. **`splice()` 操作成本**：若維護 size，接合兩個 list 需 O(n) 遍歷計算，破壞 O(1) splice 語義
3. **強制顯式意圖**：需計數時用 `distance()`，明確表達「這是 O(n) 操作」
4. **與其他容器一致性**：所有容器 `size()` 保證 O(1)，`forward_list` 無法達成則乾脆不提供

[參考討論](https://stackoverflow.com/questions/31822494/why-does-stdforward-list-have-no-size-member-function)
