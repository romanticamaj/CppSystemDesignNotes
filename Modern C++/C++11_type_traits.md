# C++11 Type Traits

## 核心概念

Type traits 是 C++11 引入的一個模板元編程庫，提供了一系列用於在編譯期間檢查、轉換和操作類型的工具。這個庫位於 `<type_traits>` 頭文件中，是現代 C++ 模板元編程的基礎設施。

Type traits 的主要功能包括：

1. **類型檢查**：判斷一個類型是否具有特定特性（如是否為整數、是否為指針等）
2. **類型轉換**：在編譯期間轉換類型的特性（如添加/移除 const、引用等）
3. **類型關係**：檢查類型之間的關係（如是否相同、是否可轉換等）
4. **類型屬性**：獲取類型的屬性（如是否有虛析構函數、是否可複製等）
5. **輔助模板**：提供一些通用的模板元編程工具（如條件類型選擇）

Type traits 庫的設計遵循一個一致的模式：大多數 traits 都是結構體模板，包含一個靜態常量 `value`（布爾值）或一個類型別名 `type`。這些 traits 可以在編譯期間被求值，從而實現靜態類型檢查和轉換。

C++11 的 type traits 庫為泛型編程和模板元編程提供了強大的工具，使得開發者可以編寫更加靈活、類型安全和高效的代碼。

## 語法

```cpp
// 類型檢查
std::is_integral<T>::value;         // 檢查 T 是否為整數類型
std::is_floating_point<T>::value;   // 檢查 T 是否為浮點類型
std::is_pointer<T>::value;          // 檢查 T 是否為指針類型
std::is_reference<T>::value;        // 檢查 T 是否為引用類型
std::is_const<T>::value;            // 檢查 T 是否為 const 類型
std::is_class<T>::value;            // 檢查 T 是否為類類型

// 類型轉換
typename std::remove_const<T>::type;       // 移除 T 的 const 限定符
typename std::add_pointer<T>::type;        // 將 T 轉換為指針類型
typename std::remove_reference<T>::type;   // 移除 T 的引用
typename std::decay<T>::type;              // 將 T 轉換為其衰減類型

// 類型關係
std::is_same<T, U>::value;                 // 檢查 T 和 U 是否為相同類型
std::is_base_of<Base, Derived>::value;     // 檢查 Base 是否為 Derived 的基類
std::is_convertible<From, To>::value;      // 檢查 From 是否可轉換為 To

// 類型屬性
std::is_trivial<T>::value;                 // 檢查 T 是否為平凡類型
std::is_standard_layout<T>::value;         // 檢查 T 是否為標準佈局類型
std::is_pod<T>::value;                     // 檢查 T 是否為 POD 類型
std::has_virtual_destructor<T>::value;     // 檢查 T 是否有虛析構函數

// 輔助模板
typename std::conditional<B, T, F>::type;  // 如果 B 為 true，則為 T，否則為 F
typename std::enable_if<B, T>::type;       // 如果 B 為 true，則為 T，否則 SFINAE 失敗

// C++14 中的 _t 和 _v 後綴（方便使用）
std::remove_const_t<T>;                    // 等同於 typename std::remove_const<T>::type
std::is_integral_v<T>;                     // 等同於 std::is_integral<T>::value
```

## 與相似概念的比較

| 特性 | Type Traits | SFINAE | 概念 (C++20) | 運行時類型信息 (RTTI) |
|------|-------------|--------|-------------|----------------------|
| 執行時機 | 編譯期 | 編譯期 | 編譯期 | 運行時 |
| 主要用途 | 類型檢查和轉換 | 函數重載解析 | 模板約束 | 動態類型識別 |
| 錯誤報告 | 較差（複雜的模板錯誤） | 較差（SFINAE 失敗） | 較好（明確的錯誤信息） | 運行時錯誤 |
| 性能影響 | 無（編譯期解析） | 無（編譯期解析） | 無（編譯期解析） | 有（運行時檢查） |
| 語法複雜度 | 中等 | 高 | 低 | 低 |
| 引入版本 | C++11 | C++98 | C++20 | C++98 |
| 可擴展性 | 有限（需要模板特化） | 有限（需要模板特化） | 高（可組合） | 有限（僅支持多態類型） |
| 典型用例 | 條件編譯、類型轉換 | 函數重載選擇 | 模板參數約束 | 動態類型轉換 |

## 使用範例

### 基本類型檢查

