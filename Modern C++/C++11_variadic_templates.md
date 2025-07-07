# C++11 可變參數模板

## 核心概念

可變參數模板（Variadic Templates）是 C++11 引入的一項強大特性，它允許模板定義接受任意數量、任意類型的參數。這項功能極大地增強了 C++ 泛型程式設計的能力，使得編寫高度通用且類型安全的程式碼變得更加容易。

在可變參數模板出現之前，C++ 程式設計師必須為不同數量的參數重載函數或使用其他變通方法（如使用容器、初始化列表或 C 風格的可變參數函數）。這些方法要麼缺乏類型安全性，要麼需要大量的重複程式碼。可變參數模板解決了這些問題，提供了一種優雅且類型安全的方式來處理不確定數量和類型的參數。

可變參數模板的核心機制包括：

1. **模板參數包（Template Parameter Pack）**：使用省略號（`...`）語法表示的一組零個或多個模板參數。
2. **函數參數包（Function Parameter Pack）**：對應於模板參數包的函數參數。
3. **包展開（Pack Expansion）**：使用省略號將參數包中的元素展開為單獨的元素。
4. **遞迴模式（Recursive Pattern）**：通常通過遞迴方式處理參數包，每次處理一個參數，然後遞迴處理剩餘參數。

可變參數模板的應用非常廣泛，從簡單的函數轉發到複雜的元編程技術，它都扮演著關鍵角色。標準庫中的許多組件，如 `std::tuple`、`std::make_shared`、`std::make_unique` 和 `std::thread` 的建構函數等，都利用了可變參數模板來實現。

## 關鍵術語解釋

在可變參數模板中，有幾個關鍵的術語和語法元素需要理解：

1. **模板參數包宣告（Template Parameter Pack Declaration）**：
   - `typename...` 是模板參數包的宣告，表示這是一個可以包含零個或多個類型的參數包。
   - 例如：`template<typename... Args>` 宣告了一個名為 `Args` 的類型參數包。
   - 也可以使用 `class...` 代替 `typename...`，效果相同。

2. **參數包名稱（Parameter Pack Name）**：
   - `Args...` 是參數包的名稱，代表一組類型。
   - 在模板定義中，它是一個佔位符，代表所有傳入的類型。
   - 例如：在 `template<typename... Args>` 中，`Args` 是參數包名稱。

3. **函數參數包（Function Parameter Pack）**：
   - 在函數參數列表中，`Args... args` 宣告了一個函數參數包。
   - `Args...` 是類型參數包，`args` 是對應的函數參數包名稱。
   - 例如：`void func(Args... args)` 中，`args` 是一個可以接受任意數量參數的函數參數包。

4. **包展開（Pack Expansion）**：
   - `args...` 是包展開操作，將參數包中的元素展開為逗號分隔的列表。
   - 展開發生在特定的上下文中，如函數調用、初始化列表等。
   - 例如：`func(args...)` 會展開為 `func(arg1, arg2, arg3, ...)`。

5. **包大小（Pack Size）**：
   - `sizeof...(Args)` 或 `sizeof...(args)` 返回參數包中的元素數量。
   - 這是一個編譯期常量表達式。
   - 例如：`static_assert(sizeof...(Args) > 0, "At least one argument required");`

```cpp
// 完整示例
template<typename... Types>  // Types 是類型參數包名稱，typename... 是參數包宣告
class Tuple {};

template<typename... Args>   // Args 是類型參數包名稱
void func(Args... args) {    // args 是函數參數包名稱
    another_func(args...);   // args... 是包展開，展開為逗號分隔的參數列表
    std::cout << "Number of arguments: " << sizeof...(args) << std::endl;  // 獲取包大小
}
```

## 語法

```cpp
// 可變參數模板函數
template<typename... Args>
return_type function_name(Args... args);

// 可變參數模板類別
template<typename... Types>
class class_name;

// 參數包展開
template<typename... Args>
void function(Args... args) {
    another_function(args...);  // 展開為 another_function(arg1, arg2, ...)
}

// 遞迴方式處理參數包
// 基本情況（終止條件）
template<typename T>
void recursive_function(T value) {
    // 處理最後一個參數
}

// 遞迴情況
template<typename T, typename... Args>
void recursive_function(T first, Args... rest) {
    // 處理第一個參數
    recursive_function(rest...);  // 遞迴處理剩餘參數
}
```

