# C++11 Lambda 表達式

## 核心概念

Lambda 表達式是 C++11 引入的一個重要特性，它允許開發者在程式碼中直接定義匿名函數物件（anonymous function objects）。Lambda 表達式本質上是一個可調用的程式碼塊，可以被視為一個內聯的函數定義，能夠捕獲其定義處的變數，並在需要時被調用。

Lambda 表達式的引入極大地簡化了函數物件的創建和使用，特別是在需要將行為作為參數傳遞給演算法的情境中。在 C++11 之前，這通常需要定義一個函數或函數物件類別，而 Lambda 表達式則提供了一種更簡潔、更直接的方式來實現相同的功能。

Lambda 表達式的基本語法包括：
- **捕獲子句**（capture clause）：指定哪些外部變數可以在 Lambda 函數體內使用，以及如何捕獲它們（按值或按引用）
- **參數列表**（可選）：與普通函數的參數列表相同
- **可變規格**（可選）：使用 `mutable` 關鍵字允許修改按值捕獲的變數
- **異常規格**（可選）：指定 Lambda 可能拋出的異常
- **返回類型**（可選）：使用尾置返回類型語法 `-> type` 指定返回類型
- **函數體**：包含 Lambda 的實際代碼

Lambda 表達式在編譯時會被轉換為一個匿名的函數物件（closure），其類型是一個只有編譯器知道的獨特類型。這個函數物件包含捕獲的變數和一個重載的函數調用運算符 `operator()`。

## 語法

```cpp
// 基本語法
[capture-list](parameters) -> return_type { body }

// 捕獲子句變體
[]        // 不捕獲任何變數
[=]       // 按值捕獲所有外部變數
[&]       // 按引用捕獲所有外部變數
[x, &y]   // 按值捕獲 x，按引用捕獲 y
[=, &z]   // 按值捕獲所有外部變數，但 z 按引用捕獲
[&, x]    // 按引用捕獲所有外部變數，但 x 按值捕獲
[this]    // 捕獲 this 指針
[*this]   // 按值捕獲當前物件（C++17）

// 可變 Lambda
[x](int y) mutable { x += y; return x; }

// 泛型 Lambda（C++14）
[](auto x, auto y) { return x + y; }

// 帶初始化捕獲的 Lambda（C++14）
[x = 10, y = 20]() { return x + y; }
```

## 與相似概念的比較

| 特性 | Lambda 表達式 | 函數指針 | 函數物件 | std::function |
|------|--------------|----------|----------|---------------|
| 語法簡潔性 | 高 | 中 | 低 | 中 |
| 可捕獲外部變數 | 是 | 否 | 是 | 是（通過綁定器） |
| 內聯潛力 | 高 | 低 | 高 | 低 |
| 類型安全 | 高 | 高 | 高 | 高 |
| 存儲開銷 | 低 | 極低 | 低 | 高 |
| 運行時開銷 | 低 | 低 | 低 | 中 |
| 可作為模板參數 | 否（C++20 前） | 是 | 是 | 否 |
| 可修改狀態 | 是（通過 mutable） | 否 | 是 | 是 |

## 使用範例

### 基本用法

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // 簡單的 Lambda 表達式
    auto greet = []() { std::cout << "Hello, Lambda!" << std::endl; };
    greet();  // 輸出: Hello, Lambda!
    
    // 帶參數的 Lambda
    auto add = [](int a, int b) { return a + b; };
    std::cout << "5 + 3 = " << add(5, 3) << std::endl;  // 輸出: 5 + 3 = 8
    
    // 使用 Lambda 與標準庫演算法
    std::vector<int> numbers = {1, -2, 3, -4, 5};
    
    // 計算正數的數量
    int positive_count = std::count_if(numbers.begin(), numbers.end(), 
                                      [](int n) { return n > 0; });
    std::cout << "Positive numbers: " << positive_count << std::endl;  // 輸出: Positive numbers: 3
    
    // 使用 Lambda 排序
    std::vector<std::pair<std::string, int>> people = {
        {"Alice", 25}, {"Bob", 30}, {"Charlie", 20}
    };
    
    // 按年齡排序
    std::sort(people.begin(), people.end(), 
             [](const auto& a, const auto& b) { return a.second < b.second; });
    
    std::cout << "People sorted by age:" << std::endl;
    for (const auto& person : people) {
        std::cout << person.first << ": " << person.second << std::endl;
    }
    // 輸出:
    // Charlie: 20
    // Alice: 25
    // Bob: 30
    
    return 0;
}
```

### 變數捕獲

```cpp
#include <iostream>
#include <functional>

