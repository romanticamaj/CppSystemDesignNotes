# C++11 std::regex

## 核心概念

`std::regex` 是 C++11 引入的正則表達式庫，提供了強大的字符串模式匹配和搜索功能。這個庫使 C++ 終於擁有了內建的正則表達式支持，不再需要依賴第三方庫（如 Boost.Regex）來處理文本模式匹配。

正則表達式是一種用於描述字符串模式的強大工具，可用於：

1. **模式匹配**：檢查字符串是否符合特定模式
2. **搜索**：在文本中查找符合模式的部分
3. **替換**：將符合模式的文本替換為其他內容
4. **分割**：根據模式將文本分割成多個部分

C++11 的正則表達式庫支持多種正則表達式語法（如 ECMAScript、basic、extended、awk、grep 和 egrep），默認使用 ECMAScript 語法，這與 JavaScript 中的正則表達式語法相似。

## 語法

### 基本用法

```cpp
// 創建正則表達式對象
std::regex pattern("pattern", std::regex_constants::ECMAScript);

// 匹配整個字符串
bool match = std::regex_match(str, pattern);

// 搜索字符串中的模式
bool search = std::regex_search(str, pattern);

// 替換匹配的內容
std::string result = std::regex_replace(str, pattern, "replacement");

// 使用匹配結果
std::smatch matches;
if (std::regex_search(str, matches, pattern)) {
    // 處理匹配結果
}
```

### 正則表達式對象

```cpp
// 基本構造
std::regex r1("pattern");

// 指定語法
std::regex r2("pattern", std::regex_constants::extended);

// 指定選項
std::regex r3("pattern", std::regex_constants::ECMAScript | std::regex_constants::icase);

// 從字符串構造
std::string pattern = "\\d+";
std::regex r4(pattern);
```

### 匹配結果

```cpp
// std::smatch 用於 std::string
std::smatch matches;

// std::cmatch 用於 C 風格字符串
std::cmatch cmatches;

// 獲取匹配結果
if (std::regex_search(str, matches, pattern)) {
    std::cout << "匹配數量: " << matches.size() << std::endl;
    std::cout << "完整匹配: " << matches[0].str() << std::endl;
    
    // 遍歷所有捕獲組
    for (size_t i = 1; i < matches.size(); ++i) {
        std::cout << "捕獲組 " << i << ": " << matches[i].str() << std::endl;
    }
}
```

### 迭代器

```cpp
// 使用正則表達式迭代器遍歷所有匹配
std::string text = "apple orange banana";
std::regex word_regex("\\w+");

auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
auto words_end = std::sregex_iterator();

for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    std::cout << match.str() << std::endl;
}
```

## 與相似概念的比較

| 特性 | std::regex | POSIX regex.h | Boost.Regex | 手動字符串處理 |
|------|------------|--------------|-------------|--------------|
| 語法複雜度 | 中等 | 低 | 高 | 低 |
| 表達能力 | 高 | 中 | 高 | 低 |
| 性能 | 中等 | 高 | 高 | 視實現而定 |
| 易用性 | 良好 | 較差 | 良好 | 較差 |
| 類型安全 | 高 | 低 | 高 | 高 |
| 捕獲組支持 | 完整 | 有限 | 完整 | 需手動實現 |
| Unicode 支持 | 有限 | 有限 | 良好 | 需手動實現 |
| 標準庫一部分 | 是 (C++11) | 是 (POSIX) | 否 | 是 |
| 跨平台一致性 | 高 | 中 | 高 | 高 |

## 使用範例

### 基本模式匹配

```cpp
#include <iostream>
#include <regex>
#include <string>

int main() {
    // 檢查字符串是否完全匹配模式
    std::string email = "user@example.com";
    std::regex email_pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    if (std::regex_match(email, email_pattern)) {
        std::cout << email << " 是有效的電子郵件地址" << std::endl;
    } else {
        std::cout << email << " 不是有效的電子郵件地址" << std::endl;
    }
    
    // 檢查字符串是否包含匹配模式的部分
    std::string text = "我的電子郵件是 user@example.com，請聯繫我";
    if (std::regex_search(text, email_pattern)) {
        std::cout << "文本中包含電子郵件地址" << std::endl;
    } else {
        std::cout << "文本中不包含電子郵件地址" << std::endl;
    }
    
    return 0;
}
```

