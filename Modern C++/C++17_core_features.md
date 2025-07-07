# C++17 核心特性

## 核心概念

C++17 是 C++ 語言的一個重要更新，引入了許多新的語言特性和標準庫組件，顯著提高了語言的表達能力和易用性。C++17 的目標是使 C++ 更加現代化、簡潔和高效，同時保持與現有代碼的兼容性。

C++17 的核心改進包括：

1. **結構化綁定 (Structured Bindings)**：允許將複合對象（如元組、數組、結構體）的成員直接解包到獨立變量中
2. **if 和 switch 語句中的初始化器 (Initializers in if/switch Statements)**：允許在條件語句中聲明和初始化變量
3. **內聯變量 (Inline Variables)**：支持在頭文件中定義內聯變量，解決多重包含問題
4. **折疊表達式 (Fold Expressions)**：簡化可變參數模板的處理
5. **constexpr if (constexpr if)**：在編譯期進行條件分支選擇
6. **類模板參數推導 (Class Template Argument Deduction, CTAD)**：無需顯式指定模板參數即可創建模板類對象
7. **執行策略 (Execution Policy)**：允許在標準庫算法中指定並行、向量化或順序執行策略
8. **標準庫的擴充 (Standard Library Extensions)**：包括文件系統庫、並行算法、`std::optional`、`std::variant`、`std::any` 等

這些特性使得 C++ 代碼更加簡潔、可讀，並且更容易維護。C++17 保持了與 C++11 和 C++14 的完全兼容性，同時提供了更多的便利性和表達能力。

## 語法

### 結構化綁定 (Structured Bindings)

```cpp
// 解包元組
std::tuple<int, double, std::string> get_person() {
    return {42, 3.14, "John"};
}

auto [id, score, name] = get_person();  // 直接解包到獨立變量

// 解包數組
int arr[] = {1, 2, 3};
auto [a, b, c] = arr;  // a=1, b=2, c=3

// 解包結構體
struct Point { int x; int y; };
Point p{10, 20};
auto [x, y] = p;  // x=10, y=20

// 解包 pair
std::map<std::string, int> ages = {{"Alice", 25}, {"Bob", 30}};
for (const auto& [name, age] : ages) {
    std::cout << name << " is " << age << " years old." << std::endl;
}
```

### if 和 switch 語句中的初始化器 (Initializers in if/switch Statements)

C++17 允許在 if 和 switch 語句中添加初始化語句，基本語法如下：

```cpp
// if 語句的基本語法
if (初始化語句; 條件表達式) {
    // 條件為真時執行
} else {
    // 條件為假時執行
}

// switch 語句的基本語法
switch (初始化語句; 表達式) {
    case 值1:
        // 代碼塊1
        break;
    case 值2:
        // 代碼塊2
        break;
    default:
        // 默認代碼塊
}
```

實際使用示例：

```cpp
// if 語句中的初始化器
if (auto it = map.find(key); it != map.end()) {
    // 使用 it
    use_value(it->second);
} else {
    // it 仍然可見，但 map.end()
    handle_missing_key();
}

// switch 語句中的初始化器
switch (auto val = get_value(); val) {
    case 1:
        handle_one();
        break;
    case 2:
        handle_two();
        break;
    default:
        handle_other(val);
}
```

這種語法的主要優點是將變量的作用域限制在 if 或 switch 語句內，避免變量泄漏到外部作用域，同時使代碼更加簡潔和清晰。

### 內聯變量 (Inline Variables)

C++17 引入內聯變量的主要目的是解決頭文件中定義全局變量和靜態成員變量時的多重定義問題。在 C++17 之前，如果在頭文件中定義變量，當該頭文件被多個源文件包含時，會導致變量被多次定義，從而違反 C++ 的單一定義規則 (One Definition Rule, ODR)，引發鏈接錯誤。

沒有內聯變量時的問題：

1. **頭文件中的全局變量**：在 C++17 之前，不能在頭文件中直接定義全局變量，只能聲明為 `extern` 然後在單個源文件中定義，這導致代碼分散且難以維護。

   ```cpp
   // header.h (C++17 之前)
   extern int global_counter;  // 只能聲明，不能定義
   extern const double pi;     // 只能聲明，不能定義
   
   // source.cpp
   int global_counter = 0;     // 必須在源文件中定義
   const double pi = 3.14159265358979323846;
   ```

