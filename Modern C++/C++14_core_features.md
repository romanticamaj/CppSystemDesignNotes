# C++14 核心特性

## 核心概念

C++14 是 C++11 的一個小型更新，被視為 C++11 的"缺陷修復版"。它引入了一些語法改進和標準庫擴展，使 C++11 的特性更加完善和易於使用。C++14 的主要目標是提高 C++11 的可用性，而不是引入大量全新的語言特性。

C++14 的核心改進包括：

1. **泛型 lambda 表達式**：允許在 lambda 參數中使用 `auto` 關鍵字
2. **返回類型推導**：函數可以使用 `auto` 作為返回類型，編譯器會自動推導
3. **decltype(auto)**：結合 decltype 和 auto 的類型推導機制，保留引用和 cv 限定符
4. **變量模板**：允許創建模板化的變量
5. **改進的 constexpr**：放寬了 constexpr 函數的限制，允許更複雜的編譯期計算
6. **二進制字面量**：支持 `0b` 前綴的二進制字面量
7. **數字分隔符**：使用單引號 `'` 分隔數字，提高可讀性
8. **標準庫的改進**：包括新的算法、容器功能和實用工具

這些改進使得 C++ 代碼更加簡潔、可讀，並且更容易維護。C++14 保持了與 C++11 的完全兼容性，同時提供了更多的便利性和表達能力。

## 語法

### 泛型 lambda 表達式

```cpp
// C++11 中的 lambda
auto lambda11 = [](int x, int y) { return x + y; };

// C++14 中的泛型 lambda
auto lambda14 = [](auto x, auto y) { return x + y; };

// 使用泛型 lambda
int sum_int = lambda14(5, 10);          // 15
double sum_double = lambda14(3.14, 2.71); // 5.85
std::string concat = lambda14(std::string("Hello, "), std::string("World!")); // "Hello, World!"
```

### 返回類型推導

```cpp
// C++11 中的返回類型後置語法
auto add11(int x, int y) -> int { return x + y; }

// C++14 中的返回類型推導
auto add14(int x, int y) { return x + y; }  // 返回類型被推導為 int

// 複雜返回類型的推導
auto get_vector() { return std::vector<int>{1, 2, 3}; }  // 返回 std::vector<int>

// 條件返回類型
auto get_value(bool use_int) {
    if (use_int)
        return 42;
    else
        return 3.14;  // 錯誤：返回類型不一致
}
```

## decltype(auto)

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <type_traits>

// decltype(auto) 作為返回類型
template<typename Container>
decltype(auto) get_first_element(Container& c) {
    return c[0];  // 保留引用和 cv 限定符
}

// 對比 auto 作為返回類型
template<typename Container>
auto get_first_element_auto(Container& c) {
    return c[0];  // 返回值，丟失引用
}

// decltype(auto) 用於變量聲明
template<typename T>
void show_type_info(T&& value) {
    decltype(auto) x = std::forward<T>(value);

    std::cout << "decltype(auto) 保留了類型: "
              << std::boolalpha
              << "是引用? " << std::is_reference<decltype(x)>::value << ", "
              << "是左值引用? " << std::is_lvalue_reference<decltype(x)>::value << ", "
              << "是右值引用? " << std::is_rvalue_reference<decltype(x)>::value << std::endl;
}

