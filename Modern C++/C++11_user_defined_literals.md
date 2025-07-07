# C++11 使用者定義字面量 (User-defined Literals)

## 核心概念

使用者定義字面量（User-defined Literals，簡稱 UDL）是 C++11 引入的一項功能，它允許程式設計者為基本字面量定義自己的後綴，從而創建具有特定類型或含義的新字面量。這項功能大大增強了 C++ 的表達能力，使代碼更加直觀、易讀且不易出錯。

使用者定義字面量的核心思想是將字面量與其單位或含義緊密結合，例如 `42km` 表示 42 公里，`3.14_rad` 表示 3.14 弧度，或 `"hello"s` 表示一個 `std::string` 類型的字符串。這種方式不僅提高了代碼的可讀性，還能在編譯時捕獲類型錯誤，避免單位轉換錯誤。

C++11 允許為以下類型的字面量定義自定義後綴：
- 整數字面量（如 `42_km`）
- 浮點數字面量（如 `3.14_pi`）
- 字符字面量（如 `'x'_char`）
- 字符串字面量（如 `"hello"_s`）

## 語法

### 定義使用者定義字面量

```cpp
// 整數字面量運算符
constexpr long long operator"" _km(unsigned long long kilometers) {
    return kilometers * 1000;  // 轉換為米
}

// 浮點數字面量運算符
constexpr long double operator"" _deg(long double degrees) {
    return degrees * 3.14159265358979323846 / 180;  // 轉換為弧度
}

// 字符串字面量運算符
std::string operator"" _s(const char* str, size_t len) {
    return std::string(str, len);
}

// 字符字面量運算符
constexpr char operator"" _uppercase(char c) {
    return c >= 'a' && c <= 'z' ? c - 32 : c;
}

// 原始字面量運算符（處理數字序列）
constexpr int operator"" _binary(const char* str, size_t len) {
    int result = 0;
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '0' || str[i] == '1') {
            result = result * 2 + (str[i] - '0');
        }
    }
    return result;
}
```

### 使用使用者定義字面量

```cpp
// 使用整數字面量
auto distance = 10_km;  // 10000 米

// 使用浮點數字面量
auto angle = 90.0_deg;  // π/2 弧度

// 使用字符串字面量
auto greeting = "hello"_s;  // std::string

// 使用字符字面量
auto uppercase_a = 'a'_uppercase;  // 'A'

// 使用原始字面量
auto binary_value = "101"_binary;  // 5
```

### 標準庫中的使用者定義字面量

C++14 在 `<chrono>`, `<string>`, `<complex>` 和 `<string_view>` (C++17) 等標準庫中引入了一些使用者定義字面量：

```cpp
// 時間字面量 (C++14, <chrono>)
using namespace std::chrono_literals;
auto day = 24h;  // 24 小時
auto minute = 1min;  // 1 分鐘
auto second = 30s;  // 30 秒
auto millisecond = 50ms;  // 50 毫秒

// 字符串字面量 (C++14, <string>)
using namespace std::string_literals;
auto str = "hello"s;  // std::string

// 複數字面量 (C++14, <complex>)
using namespace std::complex_literals;
auto c = 3.0 + 4.0i;  // std::complex<double>(3.0, 4.0)

// 字符串視圖字面量 (C++17, <string_view>)
using namespace std::string_view_literals;
auto sv = "hello"sv;  // std::string_view
```

## 與相似概念的比較

| 特性 | 使用者定義字面量 | 類型轉換函數 | 常量/枚舉 | 宏 |
|------|----------------|------------|---------|-----|
| 語法整合 | 高（直接在字面量上使用後綴） | 中（需要函數調用） | 低（需要變量名） | 高（但不類型安全） |
| 類型安全 | 高 | 高 | 高 | 低 |
| 編譯時計算 | 可以（使用 constexpr） | 可以（使用 constexpr） | 是 | 是（但有限制） |
| 可讀性 | 高（單位與值結合） | 中 | 中 | 低（容易混淆） |
| 命名空間支持 | 是 | 是 | 是 | 否 |
| 重載能力 | 有限（基於參數類型） | 高 | 不適用 | 不適用 |
| 適用場景 | 單位、特殊類型表示 | 一般類型轉換 | 命名常量 | 文本替換 |

## 使用範例

