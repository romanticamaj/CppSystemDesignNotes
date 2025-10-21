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

## Approach 2: `std::getline` with `std::stringstream`

**心法**：將字串包裝成 stream，用 `getline` 自動按分隔符號讀取

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
