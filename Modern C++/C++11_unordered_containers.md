# C++11 無序容器 (Unordered Containers)

## 核心概念

C++11 引入了四種基於雜湊表的無序容器，它們是標準庫的重要擴充：

1. **std::unordered_map**：存儲鍵值對的無序集合，每個鍵唯一
2. **std::unordered_multimap**：允許重複鍵的無序鍵值對集合
3. **std::unordered_set**：存儲唯一元素的無序集合
4. **std::unordered_multiset**：允許重複元素的無序集合

這些容器使用雜湊函數將元素映射到桶（buckets）中，從而實現近乎常數時間 O(1) 的平均查找、插入和刪除操作。相比於有序容器（如 `std::map` 和 `std::set`），無序容器在不需要元素排序的場景下通常能提供更好的性能。

無序容器的核心特點：

- **基於雜湊表**：使用雜湊函數和桶來組織數據
- **無序性**：元素沒有特定的順序
- **快速查找**：平均 O(1) 時間複雜度的查找操作
- **自定義雜湊**：支持自定義類型的雜湊函數和相等性比較

## 語法

### 基本聲明與初始化

```cpp
// 無序映射 (unordered_map)
std::unordered_map<std::string, int> word_count;  // 空容器
std::unordered_map<std::string, int> word_count = {{"hello", 1}, {"world", 2}};  // 使用初始化列表

// 無序多重映射 (unordered_multimap)
std::unordered_multimap<std::string, std::string> authors;
authors.insert({"Tolkien", "The Lord of the Rings"});
authors.insert({"Tolkien", "The Hobbit"});  // 允許重複鍵

// 無序集合 (unordered_set)
std::unordered_set<int> numbers = {1, 2, 3, 4, 5};

// 無序多重集合 (unordered_multiset)
std::unordered_multiset<int> multi_numbers = {1, 2, 2, 3, 3, 3};  // 允許重複元素
```

### 常用操作

```cpp
// 插入元素
word_count.insert({"code", 3});
word_count.emplace("program", 5);  // 就地構造，避免臨時對象
word_count["algorithm"] = 7;  // 只適用於 unordered_map，不適用於 unordered_multimap

// 查找元素
auto it = word_count.find("code");
if (it != word_count.end()) {
    std::cout << it->first << ": " << it->second << std::endl;
}

// 檢查元素是否存在
bool exists = word_count.count("program") > 0;

// 訪問元素 (只適用於 unordered_map)
int count = word_count["code"];  // 如果鍵不存在，會插入一個默認構造的值
int count_safe = word_count.at("code");  // 如果鍵不存在，拋出 std::out_of_range 異常

// 刪除元素
word_count.erase("code");  // 通過鍵刪除
word_count.erase(it);  // 通過迭代器刪除

// 遍歷容器
for (const auto& pair : word_count) {
    std::cout << pair.first << ": " << pair.second << std::endl;
}

// 獲取桶信息
size_t bucket_count = word_count.bucket_count();  // 桶的數量
size_t bucket_size = word_count.bucket_size(0);  // 第一個桶中的元素數量
float load_factor = word_count.load_factor();  // 負載因子 (元素數量/桶數量)

// 重新雜湊
word_count.rehash(20);  // 設置桶數為至少 20
word_count.reserve(15);  // 預留空間給至少 15 個元素
```

### 自定義類型的雜湊

```cpp
// 為自定義類型定義雜湊函數和相等性比較
struct Person {
    std::string name;
    int age;
    
    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};

// 方法 1：特化 std::hash 模板
namespace std {
    template<>
    struct hash<Person> {
        size_t operator()(const Person& p) const {
            return hash<string>()(p.name) ^ hash<int>()(p.age);
        }
    };
}

// 使用自定義類型
std::unordered_set<Person> people;
people.insert({"Alice", 30});

// 方法 2：提供自定義雜湊函數和相等性比較函數
struct PersonHash {
    size_t operator()(const Person& p) const {
        return std::hash<std::string>()(p.name) ^ std::hash<int>()(p.age);
    }
};

struct PersonEqual {
    bool operator()(const Person& p1, const Person& p2) const {
        return p1.name == p2.name && p1.age == p2.age;
    }
};

std::unordered_set<Person, PersonHash, PersonEqual> people2;
```