## 與相似概念的比較

| 特性 | 可變參數模板 | 函數重載 | C 風格可變參數 | 初始化列表 |
|------|--------------|----------|----------------|------------|
| 類型安全 | 是 | 是 | 否 | 是（但類型單一） |
| 參數數量 | 任意 | 固定（每個重載） | 任意 | 任意 |
| 參數類型 | 任意 | 固定（每個重載） | 不檢查 | 單一類型 |
| 編譯期處理 | 是 | 是 | 否 | 部分 |
| 程式碼量 | 少 | 多（需多個重載） | 少 | 中等 |
| 實現複雜度 | 中到高 | 低 | 低 | 低 |
| 執行效率 | 高 | 高 | 中 | 高 |

## 使用範例

### 基本用法

```cpp
#include <iostream>
#include <string>

// 可變參數模板函數
template<typename... Args>
void print_all(Args... args) {
    // 使用折疊表達式（C++17）
    ((std::cout << args << " "), ...);
    std::cout << std::endl;
}

// 在 C++11/14 中，使用遞迴方式實現
// 基本情況
void print_all_recursive() {
    std::cout << std::endl;
}

// 遞迴情況
template<typename T, typename... Args>
void print_all_recursive(T first, Args... rest) {
    std::cout << first << " ";
    print_all_recursive(rest...);
}

int main() {
    // 使用不同類型和數量的參數呼叫
    print_all(1, 2.5, "Hello", 'c');  // 輸出: 1 2.5 Hello c
    print_all(100, "World");          // 輸出: 100 World
    print_all(3.14159);               // 輸出: 3.14159
    print_all();                      // 輸出: (空行)
    
    // 使用遞迴版本
    print_all_recursive(1, 2.5, "Hello", 'c');  // 輸出: 1 2.5 Hello c
    
    return 0;
}
```

### 參數包展開

```cpp
#include <iostream>
#include <tuple>
#include <string>

// 在函數參數中展開參數包
template<typename... Args>
void forward_to_function(Args... args) {
    // 將所有參數轉發給另一個函數
    std::cout << sizeof...(args) << " arguments forwarded" << std::endl;
    another_function(args...);
}

void another_function(int a, double b, const std::string& c) {
    std::cout << "Received: " << a << ", " << b << ", " << c << std::endl;
}

// 在模板參數中展開參數包
template<typename... Types>
class Tuple {
    // 實現...
};

// 在繼承中展開參數包
template<typename... Bases>
class Derived : public Bases... {
public:
    // 使用 C++17 的折疊表達式
    Derived(const Bases&... bases) : Bases(bases)... {}
};

// 在 using 宣告中展開參數包
template<typename... Types>
using AliasTemplate = std::tuple<int, Types...>;

int main() {
    // 函數參數展開
    forward_to_function(42, 3.14, std::string("Hello"));
    
    // 模板參數展開
    AliasTemplate<char, double, std::string> t;  // 等同於 std::tuple<int, char, double, std::string>
    
    return 0;
}
```

### 遞迴處理參數包

```cpp
#include <iostream>
#include <typeinfo>
#include <string>

// 遞迴方式計算參數和
// 基本情況
template<typename T>
T sum(T value) {
    return value;
}

// 遞迴情況
template<typename T, typename... Args>
T sum(T first, Args... rest) {
    return first + sum(static_cast<T>(rest)...);
}

// 遞迴方式印出參數類型
// 基本情況
void print_types() {
    std::cout << std::endl;
}

// 遞迴情況
template<typename T, typename... Args>
void print_types(T first, Args... rest) {
    std::cout << typeid(first).name() << " ";
    print_types(rest...);
}

int main() {
    // 計算不同類型參數的和
    std::cout << "Sum: " << sum(1, 2, 3, 4, 5) << std::endl;  // 輸出: Sum: 15
    std::cout << "Sum: " << sum(1.1, 2.2, 3.3) << std::endl;  // 輸出: Sum: 6.6
    
    // 印出參數類型
    std::cout << "Types: ";
    print_types(1, 2.5, "Hello", 'c');  // 輸出依賴於編譯器實現
    
    return 0;
}
```

