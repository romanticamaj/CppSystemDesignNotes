# C++20 核心特性

## 核心概念

C++20 是 C++ 語言的一次重大更新，引入了許多革命性的特性，大幅提升了語言的表達能力、安全性和性能。這些核心特性使得 C++ 程序員能夠以更簡潔、更直觀的方式解決複雜問題，同時保持 C++ 一貫的高性能特性。

C++20 的主要核心特性包括：

1. **概念 (Concepts)**：提供了一種指定模板參數約束的方式
2. **協程 (Coroutines)**：支持非阻塞的異步編程
3. **範圍 (Ranges)**：提供了更高級別的序列抽象
4. **模塊 (Modules)**：改進了代碼組織和編譯時間
5. **三向比較運算符 (Spaceship Operator)**：簡化了比較操作的實現
6. **指定初始化 (Designated Initializers)**：允許按名稱初始化結構體成員
7. **常量表達式改進**：擴展了編譯時計算的能力
8. **聚合初始化改進**：使聚合初始化更加靈活

這些特性共同構成了 C++20 的基礎，使得 C++ 在保持其高性能特性的同時，變得更加現代化、更易於使用，並能夠更好地應對當今軟件開發的挑戰。

## 主要特性

### 概念 (Concepts)

概念是 C++20 引入的一個重要特性，它允許程序員對模板參數進行約束，使得模板錯誤更加清晰，代碼更加可讀，並支持基於約束的函數重載。

```cpp
#include <concepts>
#include <iostream>
#include <string>
#include <vector>

// 定義一個概念：要求類型支持加法操作
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

// 使用概念約束模板函數
template<Addable T>
T add(T a, T b) {
    return a + b;
}

// 定義一個更複雜的概念：要求類型是容器
template<typename T>
concept Container = requires(T c) {
    { c.begin() } -> std::input_or_output_iterator;
    { c.end() } -> std::input_or_output_iterator;
    { c.size() } -> std::convertible_to<std::size_t>;
    { c[0] }; // 要求支持下標訪問
};

// 使用概念約束模板函數
template<Container C>
void print_container(const C& container) {
    std::cout << "容器大小: " << container.size() << std::endl;
    std::cout << "容器內容: ";
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

// 使用標準庫提供的概念
template<std::integral T>
T gcd(T a, T b) {
    while (b != 0) {
        T temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    // 使用 Addable 概念約束的函數
    std::cout << "3 + 4 = " << add(3, 4) << std::endl;
    std::cout << "3.14 + 2.71 = " << add(3.14, 2.71) << std::endl;
    
    // 使用 Container 概念約束的函數
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_container(vec);
    
    // 使用標準庫概念約束的函數
    std::cout << "GCD of 24 and 36: " << gcd(24, 36) << std::endl;
    
    return 0;
}
```

### 協程 (Coroutines)

協程是一種可以暫停和恢復執行的函數，使得非阻塞的異步編程變得更加簡單。C++20 引入了協程的基礎設施，包括 `co_await`、`co_yield` 和 `co_return` 關鍵字。