int main() {
    // 基本用法
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // 使用 decltype(auto) 保留引用
    decltype(auto) first = get_first_element(vec);
    first = 100;  // 修改原始向量的第一個元素

    // 使用 auto 獲取值
    auto first_copy = get_first_element_auto(vec);
    first_copy = 200;  // 不影響原始向量

    std::cout << "vec[0] = " << vec[0] << std::endl;  // 輸出 100，不是 200
    // vec[0] = 100

    // 對比 auto 和 decltype(auto) 的行為
    int x = 42;
    int& ref_x = x;

    auto y = ref_x;          // y 是 int，丟失引用
    decltype(auto) z = ref_x; // z 是 int&，保留引用

    y = 10;  // 不影響 x
    z = 20;  // 修改 x

    std::cout << "x = " << x << ", y = " << y << ", z = " << z << std::endl;
    // x = 20, y = 10, z = 20

    // 對於表達式的類型推導
    int arr[5] = {1, 2, 3, 4, 5};

    auto arr0 = arr[0];          // int
    decltype(auto) arr0_ref = arr[0];  // int&，因為 arr[0] 是左值

    // 完美轉發場景
    int value = 42;
    show_type_info(value);        // 傳遞左值
    show_type_info(std::move(value)); // 傳遞右值
    // decltype(auto) 保留了類型: 是引用? true, 是左值引用? true, 是右值引用? false
    // decltype(auto) 保留了類型: 是引用? true, 是左值引用? false, 是右值引用? true

    return 0;
}
```

### decltype(auto) 與 auto 的差異

| 特性         | decltype(auto)       | auto                                   |
| ------------ | -------------------- | -------------------------------------- |
| 類型推導規則 | 使用 decltype 的規則 | 使用模板參數推導規則                   |
| 引用保留     | 保留引用和 cv 限定符 | 丟失引用和 cv 限定符（除非使用 auto&） |
| 表達式類型   | 保留表達式的確切類型 | 獲取表達式的值類型                     |
| 數組退化     | 不會將數組退化為指針 | 將數組退化為指針                       |
| 主要用途     | 完美轉發、保留引用   | 簡化類型聲明、避免類型重複             |

### 何時使用 decltype(auto)

1. **需要保留引用類型時**：當你需要保留引用語義，特別是在返回引用的函數中。

   ```cpp
   template<typename Container>
   decltype(auto) access(Container& c, size_t idx) {
       return c[idx];  // 返回引用，允許修改原始容器
   }
   ```

2. **完美轉發返回值時**：在泛型代碼中轉發函數調用的返回值。

   ```cpp
   template<typename Func, typename... Args>
   decltype(auto) forward_call(Func&& func, Args&&... args) {
       return std::forward<Func>(func)(std::forward<Args>(args)...);
   }
   ```

3. **處理複雜表達式時**：當表達式的確切類型（包括引用性和 cv 限定符）很重要時。

   ```cpp
   template<typename T>
   decltype(auto) process_and_return(T&& container) {
       // 做一些處理...
       return std::forward<T>(container)[0];  // 保留 container[0] 的確切類型
   }
   ```

## 變量模板

```cpp
#include <iostream>
#include <complex>
#include <type_traits>

// 基本變量模板
template<typename T>
constexpr T pi = T(3.1415926535897932385);

// 帶有非類型參數的變量模板
template<int N>
constexpr int factorial = N * factorial<N-1>;

template<>
constexpr int factorial<0> = 1;  // 特化終止條件

// 類型特性變量模板
template<typename T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;

// 數學常量變量模板
template<typename T>
constexpr T e = T(2.7182818284590452353);

template<typename T>
constexpr T sqrt2 = T(1.4142135623730950488);

// 單位轉換變量模板
template<typename T>
constexpr T deg_to_rad = pi<T> / T(180);

int main() {
    // 使用不同類型的 pi
    std::cout << "pi<float> = " << pi<float> << std::endl;
    std::cout << "pi<double> = " << pi<double> << std::endl;
    // pi<float> = 3.14159
    // pi<double> = 3.14159

    // 使用階乘變量模板
    std::cout << "factorial<5> = " << factorial<5> << std::endl;
    std::cout << "factorial<10> = " << factorial<10> << std::endl;
    // factorial<5> = 120
    // factorial<10> = 3628800

    // 使用類型特性變量模板
    std::cout << "is_pointer_v<int> = " << is_pointer_v<int> << std::endl;
    std::cout << "is_pointer_v<int*> = " << is_pointer_v<int*> << std::endl;
    // is_pointer_v<int> = 0
    // is_pointer_v<int*> = 1

    // 使用數學常量
    std::cout << "e<double> = " << e<double> << std::endl;
    std::cout << "sqrt2<double> = " << sqrt2<double> << std::endl;
    // e<double> = 2.71828
    // sqrt2<double> = 1.41421

    // 角度轉換
    double angle_deg = 45.0;
    double angle_rad = angle_deg * deg_to_rad<double>;
    std::cout << angle_deg << " 度 = " << angle_rad << " 弧度" << std::endl;
    // 45 度 = 0.785398 弧度

    // 在複數中使用 pi
    std::complex<double> z = std::polar(1.0, pi<double>);
    std::cout << "e^(i*pi) = " << z << std::endl;
    // e^(i*pi) = (-1,1.22465e-16)

    return 0;
}
```

## 改進的 constexpr

```cpp
#include <iostream>
#include <array>

// C++14 中的 constexpr 函數可以包含：
// - 局部變量聲明（非 static，非 thread_local）
// - 條件語句 (if 和 switch)
// - 循環語句 (for, while, do-while)
// - 非 constexpr 函數調用
// - 多個 return 語句