### 實現 tuple 類別

```cpp
#include <iostream>
#include <string>

// 遞迴方式實現 tuple
// 基本情況：空 tuple
template<typename... Types>
class MyTuple {};

// 遞迴情況：至少有一個元素的 tuple
template<typename Head, typename... Tail>
class MyTuple<Head, Tail...> {
private:
    Head head;
    MyTuple<Tail...> tail;
    
public:
    MyTuple() = default;
    
    MyTuple(const Head& h, const Tail&... t)
        : head(h), tail(t...) {}
    
    Head& get_head() { return head; }
    const Head& get_head() const { return head; }
    
    MyTuple<Tail...>& get_tail() { return tail; }
    const MyTuple<Tail...>& get_tail() const { return tail; }
};

// 輔助函數：獲取 tuple 中的第 N 個元素
// 基本情況：獲取第 0 個元素
template<size_t N, typename Head, typename... Tail>
struct TupleElement {
    using type = typename TupleElement<N-1, Tail...>::type;
    
    static type& get(MyTuple<Head, Tail...>& tuple) {
        return TupleElement<N-1, Tail...>::get(tuple.get_tail());
    }
};

// 特化：獲取第 0 個元素
template<typename Head, typename... Tail>
struct TupleElement<0, Head, Tail...> {
    using type = Head;
    
    static type& get(MyTuple<Head, Tail...>& tuple) {
        return tuple.get_head();
    }
};

// 使用者友好的 get 函數
template<size_t N, typename... Types>
typename TupleElement<N, Types...>::type& get(MyTuple<Types...>& tuple) {
    return TupleElement<N, Types...>::get(tuple);
}

int main() {
    // 創建一個 tuple
    MyTuple<int, double, std::string> t(42, 3.14, "Hello");
    
    // 獲取元素
    std::cout << "Element 0: " << get<0>(t) << std::endl;  // 輸出: Element 0: 42
    std::cout << "Element 1: " << get<1>(t) << std::endl;  // 輸出: Element 1: 3.14
    std::cout << "Element 2: " << get<2>(t) << std::endl;  // 輸出: Element 2: Hello
    
    // 修改元素
    get<0>(t) = 100;
    std::cout << "Modified element 0: " << get<0>(t) << std::endl;  // 輸出: Modified element 0: 100
    
    return 0;
}
```

### 完美轉發與可變參數模板

```cpp
#include <iostream>
#include <utility>
#include <string>
#include <memory>

// 使用可變參數模板和完美轉發實現通用工廠函數
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Person {
private:
    std::string name;
    int age;
    
public:
    Person(const std::string& n, int a) : name(n), age(a) {
        std::cout << "Person constructed: " << name << ", " << age << std::endl;
    }
    
    void display() const {
        std::cout << "Person: " << name << ", " << age << std::endl;
    }
};

// 使用可變參數模板實現通用包裝函數
template<typename F, typename... Args>
auto invoke(F&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
    std::cout << "Invoking function with " << sizeof...(args) << " arguments" << std::endl;
    return func(std::forward<Args>(args)...);
}

int add(int a, int b) {
    return a + b;
}

int main() {
    // 使用自定義的 make_unique
    auto person = make_unique<Person>("Alice", 30);
    person->display();
    
    // 使用通用包裝函數
    int result = invoke(add, 5, 7);
    std::cout << "Result: " << result << std::endl;  // 輸出: Result: 12
    
    // 使用 lambda 和包裝函數
    auto greeting = invoke(
        [](const std::string& name, int times) {
            std::string result;
            for (int i = 0; i < times; ++i) {
                result += "Hello, " + name + "! ";
            }
            return result;
        },
        "Bob", 3
    );
    
    std::cout << greeting << std::endl;
    // 輸出: Hello, Bob! Hello, Bob! Hello, Bob!
    
    return 0;
}
```

