# Unicode 字元與容器使用指南

## 1. 核心術語定義

| 術語                          | 英文                  | 定義                                       | 範例                     |
| :---------------------------- | :-------------------- | :----------------------------------------- | :----------------------- |
| **字元 (Character)**          | Character             | 人類可讀的最小文字單位                     | `'A'`, `'中'`, `'😀'`    |
| **碼點 (Code Point)**         | Code Point            | Unicode 分配給字元的唯一編號               | U+4E2D (中), U+1F600 (😀) |
| **碼元 (Code Unit)**          | Code Unit             | 編碼方案的基本儲存單位                     | UTF-8: 1 byte, UTF-32: 4 bytes |
| **字節 (Byte)**               | Byte                  | 8 bits，實際儲存單位                       | `0x41` (字母 A)          |
| **字詞 (Word/Token)**         | Word/Token            | 多個字元組成的語義單位                     | "apple", "中文"          |

---

## 2. ASCII 與 UTF 編碼基礎

### 2.1 編碼方案比較

| 編碼   | 碼元大小 | 字元表示                      | ASCII | 中文  | Emoji | 優缺點                         |
| :----- | :------- | :---------------------------- | :---- | :---- | :---- | :----------------------------- |
| UTF-32 | 4 bytes  | 固定長度（1 碼元 = 1 碼點）   | 4B    | 4B    | 4B    | ✅ 直接索引 ❌ 空間浪費        |
| UTF-16 | 2 bytes  | 可變長度（1-2 碼元）          | 2B    | 2B    | 4B    | ⚠️ BMP 直接，補充平面需代理對  |
| UTF-8  | 1 byte   | 可變長度（1-4 碼元）          | 1B    | 3B    | 4B    | ✅ 空間效率高 ❌ 索引需解析    |

```text
範例：'中' (U+4E2D) 的儲存
UTF-32: 00 00 4E 2D (4 bytes)
UTF-16: 4E 2D (2 bytes)
UTF-8:  E4 B8 AD (3 bytes)
```

### 2.2 C++ 字元類型

```cpp
char        // 1 byte - 儲存**字節**，非字元（常見誤解）
char8_t     // 1 byte - C++20，明確 UTF-8 碼元
char16_t    // 2 bytes - UTF-16 碼元
char32_t    // 4 bytes - UTF-32 碼元，直接對應 Unicode 碼點
wchar_t     // 平台相關 - **避免使用**

// ✅ 正確
char32_t ch = U'中';    // 1 個完整字元

// ❌ 錯誤
char c = '中';          // 編譯錯誤！3 bytes 無法放入 1 byte
```

---

## 3. Container 使用方式

### 3.1 `vector<char>` vs `vector<char32_t>`

| 類型                    | 儲存單位 | 索引意義      | 長度意義  | 適用場景           |
| :---------------------- | :------- | :------------ | :-------- | :----------------- |
| `vector<char>`          | 字節     | 第 i 個字節   | 字節數    | I/O、網路、UTF-8   |
| `vector<char32_t>`      | 字元     | 第 i 個字元   | 字元數    | 字元級操作、統計   |

```cpp
// UTF-8 字節序列
std::vector<char> utf8 = {'H', 'i', 0xE4, 0xB8, 0xAD};  // "Hi中"
utf8.size();  // 5 (bytes)

// UTF-32 字元序列
std::vector<char32_t> chars = {U'H', U'i', U'中'};
chars.size();  // 3 (characters)
chars[2];      // U'中' (第 3 個字元)
```

### 3.2 字元級容器範例

```cpp
// 字元頻率統計
std::unordered_map<char32_t, int> freq;
freq[U'中'] = 5;
freq[U'😀'] = 2;

// 字元集合
std::unordered_set<char32_t> allowed = {U'a', U'中', U'文'};
```

---

## 4. `std::string` 類型與使用

### 4.1 String 類型對應

```cpp
std::string         // UTF-8 字節序列（最常用）
std::u8string       // C++20 UTF-8 專用
std::u16string      // UTF-16 字串
std::u32string      // UTF-32 字串（字元級操作）
std::wstring        // 平台相關（避免）
```

### 4.2 `std::string` (UTF-8) 關鍵理解

**核心**：儲存**字節序列**，非字元序列

```cpp
std::string text = "你好";

// ✅ 正確理解
text.length();    // 6 (字節數，非字元數)
text[0];          // 0xE4 (第 1 個字節，非完整字元)

// ❌ 錯誤理解
// text.length() ≠ 字元數
// text[i] ≠ 第 i 個字元
```

**字元級操作需第三方庫**：

```cpp
// 使用 utf8cpp
#include <utf8.h>
size_t num_chars = utf8::distance(text.begin(), text.end());  // 2 字元
```

### 4.3 Container 使用範例

```cpp
// 字詞存儲（UTF-8 字串）
std::vector<std::string> words = {"apple", "中文", "😀"};
std::unordered_set<std::string> dict = {"cat", "dog"};
std::map<std::string, int> word_count = {{"hello", 5}};
```

---

## 5. `std::u32string` 深度使用

### 5.1 核心優勢

**字元級直接存取**（1 索引 = 1 字元）

```cpp
std::u32string text = U"Hello 中文 😀";

text.length();    // 10 (字元數)
text[0];          // U'H' (第 1 個字元)
text[6];          // U'中' (第 7 個字元)
```

### 5.2 字元操作