int main() {
    // 按值捕獲
    int x = 10;
    auto value_capture = [x]() { return x; };
    x = 20;  // 不影響 Lambda 中捕獲的值
    std::cout << "Value capture: " << value_capture() << std::endl;  // 輸出: Value capture: 10
    
    // 按引用捕獲
    int y = 10;
    auto ref_capture = [&y]() { return y; };
    y = 20;  // 影響 Lambda 中捕獲的引用
    std::cout << "Reference capture: " << ref_capture() << std::endl;  // 輸出: Reference capture: 20
    
    // 混合捕獲
    int a = 1, b = 2;
    auto mixed_capture = [a, &b]() { return a + b; };
    b = 3;  // 只影響 b 的捕獲
    std::cout << "Mixed capture: " << mixed_capture() << std::endl;  // 輸出: Mixed capture: 4
    
    // 捕獲所有變數
    int c = 30, d = 40;
    auto capture_all_by_value = [=]() { return c + d; };
    auto capture_all_by_ref = [&]() { return c + d; };
    
    c = 50;
    d = 60;
    
    std::cout << "Capture all by value: " << capture_all_by_value() << std::endl;  // 輸出: Capture all by value: 70
    std::cout << "Capture all by reference: " << capture_all_by_ref() << std::endl;  // 輸出: Capture all by reference: 110
    
    // 可變 Lambda
    int counter = 0;
    auto increment = [counter]() mutable { return ++counter; };
    std::cout << "First call: " << increment() << std::endl;  // 輸出: First call: 1
    std::cout << "Second call: " << increment() << std::endl;  // 輸出: Second call: 2
    std::cout << "Original counter: " << counter << std::endl;  // 輸出: Original counter: 0
    
    return 0;
}
```

### 進階用法

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

// 遞迴 Lambda（C++14）
int factorial(int n) {
    // 使用 std::function 來允許遞迴
    std::function<int(int)> fact = [&fact](int n) {
        return n <= 1 ? 1 : n * fact(n - 1);
    };
    return fact(n);
}

// 泛型 Lambda（C++14）
template<typename Container>
void print_container(const Container& c) {
    std::for_each(c.begin(), c.end(), [](const auto& item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;
}

// 初始化捕獲（C++14）
auto create_counter(int start) {
    return [count = start]() mutable { return count++; };
}

// Lambda 作為函數返回值
auto make_adder(int base) {
    return [base](int value) { return base + value; };
}

int main() {
    // 遞迴 Lambda
    std::cout << "5! = " << factorial(5) << std::endl;  // 輸出: 5! = 120
    
    // 泛型 Lambda
    std::vector<int> nums = {1, 2, 3, 4, 5};
    std::vector<std::string> words = {"Hello", "Lambda", "World"};
    
    std::cout << "Numbers: ";
    print_container(nums);  // 輸出: Numbers: 1 2 3 4 5
    
    std::cout << "Words: ";
    print_container(words);  // 輸出: Words: Hello Lambda World
    
    // 初始化捕獲
    auto counter = create_counter(10);
    std::cout << "Counter: " << counter() << ", " << counter() << ", " << counter() << std::endl;
    // 輸出: Counter: 10, 11, 12
    
    // Lambda 作為函數返回值
    auto add5 = make_adder(5);
    std::cout << "10 + 5 = " << add5(10) << std::endl;  // 輸出: 10 + 5 = 15
    
    // 捕獲智能指針
    auto ptr = std::make_shared<int>(100);
    auto lambda = [ptr]() { return *ptr; };
    std::cout << "Value via shared_ptr: " << lambda() << std::endl;  // 輸出: Value via shared_ptr: 100
    
    // 捕獲 this 指針
    struct Counter {
        int value = 0;
        
        auto get_incrementer() {
            return [this]() { return ++value; };
        }
        
        auto get_value_reader() {
            return [this]() { return value; };
        }
    };
    
    Counter c;
    auto inc = c.get_incrementer();
    auto reader = c.get_value_reader();
    
    inc();
    inc();
    std::cout << "Counter value: " << reader() << std::endl;  // 輸出: Counter value: 2
    
    return 0;
}
```