```cpp
#include <iostream>
#include <type_traits>
#include <string>
#include <vector>

template <typename T>
void print_type_info() {
    std::cout << "Type: " << typeid(T).name() << std::endl;
    std::cout << "Is integral: " << std::is_integral<T>::value << std::endl;
    std::cout << "Is floating point: " << std::is_floating_point<T>::value << std::endl;
    std::cout << "Is pointer: " << std::is_pointer<T>::value << std::endl;
    std::cout << "Is reference: " << std::is_reference<T>::value << std::endl;
    std::cout << "Is const: " << std::is_const<T>::value << std::endl;
    std::cout << "Is class: " << std::is_class<T>::value << std::endl;
    std::cout << "Is array: " << std::is_array<T>::value << std::endl;
    std::cout << "Is enum: " << std::is_enum<T>::value << std::endl;
    std::cout << "Is function: " << std::is_function<T>::value << std::endl;
    std::cout << "Is trivial: " << std::is_trivial<T>::value << std::endl;
    std::cout << "Is standard layout: " << std::is_standard_layout<T>::value << std::endl;
    std::cout << "Is POD: " << std::is_pod<T>::value << std::endl;
    std::cout << std::endl;
}

enum Color { Red, Green, Blue };

class SimpleClass {
public:
    int x;
    double y;
};

class ComplexClass {
public:
    ComplexClass() {}
    virtual ~ComplexClass() {}
    
private:
    int data;
};

int main() {
    std::cout << "=== Integer type ===" << std::endl;
    print_type_info<int>();
    
    std::cout << "=== Const integer type ===" << std::endl;
    print_type_info<const int>();
    
    std::cout << "=== Integer reference type ===" << std::endl;
    print_type_info<int&>();
    
    std::cout << "=== Integer pointer type ===" << std::endl;
    print_type_info<int*>();
    
    std::cout << "=== String type ===" << std::endl;
    print_type_info<std::string>();
    
    std::cout << "=== Vector type ===" << std::endl;
    print_type_info<std::vector<int>>();
    
    std::cout << "=== Enum type ===" << std::endl;
    print_type_info<Color>();
    
    std::cout << "=== Simple class type ===" << std::endl;
    print_type_info<SimpleClass>();
    
    std::cout << "=== Complex class type ===" << std::endl;
    print_type_info<ComplexClass>();
    
    std::cout << "=== Function type ===" << std::endl;
    print_type_info<void(int)>();
    
    std::cout << "=== Array type ===" << std::endl;
    print_type_info<int[10]>();
    
    return 0;
}
```

### 類型轉換

```cpp
#include <iostream>
#include <type_traits>
#include <string>

template <typename T>
void print_transformed_types() {
    std::cout << "Original type: " << typeid(T).name() << std::endl;
    
    // 添加/移除 const
    std::cout << "Add const: " << typeid(typename std::add_const<T>::type).name() << std::endl;
    std::cout << "Remove const: " << typeid(typename std::remove_const<T>::type).name() << std::endl;
    
    // 添加/移除 引用
    std::cout << "Add lvalue reference: " << typeid(typename std::add_lvalue_reference<T>::type).name() << std::endl;
    std::cout << "Add rvalue reference: " << typeid(typename std::add_rvalue_reference<T>::type).name() << std::endl;
    std::cout << "Remove reference: " << typeid(typename std::remove_reference<T>::type).name() << std::endl;
    
    // 添加/移除 指針
    std::cout << "Add pointer: " << typeid(typename std::add_pointer<T>::type).name() << std::endl;
    std::cout << "Remove pointer: " << typeid(typename std::remove_pointer<T>::type).name() << std::endl;
    
    // 複合轉換
    std::cout << "Decay: " << typeid(typename std::decay<T>::type).name() << std::endl;
    std::cout << "Remove CV: " << typeid(typename std::remove_cv<T>::type).name() << std::endl;
    
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Transform int ===" << std::endl;
    print_transformed_types<int>();
    
    std::cout << "=== Transform const int ===" << std::endl;
    print_transformed_types<const int>();
    
    std::cout << "=== Transform int& ===" << std::endl;
    print_transformed_types<int&>();
    
    std::cout << "=== Transform int* ===" << std::endl;
    print_transformed_types<int*>();
    
    std::cout << "=== Transform const int* ===" << std::endl;
    print_transformed_types<const int*>();
    
    std::cout << "=== Transform int* const ===" << std::endl;
    print_transformed_types<int* const>();
    
    std::cout << "=== Transform int[10] ===" << std::endl;
    print_transformed_types<int[10]>();
    
    std::cout << "=== Transform std::string ===" << std::endl;
    print_transformed_types<std::string>();
    
    return 0;
}
```

### 條件編譯和 SFINAE