2. **靜態成員變量**：在 C++17 之前，靜態成員變量必須在類外部定義，不能在類內初始化（除了整型常量靜態成員）。

   ```cpp
   // header.h (C++17 之前)
   class MyClass {
   public:
       static int count;       // 只能聲明，不能初始化
       static const double factor;
   };
   
   // source.cpp
   int MyClass::count = 0;     // 必須在源文件中定義
   const double MyClass::factor = 1.5;
   ```

3. **模板類的靜態成員**：對於模板類，靜態成員的外部定義更加複雜，需要為每個模板實例化提供定義。

使用內聯變量的解決方案：

```cpp
// 在頭文件中定義內聯變量
// header.h
inline int global_counter = 0;
inline constexpr double pi = 3.14159265358979323846;  // 使用 constexpr 使其可用於常量表達式

// 內聯靜態成員變量
class MyClass {
public:
    static inline int count = 0;  // 可以在類內直接初始化
    inline static constexpr double factor = 1.5;  // 使用 constexpr 使其可用於常量表達式
};
```

內聯變量的優點：

1. **簡化代碼組織**：變量的聲明和定義可以放在同一個地方（頭文件中）。
2. **提高可維護性**：不需要在頭文件和源文件之間切換來查找變量的定義。
3. **減少錯誤**：避免忘記在源文件中定義已聲明的變量。
4. **支持模板**：特別適合模板類的靜態成員，無需為每個模板實例化提供單獨的定義。
5. **保持單一定義規則**：編譯器確保所有翻譯單元中的內聯變量定義是相同的，並且只保留一個實例。

內聯變量使用與內聯函數相同的語義，允許在多個翻譯單元中定義，只要所有定義都相同，鏈接器就會將它們合併為單個實例。

### 折疊表達式 (Fold Expressions)

```cpp
// 使用折疊表達式計算總和
template<typename... Args>
auto sum(Args... args) {
    return (... + args);  // 一元左折疊: ((args1 + args2) + args3) + ...
}

// 使用折疊表達式打印參數
template<typename... Args>
void print(Args... args) {
    (std::cout << ... << args) << std::endl;  // 一元左折疊
}

// 帶初始值的折疊表達式
template<typename... Args>
auto sum_with_init(Args... args) {
    return (0 + ... + args);  // 二元左折疊: (((0 + args1) + args2) + ...)
}

// 使用折疊表達式調用多個函數
template<typename... Functions>
void call_all(Functions... funcs) {
    (funcs(), ...);  // 展開函數調用
}
```

### constexpr if

```cpp
// 使用 constexpr if 進行編譯期條件分支
template<typename T>
auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;  // 只有當 T 是指針類型時才編譯這段代碼
    } else if constexpr (std::is_array_v<T>) {
        return t[0];  // 只有當 T 是數組類型時才編譯這段代碼
    } else {
        return t;  // 其他情況
    }
}

// 使用 constexpr if 處理可變參數模板
template<typename T, typename... Rest>
void process_all(T&& first, Rest&&... rest) {
    process(std::forward<T>(first));

    if constexpr (sizeof...(rest) > 0) {
        process_all(std::forward<Rest>(rest)...);  // 只有當還有參數時才遞歸
    }
}
```

### 類模板參數推導 (Class Template Argument Deduction, CTAD)

```cpp
// C++17 之前需要顯式指定模板參數
std::pair<int, double> p1(1, 2.5);
std::tuple<int, double, std::string> t1(1, 2.5, "hello");

// C++17 允許參數推導
std::pair p2(1, 2.5);  // std::pair<int, double>
std::tuple t2(1, 2.5, "hello");  // std::tuple<int, double, const char*>

// 自定義類的模板參數推導
template<typename T>
class Vector {
public:
    Vector(std::initializer_list<T> init) : data(init) {}
private:
    std::vector<T> data;
};

// 使用推導
Vector v = {1, 2, 3, 4};  // Vector<int>
```

### 標準庫的擴充

