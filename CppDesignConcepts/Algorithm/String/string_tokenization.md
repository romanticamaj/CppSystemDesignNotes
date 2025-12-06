# String Tokenization in C++

## Core Concept

將字串依特定分隔符號切割成多個 token。兩種主流做法：

1. **手動索引法**：使用雙指標 `start`/`end` 追蹤 token 邊界
2. **`std::getline` 法**：利用 `std::stringstream` + `std::getline` 自動切割

## Approach 1: Manual Indexing (雙指標法)

**心法**：`start` 標記 token 起點，`end` 掃描到分隔符號

```cpp
string path = "/home//user/../file";
path.push_back('/'); // 尾端補分隔符，簡化解析邏輯

int start = 1, end = 1;
while (end < path.size()) {
    if (path[end] != '/') {
        end++;
        continue;
    }

    string token = path.substr(start, end - start);
    // Process token...

    start = ++end; // 下一輪起點
}
```

### 不修改原字串的寫法

如果不想補分隔符號，改用條件判斷處理邊界：

```cpp
string path = "/home//user/../file";

int start = 1, end = 1;
while (end <= path.size()) {  // 改成 <=
    if (end == path.size() || path[end] == '/') {
        if (end > start) {  // 避免空 token
            string token = path.substr(start, end - start);
            // Process token...
        }
        start = ++end;
    } else {
        end++;
    }
}
```

**優點**：完全控制解析邏輯，適合複雜規則
**缺點**：手動管理索引，容易出錯

## Approach 2: `std::getline` with `std::stringstream` (LeetCode 推薦)

**心法**：將字串包裝成 stream，用 `getline` 自動按分隔符號讀取。這是 LeetCode 上處理字串切割最簡潔、最不易出錯的標準寫法。

```cpp
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> tokenize(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) { // 過濾空 token
            tokens.push_back(token);
        }
    }
    return tokens;
}

// Usage
std::string path = "/home//user/file";
auto tokens = tokenize(path, '/');
// tokens = ["home", "user", "file"]
```

**關鍵點**：

- `std::getline(stream, str, delimiter)` 讀取到分隔符號前的內容
- 連續分隔符號會產生空字串，需手動過濾
- 預設分隔符號是 `\n`，第三參數可自訂

## 🧠 getline 回傳值的布林語意

`std::getline(istream, string, delim)` 的回傳值是 `std::istream&`，可隱式轉型為 `bool`，代表是否成功讀取一行（或一段以 delimiter 分隔的字串）。

### ✅ 正確用法：顯式轉型為 bool

```cpp
std::stringstream ss("1.2.3");
std::string token;
while (static_cast<bool>(std::getline(ss, token, '.'))) {
    // token now holds "1", then "2", then "3"
}
```

或簡寫為：

```cpp
while (std::getline(ss, token, '.')) {
    // OK: std::istream 可隱式轉型為 bool
}
```

### ⚠️ 錯誤用法：直接賦值給 bool

```cpp
bool ret = std::getline(ss, token, '.'); // ❌ 錯誤：無法從 istream 直接轉型為 bool
```

這會導致編譯錯誤，因為 `std::istream` 的 `explicit operator bool()` 不能用於隱式轉型。

### ✅ 解法：使用 `auto` + `static_cast<bool>`

```cpp
auto ret = static_cast<bool>(std::getline(ss, token, '.'));
```

這種寫法在需要保留 `getline` 結果作為布林值時非常實用，例如：

```cpp
const int value = ret ? std::stoi(token) : 0;
```

### ⚠️ getline 不會清空 token

即使 `getline()` 失敗（例如 stream 已達 EOF），`token` 的內容也不會被清空，會保留上一次成功讀取的值：

```cpp
std::stringstream ss("1.2");
std::string token;
std::getline(ss, token, '.'); // token = "1"
std::getline(ss, token, '.'); // token = "2"
std::getline(ss, token, '.'); // 失敗，token 仍為 "2"
```

✅ 若要避免誤用，建議每次讀取前先清空：

```cpp
token.clear();
std::getline(ss, token, '.');
```

## 實務應用範例

### CSV Parsing

```cpp
std::stringstream ss("Alice,30,Engineer");
std::string name, age, job;

std::getline(ss, name, ',');  // "Alice"
std::getline(ss, age, ',');   // "30"
std::getline(ss, job, ',');   // "Engineer"
```

### Path Simplification

```cpp
std::stringstream ss("/a/./b/../c/");
std::stack<std::string> dirs;
std::string token;

while (std::getline(ss, token, '/')) {
    if (token == "..") {
        if (!dirs.empty()) dirs.pop();
    } else if (!token.empty() && token != ".") {
        dirs.push(token);
    }
}
```

## Design Mindset

| 場景                     | 推薦方法                               |
| ------------------------ | -------------------------------------- |
| 簡單切割（固定分隔符號） | `std::getline`                         |
| 需要索引位置或複雜規則   | 手動索引法                             |
| 高效能要求               | `std::string_view` + 手動索引          |
| 處理多行輸入             | `std::getline(cin, line)` 搭配內層解析 |

## Complexity

- **Time**: O(n)，n 為字串長度
- **Space**: O(k)，k 為 token 數量（儲存結果）
