# C++11 初始化列表

## 核心概念

初始化列表（`std::initializer_list<T>`）是 C++11 引入的一個輕量級模板類，用於表示同類型對象的數組。它主要用於支持統一初始化語法（uniform initialization syntax），使得初始化各種容器和對象變得更加一致和直觀。

初始化列表的核心特點包括：

1. **輕量級設計**：`std::initializer_list` 只是對底層數組的一個視圖（view），不擁有其元素，因此複製一個初始化列表是一個輕量級操作。

2. **只讀訪問**：初始化列表中的元素是只讀的，不能通過初始化列表修改它們。

3. **連續存儲**：初始化列表中的元素在內存中是連續存儲的，可以像數組一樣高效訪問。

4. **統一初始化語法**：通過大括號 `{}` 語法創建，使得初始化語法更加一致。

5. **廣泛支持**：標準庫容器和自定義類型都可以支持初始化列表。

初始化列表解決了 C++ 中初始化語法不一致的問題，並提供了一種更直觀的方式來初始化容器和自定義類型。它與統一初始化語法（大括號初始化）一起，成為現代 C++ 程式設計中的重要工具。

## 語法

```cpp
// 創建初始化列表
std::initializer_list<int> numbers = {1, 2, 3, 4, 5};

// 直接使用大括號初始化容器
std::vector<int> vec = {1, 2, 3, 4, 5};

// 函數參數中使用初始化列表
void process(std::initializer_list<int> values);
process({1, 2, 3});  // 直接傳遞大括號初始化列表

// 自定義類支持初始化列表
class MyContainer {
public:
    MyContainer(std::initializer_list<int> list);
};

MyContainer container = {1, 2, 3, 4};  // 使用初始化列表構造

// 遍歷初始化列表
for (int value : {1, 2, 3, 4, 5}) {
    // 處理每個值
}

// 空的初始化列表
std::initializer_list<std::string> empty = {};
```

## 與相似概念的比較

| 特性 | std::initializer_list | 傳統數組 | std::vector | std::array |
|------|----------------------|----------|------------|------------|
| 大小固定 | 是 | 是 | 否 | 是 |
| 動態分配 | 否（通常） | 否 | 是 | 否 |
| 可修改元素 | 否 | 是 | 是 | 是 |
| 知道自身大小 | 是 | 否 | 是 | 是 |
| 支持迭代器 | 是 | 否（直接） | 是 | 是 |
| 初始化語法 | `{1, 2, 3}` | `{1, 2, 3}` 或 `[3] = {1, 2, 3}` | `{1, 2, 3}` 或 構造函數 | `{1, 2, 3}` |
| 主要用途 | 初始化和參數傳遞 | 固定大小數據 | 動態大小集合 | 固定大小集合 |
| 開銷 | 極小 | 無 | 中等 | 極小 |
| C++11 引入 | 是 | 否 | 否 | 是 |

## 使用範例

### 基本用法

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>