### 捕獲組與提取信息

```cpp
#include <iostream>
#include <regex>
#include <string>

int main() {
    std::string html = "<div><h1>標題</h1><p>段落內容</p></div>";
    std::regex tag_pattern("<([a-z0-9]+)>([^<]*)</\\1>");
    
    std::smatch matches;
    std::string::const_iterator search_start(html.cbegin());
    
    while (std::regex_search(search_start, html.cend(), matches, tag_pattern)) {
        std::cout << "找到標籤: " << matches[1].str() << std::endl;
        std::cout << "內容: " << matches[2].str() << std::endl;
        std::cout << "完整匹配: " << matches[0].str() << std::endl;
        std::cout << std::endl;
        
        // 更新搜索位置
        search_start = matches.suffix().first;
    }
    
    // 解析更複雜的字符串
    std::string log_entry = "2023-05-15 14:30:45 [INFO] User 'admin' logged in from 192.168.1.100";
    std::regex log_pattern("(\\d{4}-\\d{2}-\\d{2}) (\\d{2}:\\d{2}:\\d{2}) \\[(\\w+)\\] (.*)");
    
    if (std::regex_match(log_entry, matches, log_pattern)) {
        std::cout << "日期: " << matches[1].str() << std::endl;
        std::cout << "時間: " << matches[2].str() << std::endl;
        std::cout << "級別: " << matches[3].str() << std::endl;
        std::cout << "消息: " << matches[4].str() << std::endl;
    }
    
    return 0;
}
```

### 字符串替換

```cpp
#include <iostream>
#include <regex>
#include <string>

int main() {
    // 基本替換
    std::string text = "The quick brown fox jumps over the lazy dog";
    std::regex word_pattern("\\b(\\w)\\w*\\b");
    
    // 將每個單詞替換為其首字母加點
    std::string abbreviation = std::regex_replace(text, word_pattern, "$1.");
    std::cout << "縮寫: " << abbreviation << std::endl;
    
    // 使用格式標誌
    std::string html = "<p>段落 1</p><p>段落 2</p>";
    std::regex p_tag("<p>(.*?)</p>");
    
    // 將 HTML 段落標籤替換為 Markdown 格式
    std::string markdown = std::regex_replace(html, p_tag, "# $1");
    std::cout << "Markdown: " << markdown << std::endl;
    
    // 只替換第一次出現
    std::string first_only = std::regex_replace(html, p_tag, "# $1", 
                                              std::regex_constants::format_first_only);
    std::cout << "只替換第一個: " << first_only << std::endl;
    
    // 使用回調函數進行複雜替換（C++11 不直接支持，需要模擬）
    std::string numbers = "1 2 3 4 5";
    std::regex number_pattern("\\d+");
    std::string result;
    
    std::string::const_iterator search_start(numbers.cbegin());
    std::smatch matches;
    size_t last_pos = 0;
    
    while (std::regex_search(search_start, numbers.cend(), matches, number_pattern)) {
        // 添加匹配前的內容
        result += matches.prefix().str();
        
        // 獲取匹配的數字並進行計算
        int num = std::stoi(matches[0].str());
        result += std::to_string(num * num);  // 將數字替換為其平方
        
        // 更新搜索位置
        search_start = matches.suffix().first;
    }
    
    // 添加最後一個匹配後的內容
    result += std::string(search_start, numbers.cend());
    
    std::cout << "數字平方: " << result << std::endl;
    
    return 0;
}
```

### 使用正則表達式迭代器