```cpp
// std::optional
std::optional<int> find_value(const std::map<std::string, int>& m, const std::string& key) {
    auto it = m.find(key);
    if (it != m.end()) {
        return it->second;
    }
    return std::nullopt;
}

// std::variant
std::variant<int, double, std::string> v = "hello";
v = 42;  // 現在包含 int
v = 3.14;  // 現在包含 double

// std::any
std::any a = 42;
a = std::string("hello");
a = 3.14;

// 文件系統
namespace fs = std::filesystem;
fs::path p = "/home/user/file.txt";
if (fs::exists(p)) {
    std::cout << "File size: " << fs::file_size(p) << std::endl;
}

// 並行算法
std::vector<int> v = {1, 2, 3, 4, 5};
std::for_each(std::execution::par, v.begin(), v.end(), [](int& x) {
    x = x * x;  // 並行計算平方
});
```

## 與相似概念的比較

| 特性                 | C++17 | C++14  | C++11        | C++20              |
| -------------------- | ----- | ------ | ------------ | ------------------ |
| 結構化綁定           | 支持  | 不支持 | 不支持       | 增強（初始化捕獲） |
| 條件語句初始化器     | 支持  | 不支持 | 不支持       | 支持               |
| 內聯變量             | 支持  | 不支持 | 不支持       | 支持               |
| 折疊表達式           | 支持  | 不支持 | 不支持       | 支持               |
| constexpr if         | 支持  | 不支持 | 不支持       | 支持               |
| 類模板參數推導       | 支持  | 不支持 | 不支持       | 增強（約束和概念） |
| 執行策略             | 支持  | 不支持 | 不支持       | 增強（擴展策略）   |
| optional/variant/any | 支持  | 不支持 | 不支持       | 支持               |
| 文件系統             | 支持  | 不支持 | 不支持       | 支持               |
| 並行算法             | 支持  | 不支持 | 基本線程支持 | 協程和更多並行工具 |

## 使用範例

### 結構化綁定

```cpp
#include <iostream>
#include <tuple>
#include <map>
#include <string>

// 返回多個值的函數
std::tuple<int, double, std::string> get_student_info() {
    return {101, 3.8, "Alice"};
}

// 自定義結構體
struct Point {
    int x;
    int y;
    int z;
};

int main() {
    // 基本元組解包
    auto [id, gpa, name] = get_student_info();
    std::cout << "學生 ID: " << id << std::endl;
    std::cout << "GPA: " << gpa << std::endl;
    std::cout << "姓名: " << name << std::endl;
    // 學生 ID: 101
    // GPA: 3.8
    // 姓名: Alice

    // 數組解包
    int coords[3] = {10, 20, 30};
    auto [x, y, z] = coords;
    std::cout << "\n座標: (" << x << ", " << y << ", " << z << ")" << std::endl;
    // 座標: (10, 20, 30)

    // 結構體解包
    Point point = {100, 200, 300};
    auto [px, py, pz] = point;
    std::cout << "\n點: (" << px << ", " << py << ", " << pz << ")" << std::endl;
    // 點: (100, 200, 300)

    // 在循環中使用結構化綁定
    std::map<std::string, int> ages = {
        {"Alice", 25},
        {"Bob", 30},
        {"Charlie", 35}
    };

    std::cout << "\n年齡列表:" << std::endl;
    for (const auto& [person, age] : ages) {
        std::cout << person << " 是 " << age << " 歲" << std::endl;
    }
    // 年齡列表:
    // Alice 是 25 歲
    // Bob 是 30 歲
    // Charlie 是 35 歲

    // 解包 pair 並修改值
    std::pair<std::string, bool> result = {"Operation", true};
    auto& [operation, success] = result;
    operation = "Updated Operation";
    success = false;

    std::cout << "\n結果: " << operation << " - "
              << (success ? "成功" : "失敗") << std::endl;
    // 結果: Updated Operation - 失敗

    // 使用結構化綁定處理函數返回值
    auto process_data = [](int input) -> std::pair<bool, std::string> { // trailing return is a must or error: returning initializer list
        if (input > 0) {
            return {true, "正數"};
        } else if (input < 0) {
            return {true, "負數"};
        } else {
            return {false, "零"};
        }
    };

    int test_value = -5;
    auto [valid, description] = process_data(test_value);
    std::cout << "\n處理結果: " << test_value << " 是 "
              << (valid ? "有效的 " : "無效的 ") << description << std::endl;
    // 處理結果: -5 是 有效的 負數

    return 0;
}
```

### if 和 switch 語句中的初始化器