// 接受初始化列表的函數
void print_values(std::initializer_list<int> values) {
    std::cout << "Values: ";
    for (int val : values) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    // 使用初始化列表初始化容器
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    std::cout << "Vector elements: ";
    for (int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // 直接傳遞初始化列表給函數
    print_values({10, 20, 30, 40, 50});
    
    // 創建並使用初始化列表變量
    std::initializer_list<std::string> names = {"Alice", "Bob", "Charlie"};
    
    std::cout << "Names: ";
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // 使用初始化列表進行賦值
    numbers = {100, 200, 300};
    
    std::cout << "Updated vector: ";
    for (int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 自定義類支持初始化列表

```cpp
#include <iostream>
#include <vector>
#include <initializer_list>
#include <string>

// 自定義容器類支持初始化列表
class IntContainer {
private:
    std::vector<int> data;
    
public:
    // 初始化列表構造函數
    IntContainer(std::initializer_list<int> list) : data(list) {
        std::cout << "Constructed with " << list.size() << " elements\n";
    }
    
    // 使用初始化列表的賦值運算符
    IntContainer& operator=(std::initializer_list<int> list) {
        data = list;
        std::cout << "Assigned " << list.size() << " elements\n";
        return *this;
    }
    
    // 添加元素的方法也可以使用初始化列表
    void add_elements(std::initializer_list<int> list) {
        data.insert(data.end(), list.begin(), list.end());
        std::cout << "Added " << list.size() << " elements\n";
    }
    
    // 顯示內容
    void display() const {
        std::cout << "Container contents: ";
        for (int value : data) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    
    // 獲取元素數量
    size_t size() const {
        return data.size();
    }
};

int main() {
    // 使用初始化列表構造
    IntContainer container = {1, 2, 3, 4, 5};
    container.display();
    
    // 使用初始化列表賦值
    container = {10, 20, 30};
    container.display();
    
    // 使用初始化列表添加元素
    container.add_elements({40, 50, 60});
    container.display();
    
    // 空的初始化列表
    IntContainer empty = {};
    std::cout << "Empty container size: " << empty.size() << std::endl;
    
    return 0;
}
```

### 嵌套初始化列表

```cpp
#include <iostream>
#include <vector>
#include <initializer_list>

// 處理二維數據的類
class Matrix {
private:
    std::vector<std::vector<int>> data;
    
public:
    // 使用嵌套初始化列表構造
    Matrix(std::initializer_list<std::initializer_list<int>> lists) {
        for (auto& row : lists) {
            data.push_back(std::vector<int>(row));
        }
    }
    
    // 顯示矩陣內容
    void display() const {
        std::cout << "Matrix contents:\n";
        for (const auto& row : data) {
            for (int value : row) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    // 使用嵌套初始化列表創建矩陣
    Matrix matrix = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    matrix.display();
    
    // 標準庫容器也支持嵌套初始化列表
    std::vector<std::vector<int>> nested_vector = {
        {10, 20},
        {30, 40, 50},
        {60}
    };
    
    std::cout << "\nNested vector contents:\n";
    for (const auto& row : nested_vector) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}
```

### 初始化列表與自動類型推導

```cpp
#include <iostream>
#include <initializer_list>
#include <typeinfo>
#include <string>

// 模板函數處理不同類型的初始化列表
template<typename T>
void process_list(std::initializer_list<T> list) {
    std::cout << "Processing list of " << typeid(T).name() << " with "
              << list.size() << " elements: ";
    
    for (const auto& item : list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    // 自動類型推導
    auto int_list = {1, 2, 3, 4, 5};  // std::initializer_list<int>
    auto double_list = {1.1, 2.2, 3.3};  // std::initializer_list<double>
    auto string_list = {"hello", "world"};  // std::initializer_list<const char*>
    
    // 顯示類型和內容
    std::cout << "int_list type: " << typeid(int_list).name() << std::endl;
    std::cout << "double_list type: " << typeid(double_list).name() << std::endl;
    std::cout << "string_list type: " << typeid(string_list).name() << std::endl;
    
    // 使用模板函數處理不同類型的列表
    process_list(int_list);
    process_list(double_list);
    process_list(string_list);
    
    // 直接傳遞初始化列表給模板函數
    process_list({100, 200, 300});
    process_list({3.14, 2.71});
    process_list<std::string>({"C++", "11", "initializer", "list"});
    
    return 0;
}
```

## 使用情境

### 適合使用初始化列表的情況

1. **容器初始化**：為容器提供初始元素：
   ```cpp
   std::vector<int> numbers = {1, 2, 3, 4, 5};
   std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
   ```

2. **函數參數**：接受可變數量的同類型參數：
   ```cpp
   void process_values(std::initializer_list<int> values) {
       for (int val : values) {
           // 處理每個值
       }
   }
   
   process_values({1, 2, 3});
   process_values({10, 20, 30, 40, 50});
   ```

3. **自定義類型構造函數**：使類支持列表初始化：
   ```cpp
   class CustomContainer {
   public:
       CustomContainer(std::initializer_list<int> values) {
           // 使用 values 初始化容器
       }
   };
   
   CustomContainer container = {1, 2, 3, 4};
   ```

4. **返回多個值**：函數返回多個同類型值：
   ```cpp
   std::initializer_list<int> get_factors(int n) {
       // 計算 n 的因數
       if (n == 12) return {1, 2, 3, 4, 6, 12};
       if (n == 15) return {1, 3, 5, 15};
       return {};
   }
   ```

5. **配置選項**：提供配置或選項列表：
   ```cpp
   void configure_app(std::initializer_list<std::string> options) {
       for (const auto& option : options) {
           apply_option(option);
       }
   }
   
   configure_app({"--verbose", "--log-level=debug", "--output=file.txt"});
   ```

### 不適合使用初始化列表的情況

1. **需要修改元素**：初始化列表中的元素是只讀的：
   ```cpp
   std::initializer_list<int> values = {1, 2, 3};
   // values.begin()[0] = 10;  // 錯誤：不能修改元素
   
   // 替代方案：使用 vector
   std::vector<int> mutable_values = {1, 2, 3};
   mutable_values[0] = 10;  // 正確
   ```

2. **異構數據**：初始化列表要求所有元素類型相同：
   ```cpp
   // 不能直接使用初始化列表存儲不同類型
   // std::initializer_list<???> mixed = {1, "hello", 3.14};  // 錯誤
   
   // 替代方案：使用 tuple 或 variant
   auto mixed = std::make_tuple(1, "hello", 3.14);
   ```

3. **動態添加元素**：初始化列表大小固定：
   ```cpp
   std::initializer_list<int> fixed = {1, 2, 3};
   // fixed.push_back(4);  // 錯誤：不支持添加元素
   
   // 替代方案：使用 vector
   std::vector<int> dynamic = {1, 2, 3};
   dynamic.push_back(4);  // 正確
   ```

4. **大量數據**：初始化列表通常存儲在編譯期分配的內存中：
   ```cpp
   // 不適合非常大的數據集
   // std::initializer_list<int> huge = {1, 2, ..., 1000000};  // 可能導致編譯問題
   
   // 替代方案：動態生成
   std::vector<int> large_data;
   for (int i = 1; i <= 1000000; ++i) {
       large_data.push_back(i);
   }
   ```

5. **需要長期存儲**：初始化列表只是底層數組的視圖：
   ```cpp
   std::initializer_list<int> get_values() {
       return {1, 2, 3};  // 危險：返回的列表引用可能已失效的數據
   }
   
   // 替代方案：返回 vector
   std::vector<int> get_values_safe() {
       return {1, 2, 3};  // 安全：返回的是完整的副本
   }
   ```

### 最佳實踐

- 優先使用初始化列表來初始化容器和自定義類型
- 為自定義容器類型提供接受初始化列表的構造函數
- 理解初始化列表是只讀的，不能修改其元素
- 注意初始化列表的生命週期問題，不要返回函數局部初始化列表
- 使用 `auto` 與初始化列表結合時，注意類型會被推導為 `std::initializer_list<T>`
- 對於需要修改或動態調整大小的集合，使用 `std::vector` 而非初始化列表
- 利用初始化列表實現更清晰、更直觀的 API 設計
- 在處理初始化列表時，優先使用範圍迴圈而非迭代器，代碼更簡潔
- 理解大括號初始化的規則，特別是與自動類型推導結合使用時
- 避免在性能關鍵的內部循環中頻繁創建和銷毀初始化列表