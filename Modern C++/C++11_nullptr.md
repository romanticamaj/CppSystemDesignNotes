# C++11 nullptr 關鍵字

## 核心概念

`nullptr` 是 C++11 引入的一個關鍵字，用於表示空指標（null pointer）常量。它是一個 `std::nullptr_t` 類型的右值常量，專門設計用來替代傳統 C 和早期 C++ 中使用的 `NULL` 宏和字面值 `0`。

在 C++11 之前，空指標通常使用 `NULL`（通常被定義為 `0` 或 `(void*)0`）或直接使用整數 `0` 來表示。這種做法存在一些問題，特別是在函數重載和模板推導的情境中，因為 `0` 和 `NULL` 首先被視為整數類型，而非指標類型。

`nullptr` 解決了這些問題，它：
- 是一個明確的指標類型，而非整數類型
- 可以隱式轉換為任何指標類型
- 不能隱式轉換為整數類型（除了 `bool`）
- 在模板類型推導和函數重載解析中被正確處理為指標類型

`nullptr` 的引入使得 C++ 程式碼在處理指標時更加類型安全，並且消除了許多與 `NULL` 和 `0` 相關的歧義。

## 語法

```cpp
// 宣告指標並初始化為 nullptr
int* ptr = nullptr;

// 檢查指標是否為空
if (ptr == nullptr) { /* ... */ }
// 或更簡潔地
if (!ptr) { /* ... */ }

// 在函數參數中使用
void func(int* ptr = nullptr);

// 作為函數返回值
int* getPtr() {
    return nullptr;
}
```

## 與相似概念的比較

| 特性 | `nullptr` | `NULL` | 整數 `0` |
|------|-----------|--------|----------|
| 類型 | `std::nullptr_t` | 通常是 `int` 或 `void*` | `int` |
| 可轉換為指標類型 | 是 | 是 | 是 |
| 可轉換為整數類型 | 否（除了 `bool`） | 是 | 是 |
| 在函數重載中的行為 | 選擇指標參數版本 | 選擇整數參數版本 | 選擇整數參數版本 |
| 在模板類型推導中的行為 | 推導為指標類型 | 推導為整數類型 | 推導為整數類型 |
| 標準 | C++11 及以後 | C 和 C++ | C 和 C++ |

## 使用範例

### 基本用法

```cpp
#include <iostream>

void demo() {
    // 宣告並初始化指標
    int* p1 = nullptr;
    double* p2 = nullptr;
    void* p3 = nullptr;
    
    // 檢查指標是否為空
    if (p1 == nullptr) {
        std::cout << "p1 is null" << std::endl;
    }
    
    // 簡潔的檢查方式
    if (!p2) {
        std::cout << "p2 is null" << std::endl;
    }
    
    // 指標比較
    if (p1 == p3) {
        std::cout << "p1 and p3 are both null" << std::endl;
    }
}
```

### 函數重載解析

```cpp
#include <iostream>

// 函數重載
void func(int i) {
    std::cout << "func(int) called with " << i << std::endl;
}

void func(int* p) {
    std::cout << "func(int*) called with " << (p ? "non-null pointer" : "nullptr") << std::endl;
}

int main() {
    // 使用 nullptr，會呼叫 func(int*)
    func(nullptr);
    
    // 使用 NULL 或 0，會呼叫 func(int)
    func(NULL);  // 在大多數實現中，NULL 是 0
    func(0);
    
    return 0;
}
```

### 模板類型推導

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void check_type(T param) {
    std::cout << "T is pointer type: " << std::is_pointer<T>::value << std::endl;
}

int main() {
    // 使用 nullptr，T 被推導為 std::nullptr_t
    check_type(nullptr);  // 輸出: T is pointer type: 0
    
    // 使用 NULL 或 0，T 被推導為 int
    check_type(NULL);     // 輸出: T is pointer type: 0
    check_type(0);        // 輸出: T is pointer type: 0
    
    // 明確指定為指標類型
    check_type(static_cast<int*>(nullptr));  // 輸出: T is pointer type: 1
    
    return 0;
}
```

### 與 std::nullptr_t 類型互動

```cpp
#include <iostream>
#include <cstddef>  // 為 std::nullptr_t

// 接受 std::nullptr_t 參數的函數
void process(std::nullptr_t) {
    std::cout << "Processing nullptr" << std::endl;
}

// 模板特化
template<typename T>
struct Handler {
    static void handle() {
        std::cout << "General handler" << std::endl;
    }
};

template<>
struct Handler<std::nullptr_t> {
    static void handle() {
        std::cout << "nullptr handler" << std::endl;
    }
};

int main() {
    // 呼叫接受 nullptr_t 的函數
    process(nullptr);
    
    // 使用模板特化
    Handler<int>::handle();        // 輸出: General handler
    Handler<nullptr_t>::handle();  // 輸出: nullptr handler
    
    return 0;
}
```

## 使用情境

### 適合使用 nullptr 的情況

1. **初始化指標變數**：
   ```cpp
   int* ptr = nullptr;  // 清晰地表明這是一個空指標
   ```

2. **函數參數默認值**：
   ```cpp
   void process(const std::string& str, int* output = nullptr);
   ```

3. **函數重載**：當有同名函數接受指標和非指標參數時：
   ```cpp
   void handle(int value);
   void handle(int* ptr);
   
   handle(nullptr);  // 明確呼叫 handle(int*)
   ```

4. **條件表達式**：
   ```cpp
   return condition ? pointer : nullptr;
   ```

5. **智能指標初始化**：
   ```cpp
   std::shared_ptr<int> sp = nullptr;
   std::unique_ptr<int> up = nullptr;
   ```

### 不適合使用 nullptr 的情況

1. **需要整數 0 值的場合**：
   ```cpp
   int count = 0;  // 不要使用 nullptr
   ```

2. **布林上下文中可能造成混淆的情況**：
   ```cpp
   bool flag = false;  // 比 bool flag = nullptr; 更清晰
   ```

3. **與舊 API 互動時可能需要顯式轉換**：
   ```cpp
   // 某些 C API 可能期望 NULL 或 0
   legacy_c_function((void*)0);  // 有時可能比 legacy_c_function(nullptr) 更安全
   ```

### 最佳實踐

- 始終使用 `nullptr` 而非 `NULL` 或 `0` 來表示空指標
- 避免在非指標上下文中使用 `nullptr`
- 在條件表達式中，可以直接使用指標變數（如 `if(ptr)` 而非 `if(ptr != nullptr)`）
- 理解 `nullptr` 在函數重載解析中的行為
- 在模板程式碼中，注意 `nullptr` 的類型是 `std::nullptr_t`，而非指標類型
- 在移植舊程式碼時，系統性地將 `NULL` 和 `0`（用作指標）替換為 `nullptr`
- 使用靜態分析工具來檢測和修正不當使用 `NULL` 和 `0` 的情況