## 與相似概念的比較

| 特性 | 無序容器 (unordered_*) | 有序容器 (map/set) | 數組/向量 (array/vector) | 雜湊表 (其他實現) |
|------|----------------------|-------------------|------------------------|-----------------|
| 元素順序 | 無序 | 按鍵排序 | 按插入順序 | 通常無序 |
| 查找複雜度 | 平均 O(1)，最壞 O(n) | O(log n) | O(n) | 平均 O(1)，最壞視實現而定 |
| 插入複雜度 | 平均 O(1)，最壞 O(n) | O(log n) | 末尾 O(1)，其他 O(n) | 平均 O(1)，最壞視實現而定 |
| 刪除複雜度 | 平均 O(1)，最壞 O(n) | O(log n) | 末尾 O(1)，其他 O(n) | 平均 O(1)，最壞視實現而定 |
| 內存開銷 | 較高 | 中等 | 低 | 視實現而定 |
| 迭代器失效 | 僅當重新雜湊或刪除元素時 | 僅當刪除元素時 | 插入或刪除時可能全部失效 | 視實現而定 |
| 適用場景 | 快速查找，不需排序 | 需要有序遍歷 | 連續存儲，隨機訪問 | 特定需求的自定義實現 |

## 使用範例

### 基本用法：單詞計數

```cpp
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>

int main() {
    // 輸入文本
    std::string text = "this is a sample text with some words repeated words this is a test";
    std::istringstream iss(text);
    
    // 使用 unordered_map 計數單詞出現頻率
    std::unordered_map<std::string, int> word_count;
    std::string word;
    
    while (iss >> word) {
        ++word_count[word];
    }
    
    // 輸出結果
    std::cout << "單詞頻率統計：" << std::endl;
    for (const auto& pair : word_count) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // 查找特定單詞
    std::string search_word = "words";
    auto it = word_count.find(search_word);
    
    if (it != word_count.end()) {
        std::cout << "\n單詞 '" << search_word << "' 出現了 " << it->second << " 次" << std::endl;
    } else {
        std::cout << "\n單詞 '" << search_word << "' 未出現" << std::endl;
    }
    
    return 0;
}
```

### 使用 unordered_multimap 存儲多值映射

```cpp
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

int main() {
    // 創建一個 unordered_multimap 存儲作者和他們的作品
    std::unordered_multimap<std::string, std::string> author_works;
    
    // 插入數據
    author_works.insert({"J.K. Rowling", "Harry Potter and the Philosopher's Stone"});
    author_works.insert({"J.K. Rowling", "Harry Potter and the Chamber of Secrets"});
    author_works.insert({"J.K. Rowling", "Harry Potter and the Prisoner of Azkaban"});
    author_works.insert({"George Orwell", "1984"});
    author_works.insert({"George Orwell", "Animal Farm"});
    author_works.insert({"J.R.R. Tolkien", "The Hobbit"});
    author_works.insert({"J.R.R. Tolkien", "The Lord of the Rings"});
    author_works.insert({"J.R.R. Tolkien", "The Silmarillion"});
    
    // 查找特定作者的所有作品
    std::string author = "J.K. Rowling";
    std::cout << author << " 的作品：" << std::endl;
    
    auto range = author_works.equal_range(author);
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "- " << it->second << std::endl;
    }
    
    // 計算每個作者的作品數量
    std::unordered_map<std::string, int> works_count;
    for (const auto& pair : author_works) {
        ++works_count[pair.first];
    }
    
    std::cout << "\n各作者作品數量：" << std::endl;
    for (const auto& pair : works_count) {
        std::cout << pair.first << ": " << pair.second << " 部作品" << std::endl;
    }
    
    // 刪除特定作者的所有作品
    std::string author_to_remove = "George Orwell";
    size_t removed = author_works.erase(author_to_remove);
    
    std::cout << "\n已刪除 " << removed << " 部 " << author_to_remove << " 的作品" << std::endl;
    
    // 顯示剩餘的所有作品
    std::cout << "\n剩餘作品列表：" << std::endl;
    for (const auto& pair : author_works) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    return 0;
}
```