### 基本使用：物理單位

```cpp
#include <iostream>

// 長度單位
constexpr long double operator"" _km(long double x) { return x * 1000.0; }
constexpr long double operator"" _m(long double x) { return x; }
constexpr long double operator"" _cm(long double x) { return x * 0.01; }
constexpr long double operator"" _mm(long double x) { return x * 0.001; }

// 整數版本
constexpr long double operator"" _km(unsigned long long x) { return x * 1000.0; }
constexpr long double operator"" _m(unsigned long long x) { return x; }
constexpr long double operator"" _cm(unsigned long long x) { return x * 0.01; }
constexpr long double operator"" _mm(unsigned long long x) { return x * 0.001; }

// 時間單位
constexpr long double operator"" _h(long double x) { return x * 3600.0; }
constexpr long double operator"" _min(long double x) { return x * 60.0; }
constexpr long double operator"" _s(long double x) { return x; }
constexpr long double operator"" _ms(long double x) { return x * 0.001; }

// 整數版本
constexpr long double operator"" _h(unsigned long long x) { return x * 3600.0; }
constexpr long double operator"" _min(unsigned long long x) { return x * 60.0; }
constexpr long double operator"" _s(unsigned long long x) { return x; }
constexpr long double operator"" _ms(unsigned long long x) { return x * 0.001; }

// 計算速度（米/秒）
constexpr long double speed(long double distance, long double time) {
    return distance / time;
}

int main() {
    // 使用使用者定義字面量表示距離和時間
    auto marathon = 42.195_km;    // 馬拉松距離，單位：米
    auto lap_time = 4.5_min;      // 單圈時間，單位：秒
    
    // 計算速度
    auto marathon_speed = speed(marathon, 2_h + 30_min + 45_s);
    auto lap_speed = speed(400_m, lap_time);
    
    std::cout << "馬拉松距離: " << marathon << " 米" << std::endl;
    std::cout << "馬拉松平均速度: " << marathon_speed << " 米/秒" << std::endl;
    std::cout << "400米單圈時間: " << lap_time << " 秒" << std::endl;
    std::cout << "400米單圈平均速度: " << lap_speed << " 米/秒" << std::endl;
    
    // 混合使用不同單位
    auto total_distance = 5.0_km + 200.0_m + 15.0_cm;
    std::cout << "總距離: " << total_distance << " 米" << std::endl;
    
    // 編譯時計算
    constexpr auto compile_time_distance = 5.0_km + 400.0_m;
    constexpr auto compile_time_time = 30.0_min + 15.0_s;
    constexpr auto compile_time_speed = speed(compile_time_distance, compile_time_time);
    
    std::cout << "編譯時計算的速度: " << compile_time_speed << " 米/秒" << std::endl;
    
    return 0;
}
```

### 自定義字符串處理

```cpp
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

// 將字符串轉換為大寫的字面量運算符
std::string operator"" _upper(const char* str, size_t len) {
    std::string result(str, len);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

// 將字符串轉換為小寫的字面量運算符
std::string operator"" _lower(const char* str, size_t len) {
    std::string result(str, len);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// 反轉字符串的字面量運算符
std::string operator"" _reverse(const char* str, size_t len) {
    std::string result(str, len);
    std::reverse(result.begin(), result.end());
    return result;
}

// 計算字符串中單詞數量的字面量運算符
size_t operator"" _words(const char* str, size_t len) {
    std::string text(str, len);
    size_t word_count = 0;
    bool in_word = false;
    
    for (char c : text) {
        if (std::isspace(c)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++word_count;
        }
    }
    
    return word_count;
}

int main() {
    // 使用自定義字符串字面量
    auto upper_text = "hello, world"_upper;
    auto lower_text = "HELLO, WORLD"_lower;
    auto reversed_text = "hello, world"_reverse;
    auto word_count = "This is a sample sentence with seven words."_words;
    
    std::cout << "原始文本: " << "hello, world" << std::endl;
    std::cout << "大寫: " << upper_text << std::endl;
    std::cout << "小寫: " << lower_text << std::endl;
    std::cout << "反轉: " << reversed_text << std::endl;
    std::cout << "單詞數量: " << word_count << std::endl;
    
    // 鏈式使用
    auto processed = "Hello, World"_lower._reverse;
    std::cout << "處理後 (小寫然後反轉): " << processed << std::endl;
    
    return 0;
}
```