```cpp
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// 模擬數據庫查詢
std::string find_in_db(int id) {
    std::map<int, std::string> db = {
        {1, "Alice"},
        {2, "Bob"},
        {3, "Charlie"}
    };

    auto it = db.find(id);
    if (it != db.end()) {
        return it->second;
    }
    return "";
}

int main() {
    // 基本 if 初始化器
    if (int a = 5; a > 0) {
        std::cout << "a 是正數: " << a << std::endl;
    } else {
        std::cout << "a 不是正數: " << a << std::endl;
    }
    // 這裡 a 已經超出作用域
    // a 是正數: 5

    // 在 if 初始化器中使用函數調用
    if (auto name = find_in_db(2); !name.empty()) {
        std::cout << "找到用戶: " << name << std::endl;
    } else {
        std::cout << "未找到用戶 ID: 2" << std::endl;
    }
    // 找到用戶: Bob

    // 在 map 查找中使用 if 初始化器
    std::map<std::string, int> scores = {
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };

    if (auto it = scores.find("David"); it != scores.end()) {
        std::cout << "David 的分數: " << it->second << std::endl;
    } else {
        std::cout << "未找到 David 的分數" << std::endl;
    }
    // 未找到 David 的分數

    // 在 switch 語句中使用初始化器
    switch (int random_value = rand() % 3; random_value) {
        case 0:
            std::cout << "隨機值是 0" << std::endl;
            break;
        case 1:
            std::cout << "隨機值是 1" << std::endl;
            break;
        case 2:
            std::cout << "隨機值是 2" << std::endl;
            break;
    }
    // 隨機值是 1

    // 在 if 初始化器中使用算法
    std::vector<int> numbers = {1, 3, 5, 7, 9, 2, 4, 6, 8};

    if (auto it = std::find_if(numbers.begin(), numbers.end(),
                              [](int n) { return n % 2 == 0; });
        it != numbers.end()) {
        std::cout << "找到第一個偶數: " << *it << std::endl;
    } else {
        std::cout << "沒有找到偶數" << std::endl;
    }
    // 找到第一個偶數: 2

    // 在 if 初始化器中使用多個變量
    // Note: pair<iterator,bool> insert (const value_type& val);
    if (auto [iter, inserted] = scores.insert({"Eve", 88}); inserted) {
        std::cout << "成功添加 Eve 的分數" << std::endl;
    } else {
        std::cout << "Eve 的分數已經存在: " << iter->second << std::endl;
    }
    // 成功添加 Eve 的分數

    return 0;
}
```

### 內聯變量

```cpp
#include <iostream>
#include <string>

// 在頭文件中定義內聯變量
inline int global_counter = 0;
inline constexpr double pi = 3.14159265358979323846;  // 使用 constexpr 使其可用於常量表達式

// 使用內聯變量的類
class Logger {
public:
    // 內聯靜態成員變量
    inline static int log_count = 0;
    inline static std::string log_file = "app.log";

    static void log(const std::string& message) {
        ++log_count;
        std::cout << "[LOG " << log_count << "] " << message << std::endl;
    }
};

// 使用內聯變量的模板類
template<typename T>
class TypeInfo {
public:
    inline static int instance_count = 0;

    TypeInfo() {
        ++instance_count;
    }

    ~TypeInfo() {
        --instance_count;
    }

    static int get_count() {
        return instance_count;
    }
};

int main() {
    // 使用全局內聯變量
    std::cout << "Pi: " << pi << std::endl;
    // Pi: 3.14159

    ++global_counter;
    std::cout << "全局計數器: " << global_counter << std::endl;
    // 全局計數器: 1

    // 使用內聯靜態成員變量
    Logger::log("應用程序啟動");
    Logger::log("處理數據");
    Logger::log_file = "new_log.txt";  // 直接訪問內聯靜態成員
    Logger::log("日誌文件已更改為: " + Logger::log_file);
    // [LOG 1] 應用程序啟動
    // [LOG 2] 處理數據
    // [LOG 3] 日誌文件已更改為: new_log.txt

    // 使用模板類的內聯靜態成員
    TypeInfo<int> int1, int2, int3;
    TypeInfo<double> double1, double2;

    std::cout << "TypeInfo<int> 實例數: " << TypeInfo<int>::instance_count << std::endl;
    std::cout << "TypeInfo<double> 實例數: " << TypeInfo<double>::instance_count << std::endl;
    // TypeInfo<int> 實例數: 3
    // TypeInfo<double> 實例數: 2

    // 內聯常量在編譯期使用
    constexpr double circle_area = pi * 5.0 * 5.0;
    std::cout << "半徑為 5 的圓面積: " << circle_area << std::endl;
    // 半徑為 5 的圓面積: 78.5398

    return 0;
}
```

