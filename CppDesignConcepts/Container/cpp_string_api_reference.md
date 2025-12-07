# C++ String API 設計分析：從介面設計角度理解

從 API 設計的角度分析 C++ `std::string` 介面，理解設計決策背後的原因。

## TL;DR - LeetCode 常用 API 速查

### 基礎操作

```cpp
string s = "hello";
s.length() / s.size()           // 長度
s.empty()                        // 是否為空
s[i]                            // 存取字元（無檢查）
s.at(i)                         // 存取字元（有檢查）
```

### 子字串

```cpp
s.substr(pos, len)              // 提取子字串，O(len) 空間
s.compare(pos, len, str) == 0   // ⭐ 驗證位置，O(1) 空間
```

### 搜尋

```cpp
s.find(str)              // 找子字串，返回 pos 或 npos
s.find(str, pos)         // 從 pos 開始找
s.rfind(str)             // 從右往左找
s.find_first_of("aeiou") // 找任一字元
```

### 修改

```cpp
s += str / s.append(str)  // 追加，O(1) 均攤
s.push_back(c)            // 追加字元
s.insert(pos, str)        // 插入，O(n)
s.erase(pos, len)         // 刪除
```

### 轉換

```cpp
int n = stoi(s)          // ⭐ 字串轉數字（有錯誤檢查）
int n = stoi(s, nullptr, base)  // 指定進制
string s = to_string(n)  // 數字轉字串
```

### 字元判斷

```cpp
isdigit(c)   // 是否數字
isxdigit(c)  // 是否十六進制數字 (0-9, A-F, a-f)
isalpha(c)   // 是否字母
isalnum(c)   // 是否字母或數字
tolower(c) / toupper(c)
```

### 常見模式

```cpp
// ⭐ 檢查特定位置
if (s.compare(i, pattern.size(), pattern) == 0) { ... }

// ⭐ 避免用 substr 比較（浪費空間）
// ❌ if (s.substr(i, 2) == "ab")
// ✅ if (s.compare(i, 2, "ab") == 0)

// 字元轉數字
int digit = c - '0';  // '5' -> 5
```

---

## Table of Contents