### 使用 unordered_set 進行集合操作

```cpp
#include <iostream>
#include <unordered_set>
#include <vector>
#include <algorithm>

// 輔助函數：打印集合
template<typename T>
void print_set(const std::unordered_set<T>& s, const std::string& name) {
    std::cout << name << ": { ";
    for (const auto& elem : s) {
        std::cout << elem << " ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // 創建兩個集合
    std::unordered_set<int> set1 = {1, 2, 3, 4, 5};
    std::unordered_set<int> set2 = {4, 5, 6, 7, 8};
    
    print_set(set1, "集合 1");
    print_set(set2, "集合 2");
    
    // 計算集合的並集
    std::unordered_set<int> union_set = set1;
    union_set.insert(set2.begin(), set2.end());
    print_set(union_set, "並集");
    
    // 計算集合的交集
    std::unordered_set<int> intersection_set;
    for (const auto& elem : set1) {
        if (set2.count(elem) > 0) {
            intersection_set.insert(elem);
        }
    }
    print_set(intersection_set, "交集");
    
    // 計算集合的差集 (set1 - set2)
    std::unordered_set<int> difference_set;
    for (const auto& elem : set1) {
        if (set2.count(elem) == 0) {
            difference_set.insert(elem);
        }
    }
    print_set(difference_set, "差集 (set1 - set2)");
    
    // 檢查子集關係
    bool is_subset = true;
    for (const auto& elem : intersection_set) {
        if (set1.count(elem) == 0) {
            is_subset = false;
            break;
        }
    }
    std::cout << "交集是集合 1 的子集: " << (is_subset ? "是" : "否") << std::endl;
    
    // 使用 unordered_set 去除重複元素
    std::vector<int> vec = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
    std::unordered_set<int> unique_set(vec.begin(), vec.end());
    
    std::cout << "\n原始向量: ";
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    std::cout << "去重後: ";
    for (int num : unique_set) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 自定義類型與雜湊函數

```cpp
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

// 自定義類型
struct Student {
    int id;
    std::string name;
    std::string major;
    
    // 定義相等性比較運算符
    bool operator==(const Student& other) const {
        return id == other.id && name == other.name && major == other.major;
    }
    
    // 用於輸出
    friend std::ostream& operator<<(std::ostream& os, const Student& student) {
        os << "ID: " << student.id << ", Name: " << student.name << ", Major: " << student.major;
        return os;
    }
};

// 為 Student 特化 std::hash
namespace std {
    template<>
    struct hash<Student> {
        size_t operator()(const Student& s) const {
            // 組合各字段的雜湊值
            size_t h1 = hash<int>()(s.id);
            size_t h2 = hash<string>()(s.name);
            size_t h3 = hash<string>()(s.major);
            
            // 組合雜湊值的一種方法
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

int main() {
    // 創建存儲 Student 對象的 unordered_set
    std::unordered_set<Student> students;
    
    // 添加學生
    students.insert({1001, "Alice", "Computer Science"});
    students.insert({1002, "Bob", "Mathematics"});
    students.insert({1003, "Charlie", "Physics"});
    students.insert({1001, "Alice", "Computer Science"});  // 重複，不會被添加
    
    // 輸出學生集合
    std::cout << "學生集合 (共 " << students.size() << " 名學生):" << std::endl;
    for (const auto& student : students) {
        std::cout << student << std::endl;
    }
    
    // 查找學生
    Student search_student = {1002, "Bob", "Mathematics"};
    auto it = students.find(search_student);
    
    if (it != students.end()) {
        std::cout << "\n找到學生: " << *it << std::endl;
    } else {
        std::cout << "\n未找到學生" << std::endl;
    }
    
    // 使用 unordered_map 存儲學生成績
    std::unordered_map<Student, double> student_grades;
    
    student_grades[{1001, "Alice", "Computer Science"}] = 92.5;
    student_grades[{1002, "Bob", "Mathematics"}] = 88.0;
    student_grades[{1003, "Charlie", "Physics"}] = 95.5;
    
    // 輸出學生成績
    std::cout << "\n學生成績:" << std::endl;
    for (const auto& pair : student_grades) {
        std::cout << pair.first << " - GPA: " << pair.second << std::endl;
    }
    
    return 0;
}
```

### 性能比較：map vs unordered_map

```cpp
#include <iostream>
#include <unordered_map>
#include <map>
#include <string>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>

// 計時輔助函數
template<typename Func>
double measure_time(Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

int main() {
    // 測試參數
    const int num_elements = 1000000;  // 元素數量
    const int num_lookups = 1000000;   // 查找操作數量
    
    // 生成隨機字符串的輔助函數
    auto generate_random_string = [](int length) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, sizeof(alphanum) - 2);
        
        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; ++i) {
            result += alphanum[dist(gen)];
        }
        
        return result;
    };
    