### 折疊表達式

```cpp
#include <iostream>
#include <string>
#include <vector>

// 使用折疊表達式計算總和
template<typename... Args>
auto sum(Args... args) {
    return (... + args);  // 一元左折疊: ((args1 + args2) + args3) + ...
}

// 使用折疊表達式計算乘積
template<typename... Args>
auto product(Args... args) {
    return (... * args);  // 一元左折疊
}

// 帶初始值的折疊表達式
template<typename... Args>
auto sum_with_init(Args... args) {
    return (0 + ... + args);  // 二元左折疊: (((0 + args1) + args2) + ...)
}

// 使用折疊表達式打印參數
template<typename... Args>
void print(Args... args) {
    (std::cout << ... << args) << std::endl;  // 一元左折疊
}

// 使用折疊表達式和分隔符打印參數
template<typename... Args>
void print_separated(Args... args) {
    int dummy[] = { 0, ((std::cout << args << ", "), 0)... };
    (void)dummy;  // 避免未使用變量警告
    std::cout << std::endl;
}

// 使用折疊表達式檢查所有參數是否滿足條件
template<typename... Args>
bool all_positive(Args... args) {
    return (... && (args > 0));  // 一元左折疊與邏輯與
}

// 使用折疊表達式檢查是否有參數滿足條件
template<typename... Args>
bool any_negative(Args... args) {
    return (... || (args < 0));  // 一元左折疊與邏輯或
}

// 使用折疊表達式調用多個函數
template<typename... Functions>
void call_all(Functions... funcs) {
    (funcs(), ...);  // 展開函數調用
}

// 使用折疊表達式將所有參數添加到容器
template<typename Container, typename... Args>
void push_back_all(Container& c, Args&&... args) {
    (c.push_back(std::forward<Args>(args)), ...);
}

int main() {
    // 基本折疊表達式
    std::cout << "總和: " << sum(1, 2, 3, 4, 5) << std::endl;
    std::cout << "乘積: " << product(1, 2, 3, 4, 5) << std::endl;
    std::cout << "帶初始值的總和: " << sum_with_init() << std::endl;  // 返回 0
    // 總和: 15
    // 乘積: 120
    // 帶初始值的總和: 0

    // 打印參數
    print("Hello", ' ', "World", '!', ' ', 2023);
    print_separated(1, 2.5, "three", 'f', "five");
    // Hello World! 2023
    // 1, 2.5, three, f, five,

    // 條件檢查
    std::cout << "全部為正: " << std::boolalpha
              << all_positive(1, 2, 3, 4, 5) << std::endl;
    std::cout << "全部為正: " << std::boolalpha
              << all_positive(1, 2, -3, 4, 5) << std::endl;
    std::cout << "存在負數: " << std::boolalpha
              << any_negative(1, 2, -3, 4, 5) << std::endl;
    std::cout << "存在負數: " << std::boolalpha
              << any_negative(1, 2, 3, 4, 5) << std::endl;
    // 全部為正: true
    // 全部為正: false
    // 存在負數: true
    // 存在負數: false

    // 調用多個函數
    auto f1 = []() { std::cout << "函數 1 被調用" << std::endl; };
    auto f2 = []() { std::cout << "函數 2 被調用" << std::endl; };
    auto f3 = []() { std::cout << "函數 3 被調用" << std::endl; };

    call_all(f1, f2, f3);
    // 函數 1 被調用
    // 函數 2 被調用
    // 函數 3 被調用

    // 向容器添加多個元素
    std::vector<int> vec;
    push_back_all(vec, 10, 20, 30, 40, 50);

    std::cout << "向量內容: ";
    for (int n : vec) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    // 向量內容: 10 20 30 40 50

    return 0;
}
```

### constexpr if