1. [子字串提取設計](#子字串提取設計)
2. [搜尋介面設計](#搜尋介面設計)
3. [位置驗證設計](#位置驗證檢查特定位置的子字串)
4. [修改操作設計](#修改操作設計)
5. [字元存取設計](#字元存取設計)
6. [轉換函數設計](#轉換函數設計)
7. [API 設計原則總結](#api-設計原則總結)

---

## 子字串提取設計

### `substr(pos, len = npos)` - 提取子字串

**介面：** `string substr(size_t pos = 0, size_t len = npos) const;`

**設計重點：**

1. **參數設計：`(pos, len)` 而非 `(start, end)`**

   - 長度語義更直觀：「要 5 個字元」vs「到位置 5」
   - 避免 off-by-one 錯誤

2. **效能代價**
   ```cpp
   s.substr(0, 5);  // O(5) 時間 + 空間，創建新字串
   ```

**LeetCode 關鍵：**

```cpp
// ❌ 避免：重複創建臨時字串
for (int i = 0; i < n; i++) {
    if (s.substr(i, 3) == "abc") { ... }  // 每次 O(3) 空間！
}

// ✅ 直接索引或用 compare
for (int i = 0; i + 2 < n; i++) {
    if (s.compare(i, 3, "abc") == 0) { ... }  // O(1) 空間
}
```

**何時使用：** 需要保存子字串時才用，否則優先 `compare()` 或直接索引。

---

## 搜尋介面設計

### `find` 系列 - 搜尋子字串位置

**常用介面：**

```cpp
size_t find(const string& str, size_t pos = 0) const;      // 從左找
size_t rfind(const string& str, size_t pos = npos) const;  // 從右找
size_t find_first_of(const string& chars) const;           // 找任一字元
```

**設計重點：**

1. **返回 `size_t` 而非迭代器**

   - 索引更直觀，可直接運算 `pos + 1`
   - 失敗返回 `string::npos`（最大值，不可能是有效索引）

2. **`find_first_of` 的集合語義**

   ```cpp
   s.find_first_of("aeiou");  // 找任一母音，相當於正則 [aeiou]
   ```

**LeetCode 關鍵：**

```cpp
// ✅ 正確檢查
if (s.find("pattern") != string::npos) { ... }

// ❌ 常見錯誤
if (s.find("pattern")) { ... }  // 找到位置 0 會是 false！

// C++20 前綴檢查
if (s.find("prefix") == 0) { ... }  // 檢查是否以 prefix 開頭
```

**何時使用：** 需要「搜尋」位置時用 `find`；若已知位置只是「驗證」，用 `compare()`（見下節）。

---

## 位置驗證：檢查特定位置的子字串

**問題：** 檢查 `"abcabc"` 位置 4 是否有 `"bc"`？

**核心：** 這是「驗證」（已知位置），不是「搜尋」（找位置）

### ⭐ 推薦：`compare()`

**介面：** `int compare(size_t pos, size_t len, const string& str) const;`

```cpp
string s = "abcabc";
if (s.compare(4, 2, "bc") == 0) {  // 位置 4，長度 2，比較 "bc"
    // 匹配
}
```

**優勢：**

- O(1) 空間（無臨時字串）
- 自動邊界檢查
- 語義清晰

### 其他方法對比

| 方法          | 空間 | LeetCode 推薦 | 說明                   |
| ------------- | ---- | ------------- | ---------------------- |
| `compare()`   | O(1) | ⭐⭐⭐⭐⭐    | 標準做法               |
| `substr() ==` | O(n) | ⭐⭐          | 創建臨時字串，浪費空間 |
| 手動迭代      | O(1) | ⭐⭐⭐        | 最快但冗長             |
| `equal()`     | O(1) | ⭐⭐⭐⭐      | std algorithm，簡潔    |

```cpp
// 手動迭代（競賽用）
if (pos + pattern.length() <= s.length()) {
    bool match = equal(pattern.begin(), pattern.end(), s.begin() + pos);
}
```

### ❌ 為何不用 `find()`？

```cpp
// ❌ 錯誤：用搜尋來驗證
if (s.find("bc") == 4) { ... }
```

**問題：**

- 從頭搜尋，浪費效能（已知位置卻搜尋）
- 語義不清（為何比較 4？）
- `find("bc", 4) == 4` 會誤判（"bc" 在位置 5 也返回 true）

**LeetCode 場景：**

```cpp
// KMP 字串匹配
if (text.compare(i, pattern.size(), pattern) == 0) { ... }

// 滑動窗口
if (s.compare(left, window_size, target) == 0) { count++; }

// 前綴檢查（C++17 前）
if (s.compare(0, prefix.size(), prefix) == 0) { ... }
```

---

## 修改操作設計

**常用介面：**

```cpp
s += str / s.append(str)   // 追加，均攤 O(1)
s.push_back(c)             // 追加字元，O(1)
s.insert(pos, str)         // 插入，O(n)
s.erase(pos, len)          // 刪除，O(n)
```

**設計重點：**

1. **效能分層**

   - 尾端操作：`append`, `push_back` 均攤 O(1)
   - 中間操作：`insert`, `erase` 需移動元素，O(n)

2. **參數一致性**
   - `erase(pos, len)` 與 `substr(pos, len)` 參數相同
   - 可「先 substr 確認，再 erase」

**LeetCode 關鍵：**

```cpp
// ❌ 避免：頻繁中間插入
for (...) { s.insert(0, "x"); }  // 每次 O(n)！

// ✅ 改用：尾端追加 + 反轉
string result;
for (...) { result.push_back('x'); }  // O(1)
reverse(result.begin(), result.end());
```

---

## 字元存取設計

**常用介面：**

```cpp
s[i]          // 無邊界檢查，UB
s.at(i)       // 有邊界檢查，拋異常
s.front()     // 第一個字元
s.back()      // 最後字元
```

**設計重點：**

| 特性     | `s[i]` | `s.at(i)`      |
| -------- | ------ | -------------- |
| 檢查     | ❌     | ✅             |
| 效能     | 快     | 慢（檢查開銷） |
| LeetCode | ✅     | 除錯用         |

**字元分類（`<cctype>`）：**

```cpp
isdigit(c)    // 是否數字
isxdigit(c)   // 是否十六進制數字 (0-9, A-F, a-f)
isalpha(c)    // 是否字母
isalnum(c)    // 是否字母或數字
tolower(c) / toupper(c)

// 字元轉數字
int digit = c - '0';  // '5' -> 5
```

---

## 轉換函數設計

### ⭐ `stoi` 家族（推薦）

**介面：**

```cpp
int n = stoi(str);                  // 十進制
int n = stoi(str, nullptr, base);   // 指定進制
string s = to_string(n);            // 數字轉字串
```

**優勢：**

- 錯誤檢測：拋 `invalid_argument`, `out_of_range`
- 支援進制：2-36 進制
- 類型安全

**範例：**

```cpp
int n = stoi("42");                  // 42
int bin = stoi("1010", nullptr, 2);  // 10 (二進制)
int hex = stoi("FF", nullptr, 16);   // 255 (十六進制)

// 錯誤處理
try {
    int n = stoi("abc");  // 拋 invalid_argument
} catch (...) { }
```

### ❌ `atoi`（避免使用）

```cpp
int n = atoi("42");   // 42
int n = atoi("abc");  // 0，無法區分錯誤！
```

**問題：** 無法區分「真的是 0」和「轉換失敗」

**對比：**

| 特性     | `stoi`  | `atoi`    |
| -------- | ------- | --------- |
| 錯誤檢測 | ✅ 異常 | ❌ 返回 0 |
| 進制支持 | ✅      | ❌        |
| LeetCode | ✅      | ❌        |

---

## API 設計原則總結

### LeetCode 場景快速指南

| 場景         | 推薦 API                 | 避免             | 原因                 |
| ------------ | ------------------------ | ---------------- | -------------------- |
| 檢查位置匹配 | `compare(pos, len, str)` | `substr() ==`    | O(1) vs O(n) 空間    |
| 搜尋子字串   | `find()`                 | -                | 返回位置或 npos      |
| 字元存取     | `s[i]`                   | `s.at(i)`        | 效能（題目保證範圍） |
| 字串轉數字   | `stoi`                   | `atoi`           | 錯誤處理             |
| 建構字串     | `push_back`              | `insert(0, ...)` | O(1) vs O(n)         |
| 反轉         | `reverse()`              | 手寫             | 標準算法優化         |

### 核心設計原則

1. **參數一致性：** `(position, length)` 貫穿所有函數
2. **零成本抽象：** `[]` 無檢查（快）vs `at()` 有檢查（安全）
3. **語義清晰：** 搜尋用 `find()`，驗證用 `compare()`
4. **效能分層：** 尾端操作 O(1)，中間操作 O(n)

### 常見錯誤

```cpp
// ❌ 錯誤模式
if (s.find("x")) { ... }              // 位置 0 會是 false！
if (s.substr(i, 3) == "abc") { ... }  // 每次創建新字串
int n = atoi(s.c_str());              // 無錯誤處理

// ✅ 正確模式
if (s.find("x") != string::npos) { ... }
if (s.compare(i, 3, "abc") == 0) { ... }
int n = stoi(s);  // 會拋異常
```

### C++ 演進

- **C++98:** 基礎（substr, find, append）
- **C++11:** 轉換（stoi, to_string）
- **C++17:** string_view（O(1) substr）
- **C++20:** starts_with, ends_with

---

## 延伸閱讀

- **分割技巧:** [`string_tokenization.md`](../Algorithm/String/string_tokenization.md)
- **容器比較:** [`vector_push_back_vs_emplace_back.md`](./vector_push_back_vs_emplace_back.md)
