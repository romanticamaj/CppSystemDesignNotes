# C++ Random / Shuffle 設計分析

從 API 設計角度分析 [`std::shuffle`](https://en.cppreference.com/w/cpp/algorithm/random_shuffle) 與 [`std::iota`](https://en.cppreference.com/w/cpp/algorithm/iota)，理解設計決策。

## TL;DR

```cpp
// 生成序列
vector<int> v(10);
std::iota(v.begin(), v.end(), 0);  // [0,1,2,3,...]

// 隨機打亂 (C++11+)
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(v.begin(), v.end(), g);

// 組合：生成序列 + 隨機打亂
std::iota(nums.begin(), nums.end(), 0);
std::shuffle(nums.begin(), nums.end(), g);
```

---

## 1. 隨機打亂：random_shuffle → shuffle

### ❌ `std::random_shuffle` (C++17 已移除)

**設計缺陷：**

- 使用內部全局 RNG（實作定義，不可控）
- 無法指定 seed，不可重現
- 不同編譯器行為不一致

```cpp
std::random_shuffle(v.begin(), v.end());  // 使用哪個 RNG？無從得知
```

**為何移除？** C++ 委員會認為隱藏隨機源是設計錯誤，現代程式需要可控、可測試的隨機性。

---

### ⭐ `std::shuffle` - 現代推薦

**介面：** `void shuffle(RandomIt first, RandomIt last, URBG&& g);`

**核心設計：顯式 RNG 參數**

```cpp
std::random_device rd;     // 硬體隨機源（seed）
std::mt19937 g(rd());      // Mersenne Twister 引擎
std::shuffle(v.begin(), v.end(), g);
```

**為何要求傳入 RNG？**

- 可控性：明確使用哪個引擎
- 可重現：固定 seed 用於測試
- 可測試：單元測試可驗證

**參數設計：`URBG&&` 而非 `URBG&`**

- 允許傳入臨時對象 `std::shuffle(v.begin(), v.end(), std::mt19937{42});`
- 支援 move-only 類型

**效能：** O(n) 時間, O(1) 空間，Fisher-Yates 演算法，均勻分佈保證（每個排列機率 1/n!）

**LeetCode 模式：**

```cpp
// ✅ 可重現（測試）
std::mt19937 g(42);
std::shuffle(nums.begin(), nums.end(), g);

// ✅ 真隨機（提交）
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(nums.begin(), nums.end(), g);
```

**RNG 選擇：**

- ⭐ [`std::mt19937`](https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine): 高品質，標準選擇
- ❌ [`std::default_random_engine`](https://en.cppreference.com/w/cpp/numeric/random): 實作定義，不可移植
- ❌ `rand()`: 品質低，已棄用

---

## 2. 序列生成：std::iota

**介面：** `void iota(ForwardIt first, ForwardIt last, T value);`  
**標頭：** `<numeric>`

**設計重點：**

1. **命名：** 來自 APL 語言的 ⍳ 運算子，希臘字母 ι（最小）象徵遞增

2. **參數：`(first, last, value)` 無步長**

   ```cpp
   std::iota(v.begin(), v.end(), 0);    // [0,1,2,...]
   std::iota(v.begin(), v.end(), 100);  // [100,101,102,...]
   ```

   **為何無步長？**

   - 簡化介面（90% 場景步長為 1）
   - 避免類型轉換問題
   - 明確語義：「遞增」vs「等差」

3. **返回 `void`：** 純副作用操作，與 [`fill`](https://en.cppreference.com/w/cpp/algorithm/fill) 一致

**LeetCode 關鍵：**

```cpp
// ⭐ 生成索引數組
vector<int> indices(n);
std::iota(indices.begin(), indices.end(), 0);

// ⭐ 間接排序
std::iota(nums.begin(), nums.end(), 1);
std::sort(nums.begin(), nums.end(), [&](int a, int b) {
    return arr[a] < arr[b];
});

// ❌ 避免手動循環
for (int i = 0; i < n; i++) nums[i] = i;  // 冗長易錯
```

**需要步長？** 使用 [`std::generate`](https://en.cppreference.com/w/cpp/algorithm/generate) 或 `iota` + [`transform`](https://en.cppreference.com/w/cpp/algorithm/transform)：

```cpp
int value = 0, step = 3;
std::generate(v.begin(), v.end(), [&]() { return value += step; });
```

---

## 3. 組合應用模式

**隨機排列生成（LeetCode 384）：**

```cpp
class Solution {
    vector<int> original;
    std::mt19937 gen{std::random_device{}()};
public:
    vector<int> shuffle() {
        vector<int> result = original;
        std::shuffle(result.begin(), result.end(), gen);
        return result;
    }
};
```

**部分打亂（Fisher-Yates）：**

```cpp
std::iota(nums.begin(), nums.end(), 0);
std::shuffle(nums.begin(), nums.begin() + k, gen);  // 只打亂前 k 個
```

---

## 設計原則總結

| 場景      | 推薦                  | 避免             | 原因           |
| --------- | --------------------- | ---------------- | -------------- |
| 隨機打亂  | `shuffle` + `mt19937` | `random_shuffle` | 已移除，不可控 |
| 遞增序列  | `iota`                | 手動循環         | 語義清晰       |
| 固定 seed | `mt19937(42)`         | `srand` + `rand` | 可重現         |

**核心原則：**

1. **顯式優於隱式：** `shuffle` 要求傳入 RNG，明確控制隨機源
2. **語義優於實作：** `iota` 表達意圖，程式碼即文檔
3. **組合優於複雜：** `iota` + `shuffle` 組合 > 複雜多參數函數

**常見錯誤：**

```cpp
// ❌
std::random_shuffle(v.begin(), v.end());  // 已移除
std::default_random_engine e;             // 不可移植
for (int i = 0; i < n; i++) v[i] = i;     // 冗長

// ✅
std::mt19937 g(rd());
std::shuffle(v.begin(), v.end(), g);
std::iota(v.begin(), v.end(), 0);
```

**C++ 演進：**

- C++98: `random_shuffle` 引入
- C++11: `shuffle`, `iota`, `<random>` 引入
- C++14: `random_shuffle` deprecated
- C++17: `random_shuffle` 移除