```cpp
#include <iostream>
#include <type_traits>
#include <string>
#include <vector>

// 使用 constexpr if 處理不同類型
template<typename T>
auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;  // 只有當 T 是指針類型時才編譯這段代碼
    } else if constexpr (std::is_array_v<T>) {
        return t[0];  // 只有當 T 是數組類型時才編譯這段代碼
    } else {
        return t;  // 其他情況
    }
}

// 使用 constexpr if 實現編譯期類型分發
template<typename T>
void print_type_info(T value) {
    std::cout << "值: " << value << ", 類型: ";

    if constexpr (std::is_integral_v<T>) {
        std::cout << "整數類型";
        if constexpr (std::is_signed_v<T>) {
            std::cout << " (有符號)";
        } else {
            std::cout << " (無符號)";
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮點類型";
    } else if constexpr (std::is_pointer_v<T>) {
        std::cout << "指針類型";
    } else if constexpr (std::is_array_v<T>) {
        std::cout << "數組類型";
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "字符串類型";
    } else {
        std::cout << "其他類型";
    }

    std::cout << std::endl;
}

// 使用 constexpr if 處理可變參數模板
template<typename T, typename... Rest>
void process_all(T&& first, Rest&&... rest) {
    // 處理第一個參數
    std::cout << "處理: " << first << std::endl;

    // 只有當還有參數時才遞歸
    if constexpr (sizeof...(rest) > 0) {
        process_all(std::forward<Rest>(rest)...);
    }
}

// 使用 constexpr if 實現編譯期優化
template<typename Container>
auto get_first_element(Container& c) {
    if constexpr (std::is_same_v<Container, std::vector<bool>>) {
        // std::vector<bool> 的特殊處理
        return c.front();
    } else if constexpr (std::is_array_v<Container>) {
        // 數組的特殊處理
        return c[0];
    } else {
        // 一般容器
        return *c.begin();
    }
}

// 使用 SFINAE 檢測類是否有 serialize 方法
template<typename, typename = void>
struct has_serialize_method : std::false_type {};

template<typename T>
struct has_serialize_method<T,
    std::void_t<decltype(std::declval<T>().serialize())>>
    : std::true_type {};

// 使用 constexpr if 實現 SFINAE 的替代方案
template<typename T>
auto serialize(const T& obj) {
    if constexpr (std::is_arithmetic_v<T>) {
        // 數值類型序列化
        return std::to_string(obj);
    } else if constexpr (std::is_same_v<T, std::string>) {
        // 字符串序列化
        return "\"" + obj + "\"";
    } else if constexpr (has_serialize_method<T>::value) {
        // 如果 T 有 serialize 方法 (C++17 方式)
        return obj.serialize();
    } else {
        // 默認情況
        return "{}";
    }
}

int main() {
    // 測試 get_value
    int x = 42;
    int* p_x = &x;
    int arr[] = {1, 2, 3};

    std::cout << "get_value(x): " << get_value(x) << std::endl;
    std::cout << "get_value(p_x): " << get_value(p_x) << std::endl;
    std::cout << "get_value(arr): " << get_value(arr) << std::endl;
    // get_value(x): 42
    // get_value(p_x): 42
    // get_value(arr): 1

    // 測試 print_type_info
    print_type_info(42);
    print_type_info(42u);
    print_type_info(3.14);
    print_type_info("hello");
    print_type_info(std::string("world"));
    print_type_info(p_x);
    // 值: 42, 類型: 整數類型 (有符號)
    // 值: 42, 類型: 整數類型 (無符號)
    // 值: 3.14, 類型: 浮點類型
    // 值: hello, 類型: 指針類型
    // 值: world, 類型: 字符串類型
    // 值: 0x7fff2e12a3bc, 類型: 指針類型

    // 測試 process_all
    process_all(1, "hello", 3.14, 'c');
    // 處理: 1
    // 處理: hello
    // 處理: 3.14
    // 處理: c

    // 測試 get_first_element
    std::vector<int> vec = {10, 20, 30};
    int simple_array[] = {100, 200, 300};
    std::vector<bool> bool_vec = {true, false, true};

    std::cout << "vec 的第一個元素: " << get_first_element(vec) << std::endl;
    std::cout << "simple_array 的第一個元素: " << get_first_element(simple_array) << std::endl;
    std::cout << "bool_vec 的第一個元素: " << std::boolalpha << get_first_element(bool_vec) << std::endl;
    // vec 的第一個元素: 10
    // simple_array 的第一個元素: 100
    // bool_vec 的第一個元素: true

    // 測試 serialize
    std::cout << "serialize(42): " << serialize(42) << std::endl;
    std::cout << "serialize(3.14): " << serialize(3.14) << std::endl;
    std::cout << "serialize(\"hello\"): " << serialize(std::string("hello")) << std::endl;
    // serialize(42): 42
    // serialize(3.14): 3.140000
    // serialize("hello"): "hello"

    return 0;
}
```