```cpp
#include <iostream>
#include <type_traits>
#include <vector>
#include <list>

// 使用 enable_if 實現 SFINAE
// 只有當 T 是整數類型時，這個函數才會被選中
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
is_even(T value) {
    std::cout << "Integer version called" << std::endl;
    return value % 2 == 0;
}

// 對於非整數類型，這個函數會被選中
template <typename T>
typename std::enable_if<!std::is_integral<T>::value, bool>::type
is_even(T value) {
    std::cout << "Non-integer version called" << std::endl;
    return false; // 非整數類型沒有偶數的概念
}

// 使用 conditional 選擇類型
template <typename T>
struct container_selector {
    using type = typename std::conditional<
        std::is_pod<T>::value,
        std::vector<T>,    // 如果 T 是 POD 類型，使用 vector
        std::list<T>       // 否則使用 list
    >::type;
};

// 使用 void_t 檢測是否存在特定成員
template <typename, typename = void>
struct has_size_method : std::false_type {};

template <typename T>
struct has_size_method<T, 
    typename std::enable_if<
        !std::is_void<decltype(std::declval<T>().size())>::value
    >::type
> : std::true_type {};

// 使用 is_base_of 進行類型關係檢查
class Base {};
class Derived : public Base {};
class Unrelated {};

int main() {
    // 測試 enable_if
    std::cout << "is_even(10): " << is_even(10) << std::endl;
    std::cout << "is_even(3.14): " << is_even(3.14) << std::endl;
    
    // 測試 conditional
    container_selector<int>::type vec_int;
    container_selector<std::string>::type list_string;
    
    std::cout << "Container for int is vector: " 
              << std::is_same<decltype(vec_int), std::vector<int>>::value << std::endl;
    std::cout << "Container for string is list: " 
              << std::is_same<decltype(list_string), std::list<std::string>>::value << std::endl;
    
    // 測試 has_size_method
    std::cout << "Vector has size method: " << has_size_method<std::vector<int>>::value << std::endl;
    std::cout << "Int has size method: " << has_size_method<int>::value << std::endl;
    
    // 測試 is_base_of
    std::cout << "Base is base of Derived: " << std::is_base_of<Base, Derived>::value << std::endl;
    std::cout << "Derived is base of Base: " << std::is_base_of<Derived, Base>::value << std::endl;
    std::cout << "Base is base of Unrelated: " << std::is_base_of<Base, Unrelated>::value << std::endl;
    
    return 0;
}
```

## 使用情境

### 適合使用 Type Traits 的情況

1. **泛型編程**：當需要根據類型特性選擇不同的實現時
   ```cpp
   template <typename T>
   void process(T value) {
       if constexpr (std::is_integral<T>::value) {
           // 整數類型的處理邏輯
       } else if constexpr (std::is_floating_point<T>::value) {
           // 浮點類型的處理邏輯
       } else {
           // 其他類型的處理邏輯
       }
   }
   ```

2. **SFINAE 和函數重載**：使用 `enable_if` 控制函數重載的選擇
   ```cpp
   // 只有當 T 是算術類型時才啟用此重載
   template <typename T>
   typename std::enable_if<std::is_arithmetic<T>::value, T>::type
   square(T value) {
       return value * value;
   }
   
   // 對於非算術類型，提供一個編譯錯誤
   template <typename T>
   typename std::enable_if<!std::is_arithmetic<T>::value, T>::type
   square(T value) {
       static_assert(std::is_arithmetic<T>::value, "Type must be arithmetic");
       return T();
   }
   ```

3. **類型安全的轉換**：使用 type traits 確保類型轉換的安全性
   ```cpp
   template <typename To, typename From>
   To safe_cast(From value) {
       static_assert(std::is_convertible<From, To>::value, 
                    "Types are not convertible");
       return static_cast<To>(value);
   }
   ```

4. **優化特定類型的操作**：根據類型特性選擇最優實現
   ```cpp
   template <typename T>
   void copy_data(T* dest, const T* src, size_t count) {
       if constexpr (std::is_trivially_copyable<T>::value) {
           // 對於可平凡複製的類型，使用 memcpy
           std::memcpy(dest, src, count * sizeof(T));
       } else {
           // 對於其他類型，使用逐個元素的複製
           for (size_t i = 0; i < count; ++i) {
               dest[i] = src[i];
           }
       }
   }
   ```

### 不適合使用 Type Traits 的情況

1. **運行時類型檢查**：Type traits 是編譯期工具，不適用於運行時類型檢查
   ```cpp
   // 不推薦：運行時無法使用 type traits
   void process(const std::any& value) {
       if (std::is_integral<decltype(value)>::value) { // 錯誤：無法在運行時檢查
           // ...
       }
   }
   
   // 推薦：使用 RTTI 或 std::any 的 type() 方法
   void process(const std::any& value) {
       if (value.type() == typeid(int)) {
           // ...
       }
   }
   ```