## 使用情境

### 適合使用可變參數模板的情況

1. **通用工廠函數**：創建物件的函數，將參數完美轉發給建構函數：
   ```cpp
   template<typename T, typename... Args>
   std::shared_ptr<T> make_shared(Args&&... args) {
       // 實現...
   }
   ```

2. **容器和資料結構**：實現能存儲不同類型元素的容器：
   ```cpp
   template<typename... Types>
   class Tuple {
       // 實現...
   };
   ```

3. **函數包裝和轉發**：包裝函數調用或將參數轉發給其他函數：
   ```cpp
   template<typename F, typename... Args>
   auto invoke(F&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
       // 前處理...
       auto result = func(std::forward<Args>(args)...);
       // 後處理...
       return result;
   }
   ```

4. **變參數函數的類型安全替代**：替代 C 風格的 `printf` 等函數：
   ```cpp
   template<typename... Args>
   void safe_printf(const char* format, Args... args) {
       // 類型安全的實現...
   }
   ```

5. **事件處理和回調系統**：實現能處理不同類型和數量參數的事件系統：
   ```cpp
   template<typename... Args>
   class Event {
   public:
       using Handler = std::function<void(Args...)>;
       
       void subscribe(Handler handler) {
           handlers.push_back(handler);
       }
       
       void trigger(Args... args) {
           for (auto& handler : handlers) {
               handler(args...);
           }
       }
       
   private:
       std::vector<Handler> handlers;
   };
   ```

6. **元編程技術**：實現編譯期計算和類型操作：
   ```cpp
   template<typename... Types>
   struct TypeList {
       static constexpr size_t size = sizeof...(Types);
       
       template<typename T>
       using Append = TypeList<Types..., T>;
   };
   ```

### 不適合使用可變參數模板的情況

1. **參數數量和類型固定的情況**：
   ```cpp
   // 過度使用可變參數模板
   template<typename... Args>
   void fixed_function(Args... args) {
       // 總是期望兩個 int 參數
   }
   
   // 更好的方式
   void fixed_function(int a, int b) {
       // 更清晰的接口
   }
   ```

2. **需要運行時決定參數數量的情況**：
   ```cpp
   // 可變參數模板在編譯期確定參數
   // 對於運行時可變參數，考慮使用容器或其他方法
   void runtime_variable_args(const std::vector<int>& values) {
       // 處理運行時確定的參數列表
   }
   ```

3. **可能導致程式碼膨脹的情況**：
   ```cpp
   // 可能導致大量模板實例化
   template<typename... Args>
   void heavy_function(Args... args) {
       // 複雜實現...
   }
   
   // 考慮使用類型擦除或其他技術
   void type_erased_function(const std::vector<std::any>& args) {
       // 使用類型擦除處理不同類型
   }
   ```

4. **需要簡單接口的公共 API**：
   ```cpp
   // 對於公共 API，可能更偏好明確的接口
   class PublicAPI {
   public:
       void simple_method(int a, const std::string& b);
       
   private:
       // 在內部實現中使用可變參數模板
       template<typename... Args>
       void internal_helper(Args&&... args);
   };
   ```

### 最佳實踐

- 使用 `sizeof...(args)` 獲取參數包中的參數數量
- 在 C++17 中，優先使用折疊表達式處理參數包，而不是遞迴模式
- 結合 `std::forward` 實現完美轉發，保留參數的值類別
- 為可變參數模板提供清晰的文檔，說明預期的參數類型和數量
- 考慮提供非模板的重載版本，以處理常見情況，提高編譯速度
- 使用 `static_assert` 和 SFINAE 技術對參數類型進行約束
- 注意可能的程式碼膨脹問題，特別是在處理大量不同類型組合時
- 在處理同質參數包（相同類型的參數）時，考慮使用初始化列表或容器作為替代
- 理解參數包展開的上下文和規則，確保正確使用
- 在 C++14 及以後，利用通用引用（`auto&&`）和 `decltype(auto)` 簡化可變參數模板的使用