### 類模板參數推導

```cpp
#include <iostream>
#include <vector>
#include <array>
#include <utility>
#include <tuple>
#include <string>
#include <memory>

// 自定義類模板
template<typename T, typename U>
class Pair {
public:
    T first;
    U second;

    Pair(const T& t, const U& u) : first(t), second(u) {}

    void print() const {
        std::cout << "(" << first << ", " << second << ")" << std::endl;
    }
};

// 為自定義類模板提供推導指引
template<typename T, typename U>
Pair(T, U) -> Pair<T, U>;

// 帶有初始化列表的類模板
template<typename T>
class Vector {
private:
    std::vector<T> data;

public:
    Vector(std::initializer_list<T> init) : data(init) {}

    void print() const {
        std::cout << "Vector: ";
        for (const auto& item : data) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// 帶有多個模板參數的類
template<typename T, size_t N>
class FixedArray {
private:
    std::array<T, N> data;

public:
    FixedArray(const std::array<T, N>& arr) : data(arr) {}
    
    // C++17 compatible variadic constructor using SFINAE
    template<typename... Args,
             typename = std::enable_if_t<sizeof...(Args) == N>>
    FixedArray(Args&&... args) {
        size_t i = 0;
        ((data[i++] = std::forward<Args>(args)), ...);
    }
    
    void print() const {
        std::cout << "FixedArray[" << N << "]: ";
        for (const auto& item : data) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// 為 FixedArray 提供推導指引
template<typename T, typename... U>
FixedArray(T, U...) -> FixedArray<T, 1 + sizeof...(U)>;

int main() {
    // 標準庫類模板的參數推導
    std::pair p1(42, "hello");  // std::pair<int, const char*>
    std::tuple t1(1, 2.5, "world");  // std::tuple<int, double, const char*>
    
    // 使用 auto 關鍵字進行比較
    auto p2 = std::make_pair(42, "hello");  // 與 p1 相同
    auto t2 = std::make_tuple(1, 2.5, "world");  // 與 t1 相同
    
    std::cout << "p1: (" << p1.first << ", " << p1.second << ")" << std::endl;
    std::cout << "t1: (" << std::get<0>(t1) << ", " << std::get<1>(t1) << ", " << std::get<2>(t1) << ")" << std::endl;
    // p1: (42, hello)
    // t1: (1, 2.5, world)
    
    // 自定義類模板的參數推導
    Pair p3(3.14, 'A');  // Pair<double, char>
    p3.print();
    // (3.14, A)
    
    // 使用初始化列表的類模板參數推導
    Vector v = {1, 2, 3, 4, 5};  // Vector<int>
    v.print();
    // Vector: 1 2 3 4 5 
    
    // 多模板參數的推導
    FixedArray arr1 = {1, 2, 3, 4};  // FixedArray<int, 4>
    arr1.print();
    // FixedArray[4]: 1 2 3 4
    
    FixedArray arr2 = {1.1, 2.2, 3.3};  // FixedArray<double, 3>
    arr2.print();
    // FixedArray[3]: 1.1 2.2 3.3
    
    // 混合類型的推導 (注意第一個參數的類型決定了 T)
    FixedArray arr3 = {1, 2.2, 3};  // FixedArray<int, 3>，注意 2.2 會被轉換為 int
    arr3.print();
    // FixedArray[3]: 1 2 3 
    
    // 使用 std::array 構造
    std::array<double, 2> std_arr = {5.5, 6.6};
    FixedArray arr4(std_arr);  // FixedArray<double, 2>
    arr4.print();
    // FixedArray[2]: 5.5 6.6 
    
    return 0;
}
```

### 執行策略 (Execution Policy)

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include <random>
#include <chrono>