// 使用循環的 constexpr 函數
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// 使用條件語句的 constexpr 函數
constexpr int fibonacci(int n) {
    if (n <= 1) return n;

    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int tmp = a + b;
        a = b;
        b = tmp;
    }
    return b;
}

// 使用局部變量和多個 return 的 constexpr 函數
constexpr int max_of_three(int a, int b, int c) {
    int max_val = a;
    if (b > max_val) max_val = b;
    if (c > max_val) max_val = c;
    return max_val;
}

// 使用 constexpr 函數生成編譯期數組
// 輔助函數：使用遞迴模板展開來初始化數組
template<size_t... Is>
constexpr std::array<int, sizeof...(Is)> make_fibonacci_array(std::index_sequence<Is...>) {
    return { fibonacci(Is)... };  // 使用初始化列表而非 operator[]
}

template<size_t N>
constexpr std::array<int, N> generate_fibonacci_array() {
    return make_fibonacci_array(std::make_index_sequence<N>{});
}

// 遞歸 constexpr 函數
constexpr int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

int main() {
    // 編譯期計算
    constexpr int fact5 = factorial(5);
    constexpr int fib10 = fibonacci(10);
    constexpr int max_val = max_of_three(10, 5, 15);

    std::cout << "5! = " << fact5 << std::endl;
    std::cout << "fibonacci(10) = " << fib10 << std::endl;
    std::cout << "max(10, 5, 15) = " << max_val << std::endl;
    // 5! = 120
    // fibonacci(10) = 55
    // max(10, 5, 15) = 15

    // 編譯期生成數組
    constexpr auto fib_array = generate_fibonacci_array<10>();

    std::cout << "Fibonacci 數列 (前10個): ";
    for (auto val : fib_array) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    // Fibonacci 數列 (前10個): 0 1 1 2 3 5 8 13 21 34

    // 運行時使用 constexpr 函數
    int n;
    std::cout << "請輸入一個數字計算階乘: ";
    std::cin >> n;
    std::cout << n << "! = " << factorial(n) << std::endl;
    // 請輸入一個數字計算階乘: 5
    // 5! = 120

    // 編譯期計算最大公約數
    constexpr int a = 48, b = 18;
    constexpr int result = gcd(a, b);
    std::cout << "gcd(" << a << ", " << b << ") = " << result << std::endl;
    // gcd(48, 18) = 6

    return 0;
}
```

## 二進制字面量和數字分隔符

```cpp
#include <iostream>
#include <bitset>
#include <iomanip>