    // 生成測試數據
    std::cout << "生成 " << num_elements << " 個隨機鍵值對..." << std::endl;
    std::vector<std::pair<std::string, int>> data;
    data.reserve(num_elements);
    
    for (int i = 0; i < num_elements; ++i) {
        data.emplace_back(generate_random_string(10), i);
    }
    
    // 生成查找用的鍵
    std::vector<std::string> lookup_keys;
    lookup_keys.reserve(num_lookups);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, num_elements - 1);
    
    for (int i = 0; i < num_lookups; ++i) {
        // 75% 的查找是已存在的鍵，25% 是不存在的鍵
        if (i % 4 != 0) {
            lookup_keys.push_back(data[dist(gen)].first);
        } else {
            lookup_keys.push_back(generate_random_string(10));
        }
    }
    
    // 測試 std::map
    std::map<std::string, int> ordered_map;
    
    double map_insert_time = measure_time([&]() {
        for (const auto& pair : data) {
            ordered_map[pair.first] = pair.second;
        }
    });
    
    double map_lookup_time = measure_time([&]() {
        int sum = 0;
        for (const auto& key : lookup_keys) {
            auto it = ordered_map.find(key);
            if (it != ordered_map.end()) {
                sum += it->second;
            }
        }
    });
    
    // 測試 std::unordered_map
    std::unordered_map<std::string, int> unordered_map;
    
    double unordered_map_insert_time = measure_time([&]() {
        for (const auto& pair : data) {
            unordered_map[pair.first] = pair.second;
        }
    });
    
    double unordered_map_lookup_time = measure_time([&]() {
        int sum = 0;
        for (const auto& key : lookup_keys) {
            auto it = unordered_map.find(key);
            if (it != unordered_map.end()) {
                sum += it->second;
            }
        }
    });
    
    // 輸出結果
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n性能比較 (時間單位: 毫秒)" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    std::cout << std::left << std::setw(20) << "操作" 
              << std::setw(15) << "std::map" 
              << std::setw(15) << "std::unordered_map" 
              << "速度提升" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    std::cout << std::left << std::setw(20) << "插入 " + std::to_string(num_elements) + " 個元素" 
              << std::setw(15) << map_insert_time 
              << std::setw(15) << unordered_map_insert_time 
              << (map_insert_time / unordered_map_insert_time) << "x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "查找 " + std::to_string(num_lookups) + " 次" 
              << std::setw(15) << map_lookup_time 
              << std::setw(15) << unordered_map_lookup_time 
              << (map_lookup_time / unordered_map_lookup_time) << "x" << std::endl;
    
    // 輸出容器的一些統計信息
    std::cout << "\n容器統計信息:" << std::endl;
    std::cout << "std::map 大小: " << ordered_map.size() << std::endl;
    std::cout << "std::unordered_map 大小: " << unordered_map.size() << std::endl;
    std::cout << "std::unordered_map 桶數: " << unordered_map.bucket_count() << std::endl;
    std::cout << "std::unordered_map 負載因子: " << unordered_map.load_factor() << std::endl;
    std::cout << "std::unordered_map 最大負載因子: " << unordered_map.max_load_factor() << std::endl;
    