```cpp
// 字元統計
std::unordered_map<char32_t, int> freq;
for (char32_t ch : text) { freq[ch]++; }

// 字元查找
size_t pos = text.find(U'中');  // 6

// 子字串（字元級）
text.substr(6, 2);  // U"中文"
```

### 5.3 限制

```cpp
// ❌ 無法直接 I/O
std::ofstream file("out.txt");
// file << u32_text;  // 編譯錯誤

// ❌ 記憶體佔用大
std::string utf8 = "你好";      // 6 bytes
std::u32string utf32 = U"你好"; // 8 bytes (2 × 4)

// ✅ 需轉換為 UTF-8
std::string utf8 = convert_to_utf8(utf32);  // 使用庫
```

---

## 6. 關鍵存取方式總結

### 6.1 字元 vs 字節存取

| 類型                    | 存取單位 | 索引                   | 長度          |
| :---------------------- | :------- | :--------------------- | :------------ |
| `std::u32string`        | 字元     | `[i]` = 第 i 個字元    | 字元數        |
| `std::vector<char32_t>` | 字元     | `[i]` = 第 i 個字元    | 字元數        |
| `std::string` (UTF-8)   | 字節     | `[i]` = 第 i 個字節    | 字節數        |
| `std::vector<char>`     | 字節     | `[i]` = 第 i 個字節    | 字節數        |

### 6.2 迭代差異

```cpp
// UTF-8: 迭代字節
std::string utf8 = "Hi中";
for (char byte : utf8) { }  // 5 次迭代 (5 bytes)

// UTF-32: 迭代字元
std::u32string utf32 = U"Hi中";
for (char32_t ch : utf32) { }  // 3 次迭代 (3 字元)
```

### 6.3 字詞 (Word) 存取

**字詞 ≠ 字元**，字詞是多個字元組成

```cpp
std::vector<std::string> words = {"apple", "中文"};
words[1];  // "中文" (整個字詞)
words[1].size();  // 6 bytes (非字元數)
```

---

## 7. 實務選擇指南

| 場景                   | 推薦方案                        | 理由                   |
| :--------------------- | :------------------------------ | :--------------------- |
| 字元頻率統計           | `unordered_map<char32_t, int>`  | 字元級操作             |
| 單詞/詞彙存儲          | `unordered_set<string>` (UTF-8) | 處理詞非字元，節省空間 |
| 需索引第 n 個字元      | `u32string` / `vector<char32_t>` | 固定大小直接索引       |
| 文件 I/O / 網路傳輸    | `string` (UTF-8)                | 標準格式，空間效率高   |
| 字元級算法（排序）     | `u32string` / `vector<char32_t>` | 每個元素是完整字元     |

**性能權衡**：

```cpp
// "你好世界" (4 字元)
std::string utf8 = "你好世界";      // 12 bytes, O(n) 索引字元
std::u32string utf32 = U"你好世界"; // 16 bytes, O(1) 索引字元
```

---

## 8. 常見陷阱

### 陷阱 1：混淆字節與字元

```cpp
// ❌ 錯誤
std::string text = "你好";
if (text.length() == 2) { }  // false! (6 bytes)

// ✅ 正確
std::u32string text = U"你好";
if (text.length() == 2) { }  // true! (2 字元)
```

### 陷阱 2：用 `char` 存 Unicode

```cpp
// ❌ 編譯錯誤
char c = '中';

// ✅ 正確
char32_t c = U'中';
```

### 陷阱 3：假設索引返回字元

```cpp
std::string utf8 = "中文";
utf8[0];  // 0xE4 (第 1 個字節，非 '中')

std::u32string utf32 = U"中文";
utf32[0];  // U'中' (第 1 個字元)
```

### 陷阱 4：平台相關 `wchar_t`

```cpp
// ❌ 避免
std::wstring ws = L"text";  // Windows: 2B, Linux: 4B

// ✅ 明確
std::u32string u32 = U"text";  // 明確 4 bytes
```

---

## 9. 編碼轉換

### 標準庫（已棄用）

```cpp
// ⚠️ C++17 已棄用，不建議使用
#include <codecvt>
```

### 推薦第三方庫

```cpp
// 1. utf8cpp (header-only)
#include <utf8.h>
std::u32string utf32;
utf8::utf8to32(utf8_str.begin(), utf8_str.end(), back_inserter(utf32));

// 2. ICU - 功能最完整
// 3. Boost.Locale
#include <boost/locale.hpp>
auto utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8_str);
```

---

## 10. 快速決策

### 決策樹

```text
需要處理 Unicode？
├─ 是 → 操作單位是？
│   ├─ 字元 → char32_t / u32string
│   └─ 字詞 → string (UTF-8)
└─ 否 (僅 ASCII) → char / string
```

### 核心原則

1. **處理字元**？→ `char32_t` / `u32string`
2. **處理字詞/文本**？→ `string` (UTF-8)
3. **需字元級索引**？→ `u32string`
4. **需 I/O / 網路**？→ `string` (UTF-8)
5. **避免**：`char` 存 Unicode、`wchar_t`

### 完整範例

```cpp
// 字元頻率統計
std::u32string text = U"Hello 世界 😀😀";
std::unordered_map<char32_t, int> freq;
for (char32_t ch : text) {
    if (ch != U' ') freq[ch]++;
}

// 詞彙表去重
std::vector<std::string> words = {"apple", "apple", "中文", "中文"};
std::unordered_set<std::string> unique(words.begin(), words.end());
```