### 自定義數值類型

```cpp
#include <iostream>
#include <cmath>

// 分數類
class Fraction {
private:
    int numerator;
    int denominator;
    
    // 計算最大公約數
    static int gcd(int a, int b) {
        return b == 0 ? a : gcd(b, a % b);
    }
    
    // 化簡分數
    void simplify() {
        int g = gcd(std::abs(numerator), denominator);
        numerator /= g;
        denominator /= g;
    }
    
public:
    constexpr Fraction(int num, int denom)
        : numerator(num), denominator(denom) {
        if (denominator == 0) {
            throw std::invalid_argument("Denominator cannot be zero");
        }
        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
    }
    
    // 分數加法
    Fraction operator+(const Fraction& other) const {
        return Fraction(numerator * other.denominator + other.numerator * denominator,
                       denominator * other.denominator);
    }
    
    // 分數減法
    Fraction operator-(const Fraction& other) const {
        return Fraction(numerator * other.denominator - other.numerator * denominator,
                       denominator * other.denominator);
    }
    
    // 分數乘法
    Fraction operator*(const Fraction& other) const {
        return Fraction(numerator * other.numerator,
                       denominator * other.denominator);
    }
    
    // 分數除法
    Fraction operator/(const Fraction& other) const {
        if (other.numerator == 0) {
            throw std::invalid_argument("Division by zero");
        }
        return Fraction(numerator * other.denominator,
                       denominator * other.numerator);
    }
    
    // 轉換為浮點數
    double to_double() const {
        return static_cast<double>(numerator) / denominator;
    }
    
    // 輸出運算符
    friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
        Fraction simplified = f;
        simplified.simplify();
        
        if (simplified.denominator == 1) {
            os << simplified.numerator;
        } else {
            os << simplified.numerator << "/" << simplified.denominator;
        }
        return os;
    }
};

// 使用者定義字面量：創建分數
constexpr Fraction operator"" _frac(unsigned long long n) {
    return Fraction(n, 1);
}

// 使用者定義字面量：創建帶小數點的分數
Fraction operator"" _frac(long double d) {
    // 將浮點數轉換為分數的簡單實現
    // 這裡使用一個簡單的方法，實際應用中可能需要更複雜的算法
    const int precision = 1000000;  // 精度
    int num = static_cast<int>(d * precision);
    int denom = precision;
    return Fraction(num, denom);
}

// 使用者定義字面量：以 n/d 格式創建分數
constexpr Fraction operator"" _nd(unsigned long long n) {
    // 這個字面量期望格式為 a_nd，表示分數 a/d
    // 例如 3_nd 表示 3/d，其中 d 需要在後面指定
    return Fraction(n, 1);
}

// 分數除法運算符
constexpr Fraction operator/(unsigned long long n, const Fraction& d) {
    return Fraction(n, 1) / d;
}

int main() {
    // 使用使用者定義字面量創建分數
    auto f1 = 3_frac;          // 3/1
    auto f2 = 0.5_frac;        // 1/2 (近似值)
    auto f3 = 1_nd / 4_frac;   // 1/4
    
    std::cout << "f1 = " << f1 << std::endl;
    std::cout << "f2 = " << f2 << std::endl;
    std::cout << "f3 = " << f3 << std::endl;
    
    // 分數運算
    auto sum = f1 + f3;
    auto difference = f1 - f2;
    auto product = f2 * f3;
    auto quotient = f1 / f3;
    
    std::cout << "f1 + f3 = " << sum << std::endl;
    std::cout << "f1 - f2 = " << difference << std::endl;
    std::cout << "f2 * f3 = " << product << std::endl;
    std::cout << "f1 / f3 = " << quotient << std::endl;
    
    // 轉換為浮點數
    std::cout << "f3 as double: " << f3.to_double() << std::endl;
    
    // 複雜表達式
    auto complex_expr = 5_frac / 2_frac + 1_frac / 3_frac - 1_frac / 6_frac;
    std::cout << "5/2 + 1/3 - 1/6 = " << complex_expr << std::endl;
    std::cout << "As double: " << complex_expr.to_double() << std::endl;
    
    return 0;
}
```

### 使用標準庫字面量