    return 0;
}
```

## 使用情境

### 適合使用無序容器的情況

1. **需要快速查找、插入和刪除操作**：當性能是關鍵考慮因素，且不需要元素有序時
   ```cpp
   // 快速查詢用戶信息
   std::unordered_map<std::string, UserProfile> user_database;
   
   // 快速檢查元素是否存在
   std::unordered_set<std::string> visited_urls;
   if (visited_urls.count(url) == 0) {
       // 這是一個新 URL
       visited_urls.insert(url);
   }
   ```

2. **實現快取或記憶化**：存儲已計算的結果以避免重複計算
   ```cpp
   // 斐波那契數列的記憶化實現
   int fibonacci(int n, std::unordered_map<int, int>& cache) {
       if (n <= 1) return n;
       
       auto it = cache.find(n);
       if (it != cache.end()) {
           return it->second;  // 返回已計算的結果
       }
       
       int result = fibonacci(n - 1, cache) + fibonacci(n - 2, cache);
       cache[n] = result;  // 存儲結果
       return result;
   }
   ```

3. **需要多值映射**：當一個鍵需要對應多個值時
   ```cpp
   // 存儲學生選修的課程
   std::unordered_multimap<std::string, std::string> student_courses;
   student_courses.insert({"Alice", "Mathematics"});
   student_courses.insert({"Alice", "Physics"});
   student_courses.insert({"Alice", "Computer Science"});
   
   // 獲取特定學生的所有課程
   auto range = student_courses.equal_range("Alice");
   for (auto it = range.first; it != range.second; ++it) {
       std::cout << it->second << std::endl;
   }
   ```

4. **需要快速去重**：當需要快速過濾重複元素時
   ```cpp
   // 從輸入中去除重複單詞
   std::vector<std::string> words = {"apple", "banana", "apple", "orange", "banana", "grape"};
   std::unordered_set<std::string> unique_words(words.begin(), words.end());
   
   // unique_words 現在包含 {"apple", "banana", "orange", "grape"}
   ```

5. **實現關聯數組**：當需要通過鍵快速訪問值時
   ```cpp
   // 配置設置
   std::unordered_map<std::string, std::string> config;
   config["host"] = "localhost";
   config["port"] = "8080";
   config["user"] = "admin";
   
   // 快速訪問設置
   std::cout << "連接到 " << config["host"] << ":" << config["port"] << std::endl;
   ```

### 不適合使用無序容器的情況

1. **需要有序遍歷**：當需要按鍵的順序訪問元素時
   ```cpp
   // 不適合：按字母順序列出名字
   // 應該使用 std::map 而不是 std::unordered_map
   std::map<std::string, PhoneNumber> phone_book;
   // 遍歷時會按名字字母順序排列
   ```

2. **需要範圍查詢**：當需要查找落在某個範圍內的所有鍵時
   ```cpp
   // 不適合：查找特定分數範圍內的學生
   // 應該使用 std::map 而不是 std::unordered_map
   std::map<int, std::string> scores;
   // 可以使用 lower_bound 和 upper_bound 進行範圍查詢
   auto start = scores.lower_bound(80);
   auto end = scores.upper_bound(90);
   ```

3. **對內存使用敏感**：當內存使用是關鍵考慮因素時
   ```cpp
   // 不適合：在內存受限的環境中
   // 可能應該使用 std::map 或其他更節省內存的數據結構
   ```

4. **需要穩定的迭代順序**：當代碼依賴於容器的迭代順序時
   ```cpp
   // 不適合：需要穩定順序的情況
   // 無序容器的迭代順序可能在插入和刪除後改變
   ```

5. **自定義類型沒有好的雜湊函數**：當難以為自定義類型定義高效的雜湊函數時
   ```cpp
   // 不適合：複雜對象沒有明顯的雜湊策略
   // 可能應該使用 std::map 或其他基於比較的容器
   ```

### 最佳實踐

1. **選擇合適的容器**：根據需求選擇合適的無序容器類型
   ```cpp
   // 需要鍵值對且鍵唯一
   std::unordered_map<Key,