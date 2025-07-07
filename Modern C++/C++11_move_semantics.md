# C++11 移動語義與右值引用

## 核心概念

移動語義（Move Semantics）是 C++11 引入的一個重要特性，它通過右值引用（Rvalue References）實現，旨在顯著提高程式效能，特別是在處理資源管理（如動態記憶體、檔案控制代碼等）時。移動語義允許資源的「轉移」而非「複製」，從而避免了不必要的資源複製開銷。

在 C++11 之前，當物件被傳遞或賦值時，通常會發生深度複製（deep copy），這在處理大型資料結構時可能導致顯著的效能損失。移動語義允許「竊取」即將被銷毀的物件（右值）的資源，而不是複製它們，從而提高效率。

右值引用是實現移動語義的關鍵機制。它使用 `&&` 語法，能夠綁定到右值（臨時物件、即將被銷毀的物件）。與傳統的左值引用（使用 `&` 語法，綁定到具名物件）不同，右值引用使得函數能夠區分參數是臨時物件還是持久物件，從而選擇最佳的處理策略。

移動語義的核心組件包括：
- **右值引用**（`T&&`）：一種新的引用類型，可以綁定到右值
- **移動建構函數**：接受右值引用參數的建構函數，從參數「竊取」資源
- **移動賦值運算子**：接受右值引用參數的賦值運算子，從參數「竊取」資源
- **std::move**：一個將左值轉換為右值引用的工具函數
- **完美轉發**：通過 `std::forward` 在泛型程式碼中保留參數的值類別（左值/右值）

## 語法

```cpp
// 右值引用
Type&& var_name = expression;

// 移動建構函數
class MyClass {
public:
    MyClass(MyClass&& other) noexcept;
};

// 移動賦值運算子
class MyClass {
public:
    MyClass& operator=(MyClass&& other) noexcept;
};

// std::move 使用
#include <utility>
Type&& rref = std::move(lvalue);

// 完美轉發
template<typename T>
void forward_function(T&& param) {
    some_function(std::forward<T>(param));
}
```

## 與相似概念的比較

| 特性 | 移動語義 | 複製語義 | 引用語義 | 智能指針 |
|------|----------|----------|----------|----------|
| 資源所有權 | 轉移 | 複製 | 共享 | 可變（依類型） |
| 效率 | 高 | 低 | 高 | 中到高 |
| 安全性 | 中（需小心使用） | 高 | 低（生命週期問題） | 高 |
| 適用場景 | 資源轉移 | 需要獨立副本 | 避免複製開銷 | 自動資源管理 |
| 實現複雜度 | 中 | 低 | 低 | 高 |
| 對原物件的影響 | 可能使其處於有效但未指定狀態 | 無影響 | 無影響 | 依類型而定 |

## 使用範例

### 基本右值引用與 std::move

```cpp
#include <iostream>
#include <string>
#include <utility>

void reference_demo() {
    std::string s1 = "Hello";
    
    // 左值引用
    std::string& ref1 = s1;  // 正確：ref1 引用 s1
    // std::string& ref2 = std::string("World");  // 錯誤：不能將左值引用綁定到右值
    
    // 右值引用
    // std::string&& rref1 = s1;  // 錯誤：不能將右值引用綁定到左值
    std::string&& rref2 = std::string("World");  // 正確：rref2 引用臨時物件
    
    // 使用 std::move 將左值轉換為右值引用
    std::string&& rref3 = std::move(s1);  // 正確：std::move 將 s1 轉換為右值引用
    
    std::cout << "rref2: " << rref2 << std::endl;  // 輸出: World
    std::cout << "rref3: " << rref3 << std::endl;  // 輸出: Hello
    
    // 注意：s1 現在處於有效但未指定的狀態
    std::cout << "s1 after move: " << s1 << std::endl;  // 輸出可能為空或原值，取決於實現
}

int main() {
    reference_demo();
    return 0;
}
```

### 移動建構函數與移動賦值運算子

