# C++11 constexpr 關鍵字

## 核心概念

`constexpr` 是 C++11 引入的一個關鍵字，用於指定表達式或函數可以在編譯期間求值。它的主要目的是將計算從執行期移至編譯期，從而提高程式的執行效率，並啟用更多的編譯期程式設計技術。

`constexpr` 可以應用於：
- 變數：宣告一個常量表達式變數，其值在編譯期確定
- 函數：宣告一個可以在編譯期求值的函數
- 建構函數：宣告一個可以在編譯期建構物件的建構函數

在 C++11 中，`constexpr` 函數有嚴格的限制：
- 只能包含一個 return 語句（C++14 放寬了這一限制）
- 不能包含非字面值操作（如動態記憶體分配、虛函數呼叫等）
- 必須返回字面值類型（如整數、浮點數、指標等）
- 所有呼叫的函數也必須是 `constexpr`

`constexpr` 與 `const` 的主要區別在於：`const` 僅表示變數不可修改，而 `constexpr` 則進一步要求其值必須在編譯期可確定。所有的 `constexpr` 變數都是 `const`，但並非所有 `const` 變數都是 `constexpr`。

## 語法

```cpp
// constexpr 變數
constexpr int value = 42;
constexpr double pi = 3.14159265358979;

// constexpr 函數
constexpr int square(int x) {
    return x * x;
}

// constexpr 建構函數
class Point {
private:
    int x_, y_;
public:
    constexpr Point(int x, int y) : x_(x), y_(y) {}
    constexpr int x() const { return x_; }
    constexpr int y() const { return y_; }
};

// 使用 constexpr 函數和物件
constexpr Point p(1, 2);
constexpr int x_coord = p.x();
```

## 與相似概念的比較

| 特性 | `constexpr` | `const` | `#define` 宏 | `enum` 常量 |
|------|-------------|---------|--------------|-------------|
| 編譯期求值 | 是 | 可能 | 是 | 是 |
| 類型安全 | 是 | 是 | 否 | 是 |
| 可用於函數 | 是 | 是 | 是（但僅為文本替換） | 否 |
| 可用於物件建構 | 是 | 否 | 否 | 否 |
| 可在執行期求值 | 是 | 是 | 否 | 否 |
| 可用於陣列大小 | 是 | 是（C++11 後） | 是 | 是 |
| 可用於模板參數 | 是 | 是（C++11 後） | 是 | 是 |
| 可用於 switch 語句 | 是 | 是 | 是 | 是 |
| 可用於指標操作 | 是 | 是 | 有限制 | 否 |

## 使用範例

### 基本用法

```cpp
#include <iostream>

// constexpr 變數
constexpr int max_size = 100;

// constexpr 函數
constexpr int factorial(int n) {
    return n <= 1 ? 1 : (n * factorial(n - 1));
}

int main() {
    // 編譯期計算階乘
    constexpr int fact5 = factorial(5);
    std::cout << "5! = " << fact5 << std::endl;  // 輸出: 5! = 120
    
    // 用於陣列大小
    int array[factorial(4)];  // 陣列大小為 24
    std::cout << "Size of array: " << sizeof(array) / sizeof(int) << std::endl;  // 輸出: Size of array: 24
    
    // 執行期計算
    int n;
    std::cout << "Enter a number: ";
    std::cin >> n;
    
    // factorial 也可以在執行期使用
    std::cout << n << "! = " << factorial(n) << std::endl;
    
    return 0;
}
```

### constexpr 類別和物件

```cpp
#include <iostream>

// constexpr 類別
class Circle {
private:
    double radius_;
public:
    constexpr Circle(double r) : radius_(r) {}
    constexpr double radius() const { return radius_; }
    constexpr double area() const { return 3.14159265358979 * radius_ * radius_; }
    constexpr double circumference() const { return 2 * 3.14159265358979 * radius_; }
};

int main() {
    // 編譯期建構和計算
    constexpr Circle c1(2.0);
    constexpr double a1 = c1.area();
    constexpr double c1_circum = c1.circumference();
    
    std::cout << "Circle with radius " << c1.radius() << ":" << std::endl;
    std::cout << "  Area: " << a1 << std::endl;
    std::cout << "  Circumference: " << c1_circum << std::endl;
    
    // 執行期建構和計算
    double r;
    std::cout << "Enter radius: ";
    std::cin >> r;
    
    Circle c2(r);
    std::cout << "Circle with radius " << c2.radius() << ":" << std::endl;
    std::cout << "  Area: " << c2.area() << std::endl;
    std::cout << "  Circumference: " << c2.circumference() << std::endl;
    
    return 0;
}
```