## 使用情境

### 適合使用 Lambda 的情況

1. **與標準庫演算法一起使用**：
   ```cpp
   std::vector<int> v = {1, 2, 3, 4, 5};
   std::transform(v.begin(), v.end(), v.begin(), 
                 [](int n) { return n * n; });
   ```

2. **簡短的一次性函數**：
   ```cpp
   auto button_callback = [](Button* btn, Event* evt) {
       std::cout << "Button " << btn->id() << " clicked!" << std::endl;
   };
   button->set_on_click(button_callback);
   ```

3. **需要捕獲局部變數的函數**：
   ```cpp
   void process_data(const std::vector<int>& data, int threshold) {
       auto is_above_threshold = [threshold](int value) {
           return value > threshold;
       };
       int count = std::count_if(data.begin(), data.end(), is_above_threshold);
   }
   ```

4. **延遲執行或異步操作**：
   ```cpp
   void async_operation() {
       int local_var = 42;
       std::thread worker([local_var]() {
           std::this_thread::sleep_for(std::chrono::seconds(1));
           std::cout << "Async operation with value: " << local_var << std::endl;
       });
       worker.detach();
   }
   ```

5. **自定義排序或比較邏輯**：
   ```cpp
   std::sort(objects.begin(), objects.end(), 
            [](const Object& a, const Object& b) {
                if (a.priority() != b.priority())
                    return a.priority() > b.priority();
                return a.timestamp() < b.timestamp();
            });
   ```

6. **創建閉包（closure）**：
   ```cpp
   auto make_multiplier(int factor) {
       return [factor](int value) { return factor * value; };
   }
   
   auto double_it = make_multiplier(2);
   auto triple_it = make_multiplier(3);
   ```

### 不適合使用 Lambda 的情況

1. **複雜的多行函數**：
   ```cpp
   // 過於複雜，應該定義為命名函數
   [](const Data& data) {
       // 數十行複雜邏輯...
   }
   ```

2. **需要在多處重用的函數**：
   ```cpp
   // 如果需要在多處使用相同邏輯，定義命名函數更好
   bool is_valid_data(const Data& data) {
       return data.size() > 0 && data.is_initialized();
   }
   ```

3. **需要遞迴且性能敏感的場景**（C++14 前）：
   ```cpp
   // 在 C++14 前，Lambda 遞迴需要 std::function，可能有性能開銷
   std::function<int(int)> fibonacci = [&fibonacci](int n) {
       return n <= 1 ? n : fibonacci(n-1) + fibonacci(n-2);
   };
   ```

4. **需要指定模板參數的場景**（C++20 前）：
   ```cpp
   // C++20 前，Lambda 不能有模板參數列表
   template<typename T>
   void process(const T& value) { /* ... */ }
   
   // 而不是
   auto process = [](const auto& value) { /* ... */ };
   ```

5. **需要作為非類型模板參數的場景**（C++20 前）：
   ```cpp
   // C++20 前，Lambda 不能用作非類型模板參數
   template<auto Comparator>
   class SortedContainer { /* ... */ };
   
   // 不能這樣使用
   SortedContainer<[](int a, int b) { return a < b; }> container;
   ```

### 最佳實踐

- 保持 Lambda 表達式簡短且專注於單一任務
- 明智地選擇捕獲模式，避免不必要的捕獲
- 優先使用按值捕獲而非按引用捕獲，除非確實需要修改外部變數
- 注意 Lambda 捕獲的生命週期問題，特別是捕獲引用或指針時
- 使用 `auto` 來存儲 Lambda 表達式，而不是 `std::function`（除非需要特定功能）
- 當 Lambda 需要修改捕獲的變數時，使用 `mutable` 關鍵字
- 在 C++14 及以後，優先使用初始化捕獲而非簡單捕獲
- 在 C++14 及以後，利用泛型 Lambda 提高代碼的通用性
- 在 C++17 及以後，考慮使用 `[*this]` 捕獲當前物件的副本，避免 `this` 指針失效問題
- 在 C++20 中，利用 Lambda 模板和 Lambda 作為非類型模板參數的新功能