```cpp
#include <iostream>
#include <utility>

class Resource {
public:
    int* data;
    size_t size;
    
    // 建構函數
    Resource(size_t sz) : size(sz) {
        std::cout << "Allocating " << size << " integers\n";
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<int>(i);
        }
    }
    
    // 解構函數
    ~Resource() {
        std::cout << "Deallocating resource\n";
        delete[] data;
    }
    
    // 複製建構函數
    Resource(const Resource& other) : size(other.size) {
        std::cout << "Copy constructor called\n";
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }
    
    // 複製賦值運算子
    Resource& operator=(const Resource& other) {
        std::cout << "Copy assignment operator called\n";
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = new int[size];
            for (size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }
    
    // 移動建構函數
    Resource(Resource&& other) noexcept : data(other.data), size(other.size) {
        std::cout << "Move constructor called\n";
        // 重要：將 other 的指針設為 nullptr，避免雙重釋放
        other.data = nullptr;
        other.size = 0;
    }
    
    // 移動賦值運算子
    Resource& operator=(Resource&& other) noexcept {
        std::cout << "Move assignment operator called\n";
        if (this != &other) {
            delete[] data;
            
            // 轉移資源所有權
            data = other.data;
            size = other.size;
            
            // 重要：將 other 的指針設為 nullptr，避免雙重釋放
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
    
    // 顯示資源內容
    void display() const {
        if (data) {
            std::cout << "Resource data: ";
            for (size_t i = 0; i < size && i < 5; ++i) {
                std::cout << data[i] << " ";
            }
            if (size > 5) std::cout << "...";
            std::cout << std::endl;
        } else {
            std::cout << "Resource is empty (moved from)\n";
        }
    }
};

int main() {
    // 建立資源
    Resource r1(1000);
    r1.display();
    
    std::cout << "\n--- Copy semantics ---\n";
    Resource r2 = r1;  // 複製建構函數
    r1.display();
    r2.display();
    
    std::cout << "\n--- Move semantics ---\n";
    Resource r3 = std::move(r2);  // 移動建構函數
    r2.display();  // r2 現在應該是空的
    r3.display();
    
    std::cout << "\n--- Move assignment ---\n";
    Resource r4(10);
    r4 = std::move(r3);  // 移動賦值運算子
    r3.display();  // r3 現在應該是空的
    r4.display();
    
    return 0;
}
```

### 完美轉發與通用引用

```cpp
#include <iostream>
#include <utility>
#include <string>

// 處理左值的函數
void process(const std::string& str) {
    std::cout << "Processing lvalue: " << str << std::endl;
}

// 處理右值的函數
void process(std::string&& str) {
    std::cout << "Processing rvalue: " << str << std::endl;
}

// 使用完美轉發的包裝函數
template<typename T>
void wrapper(T&& arg) {
    // std::forward 保留參數的值類別（左值/右值）
    process(std::forward<T>(arg));
}

int main() {
    std::string hello = "Hello";
    
    // 傳遞左值
    wrapper(hello);  // 呼叫 process(const std::string&)
    
    // 傳遞右值
    wrapper(std::string("World"));  // 呼叫 process(std::string&&)
    wrapper(std::move(hello));      // 呼叫 process(std::string&&)
    
    return 0;
}
```

### 移動語義在標準庫中的應用

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <utility>