### 編譯期計算與條件判斷

```cpp
#include <iostream>
#include <array>

// 編譯期計算質數
constexpr bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

// 編譯期生成質數陣列
template<int N>
constexpr std::array<int, N> generate_primes() {
    std::array<int, N> result = {};
    int count = 0;
    for (int i = 2; count < N; ++i) {
        if (is_prime(i)) {
            result[count++] = i;
        }
    }
    return result;
}

int main() {
    // 編譯期生成前 10 個質數
    constexpr auto primes = generate_primes<10>();
    
    std::cout << "First 10 prime numbers:" << std::endl;
    for (auto prime : primes) {
        std::cout << prime << " ";
    }
    std::cout << std::endl;
    
    // 編譯期條件判斷
    if constexpr (is_prime(17)) {
        std::cout << "17 is a prime number (verified at compile time)" << std::endl;
    }
    
    return 0;
}
```

### C++14 中的增強功能

```cpp
#include <iostream>

// C++14 允許 constexpr 函數包含多個語句
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

int main() {
    // 編譯期計算斐波那契數列
    constexpr int fib10 = fibonacci(10);
    std::cout << "Fibonacci(10) = " << fib10 << std::endl;  // 輸出: Fibonacci(10) = 55
    
    return 0;
}
```

## 使用情境

### 適合使用 constexpr 的情況

1. **數學常量和計算**：
   ```cpp
   constexpr double pi = 3.14159265358979;
   constexpr double e = 2.71828182845904;
   constexpr double sqrt2 = 1.41421356237309;
   ```

2. **編譯期計算複雜數學函數**：
   ```cpp
   constexpr double power(double base, int exponent) {
       return exponent == 0 ? 1.0 :
              exponent > 0 ? base * power(base, exponent - 1) :
              1.0 / power(base, -exponent);
   }
   ```

3. **陣列大小和索引**：
   ```cpp
   constexpr int max_students = 30;
   std::array<Student, max_students> classroom;
   ```

4. **查表法和編譯期生成表格**：
   ```cpp
   template<int N>
   constexpr std::array<int, N> create_lookup_table() {
       std::array<int, N> table = {};
       for (int i = 0; i < N; ++i) {
           table[i] = /* 某種計算 */;
       }
       return table;
   }
   
   constexpr auto lookup_table = create_lookup_table<256>();
   ```

5. **編譯期優化和檢查**：
   ```cpp
   template<typename T>
   constexpr bool is_power_of_two(T x) {
       return x > 0 && (x & (x - 1)) == 0;
   }
   
   // 編譯期檢查
   static_assert(is_power_of_two(16), "16 should be a power of 2");
   ```

6. **元編程和類型特性**：
   ```cpp
   template<typename T>
   constexpr bool is_integral_power_of_two(T x) {
       return std::is_integral<T>::value && is_power_of_two(x);
   }
   ```

### 不適合使用 constexpr 的情況

1. **需要執行期資訊的計算**：
   ```cpp
   int get_user_input();
   int result = get_user_input() * 2;  // 不能是 constexpr
   ```

2. **涉及動態記憶體分配的操作**：
   ```cpp
   std::string create_string() {
       return "Hello, world!";  // 在 C++11 中不能是 constexpr
   }
   ```

3. **有副作用的操作**：
   ```cpp
   int counter = 0;
   int increment() {
       return ++counter;  // 不能是 constexpr
   }
   ```

4. **過於複雜的計算**：
   ```cpp
   // 可能導致編譯時間過長
   constexpr int factorial(int n) {
       return n <= 1 ? 1 : n * factorial(n - 1);
   }
   
   constexpr int large_factorial = factorial(20);  // 可能導致編譯時間增加
   ```

### 最佳實踐

- 對於已知在編譯期可確定的常量，優先使用 `constexpr` 而非 `const`
- 設計 `constexpr` 函數時，確保它們在編譯期和執行期都能正確工作
- 利用 `constexpr` 將適合的計算從執行期移至編譯期，提高程式效率
- 在模板元編程中，結合 `constexpr` 和 `static_assert` 進行編譯期檢查
- 注意 C++11、C++14、C++17 和 C++20 中 `constexpr` 功能的差異
- 避免過度使用 `constexpr`，特別是對於可能導致編譯時間顯著增加的複雜計算
- 使用 `if constexpr` (C++17) 進行編譯期條件分支，簡化模板代碼
- 理解 `constexpr` 函數在不同上下文中的行為：當用於編譯期上下文時必須能夠編譯期求值，而在執行期上下文中則可以像普通函數一樣運作