2. **過度複雜的模板元編程**：過度使用 type traits 可能導致代碼難以理解和維護
   ```cpp
   // 不推薦：過度複雜的模板元編程
   template <typename T>
   struct complex_trait {
       using type = typename std::conditional<
           std::is_same<T, int>::value,
           typename std::conditional<
               std::is_const<T>::value,
               typename std::add_pointer<T>::type,
               typename std::add_lvalue_reference<T>::type
           >::type,
           typename std::decay<T>::type
       >::type;
   };
   
   // 推薦：分解為更小、更清晰的步驟
   template <typename T>
   struct complex_trait {
       using step1 = typename std::conditional<std::is_same<T, int>::value, T, typename std::decay<T>::type>::type;
       using step2 = typename std::conditional<std::is_const<T>::value, 
                                             typename std::add_pointer<step1>::type,
                                             typename std::add_lvalue_reference<step1>::type>::type;
       using type = step2;
   };
   ```

### 最佳實踐

1. **使用 C++14 的 `_t` 和 `_v` 後綴**：簡化代碼並提高可讀性
   ```cpp
   // C++11 風格
   typename std::remove_reference<T>::type x = value;
   if (std::is_integral<T>::value) { ... }
   
   // C++14 風格（更簡潔）
   std::remove_reference_t<T> x = value;
   if (std::is_integral_v<T>) { ... }
   ```

2. **使用 `if constexpr` (C++17) 簡化條件編譯**：比傳統的 SFINAE 更清晰
   ```cpp
   template <typename T>
   void process(T value) {
       if constexpr (std::is_integral_v<T>) {
           // 整數類型的處理邏輯
       } else if constexpr (std::is_floating_point_v<T>) {
           // 浮點類型的處理邏輯
       } else {
           // 其他類型的處理邏輯
       }
   }
   ```

3. **組合使用 type traits**：將多個 traits 組合使用以表達複雜的類型關係
   ```cpp
   template <typename T>
   using is_string_like = std::conjunction<
       std::negation<std::is_array<std::remove_cv_t<T>>>,
       std::is_convertible<T, std::string_view>
   >;
   ```

4. **創建自定義 type traits**：擴展標準庫以滿足特定需求
   ```cpp
   // 檢查類型是否有 to_string 方法
   template <typename T, typename = void>
   struct has_to_string : std::false_type {};
   
   template <typename T>
   struct has_to_string<T, 
       std::void_t<decltype(std::declval<T>().to_string())>
   > : std::true_type {};
   
   // C++14 風格的輔助變數模板
   template <typename T>
   inline constexpr bool has_to_string_v = has_to_string<T>::value;
   ```

5. **使用 `static_assert` 提供清晰的錯誤信息**：幫助開發者理解類型約束
   ```cpp
   template <typename T>
   void process_numeric(T value) {
       static_assert(std::is_arithmetic_v<T>, 
                    "process_numeric requires an arithmetic type");
       // ...
   }
   ```

6. **避免過度使用 type traits**：保持代碼的可讀性和可維護性
   ```cpp
   // 不推薦：過度使用 type traits 導致難以理解
   template <typename T>
   using complex_type = std::conditional_t<
       std::is_pointer_v<T>,
       std::add_lvalue_reference_t<std::remove_pointer_t<T>>,
       std::conditional_t<
           std::is_reference_v<T>,
           std::remove_reference_t<T>,
           std::add_const_t<T>
       >
   >;
   
   // 推薦：分解為更小、更清晰的步驟
   template <typename T>
   struct type_transformer {
       using step1 = std::conditional_t<std::is_pointer_v<T>, 
                                      std::remove_pointer_t<T>, 
                                      T>;
       using step2 = std::conditional_t<std::is_reference_v<step1>, 
                                      std::remove_reference_t<step1>, 
                                      step1>;
       using type = std::conditional_t<std::is_pointer_v<T>, 
                                     std::add_lvalue_reference_t<step2>, 
                                     std::add_const_t<step2>>;
   };
   ```

7. **考慮升級到 C++20 概念**：如果可能，使用更現代的語言特性
   ```cpp
   // C++11/14/17 使用 type traits
   template <typename T>
   typename std::enable_if<std::is_integral<T>::value, void>::type
   process(T value) {
       // ...
   }
   
   // C++20 使用概念
   template <std::integral T>
   void process(T value) {
       // ...
   }