int main() {
    // 使用移動語義優化 vector 操作
    std::vector<std::string> vec;
    
    std::string str = "This is a long string that would be expensive to copy";
    
    std::cout << "Before push_back: " << str << std::endl;
    
    // 使用移動語義，避免複製
    vec.push_back(std::move(str));
    
    std::cout << "After push_back: " << str << std::endl;  // str 可能為空
    std::cout << "Vector element: " << vec[0] << std::endl;
    
    // 使用 emplace_back 直接在容器中建構物件
    vec.emplace_back("Directly constructed in the vector");
    
    std::cout << "Second element: " << vec[1] << std::endl;
    
    // 移動整個容器
    std::vector<std::string> vec2 = std::move(vec);
    
    std::cout << "vec size after move: " << vec.size() << std::endl;  // 可能為 0
    std::cout << "vec2 size: " << vec2.size() << std::endl;
    
    return 0;
}
```

## 使用情境

### 適合使用移動語義的情況

1. **資源轉移**：當需要將資源（如動態記憶體、檔案控制代碼、網路連接等）從一個物件轉移到另一個物件時：
   ```cpp
   std::unique_ptr<Resource> create_resource() {
       return std::unique_ptr<Resource>(new Resource());
   }
   
   auto resource = create_resource();  // 資源所有權從函數返回值轉移到 resource
   ```

2. **容器操作**：當向容器中插入大型物件或從容器中取出物件時：
   ```cpp
   std::vector<LargeObject> objects;
   LargeObject obj = create_large_object();
   objects.push_back(std::move(obj));  // 移動而非複製
   ```

3. **實現移動專用類型**：當設計不應被複製但可以被移動的類型時：
   ```cpp
   class MoveOnlyType {
   public:
       MoveOnlyType(MoveOnlyType&& other) noexcept = default;
       MoveOnlyType& operator=(MoveOnlyType&& other) noexcept = default;
       
       // 刪除複製操作
       MoveOnlyType(const MoveOnlyType&) = delete;
       MoveOnlyType& operator=(const MoveOnlyType&) = delete;
   };
   ```

4. **返回大型物件**：當函數需要返回大型物件時：
   ```cpp
   BigObject create_and_process() {
       BigObject obj;
       // 處理 obj...
       return obj;  // 編譯器可能使用返回值優化（RVO）或移動語義
   }
   ```

5. **實現交換操作**：高效實現物件交換：
   ```cpp
   template<typename T>
   void my_swap(T& a, T& b) {
       T temp = std::move(a);
       a = std::move(b);
       b = std::move(temp);
   }
   ```

6. **智能指針轉移所有權**：
   ```cpp
   std::unique_ptr<Resource> ptr1(new Resource());
   std::unique_ptr<Resource> ptr2 = std::move(ptr1);  // ptr1 現在為 nullptr
   ```

### 不適合使用移動語義的情況

1. **當物件在移動後仍需使用**：
   ```cpp
   std::string name = "John";
   process_name(std::move(name));  // 不好的做法，如果之後還需要使用 name
   std::cout << "Hello, " << name;  // name 的狀態不確定
   ```

2. **對於小型、簡單的物件**：
   ```cpp
   int a = 5;
   int b = std::move(a);  // 沒有效益，整數複製已經很快
   ```

3. **對於沒有實現移動語義的類型**：
   ```cpp
   LegacyClass obj1;
   LegacyClass obj2 = std::move(obj1);  // 如果 LegacyClass 沒有移動語義，這只會呼叫複製建構函數
   ```

4. **在 const 物件上**：
   ```cpp
   const std::string str = "Hello";
   std::string other = std::move(str);  // 仍然會複製，因為 str 是 const
   ```

5. **在多執行緒環境中共享物件**：
   ```cpp
   // 不好的做法：移動共享資源
   std::shared_ptr<Resource> shared_resource = get_shared_resource();
   thread_local_storage = std::move(shared_resource);  // 可能導致其他執行緒失去存取權
   ```

### 最佳實踐

- 實現類別的移動操作時，始終將它們標記為 `noexcept`，這對標準庫容器的效能至關重要
- 在移動操作中，確保將源物件置於有效但未指定的狀態，通常是將指針設為 `nullptr` 並重置其他成員
- 使用 `std::move` 時要小心，確保被移動的物件之後不再被使用，或僅以安全的方式使用
- 遵循「規則of five」：如果需要自定義任何一個複製/移動操作或解構函數，通常需要考慮所有五個特殊成員函數
- 優先使用編譯器生成的移動操作（`= default`），除非需要特殊行為
- 在泛型程式碼中，使用 `std::forward` 實現完美轉發，保留參數的值類別
- 考慮為不應被複製的類型刪除複製操作（`= delete`），只允許移動
- 理解移動語義與返回值優化（RVO）的關係，避免不必要的 `std::move` 用於函數返回值
- 使用 `std::move_if_noexcept` 在有異常安全性要求的情境中有條件地使用移動語義
- 在實現移動操作時，確保不拋出異常，或者提供強異常安全保證