int main() {
    // 創建一個大型向量用於測試
    std::vector<int> large_vec(10'000'000);

    // 使用隨機數填充向量
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);

    for (auto& num : large_vec) {
        num = dis(gen);
    }

    // 創建向量的副本用於不同的排序策略
    auto vec_seq = large_vec;
    auto vec_par = large_vec;
    auto vec_par_unseq = large_vec;

    // 使用順序執行策略
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, vec_seq.begin(), vec_seq.end());
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_time = end - start;

    // 使用並行執行策略
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, vec_par.begin(), vec_par.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_time = end - start;

    // 使用並行+向量化執行策略
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par_unseq, vec_par_unseq.begin(), vec_par_unseq.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_unseq_time = end - start;

    // 輸出結果
    std::cout << "順序執行時間: " << seq_time.count() << " 秒" << std::endl;
    std::cout << "並行執行時間: " << par_time.count() << " 秒" << std::endl;
    std::cout << "並行+向量化執行時間: " << par_unseq_time.count() << " 秒" << std::endl;
    // 順序執行時間: 8.79625 秒
    // 並行執行時間: 8.95272 秒
    // 並行+向量化執行時間: 13.8388 秒

    // 使用 for_each 算法的並行版本
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::cout << "\n原始數據: ";
    for (int n : data) std::cout << n << " ";
    std::cout << std::endl;
    // 原始數據: 1 2 3 4 5 6 7 8 9 10 

    // 使用並行執行策略對每個元素進行平方
    std::for_each(std::execution::par, data.begin(), data.end(),
                 [](int& n) { n = n * n; });

    std::cout << "平方後: ";
    for (int n : data) std::cout << n << " ";
    std::cout << std::endl;
    // 平方後: 1 4 9 16 25 36 49 64 81 100 

    // 使用 transform 算法的並行版本
    std::vector<int> source = {1, 2, 3, 4, 5};
    std::vector<int> dest(source.size());

    std::transform(std::execution::par, source.begin(), source.end(), dest.begin(),
                  [](int n) { return n * 10; });

    std::cout << "\n轉換後: ";
    for (int n : dest) std::cout << n << " ";
    std::cout << std::endl;
    // 轉換後: 10 20 30 40 50 

    return 0;
}
```

C++17 引入了執行策略 (Execution Policy)，允許開發者指定標準庫算法的執行方式。執行策略定義在 `<execution>` 頭文件中，提供了三種基本策略：

1. **std::execution::seq**：順序執行策略。算法按順序執行，不允許並行化或向量化。這是默認的執行策略，與 C++17 之前的行為相同。

2. **std::execution::par**：並行執行策略。允許算法在多個線程上並行執行，但不允許在單個線程內進行向量化。適合計算密集型任務，可以利用多核處理器提高性能。

3. **std::execution::par_unseq**：並行+向量化執行策略。允許算法在多個線程上並行執行，並且允許在單個線程內進行向量化（使用 SIMD 指令）。這提供了最大的並行化潛力，但對算法實現有更嚴格的要求。

執行策略可以應用於大多數標準庫算法，如 `std::sort`、`std::for_each`、`std::transform` 等。使用執行策略時，算法的第一個參數就是執行策略對象。

```cpp
// 順序執行
std::sort(std::execution::seq, vec.begin(), vec.end());

// 並行執行
std::for_each(std::execution::par, vec.begin(), vec.end(), func);

// 並行+向量化執行
std::transform(std::execution::par_unseq, src.begin(), src.end(), dest.begin(), func);
```

使用執行策略的注意事項：

1. 使用並行執行策略時，提供給算法的函數對象必須是線程安全的。
2. 使用 `par_unseq` 策略時，函數對象不應該包含同步操作或依賴於線程局部狀態。
3. 並行執行可能導致非確定性的執行順序，因此不應該依賴於特定的執行順序。
4. 對於小數據集，並行化的開銷可能超過其帶來的性能提升。
5. 使用執行策略需要包含 `<execution>` 頭文件，並且可能需要鏈接特定的庫（如 Intel 的 TBB 或 Microsoft 的 PPL）。

執行策略為 C++ 標準庫算法提供了簡單而強大的並行化機制，使開發者能夠輕鬆利用現代多核處理器的計算能力，而無需手動管理線程。