```cpp
#include <iostream>
#include <regex>
#include <string>
#include <vector>

int main() {
    std::string text = "IPv4 地址: 192.168.1.1 和 10.0.0.1，IPv6 地址: 2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    std::regex ipv4_pattern("\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b");
    
    // 使用迭代器查找所有 IPv4 地址
    std::vector<std::string> ipv4_addresses;
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), ipv4_pattern);
    auto words_end = std::sregex_iterator();
    
    std::cout << "找到 " << std::distance(words_begin, words_end) << " 個 IPv4 地址:" << std::endl;
    
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string ip = match.str();
        ipv4_addresses.push_back(ip);
        std::cout << "  " << ip << std::endl;
    }
    
    // 使用 token_iterator 分割字符串
    std::string csv = "apple,orange,banana,\"grape,kiwi\",melon";
    std::regex csv_pattern(",(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)");  // 匹配不在引號內的逗號
    
    std::cout << "\n解析 CSV 字段:" << std::endl;
    
    std::sregex_token_iterator fields(csv.begin(), csv.end(), csv_pattern, -1);
    std::sregex_token_iterator end;
    
    while (fields != end) {
        std::cout << "  " << *fields++ << std::endl;
    }
    
    return 0;
}
```

### 正則表達式異常處理

```cpp
#include <iostream>
#include <regex>
#include <string>

int main() {
    try {
        // 嘗試編譯一個無效的正則表達式
        std::regex bad_pattern("(unclosed parenthesis");
    } catch (const std::regex_error& e) {
        std::cout << "正則表達式錯誤: " << e.what() << std::endl;
        std::cout << "錯誤代碼: " << e.code() << std::endl;
        
        // 解釋錯誤代碼
        switch (e.code()) {
            case std::regex_constants::error_collate:
                std::cout << "無效的排序元素引用" << std::endl;
                break;
            case std::regex_constants::error_ctype:
                std::cout << "無效的字符類" << std::endl;
                break;
            case std::regex_constants::error_escape:
                std::cout << "無效的轉義序列" << std::endl;
                break;
            case std::regex_constants::error_backref:
                std::cout << "無效的反向引用" << std::endl;
                break;
            case std::regex_constants::error_brack:
                std::cout << "不匹配的方括號" << std::endl;
                break;
            case std::regex_constants::error_paren:
                std::cout << "不匹配的括號" << std::endl;
                break;
            case std::regex_constants::error_brace:
                std::cout << "不匹配的大括號" << std::endl;
                break;
            case std::regex_constants::error_badbrace:
                std::cout << "大括號中的範圍無效" << std::endl;
                break;
            case std::regex_constants::error_range:
                std::cout << "無效的字符範圍" << std::endl;
                break;
            case std::regex_constants::error_space:
                std::cout << "內存不足" << std::endl;
                break;
            case std::regex_constants::error_badrepeat:
                std::cout << "重複符號（*、+、? 或 {}）沒有有效的前導表達式" << std::endl;
                break;
            case std::regex_constants::error_complexity:
                std::cout << "匹配結果過於複雜" << std::endl;
                break;
            case std::regex_constants::error_stack:
                std::cout << "棧空間不足" << std::endl;
                break;
            default:
                std::cout << "未知錯誤" << std::endl;
                break;
        }
    }
    
    // 安全地使用正則表達式
    std::string pattern_str = "\\d+";
    try {
        std::regex pattern(pattern_str);
        std::string text = "abc123def456";
        
        std::smatch matches;
        if (std::regex_search(text, matches, pattern)) {
            std::cout << "找到數字: " << matches[0].str() << std::endl;
        }
    } catch (const std::regex_error& e) {
        std::cout << "處理模式 '" << pattern_str << "' 時出錯: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## 使用情境

### 適合使用 std::regex 的情況

1. **表單驗證**：驗證用戶輸入是否符合特定格式
   ```cpp
   bool validate_email(const std::string& email) {
       std::regex email_pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
       return std::regex_match(email, email_pattern);
   }
   
   bool validate_phone(const std::string& phone) {
       std::regex phone_pattern("^\\+?[0-9]{1,3}[- ]?[0-9]{3,4}[- ]?[0-9]{3,4}[- ]?[0-9]{3,4}$");
       return std::regex_match(phone, phone_pattern);
   }
   ```

2. **文本解析**：從結構化文本中提取信息
   ```cpp
   std::map<std::string, std::string> parse_config_file(const std::string& content) {
       std::map<std::string, std::string> config;
       std::regex line_pattern("^\\s*(\\w+)\\s*=\\s*(.*)\\s*$");
       
       std::istringstream iss(content);
       std::string line;
       
       while (std::getline(iss, line)) {
           std::smatch matches;
           if (std::regex_match(line, matches, line_pattern)) {
               config[matches[1].str()] = matches[2].str();
           }
       }
       
       return config;
   }
   ```

3. **URL 解析**：分解 URL 成各個組件
   ```cpp
   struct URL {
       std::string protocol;
       std::string host;
       std::string path;
       std::string query;
       std::string fragment;
   };
   
   URL parse_url(const std::string& url_str) {
       URL result;
       std::regex url_pattern("^(https?|ftp)://([^/\\s]+)(/[^\\s?#]*)?(\\?[^\\s#]*)?(#.*)?$");
       
       std::smatch matches;
       if (std::regex_match(url_str, matches, url_pattern)) {
           result.protocol = matches[1].str();
           result.host = matches[2].str();
           result.path = matches[3].str();
           result.query = matches[4].str();
           result.fragment = matches[5].str();
       }
       
       return result;
   }
   ```

4. **文本替換**：格式化或清理文本
   ```cpp
   std::string sanitize_html(const std::string& html) {
       std::regex tag_pattern("<[^>]*>");
       return std::regex_replace(html, tag_pattern, "");
   }
   
   std::string format_phone_number(const std::string& phone) {
       std::regex digits_pattern("\\d+");
       std::string digits;
       
       auto digits_begin = std::sregex_iterator(phone.begin(), phone.end(), digits_pattern);
       auto digits_end = std::sregex_iterator();
       
       for (auto it = digits_begin; it != digits_end; ++it) {
           digits += it->str();
       }
       
       if (digits.length() == 10) {
           return "(" + digits.substr(0, 3) + ") " + digits.substr(3, 3) + "-" + digits.substr(6);
       }
       
       return phone;  // 如果不是 10 位數字，則返回原始字符串
   }
   ```

5. **日誌分析**：解析和過濾日誌文件
   ```cpp
   std::vector<LogEntry> parse_log_file(const std::string& log_content) {
       std::vector<LogEntry> entries;
       std::regex log_pattern("(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}) \\[(\\w+)\\] (.*)");
       
       std::istringstream iss(log_content);
       std::string line;
       
       while (std::getline(iss, line)) {
           std::smatch matches;
           if (std::regex_match(line, matches, log_pattern)) {
               LogEntry entry;
               entry.timestamp = matches[1].str();
               entry.level = matches[2].str();
               entry.message = matches[3].str();
               entries.push_back(entry);
           }
       }
       
       return entries;
   }
   ```

### 不適合使用 std::regex 的情況

1. **簡單的字符串操作**：當只需要簡單的字符串操作時，使用標準字符串函數更高效
   ```cpp
   // 不需要正則表達式
   bool contains_substring(const std::string& str, const std::string& substr) {
       return str.find(substr) != std::string::npos;
   }
   
   // 不需要正則表達式
   std::string to_uppercase(std::string str) {
       std::transform(str.begin(), str.end(), str.begin(), ::toupper);
       return str;
   }
   ```

2. **性能關鍵的代碼**：正則表達式處理可能比專門的算法慢
   ```cpp
   // 對於大量文本處理，可能需要更高效的算法
   size_t count_words_regex(const std::string& text) {
       std::regex word_pattern("\\b\\w+\\b");
       return std::distance(
           std::sregex_iterator(text.begin(), text.end(), word_pattern),
           std::sregex_iterator()
       );
   }
   
   // 更高效的方法
   size_t count_words_manual(const std::string& text) {
       size_t count = 0;
       bool in_word = false;
       
       for (char c : text) {
           bool is_word_char = std::isalnum(c) || c == '_';
           
           if (is_word_char && !in_word) {
               in_word = true;
               count++;
           } else if (!is_word_char) {
               in_word = false;
           }
       }
       
       return count;
   }
   ```

3. **極其複雜的文本處理**：對於需要上下文敏感的語法分析，應使用專門的解析器
   ```cpp
   // 正則表達式不適合解析嵌套結構
   // 例如，解析 JSON 或 XML 應該使用專門的解析器
   
   // 不要這樣做：
   std::regex json_pattern("\\{([^{}]*)\\}");  // 這無法正確處理嵌套的大括號
   
   // 而應該使用專門的 JSON 解析庫
   ```

4. **大規模文本搜索**：對於大文件或需要高性能的搜索，考慮使用專門的搜索算法
   ```cpp
   // 對於大文件中的模式搜索，考慮使用 Boyer-Moore 或 Aho-Corasick 算法
   // 而不是正則表達式
   ```

### 最佳實踐

1. **預編譯正則表達式**：重複使用同一個正則表達式對象，而不是每次都創建新的
   ```cpp
   // 不好的做法：每次調用都創建新的正則表達式
   bool validate_email_bad(const std::string& email) {
       std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
       return std::regex_match(email, pattern);
   }
   
   // 好的做法：使用靜態正則表達式
   bool validate_email_good(const std::string& email) {
       static const std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
       return std::regex_match(email, pattern);
   }
   ```

2. **使用適當的正則表達式語法**：根據需求選擇合適的語法
   ```cpp
   // 使用 ECMAScript 語法（默認）
   std::regex pattern1("\\d+", std::regex_constants::ECMAScript);
   
   // 使用 basic 語法（類似 grep）
   std::regex pattern2("\\([0-9]\\+\\)", std::regex_constants::basic);
   
   // 使用 extended 語法（類似 egrep）
   std::regex pattern3("\\([0-9]+\\)", std::regex_constants::extended);
   ```

3. **處理正則表達式異常**：捕獲並適當處理可能的異常
   ```cpp
   std::regex pattern;
   try {
       pattern = std::regex(user_input);
   } catch (const std::regex_error& e) {
       std::cerr << "無效的正則表達式: " << e.what() << std::endl;
       // 處理錯誤...
   }
   ```

4. **避免過於複雜的正則表達式**：將複雜的模式分解為多個簡單的模式
   ```cpp
   // 不好的做法：一個非常複雜的正則表達式
   std::regex complex_pattern("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
   
   // 好的做法：分解為更簡單的步驟
   bool is_valid_ipv4(const std::string& ip) {
       std::regex octet_pattern("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
       
       std::vector<std::string> octets;
       std::istringstream iss(ip);
       std::string octet;
       
       while (std::getline(iss, octet, '.')) {
           octets.push_back(octet);
       }
       
       if (octets.size() != 4) {
           return false;
       }
       
       for (const auto& o : octets) {
           if (!std::regex_match(o, octet_pattern)) {
               return false;
           }
       }
       
       return true;
   }
   ```

5. **使用命名捕獲組**：提高代碼可讀性（C++11 不直接支持，但可以通過索引和註釋模擬）
   ```cpp
   // 使用註釋來標記捕獲組的含義
   std::regex date_pattern("(\\d{4})-(\\d{2})-(\\d{2})");  // 年-月-日
   
   std::smatch matches;
   if (std::regex_match(date_str, matches, date_pattern)) {
       int year = std::stoi(matches[1].str());   // 第 1 組：年
       int month = std::stoi(matches[2].str());  // 第 2 組：月
       int day = std::stoi(matches[3].str());    // 第 3 組：日
   }
   ```

## 總結

C++11 的 `std::regex` 庫為 C++ 程序員提供了強大的正則表達式處理能力，使文本模式匹配和處理變得更加簡單和標準化。它支持多種正則表達式語法，提供了豐富的功能，包括模式匹配、搜索、替換和迭代。

雖然 `std::regex` 在某些實現中可能存在性能問題，特別是與專門的字符串處理算法相比，但它的類型安全、標準化和豐富的功能使其成為處理文本模式的強大工具。對於大多數應用程序來說，`std::regex` 提供了良好的平衡，結合了易用性和功能性。

在使用 `std::regex` 時，應該注意預編譯正則表達式以提高性能，處理可能的異常，並避免過於複雜的模式。對於簡單的字符串操作，標準字符串函數通常更高效；而對於極其複雜的文本處理，可能需要考慮專門的解析器。

總的來說，`std::regex` 是 C++11 中一個重要的新增功能，它使 C++ 在文本處理能力上更加完整和強大，為開發者提供了一個標準化的工具來處理各種文本模式匹配和處理任務。