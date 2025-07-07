# C++11 decltype 關鍵字

## 核心概念

`decltype` 是 C++11 引入的一個運算子，其功能是產生一個實體（entity）或表達式（expression）的「宣告類型」（declared type），而不會實際對該表達式進行求值。這與 `sizeof` 運算子相似，其運算元也是在未求值上下文中處理。

與 `auto` 基於初始化式推導類型不同，`decltype` 直接作用於表達式本身，保留了引用和 CV 限定符（const 和 volatile）。它的主要應用場景是在泛型程式碼中，尤其是模板程式庫，其中類型依賴於模板參數，難以或無法預先確定。

`decltype` 的推導規則極其精確，但其行為根據表達式的形式而有所不同：

1. **「變數 decltype」**：如果表達式 e 是一個「未加括號的標識符表達式」（如變數名 x），或是一個「未加括號的類別成員存取」（如 a->x），那麼 `decltype(e)` 會產生該實體被宣告時的**確切類型**。

2. **「表達式 decltype」**：對於任何其他形式的表達式 e，包括被括號包圍的標識符如 (x)，其推導結果將取決於該表達式的**值類別（value category）**：
   - 若 e 是**左值（lvalue）** 且類型為 T，則 `decltype(e)` 的結果是 T&（左值引用）
   - 若 e 是**xvalue** 且類型為 T，則 `decltype(e)` 的結果是 T&&（右值引用）
   - 若 e 是**純右值（prvalue）** 且類型為 T，則 `decltype(e)` 的結果是 T

## 語法

```cpp
// 基本語法
decltype(expression) variable_name;

// 用於宣告變數
decltype(some_variable) another_variable;

// 用於函數返回類型
template<typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {
    return t + u;
}

// C++14 後可以直接用於函數返回類型
template<typename T, typename U>
decltype(auto) add(T t, U u) {
    return t + u;
}
```

## 與相似概念的比較

| 特性 | `decltype` | `auto` | `std::declval` |
|------|------------|--------|----------------|
| 推導來源 | 表達式本身 | 初始化表達式 | 類型本身 |
| 保留引用 | 是 | 否（除非使用 `auto&`） | 返回右值引用 |
| 保留 const/volatile | 是 | 否（除非顯式加上） | 不適用 |
| 可用於未求值上下文 | 是 | 否 | 僅限未求值上下文 |
| 值類別敏感 | 是 | 否 | 不適用 |

## 使用範例

### 基本用法與值類別處理

```cpp
#include <iostream>
#include <utility> // for std::move
#include <type_traits> // for std::is_same_v

struct A { double x; };
const A* a_ptr = new A();

int i = 42;
int& r = i;
int&& rr = 42;

int get_val() { return 0; }
int& get_lval_ref() { return i; }
int&& get_rval_ref() { return 42; }

int main() {
    // 規則 1: 未加括號的標識符表達式
    decltype(i) var1; // var1 的類型是 int
    decltype(r) var2 = i; // var2 的類型是 int&
    decltype(rr) var3 = 42; // var3 的類型是 int&&
    decltype(a_ptr->x) var4; // var4 的類型是 double (類別成員存取)

    // 規則 2: 基於值類別的表達式推導
    // (i) 是左值表達式
    decltype((i)) lvalue_ref = i; // lvalue_ref 的類型是 int&

    // std::move(i) 是 xvalue 表達式
    decltype(std::move(i)) xvalue_ref = 42; // xvalue_ref 的類型是 int&&

    // get_val() 是 prvalue 表達式
    decltype(get_val()) prvalue_val; // prvalue_val 的類型是 int

    // 函數呼叫表達式
    decltype(get_lval_ref()) lval_func_ref = i; // 返回左值引用的函數，表達式為左值，類型為 int&
    decltype(get_rval_ref()) rval_func_ref = 42; // 返回右值引用的函數，表達式為xvalue，類型為 int&&

    std::cout << "decltype(i) is int: " << std::is_same_v<decltype(var1), int> << std::endl;
    std::cout << "decltype((i)) is int&: " << std::is_same_v<decltype(lvalue_ref), int&> << std::endl;
    std::cout << "decltype(std::move(i)) is int&&: " << std::is_same_v<decltype(xvalue_ref), int&&> << std::endl;
    std::cout << "decltype(get_val()) is int: " << std::is_same_v<decltype(prvalue_val), int> << std::endl;

    delete a_ptr;
    return 0;
}
```

### 在模板中使用 decltype

```cpp
#include <iostream>
#include <vector>

// 使用 decltype 推導返回類型
template<typename Container, typename Index>
auto access(Container& c, Index i) -> decltype(c[i]) {
    return c[i];
}

int main() {
    // 對於 vector<int>，access 返回 int&
    std::vector<int> v = {1, 2, 3};
    decltype(access(v, 0)) ref = access(v, 0); // ref 的類型是 int&
    ref = 42;
    std::cout << "v[0] = " << v[0] << std::endl; // 輸出 42
    
    // 對於 int[3]，access 返回 int&
    int arr[3] = {1, 2, 3};
    decltype(access(arr, 0)) arr_ref = access(arr, 0); // arr_ref 的類型是 int&
    arr_ref = 100;
    std::cout << "arr[0] = " << arr[0] << std::endl; // 輸出 100
    
    return 0;
}
```

## 使用情境

### 適合使用 decltype 的情況

1. **泛型程式設計中推導返回類型**：
   ```cpp
   template<typename T, typename U>
   auto multiply(T t, U u) -> decltype(t * u) {
       return t * u;
   }
   ```

2. **SFINAE（替換失敗並非錯誤）技術**：
   ```cpp
   template<typename T>
   auto has_member(T& t) -> decltype(t.member, void()) {
       // 只有當 T 有名為 member 的成員時，此函數才會被實例化
   }
   ```

3. **保留表達式的值類別**：
   ```cpp
   template<typename T>
   decltype(auto) forward_value(T&& t) {
       return std::forward<T>(t); // 完美轉發，保留值類別
   }
   ```

4. **元編程中檢查表達式有效性**：
   ```cpp
   template<typename T, typename U>
   struct can_add {
       template<typename V, typename W>
       static auto test(int) -> decltype(std::declval<V>() + std::declval<W>(), std::true_type{});
       
       template<typename, typename>
       static auto test(...) -> std::false_type;
       
       static constexpr bool value = decltype(test<T, U>(0))::value;
   };
   ```

### 不適合使用 decltype 的情況

1. **簡單變數宣告**：對於簡單類型，直接使用類型名稱更清晰。
   ```cpp
   int x = 10;
   decltype(x) y = 20; // 不如直接寫 int y = 20;
   ```

2. **可能導致程式碼難以理解的場合**：
   ```cpp
   decltype((std::declval<T>().foo())) result; // 過於複雜，難以理解
   ```

3. **當 auto 足夠時**：
   ```cpp
   auto it = container.begin(); // 比 decltype(container)::iterator it = container.begin(); 更簡潔
   ```

### 最佳實踐

- 理解 `decltype(x)` 和 `decltype((x))` 的區別，前者是變數的宣告類型，後者是左值表達式，通常是引用類型
- 在泛型程式設計中，使用 `decltype` 來準確推導表達式的類型，特別是當需要保留引用和 CV 限定符時
- 與 `std::declval` 結合使用，可以在未求值上下文中「假裝」有一個特定類型的物件
- 在 C++14 後，考慮使用 `decltype(auto)` 來簡化返回類型推導，特別是在轉發函數中
- 避免過度使用 `decltype`，這可能會使代碼變得難以理解