```cpp
#include <iostream>
#include <coroutine>
#include <thread>
#include <chrono>

// 一個簡單的協程返回對象
struct SimpleAwaiter {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const noexcept {
        std::thread([h]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            h.resume();
        }).detach();
    }
    void await_resume() const noexcept {}
};

// 一個簡單的協程返回類型
struct SimpleTask {
    struct promise_type {
        SimpleTask get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

// 一個使用 co_await 的協程
SimpleTask simple_coroutine() {
    std::cout << "協程開始執行" << std::endl;
    
    std::cout << "協程暫停 1 秒" << std::endl;
    co_await SimpleAwaiter{};
    
    std::cout << "協程恢復執行" << std::endl;
    
    std::cout << "協程再次暫停 1 秒" << std::endl;
    co_await SimpleAwaiter{};
    
    std::cout << "協程完成" << std::endl;
}

// 一個生成器協程返回類型
template<typename T>
struct Generator {
    struct promise_type {
        T value;
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T val) {
            value = val;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() { if (handle) handle.destroy(); }
    
    T current_value() { return handle.promise().value; }
    
    bool move_next() {
        if (handle.done()) return false;
        handle.resume();
        return !handle.done();
    }
};

// 一個使用 co_yield 的生成器協程
Generator<int> fibonacci(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; ++i) {
        co_yield a;
        int temp = a;
        a = b;
        b = temp + b;
    }
}

int main() {
    // 使用 co_await 的協程
    simple_coroutine();
    
    // 給協程一些時間執行
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 使用 co_yield 的生成器協程
    std::cout << "\n斐波那契數列:" << std::endl;
    auto fib = fibonacci(10);
    while (fib.move_next()) {
        std::cout << fib.current_value() << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 範圍 (Ranges)

範圍庫提供了一種新的方式來處理元素序列，使得複雜的序列操作變得更加簡潔和可讀。範圍庫建立在迭代器之上，但提供了更高級別的抽象和更強大的組合能力。

```cpp
#include <iostream>
#include <ranges>
#include <vector>
#include <algorithm>
#include <string>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 使用範圍視圖過濾和轉換數據
    auto even_numbers = numbers | std::views::filter([](int n) { return n % 2 == 0; });
    
    std::cout << "偶數: ";
    for (int n : even_numbers) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // 鏈接多個範圍視圖
    auto squared_even_numbers = numbers
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });
    
    std::cout << "偶數的平方: ";
    for (int n : squared_even_numbers) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // 使用範圍視圖處理字符串
    std::string text = "Hello, C++20 Ranges!";
    
    auto words = text
        | std::views::split(' ')
        | std::views::transform([](auto&& word) {
            return std::string_view(&*word.begin(), std::ranges::distance(word));
        });
    
    std::cout << "單詞: ";
    for (auto word : words) {
        std::cout << "\"" << word << "\" ";
    }
    std::cout << std::endl;
    
    // 使用範圍算法
    std::vector<int> data = {5, 3, 8, 1, 9, 4, 7, 2, 6};
    
    // 對前 5 個元素排序
    std::ranges::sort(data | std::views::take(5));
    
    std::cout << "部分排序後: ";
    for (int n : data) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // 查找第一個大於 5 的元素
    auto it = std::ranges::find_if(data, [](int n) { return n > 5; });
    if (it != data.end()) {
        std::cout << "第一個大於 5 的元素: " << *it << std::endl;
    }
    
    // 使用投影
    std::vector<std::pair<int, std::string>> pairs = {
        {3, "three"}, {1, "one"}, {4, "four"}, {2, "two"}
    };
    
    // 按照 pair 的第一個元素排序
    std::ranges::sort(pairs, {}, &std::pair<int, std::string>::first);
    
    std::cout << "按數字排序的對: ";
    for (const auto& [num, name] : pairs) {
        std::cout << "(" << num << ", " << name << ") ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 模塊 (Modules)

模塊是 C++20 引入的一個重要特性，旨在替代傳統的頭文件包含機制，提供更好的代碼組織、更快的編譯時間和更清晰的依賴關係。

```cpp
// math.cppm - 模塊接口文件
module;  // 全局模塊片段開始

#include <cmath>  // 傳統包含

export module math;  // 模塊聲明

// 導出函數
export double square(double x) {
    return x * x;
}

export double cube(double x) {
    return x * x * x;
}

// 導出命名空間
export namespace geometry {
    const double pi = 3.14159265358979323846;
    
    double circle_area(double radius) {
        return pi * square(radius);
    }
    
    double sphere_volume(double radius) {
        return (4.0/3.0) * pi * cube(radius);
    }
}

// 非導出函數（模塊內部實現細節）
double power(double base, int exponent) {
    double result = 1.0;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

// 導出使用內部函數的函數
export double power_of_two(int exponent) {
    return power(2.0, exponent);
}

// main.cpp - 使用模塊的文件
import <iostream>;  // 標準庫模塊（未來的語法）
import math;  // 導入自定義模塊

int main() {
    double x = 3.0;
    
    std::cout << "x² = " << square(x) << std::endl;
    std::cout << "x³ = " << cube(x) << std::endl;
    
    std::cout << "圓面積 (r = " << x << "): " << geometry::circle_area(x) << std::endl;
    std::cout << "球體積 (r = " << x << "): " << geometry::sphere_volume(x) << std::endl;
    
    std::cout << "2^8 = " << power_of_two(8) << std::endl;
    
    // 錯誤：power 不是導出函數
    // std::cout << "3^4 = " << power(3.0, 4) << std::endl;
    
    return 0;
}
```

### 三向比較運算符 (Spaceship Operator)

三向比較運算符 `<=>` 簡化了比較操作的實現，通過一次比較生成所有六種關係運算符（`<`, `<=`, `>`, `>=`, `==`, `!=`）。

```cpp
#include <iostream>
#include <compare>
#include <string>

// 使用三向比較運算符的簡單類
class Version {
private:
    int major;
    int minor;
    int patch;
    
public:
    Version(int maj, int min, int pat) : major(maj), minor(min), patch(pat) {}
    
    // 自動生成所有六種比較運算符
    auto operator<=>(const Version& other) const = default;
};

// 自定義三向比較運算符的類
class Person {
private:
    std::string name;
    int age;
    
public:
    Person(std::string n, int a) : name(std::move(n)), age(a) {}
    
    // 自定義三向比較運算符
    std::strong_ordering operator<=>(const Person& other) const {
        // 首先按姓名比較
        if (auto cmp = name <=> other.name; cmp != 0) {
            return cmp;
        }
        // 如果姓名相同，則按年齡比較
        return age <=> other.age;
    }
    
    // 仍然需要顯式定義相等運算符
    bool operator==(const Person& other) const = default;
};

int main() {
    // 使用 Version 類
    Version v1(1, 2, 3);
    Version v2(1, 3, 0);
    
    std::cout << "v1 < v2: " << (v1 < v2) << std::endl;
    std::cout << "v1 <= v2: " << (v1 <= v2) << std::endl;
    std::cout << "v1 > v2: " << (v1 > v2) << std::endl;
    std::cout << "v1 >= v2: " << (v1 >= v2) << std::endl;
    std::cout << "v1 == v2: " << (v1 == v2) << std::endl;
    std::cout << "v1 != v2: " << (v1 != v2) << std::endl;
    
    // 使用 Person 類
    Person p1("Alice", 30);
    Person p2("Alice", 25);
    Person p3("Bob", 30);
    
    std::cout << "\np1 < p2: " << (p1 < p2) << std::endl;  // false，Alice(30) > Alice(25)
    std::cout << "p1 < p3: " << (p1 < p3) << std::endl;    // true，Alice < Bob
    
    // 使用三向比較運算符的結果
    auto result = p1 <=> p2;
    std::cout << "p1 <=> p2 是小於: " << (result < 0) << std::endl;
    std::cout << "p1 <=> p2 是等於: " << (result == 0) << std::endl;
    std::cout << "p1 <=> p2 是大於: " << (result > 0) << std::endl;
    
    return 0;
}
```

### 指定初始化 (Designated Initializers)

指定初始化允許按名稱初始化結構體或類的成員，使代碼更加清晰和可讀。

```cpp
#include <iostream>

// 一個簡單的結構體
struct Point {
    double x;
    double y;
    double z;
};

// 一個表示矩形的結構體
struct Rectangle {
    Point top_left;
    Point bottom_right;
    std::string color;
    bool filled;
};

// 一個表示配置的結構體
struct Config {
    int max_connections;
    bool verbose_logging;
    std::string log_file;
    double timeout_seconds;
};

int main() {
    // 使用指定初始化
    Point p1 = { .x = 1.0, .y = 2.0, .z = 3.0 };
    std::cout << "點: (" << p1.x << ", " << p1.y << ", " << p1.z << ")" << std::endl;
    
    // 部分成員初始化，未指定的成員使用默認值
    Point p2 = { .x = 5.0, .z = 10.0 };  // y 未初始化，將為 0
    std::cout << "點: (" << p2.x << ", " << p2.y << ", " << p2.z << ")" << std::endl;
    
    // 嵌套結構體的指定初始化
    Rectangle rect = {
        .top_left = { .x = 0.0, .y = 0.0, .z = 0.0 },
        .bottom_right = { .x = 10.0, .y = 10.0, .z = 0.0 },
        .color = "red",
        .filled = true
    };
    
    std::cout << "矩形: 從 (" << rect.top_left.x << ", " << rect.top_left.y << ") 到 ("
              << rect.bottom_right.x << ", " << rect.bottom_right.y << "), 顏色: "
              << rect.color << ", 填充: " << std::boolalpha << rect.filled << std::endl;
    
    // 使用指定初始化設置配置
    Config cfg = {
        .max_connections = 100,
        .verbose_logging = true,
        .log_file = "/var/log/app.log",
        .timeout_seconds = 30.0
    };
    
    std::cout << "配置:\n"
              << "  最大連接數: " << cfg.max_connections << "\n"
              << "  詳細日誌: " << cfg.verbose_logging << "\n"
              << "  日誌文件: " << cfg.log_file << "\n"
              << "  超時(秒): " << cfg.timeout_seconds << std::endl;
    
    return 0;
}
```

### 常量表達式改進

C++20 進一步擴展了常量表達式的能力，包括在常量表達式中使用 `try-catch`、`dynamic_cast`、`typeid`、虛函數調用等。此外，還引入了 `consteval` 和 `constinit` 關鍵字。

```cpp
#include <iostream>
#include <string_view>
#include <array>

// consteval 函數：必須在編譯時求值
consteval int square(int n) {
    return n * n;
}

// constexpr 函數：可以在編譯時或運行時求值
constexpr int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// 在 constexpr 函數中使用 try-catch
constexpr int safe_divide(int a, int b) {
    try {
        if (b == 0) throw std::runtime_error("除以零");
        return a / b;
    } catch (...) {
        return 0;
    }
}

// 在 constexpr 函數中使用虛函數
struct Base {
    constexpr virtual int value() const { return 1; }
};

struct Derived : Base {
    constexpr int value() const override { return 2; }
};

constexpr int get_value(const Base& obj) {
    return obj.value();
}

// constinit 變量：必須在編譯時初始化，但可以在運行時修改
constinit int global_value = factorial(5);

int main() {
    // consteval 函數必須在編譯時求值
    constexpr int sq5 = square(5);  // OK
    // int x = 5;
    // int sq_x = square(x);  // 錯誤：x 不是常量表達式
    
    std::cout << "5 的平方: " << sq5 << std::endl;
    
    // constexpr 函數可以在編譯時或運行時求值
    constexpr int fact5 = factorial(5);  // 編譯時求值
    int n = 5;
    int fact_n = factorial(n);  // 運行時求值
    
    std::cout << "5 的階乘: " << fact5 << std::endl;
    std::cout << n << " 的階乘: " << fact_n << std::endl;
    
    // 在 constexpr 中使用 try-catch
    constexpr int result1 = safe_divide(10, 2);  // 編譯時求值
    constexpr int result2 = safe_divide(10, 0);  // 編譯時求值，捕獲異常
    
    std::cout << "10 / 2 = " << result1 << std::endl;
    std::cout << "10 / 0 = " << result2 << " (安全處理)" << std::endl;
    
    // 在 constexpr 中使用虛函數
    constexpr Derived d;
    constexpr int val = get_value(d);  // 編譯時調用虛函數
    
    std::cout << "虛函數值: " << val << std::endl;
    
    // constinit 變量
    std::cout << "全局值 (初始為 5!): " << global_value << std::endl;
    global_value = 100;  // 可以在運行時修改
    std::cout << "修改後的全局值: " << global_value << std::endl;
    
    // 編譯時創建和填充數組
    constexpr auto create_squares_array(int size) {
        std::array<int, 10> arr{};
        for (int i = 0; i < size && i < 10; ++i) {
            arr[i] = i * i;
        }
        return arr;
    }
    
    constexpr auto squares = create_squares_array(10);
    
    std::cout << "平方數組: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << squares[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 聚合初始化改進

C++20 改進了聚合初始化，允許基類成員的初始化，並放寬了一些限制。

```cpp
#include <iostream>
#include <string>

// 基類
struct Base {
    int id;
    std::string name;
};

// 派生類，是一個聚合體
struct Derived : Base {
    double value;
    bool flag;
};

// 另一個聚合體示例
struct Point {
    int x;
    int y;
};

struct Rectangle {
    Point top_left;
    Point bottom_right;
    std::string color;
};

int main() {
    // C++20 允許初始化基類成員
    Derived d = {
        {42, "base"},  // 基類 Base 的成員
        3.14,          // Derived::value
        true           // Derived::flag
    };
    
    std::cout << "Derived 對象:\n"
              << "  id: " << d.id << "\n"
              << "  name: " << d.name << "\n"
              << "  value: " << d.value << "\n"
              << "  flag: " << std::boolalpha << d.flag << std::endl;
    
    // 嵌套聚合初始化
    Rectangle rect = {
        {0, 0},        // top_left
        {10, 10},      // bottom_right
        "blue"         // color
    };
    
    std::cout << "\nRectangle 對象:\n"
              << "  top_left: (" << rect.top_left.x << ", " << rect.top_left.y << ")\n"
              << "  bottom_right: (" << rect.bottom_right.x << ", " << rect.bottom_right.y << ")\n"
              << "  color: " << rect.color << std::endl;
    
    return 0;
}
```

## 與其他特性的比較

C++20 核心特性與之前版本的特性相比有以下優勢：

1. **更強的類型安全**：概念提供了更強的模板參數約束，減少了模板錯誤，提高了代碼的可讀性和可維護性。

2. **更好的異步支持**：協程提供了一種自然的方式來表達異步操作，比回調或 Promise 更加直觀和易於使用。

3. **更高級別的抽象**：範圍庫提供了更高級別的序列抽象，使得複雜的序列操作變得更加簡潔和可讀。

4. **更快的編譯時間**：模塊提供了比頭文件包含更高效的代碼組織方式，可以顯著減少編譯時間。

5. **更簡潔的代碼**：三向比較運算符和指定初始化等特性使得代碼更加簡潔和可讀。

## 適用場景

C++20 核心特性適用於以下場景：

1. **大型項目**：模塊可以改善大型項目的代碼組織和編譯時間。

2. **泛型編程**：概念使得泛型代碼更加清晰和可靠。

3. **異步編程**：協程提供了一種自然的方式來處理異步操作，適用於網絡編程、GUI 編程等場景。

4. **數據處理**：範圍庫使得複雜的數據處理操作變得更加簡潔和可讀。

5. **API 設計**：三向比較運算符和指定初始化等特性可以使 API 更加用戶友好。

## 最佳實踐

使用 C++20 核心特性時，應遵循以下最佳實踐：

1. **優先使用概念約束模板參數**：這可以提供更好的錯誤消息和更清晰的代碼。

2. **合理使用協程**：協程適用於異步操作，但不應過度使用，特別是對於簡單的同步操作。

3. **利用範圍視圖的惰性求值**：範圍視圖是惰性求值的，可以避免不必要的臨時對象創建。

4. **逐步遷移到模塊**：模塊是一個重大變化，應該逐步遷移，而不是一次性重構所有代碼。

5. **使用三向比較運算符簡化比較操作**：對於需要多種比較操作的類，使用三向比較運算符可以
5. **使用三向比較運算符簡化比較操作**：對於需要多種比較操作的類，使用三向比較運算符可以大大簡化代碼。

6. **使用指定初始化提高代碼可讀性**：特別是對於具有多個成員的結構體，指定初始化可以使代碼更加清晰。

7. **利用常量表達式改進進行編譯時計算**：盡可能將計算移至編譯時，以提高運行時性能。

## 結論

C++20 的核心特性代表了 C++ 語言的一次重大進步，使得 C++ 在保持其高性能特性的同時，變得更加現代化、更易於使用。這些特性共同解決了 C++ 長期以來的一些痛點，如模板錯誤消息難以理解、異步編程複雜、頭文件包含效率低下等。

隨著編譯器對 C++20 特性的支持不斷完善，開發者可以逐步採用這些新特性，提高代碼質量和開發效率。雖然完全掌握所有 C++20 特性需要時間，但即使是部分採用也能帶來顯著的好處。

C++20 為 C++ 的未來發展奠定了堅實的基礎，使得 C++ 能夠更好地應對現代軟件開發的挑戰，並在高性能計算、系統編程、遊戲開發等領域保持其競爭力。