int main() {
    // 二進制字面量
    int binary1 = 0b1010;  // 10 in decimal
    int binary2 = 0b1100;  // 12 in decimal

    std::cout << "0b1010 = " << binary1 << std::endl;
    std::cout << "0b1100 = " << binary2 << std::endl;
    // 0b1010 = 10
    // 0b1100 = 12

    // 位操作
    int bit_and = binary1 & binary2;  // 0b1000 (8)
    int bit_or = binary1 | binary2;   // 0b1110 (14)
    int bit_xor = binary1 ^ binary2;  // 0b0110 (6)

    std::cout << "0b1010 & 0b1100 = 0b" << std::bitset<4>(bit_and) << " (" << bit_and << ")" << std::endl;
    std::cout << "0b1010 | 0b1100 = 0b" << std::bitset<4>(bit_or) << " (" << bit_or << ")" << std::endl;
    std::cout << "0b1010 ^ 0b1100 = 0b" << std::bitset<4>(bit_xor) << " (" << bit_xor << ")" << std::endl;
    // 0b1010 & 0b1100 = 0b1000 (8)
    // 0b1010 | 0b1100 = 0b1110 (14)
    // 0b1010 ^ 0b1100 = 0b0110 (6)

    // 使用數字分隔符的二進制字面量
    int mask = 0b1010'1111'0000'1111;
    std::cout << "掩碼 = 0b" << std::bitset<16>(mask) << std::endl;
    // 掩碼 = 0b1010111100001111

    // 數字分隔符用於十進制
    int million = 1'000'000;
    std::cout << "一百萬 = " << million << std::endl;
    // 一百萬 = 1000000

    // 數字分隔符用於十六進制
    int hex_value = 0xDEAD'BEEF;
    std::cout << "0xDEAD'BEEF = " << std::hex << std::uppercase << hex_value << std::dec << std::endl;
    // 0xDEAD'BEEF = DEADBEEF

    // 數字分隔符用於浮點數
    double pi = 3.141'592'653'589'793;
    std::cout << "π ≈ " << std::fixed << std::setprecision(15) << pi << std::endl;
    // π ≈ 3.141592653589793

    // 數字分隔符用於科學計數法
    double avogadro = 6.022'140'76e23;
    std::cout << "阿伏伽德羅常數 ≈ " << std::scientific << avogadro << std::endl;
    // 阿伏伽德羅常數 ≈ 6.022140760000000E+23

    // 混合使用二進制字面量和數字分隔符
    unsigned char flags = 0b0010'1010;
    std::cout << "標誌位 = " << std::bitset<8>(flags) << std::endl;
    // 標誌位 = 00101010

    // 檢查特定位
    bool is_bit_3_set = (flags & 0b0000'1000) != 0;  // 檢查第3位 (0-indexed)
    std::cout << "第3位是否設置: " << std::boolalpha << is_bit_3_set << std::endl;
    // 第3位是否設置: true

    return 0;
}
```

## 標準庫的改進

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <map>
#include <mutex> // for unique_lock

// 使用 std::make_unique
void demonstrate_make_unique() {
    // 創建單個對象
    auto p1 = std::make_unique<int>(42);
    std::cout << "p1: " << *p1 << std::endl;
    // p1: 42

    // 創建數組
    auto p2 = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) {
        p2[i] = i * i;
    }

    std::cout << "p2[3]: " << p2[3] << std::endl;
    // p2[3]: 9

    // 創建自定義類型
    struct Point { int x, y; };
    auto p3 = std::make_unique<Point>(Point{10, 20});
    std::cout << "p3: (" << p3->x << ", " << p3->y << ")" << std::endl;
    // p3: (10, 20)
}

// 使用 std::exchange
void demonstrate_exchange() {
    int a = 5;
    int b = std::exchange(a, 10);  // a 變為 10，返回舊值 5

    std::cout << "a = " << a << ", b = " << b << std::endl;
    // a = 10, b = 5

    // 在移動構造函數中使用 std::exchange
    class Resource {
    public:
        int* data;

        Resource() : data(new int[100]) {}

        ~Resource() { delete[] data; }

        // 移動構造函數
        Resource(Resource&& other) noexcept
            : data(std::exchange(other.data, nullptr)) {}

        // 移動賦值運算符
        Resource& operator=(Resource&& other) noexcept {
            if (this != &other) {
                delete[] data;
                data = std::exchange(other.data, nullptr);
            }
            return *this;
        }
    };

    Resource r1;
    Resource r2 = std::move(r1);  // r1.data 現在是 nullptr

    std::cout << "r1.data is " << (r1.data == nullptr ? "nullptr" : "not nullptr") << std::endl;
    // r1.data is nullptr
}

// 使用 std::shared_timed_mutex 和 std::shared_lock
void demonstrate_shared_mutex() {
    std::shared_timed_mutex mtx; // shared lock
    std::map<int, std::string> data = {{1, "one"}, {2, "two"}}; // shared data

    // 模擬讀線程
    auto reader = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            {
                // 獲取共享（讀）鎖
                std::shared_lock<std::shared_timed_mutex> lock(mtx); // shared lock as read lock
                std::cout << "Reader " << id << " sees: ";
                for (const auto& pair : data) {
                    std::cout << pair.first << "=" << pair.second << " ";
                }
                std::cout << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    // 模擬寫線程
    auto writer = [&]() {
        for (int i = 3; i <= 5; ++i) {
            {
                // 獲取獨占（寫）鎖
                std::unique_lock<std::shared_timed_mutex> lock(mtx); // shared lock as `unique` write lock
                data[i] = "value" + std::to_string(i);
                std::cout << "Writer added: " << i << "=value" << i << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };

    // 啟動線程
    std::thread w(writer);
    std::thread r1(reader, 1);
    std::thread r2(reader, 2);

    w.join();
    r1.join();
    r2.join();
    // Writer added: 3=value3
    // Reader 1 sees: 1=one 2=two 3=value3
    // Reader 2 sees: 1=one 2=two 3=value3
    // Reader 1 sees: 1=one 2=two 3=value3
    // Reader 2 sees: 1=one 2=two 3=value3
    // Writer added: 4=value4
    // Reader 2 sees: 1=one 2=two 3=value3 4=value4
    // Reader 1 sees: 1=one 2=two 3=value3 4=value4
    // Writer added: 5=value5
}

// 使用 std::integer_sequence
template<typename T, T... Indices>
void print_sequence(std::integer_sequence<T, Indices...>) {
    std::cout << "Sequence: ";
    ((std::cout << Indices << ' '), ...);  // C++17 折疊表達式
    std::cout << std::endl;
}

void demonstrate_integer_sequence() {
    // 創建整數序列
    using seq_t = std::integer_sequence<int, 0, 1, 2, 3, 4>;
    print_sequence(seq_t{});
    // Sequence: 0 1 2 3 4

    // 創建索引序列
    print_sequence(std::make_index_sequence<5>{});
    // Sequence: 0 1 2 3 4

    // 使用整數序列展開元組
    auto tuple_printer = [](auto&&... args) {
        std::cout << "Tuple: ";
        ((std::cout << args << ' '), ...);
        std::cout << std::endl;
    };

    auto tuple_elements = std::make_tuple(10, "hello", 3.14);

    // 使用整數序列訪問元組元素
    auto print_tuple = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        tuple_printer(std::get<Is>(tuple_elements)...);
    };

    print_tuple(std::make_index_sequence<std::tuple_size_v<decltype(tuple_elements)>>{});
    // Tuple: 10 hello 3.14
}

int main() {
    std::cout << "=== std::make_unique 示例 ===" << std::endl;
    demonstrate_make_unique();

    std::cout << "\n=== std::exchange 示例 ===" << std::endl;
    demonstrate_exchange();

    std::cout << "\n=== std::shared_timed_mutex 示例 ===" << std::endl;
    demonstrate_shared_mutex();

    std::cout << "\n=== std::integer_sequence 示例 ===" << std::endl;
    demonstrate_integer_sequence();

    return 0;
}
```

## 與相似概念的比較

| 特性           | C++14                    | C++11            | C++17              | C++98/03 |
| -------------- | ------------------------ | ---------------- | ------------------ | -------- |
| Lambda 表達式  | 泛型 lambda              | 基本 lambda      | 捕獲 \*this        | 不支持   |
| 返回類型推導   | 完全支持                 | 僅後置返回類型   | 增強的推導規則     | 不支持   |
| decltype(auto) | 支持                     | 不支持           | 支持               | 不支持   |
| constexpr      | 允許局部變量、循環、條件 | 單一 return 語句 | if constexpr       | 不支持   |
| 變量模板       | 支持                     | 不支持           | 改進的模板參數推導 | 不支持   |
| 二進制字面量   | 支持                     | 不支持           | 支持               | 不支持   |
| 數字分隔符     | 支持                     | 不支持           | 支持               | 不支持   |
| 智能指針工廠   | make_unique              | make_shared      | 增強的 make_shared | 不支持   |
| 並發支持       | 共享鎖                   | 基本線程支持     | 並行算法           | 不支持   |

## 使用情境

### 適合使用 C++14 特性的情況

1. **泛型編程**：當需要編寫可處理多種類型的通用代碼時

   ```cpp
   // 泛型 lambda 處理不同類型的容器
   auto print_container = [](const auto& container) {
       for (const auto& item : container) {
           std::cout << item << " ";
       }
       std::cout << std::endl;
   };

   std::vector<int> vec = {1, 2, 3};
   std::list<double> lst = {1.1, 2.2, 3.3};
   print_container(vec);  // 處理 vector<int>
   print_container(lst);  // 處理 list<double>
   ```

2. **編譯期計算**：當需要在編譯期執行複雜計算時

   ```cpp
   // 編譯期計算查找表
   // 輔助函數：使用遞迴模板展開來初始化正弦表
   template<size_t... Is>
   constexpr std::array<double, sizeof...(Is)> make_sin_table(std::index_sequence<Is...>) {
       return { std::sin(2.0 * pi<double> * Is / sizeof...(Is))... };  // 使用初始化列表而非 operator[]
   }

   template<size_t N>
   constexpr auto generate_sin_table() {
       return make_sin_table(std::make_index_sequence<N>{});
   }

   // 編譯期生成的查找表
   constexpr auto sin_table = generate_sin_table<360>();
   ```

3. **簡化函數返回類型**：當函數返回類型複雜或難以表達時

   ```cpp
   // 返回類型複雜的函數
   auto get_data_processor(bool use_parallel) {
       if (use_parallel) {
           return [](auto& data) { /* 並行處理 */ };
       } else {
           return [](auto& data) { /* 串行處理 */ };
       }
   }
   ```

4. **完美轉發和類型保留**：當需要精確保留類型信息時

   ```cpp
   // 使用 decltype(auto) 完美轉發函數返回值
   template<typename Func, typename... Args>
   decltype(auto) call_and_forward(Func&& func, Args&&... args) {
       // 可能進行一些日誌記錄或性能測量
       return std::forward<Func>(func)(std::forward<Args>(args)...);
   }
   ```

5. **數值計算和科學計算**：當需要高效處理數值數據時

   ```cpp
   // 使用數字分隔符提高可讀性
   constexpr double avogadro = 6.022'140'76e23;
   constexpr double planck = 6.626'070'15e-34;

   // 使用變量模板定義物理常數
   template<typename T>
   constexpr T speed_of_light = T(299'792'458);
   ```

## 最佳實踐

1. **優先使用 auto 而非顯式類型**：當類型明顯或不重要時，使用 auto 可以提高代碼可讀性和維護性。

   ```cpp
   // 好的做法
   auto it = container.begin();
   auto result = compute_value();

   // 避免冗長的類型聲明
   // std::map<std::string, std::vector<int>>::iterator it = container.begin();
   ```

2. **使用 decltype(auto) 保留引用和 cv 限定符**：在泛型代碼中轉發返回值時使用。

   ```cpp
   template<typename F, typename... Args>
   decltype(auto) wrapper(F&& f, Args&&... args) {
       // 前置處理...
       decltype(auto) result = std::forward<F>(f)(std::forward<Args>(args)...);
       // 後置處理...
       return result;
   }
   ```

3. **利用改進的 constexpr 進行編譯期計算**：將適合的計算移至編譯期可以提高運行時性能。

   ```cpp
   // 編譯期生成查找表

   // 假設的複雜計算函數
   constexpr double complex_calculation(size_t index) {
       // 這裡是複雜計算的實現
       return static_cast<double>(index * index); // 示例計算
   }

   // 輔助函數：使用遞迴模板展開來初始化查找表
   template<size_t... Is>
   constexpr std::array<double, sizeof...(Is)> make_lookup_table(std::index_sequence<Is...>) {
       return { complex_calculation(Is)... }; // 使用初始化列表而非 operator[]
   }

   template<size_t N>
   constexpr auto generate_lookup_table() {
       return make_lookup_table(std::make_index_sequence<N>{});
   }

   constexpr auto lookup_table = generate_lookup_table<1024>();
   ```

4. **使用數字分隔符提高可讀性**：對於長數字，使用分隔符可以顯著提高代碼可讀性。

```cpp
// 使用數字分隔符提高可讀性
constexpr int billion = 1'000'000'000;
constexpr double pi = 3.141'592'653'589'793;
constexpr uint32_t ipv4 = 0xFF'FF'FF'FF;
constexpr uint64_t credit_card = 1234'5678'9012'3456;
```

5. **泛型 lambda 用於簡化算法**：使用泛型 lambda 可以簡化與標準庫算法的交互。

   ```cpp
   std::vector<std::string> names = {"Alice", "Bob", "Charlie"};

   // 使用泛型 lambda 進行轉換
   auto to_upper = [](auto& s) {
       std::transform(s.begin(), s.end(), s.begin(), ::toupper);
   };

   // 對不同類型的容器應用相同的操作
   std::for_each(names.begin(), names.end(), to_upper);

   // 使用泛型 lambda 進行排序
   auto sort_by_size = [](const auto& a, const auto& b) {
       return a.size() < b.size();
   };

   std::vector<std::string> words = {"hello", "world", "C++14", "programming"};
   std::sort(words.begin(), words.end(), sort_by_size);
   ```

## 結論

C++14 雖然是一個相對較小的更新，但它引入的特性顯著提高了 C++11 的可用性和表達能力。泛型 lambda 表達式、返回類型推導和 decltype(auto) 使得泛型編程更加簡潔和直觀。改進的 constexpr 支持使得編譯期計算更加強大和靈活。變量模板、二進制字面量和數字分隔符等特性則進一步提高了代碼的可讀性和可維護性。

C++14 的標準庫改進，如 std::make_unique、std::shared_timed_mutex 和 std::integer_sequence，填補了 C++11 標準庫的一些空白，提供了更完整的工具集。這些改進使得 C++ 在系統編程、高性能計算、嵌入式系統和遊戲開發等領域的應用更加高效和便捷。

總的來說，C++14 是 C++11 的自然延續，它保持了與 C++11 的完全兼容性，同時通過一系列精心設計的改進，使 C++ 成為一個更加現代、強大和易用的編程語言。這些改進為 C++17 和 C++20 中更大規模的語言演進奠定了基礎。
