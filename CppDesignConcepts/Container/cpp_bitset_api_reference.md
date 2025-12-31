# C++ Bitset API 設計分析：超越 32/64 位限制的位元操作

從 API 設計角度分析 `std::bitset`，理解何時需要突破整數位元操作的限制。

## TL;DR - LeetCode 位元操作決策樹

```cpp
// 決策流程
位數 ≤ 32  → int + (1 << i)
位數 ≤ 64  → long long + (1LL << i)
位數固定 ≤ 幾千 → std::bitset<N>        // ⭐ 本文重點
位數動態 → std::vector<bool>          // 空間優化但無位元運算
```

### 快速參考

```cpp
bitset<100> bs;
bs.set(i)       // 設定第 i 位，O(1)
bs.reset(i)     // 清除第 i 位，O(1)
bs.flip(i)      // 翻轉第 i 位，O(1)
bs.test(i)      // 測試第 i 位，O(1)
bs[i]           // 存取（可修改），無檢查
bs.count()      // 1 的個數，O(N/64)
bs.any()/none()/all()  // O(N/64)
bs & bs2 / bs | bs2 / bs ^ bs2  // 位元運算，O(N/64)
```

---

## Table of Contents

1. [何時需要 bitset](#何時需要-bitset)
2. [核心 API 設計分析](#核心-api-設計分析)
3. [位元運算設計](#位元運算設計)
4. [效能保證與實作原理](#效能保證與實作原理)
5. [LeetCode 應用場景](#leetcode-應用場景)
6. [設計權衡總結](#設計權衡總結)

---

## 何時需要 bitset

### ✅ 使用 bitset 的場景

**核心判斷：** 需要表示 > 64 個布林狀態且大小固定

```cpp
// 場景 1：大規模集合狀態壓縮（200 個元素）
bitset<200> visited;  // 比 bool[200] 省 200/8 ≈ 25 倍空間

// 場景 2：質數篩（Sieve of Eratosthenes）
bitset<1000001> is_prime;
is_prime.set();  // 全設為 1

// 場景 3：DP 狀態壓縮（100 個布林維度）
bitset<100> dp_state;
```

### ❌ 不需要 bitset 的場景

```cpp
// 場景 1：位數 ≤ 64
int mask = 0;  // 32 位足夠
long long mask = 0;  // 64 位足夠

// 場景 2：不需要位元運算
vector<bool> flags(n);  // 動態大小，空間優化

// 場景 3：需要動態大小
// bitset<N> 的 N 必須編譯期常數，無法 bitset<n>
```

---

## 核心 API 設計分析

### 1. [`set()`](https://en.cppreference.com/w/cpp/utility/bitset/set) / [`reset()`](https://en.cppreference.com/w/cpp/utility/bitset/reset) / [`flip()`](https://en.cppreference.com/w/cpp/utility/bitset/flip) - 單點修改

**介面：**

```cpp
bitset& set(size_t pos, bool value = true);   // 設定
bitset& reset(size_t pos);                     // 清除
bitset& flip(size_t pos);                      // 翻轉
```

**設計重點：**

1. **返回 `bitset&` 支援鏈式調用**

   ```cpp
   bs.set(1).set(3).set(5);  // 設定多個位置
   ```

2. **重載：單點 vs 全體**

   ```cpp
   bs.set(5);     // 設定第 5 位
   bs.set();      // 設定全部位
   bs.reset(3);   // 清除第 3 位
   bs.reset();    // 清除全部
   ```

3. **對比整數操作**
   | 操作 | bitset | int/long long |
   |------|--------|---------------|
   | 設定 | `bs.set(i)` | `mask \|= (1LL << i)` |
   | 清除 | `bs.reset(i)` | `mask &= ~(1LL << i)` |
   | 翻轉 | `bs.flip(i)` | `mask ^= (1LL << i)` |

**LeetCode 場景：**

```cpp
// DP：記錄子集狀態
bitset<100> reachable;
for (int x : nums) reachable.set(x);
```

### 2. [`test()`](https://en.cppreference.com/w/cpp/utility/bitset/test) / [`operator[]`](https://en.cppreference.com/w/cpp/utility/bitset/operator_at) - 查詢設計

**介面：**

```cpp
bool test(size_t pos) const;         // 有邊界檢查，拋異常
reference operator[](size_t pos);    // 無檢查，可修改
```

**設計重點：**

1. **`test()` vs `[]` 權衡**
   | 特性 | `test(i)` | `bs[i]` |
   |------|-----------|---------|
   | 邊界檢查 | ✅ 拋 `out_of_range` | ❌ UB |
   | 效能 | 慢（檢查開銷） | 快 |
   | 修改 | ❌ const | ✅ `bs[i] = true` |
   | LeetCode | 除錯用 | ⭐ 推薦 |

2. **`operator[]` 返回 proxy reference**
   ```cpp
   bs[5] = true;      // ✅ 可修改
   bool val = bs[5];  // ✅ 可讀取
   auto& ref = bs[5]; // ⚠️ 不是真正的 bool&，是 proxy
   ```

**LeetCode 模式：**

```cpp
// ✅ 範圍已知，用 []
if (bs[i]) { ... }

// ✅ 動態範圍，用 test()
if (i < N && bs.test(i)) { ... }
```

### 3. [`count()`](https://en.cppreference.com/w/cpp/utility/bitset/count) / [`any()`](https://en.cppreference.com/w/cpp/utility/bitset/any) / [`none()`](https://en.cppreference.com/w/cpp/utility/bitset/none) / [`all()`](https://en.cppreference.com/w/cpp/utility/bitset/all) - 聚合查詢

**介面：**

```cpp
size_t count() const noexcept;  // 計算 1 的個數
bool any() const noexcept;      // 是否有任一位為 1
bool none() const noexcept;     // 是否全為 0
bool all() const noexcept;      // 是否全為 1
```

**設計重點：**

1. **`count()` 效能保證：O(N/64)**

   - 實作使用 `__builtin_popcountll()` 或查表法
   - 比逐位檢查快 64 倍

2. **語義清晰的命名**
   ```cpp
   if (bs.any())   // 有任何訪問過的節點？
   if (bs.none())  // 完全沒訪問過？
   if (bs.all())   // 全部訪問過？
   ```

**LeetCode 場景：**

```cpp
// 檢查是否訪問所有節點
bitset<100> visited;
// ... 遍歷過程 ...
if (visited.count() == n) { return valid; }

// 提前終止
if (visited.all()) break;
```

---

## 位元運算設計

### [`operator&`](https://en.cppreference.com/w/cpp/utility/bitset/operator_and2) / [`operator|`](https://en.cppreference.com/w/cpp/utility/bitset/operator_or2) / [`operator^`](https://en.cppreference.com/w/cpp/utility/bitset/operator_xor2) - 集合運算

**介面：**

```cpp
bitset operator&(const bitset& rhs) const;  // 交集
bitset operator|(const bitset& rhs) const;  // 聯集
bitset operator^(const bitset& rhs) const;  // 對稱差
bitset operator~() const;                    // 補集
```

**設計重點：**

1. **集合語義**

   ```cpp
   bitset<100> setA, setB;
   auto intersection = setA & setB;  // 交集
   auto union_set = setA | setB;     // 聯集
   auto diff = setA ^ setB;          // 對稱差
   ```

2. **效能：O(N/64) 而非 O(N)**
   - 內部以 64-bit word 為單位處理
   - `bitset<200>` 只需 4 次 64-bit 運算

**LeetCode 場景：**

```cpp
// 找兩個集合的共同元素
bitset<26> chars1, chars2;  // 字母集合
auto common = chars1 & chars2;
if (common.any()) { ... }  // 有共同字母

// DP：狀態合併
bitset<100> state1, state2;
auto merged = state1 | state2;
```

---

## 效能保證與實作原理

### 空間複雜度

```cpp
bitset<N>       // 恰好 N 位，向上取整到 64-bit word
                // 實際: ⌈N/64⌉ * 8 bytes

bitset<100>     // 2 * 8 = 16 bytes
bool[100]       // 100 bytes（省 6.25 倍）
```

### 時間複雜度對比

| 操作      | bitset  | int/long long               | 說明                   |
| --------- | ------- | --------------------------- | ---------------------- |
| `set(i)`  | O(1)    | O(1)                        | 相同                   |
| `count()` | O(N/64) | O(1) `__builtin_popcountll` | bitset 需遍歷多個 word |
| `&/\|/^`  | O(N/64) | O(1)                        | bitset 需處理多個 word |

### 實作原理

```cpp
// 概念模型（實際實作因編譯器而異）
template<size_t N>
class bitset {
    uint64_t words[(N + 63) / 64];  // 分塊存儲

    void set(size_t pos) {
        words[pos / 64] |= (1ULL << (pos % 64));
    }

    size_t count() const {
        size_t cnt = 0;
        for (auto w : words) cnt += __builtin_popcountll(w);
        return cnt;
    }
};
```

---

## LeetCode 應用場景

### 1. 質數篩（Sieve of Eratosthenes）

```cpp
bitset<1000001> is_prime;
is_prime.set();  // 全設為 1
is_prime[0] = is_prime[1] = 0;
for (int i = 2; i * i <= 1000000; ++i) {
    if (is_prime[i]) {
        for (int j = i * i; j <= 1000000; j += i)
            is_prime[j] = 0;
    }
}
// is_prime.count() = 質數個數
```

### 2. 大規模 DP 狀態壓縮

```cpp
// 問題：200 個物品，哪些子集可達目標和
bitset<10001> dp;  // dp[sum] = 是否可達
dp[0] = 1;
for (int x : nums) {
    dp |= (dp << x);  // ⭐ 位移 + OR = 狀態轉移
}
```

### 3. 字元集合快速判斷

```cpp
// 判斷兩字串是否有共同字母
bitset<26> s1_chars, s2_chars;
for (char c : s1) s1_chars[c - 'a'] = 1;
for (char c : s2) s2_chars[c - 'a'] = 1;
if ((s1_chars & s2_chars).any()) { ... }
```

---

## 設計權衡總結

### 核心設計原則

1. **編譯期大小限制 = 極致效能**

   - 無動態分配開銷
   - 編譯器可充分優化

2. **64-bit word 分塊實作**

   - 聚合操作（`count`, `&`, `|`）以 64-bit 為單位
   - 在 N > 64 時仍保持高效

3. **統一的 set theory 語義**
   - `&` = 交集，`|` = 聯集，`^` = 對稱差
   - 直觀對應數學集合運算

### 決策指南

| 場景               | 推薦工具                    | 原因                   |
| ------------------ | --------------------------- | ---------------------- |
| n ≤ 32             | `int`                       | 原生支援，最快         |
| n ≤ 64             | `long long`                 | 原生支援，最快         |
| n 固定 ≤ 幾千      | `bitset<N>`                 | ⭐ 空間效率 + 豐富 API |
| n 動態，無位元運算 | `vector<bool>`              | 動態大小 + 空間優化    |
| n 動態，需位元運算 | 自行實作 `vector<uint64_t>` | 靈活性                 |

### 常見錯誤

```cpp
// ❌ 錯誤：試圖用變數指定大小
int n = 100;
bitset<n> bs;  // 編譯錯誤！n 必須是常數

// ✅ 正確：編譯期常數
constexpr int N = 100;
bitset<N> bs;

// ❌ 錯誤：假設 operator[] 返回 bool&
auto& ref = bs[5];  // ref 是 proxy，不是 bool&

// ✅ 正確：取值或修改
bool val = bs[5];
bs[5] = true;
```

### C++ 演進

- **C++98:** 引入 `bitset`
- **C++11:** `noexcept`, constexpr 支援
- **C++23:** `operator<=>` 三向比較

---

## 延伸閱讀

- **動態位元操作:** [`vector_bool_space_analysis.md`](vector_bool_space_analysis.md)
- **位元操作演算法:** `Algorithms/Bit_Manipulation/`