```cpp
#include <iostream>
#include <string>
#include <complex>
#include <chrono>
#include <iomanip>

int main() {
    // 使用標準庫中的字面量
    
    // 字符串字面量 (C++14)
    using namespace std::string_literals;
    auto str1 = "Hello, world"s;  // std::string
    auto str2 = u8"UTF-8 string"s;  // UTF-8 編碼的 std::string
    auto str3 = u"UTF-16 string"s;  // std::u16string
    auto str4 = U"UTF-32 string"s;  // std::u32string
    
    std::cout << "std::string: " << str1 << std::endl;
    std::cout << "字符串長度: " << str1.length() << std::endl;
    
    // 複數字面量 (C++14)
    using namespace std::complex_literals;
    auto c1 = 3.0 + 4.0i;  // std::complex<double>(3.0, 4.0)
    auto c2 = 2.0 + 3.0i;
    
    std::cout << "\n複數 c1: " << c1 << std::endl;
    std::cout << "複數 c2: " << c2 << std::endl;
    std::cout << "c1 + c2: " << c1 + c2 << std::endl;
    std::cout << "c1 * c2: " << c1 * c2 << std::endl;
    std::cout << "c1 的模: " << std::abs(c1) << std::endl;
    
    // 時間字面量 (C++14)
    using namespace std::chrono_literals;
    auto day = 24h;
    auto hour = 1h;
    auto minute = 1min;
    auto second = 1s;
    auto millisecond = 1ms;
    auto microsecond = 1us;
    auto nanosecond = 1ns;
    
    std::cout << "\n時間轉換:" << std::endl;
    std::cout << "1 天 = " << day.count() << " 小時" << std::endl;
    std::cout << "1 小時 = " << std::chrono::duration_cast<std::chrono::minutes>(hour).count() << " 分鐘" << std::endl;
    std::cout << "1 分鐘 = " << std::chrono::duration_cast<std::chrono::seconds>(minute).count() << " 秒" << std::endl;
    std::cout << "1 秒 = " << std::chrono::duration_cast<std::chrono::milliseconds>(second).count() << " 毫秒" << std::endl;
    
    // 時間計算
    auto total_time = 2h + 30min + 45s;
    std::cout << "總時間: " 
              << std::chrono::duration_cast<std::chrono::hours>(total_time).count() << " 小時 "
              << std::chrono::duration_cast<std::chrono::minutes>(total_time % 1h).count() << " 分鐘 "
              << std::chrono::duration_cast<std::chrono::seconds>(total_time % 1min).count() << " 秒"
              << std::endl;
    
    // 使用時間點
    auto now = std::chrono::system_clock::now();
    auto future = now + 3h + 30min;
    auto duration = future - now;
    
    std::cout << "時間差: " 
              << std::chrono::duration_cast<std::chrono::hours>(duration).count() << " 小時 "
              << std::chrono::duration_cast<std::chrono::minutes>(duration % 1h).count() << " 分鐘"
              << std::endl;
    
    // 將時間點轉換為日曆時間
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto future_time_t = std::chrono::system_clock::to_time_t(future);
    
    std::cout << "當前時間: " << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "未來時間: " << std::put_time(std::localtime(&future_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    
    return 0;
}
```

### 二進制字面量

```cpp
#include <iostream>
#include <bitset>
#include <string>

// 二進制字面量運算符
constexpr unsigned long long operator"" _b(const char* str, size_t len) {
    unsigned long long result = 0;
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '0' || str[i] == '1') {
            result = (result << 1) | (str[i] - '0');
        }
    }
    return result;
}

// 十六進制字面量運算符
constexpr unsigned long long operator"" _hex(const char* str, size_t len) {
    unsigned long long result = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        int digit;
        
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            continue;  // 忽略無效字符
        }
        
        result = (result << 4) | digit;
    }
    return result;
}

// 八進制字面量運算符
constexpr unsigned long long operator"" _oct(const char* str, size_t len) {
    unsigned long long result = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (c >= '0' && c <= '7') {
            result = (result << 3) | (c - '0');
        }
    }
    return result;
}

int main() {
    // 使用二進制字面量
    auto binary = "1010"_b;  // 10 in decimal
    std::cout << "二進制 1010 = " << binary << " (十進制)" << std::endl;
    
    // 使用十六進制字面量
    auto hex = "1A"_hex;  // 26 in decimal
    std::cout << "十六進制 1A = " << hex << " (十進制)" << std::endl;
    
    // 使用八進制字面量
    auto octal = "17"_oct;  // 15 in decimal
    std::cout << "八進制 17 = " << octal << " (十進制)" << std::endl;
    
    // 位操作
    auto result1 = "1010"_b | "0101"_b;
    auto result2 = "1010"_b & "0110"_b;
    auto result3 = "1010"_b ^ "0101"_b;
    auto result4 = ~"1010"_b & 0xF;  // 限制為 4 位
    
    std::cout << "\n位操作:" << std::endl;
    std::cout << "1010 | 0101 = " << std::bitset<4>(result1) << " (" << result1 << ")" << std::endl;
    std::cout << "1010 & 0110 = " << std::bitset<4>(result2) << " (" << result2 << ")" << std::endl;
    std::cout << "1010 ^ 0101 = " << std::bitset<4>(result3) << " (" << result3 << ")" << std::endl;
    std::cout << "~1010 (4位) = " << std::bitset<4>(result4) << " (" << result4 << ")" << std::endl;
    
    // 混合使用不同進制
    auto mixed = "10"_b + "F"_hex + "10"_oct;
    std::cout << "\n混合計算: 0b10 + 0xF + 010 = " << mixed << std::endl;
    std::cout << "二進制表示: " << std::bitset<8>(mixed) << std::endl;
    
    // 使用更長的二進制數
    auto long_binary = "1101001010111"_b;
    std::cout << "\n長二進制: " << long_binary << std::endl;
    std::cout << "二進制表示: " << std::bitset<16>(long_binary) << std::endl;
    
    return 0;
}
```

## 使用情境

### 適合使用使用者定義字面量的情況

1. **物理單位和度量**：當需要表示具有單位的值時
   ```cpp
   // 物理單位
   auto distance = 5.0_km;
   auto mass = 75.0_kg;
   auto time = 3.5_h;
   auto force = 10.0_N;
   
   // 計算功率
   auto power = (force * distance) / time;
   ```

2. **特殊數值類型**：如分數、複數、大整數等
   ```cpp
   // 分數
   auto fraction = 3_frac / 4_frac;
   
   // 大整數
   auto big_num = "123456789012345678901234567890"_bigint;
   ```

3. **特定領域語言 (DSL)**：為特定領域創建更自然的語法
   ```cpp
   // SQL 查詢構建
   auto query = "SELECT * FROM users"_sql.where("age > 18"_expr).orderBy("name"_field);
   
   // 正則表達式
   auto pattern = "\\d+"_regex;
   ```

4. **自定義字符串類型**：如特殊編碼或處理的字符串
   ```cpp
   // JSON 字符串
   auto json = "{\"name\": \"John\", \"age\": 30}"_json;
   
   // 路徑
   auto file_path = "/home/user/documents/file.txt"_path;
   ```

5. **時間和日期**：表示時間點、持續時間或日期
   ```cpp
   // 時間點
   auto deadline = "2023-12-31"_date + 5_days;
   
   // 時間間隔
   auto timeout = 30_s + 500_ms;
   ```

### 不適合使用使用者定義字面量的情況

1. **過度使用導致混淆**：當有太多自定義字面量時
   ```cpp
   // 過多的字面量可能導致混淆
   auto result = 5_a + 10_b * 3_c - 2_d / 4_e;  // 難以理解每個後綴的含義
   ```

2. **複雜的轉換邏輯**：當字面量轉換邏輯過於複雜時
   ```cpp
   // 複雜的轉換邏輯應該使用命名函數
   auto complex_result = "complex_string_with_special_format"_complex;  // 不直觀
   
   // 更好的方式
   auto better_result = parse_complex_format("complex_string_with_special_format");
   ```

3. **可能與標準庫或第三方庫衝突**：當後綴可能與其他庫衝突時
   ```cpp
   // 可能與標準庫衝突
   auto str = "hello"_s;  // 如果多個庫定義了 _s 後綴
   ```

4. **需要大量參數的操作**：當操作需要多個參數時
   ```cpp
   // 不適合使用字面量的複雜操作
   auto matrix = "1,2,3;4,5,6;7,8,9"_matrix;  // 難以擴展和自定義
   
   //