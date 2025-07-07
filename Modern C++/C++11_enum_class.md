# C++11 強類型枚舉（enum class）

## 核心概念

強類型枚舉（Scoped Enumeration 或 enum class）是 C++11 引入的一種新型枚舉類型，旨在解決傳統 C++ 枚舉（現在稱為無作用域枚舉或 unscoped enum）的一些缺點。強類型枚舉提供了更好的類型安全性、作用域控制和前向宣告能力。

傳統枚舉存在以下問題：
1. **命名空間污染**：枚舉值直接位於包含枚舉的作用域中，容易與其他標識符衝突
2. **類型安全性不足**：枚舉值可以隱式轉換為整數，甚至可以與不同枚舉類型的值進行比較
3. **無法前向宣告**：必須在使用前完整定義枚舉類型
4. **底層類型不可控**：無法明確指定枚舉的底層整數類型

強類型枚舉通過以下方式解決這些問題：
1. **限定作用域**：枚舉值必須通過枚舉類型名稱限定（如 `Color::Red`）
2. **增強類型安全**：不會隱式轉換為整數，需要顯式轉換
3. **支持前向宣告**：可以在完整定義前宣告枚舉
4. **指定底層類型**：可以明確指定枚舉的底層整數類型

強類型枚舉的這些特性使得程式碼更加安全、清晰，並減少了錯誤的可能性，特別是在大型專案中。

## 語法

```cpp
// 基本語法
enum class EnumName {
    Value1,
    Value2,
    Value3
};

// 指定底層類型
enum class EnumName : underlying_type {
    Value1,
    Value2,
    Value3
};

// 前向宣告
enum class EnumName : underlying_type;

// 使用枚舉值
EnumName value = EnumName::Value1;

// 顯式轉換為整數
int numeric_value = static_cast<int>(EnumName::Value2);

// 顯式從整數轉換為枚舉
EnumName converted = static_cast<EnumName>(numeric_value);
```

## 與相似概念的比較

| 特性 | 強類型枚舉 (enum class) | 傳統枚舉 (unscoped enum) | #define 常量 | const 常量 |
|------|------------------------|------------------------|-------------|-----------|
| 作用域控制 | 良好（限定作用域） | 差（全局作用域） | 差（預處理器作用域） | 良好（變數作用域） |
| 類型安全性 | 高（不隱式轉換） | 低（隱式轉換為整數） | 無（純文本替換） | 中（依賴於變數類型） |
| 前向宣告 | 支持 | C++11 後有限支持 | 不適用 | 不需要 |
| 指定底層類型 | 支持 | C++11 後支持 | 不適用 | 直接指定 |
| 調試友好性 | 高 | 中 | 低 | 高 |
| 編譯時計算 | 支持 | 支持 | 有限支持 | 支持（constexpr） |
| 命名空間支持 | 內建 | 需要顯式使用命名空間 | 不支持 | 支持 |
| 可用於 switch 語句 | 是 | 是 | 是 | 有條件（constexpr） |
| 可用於模板參數 | 是 | 是 | 否 | 有條件（constexpr） |

## 使用範例

### 基本用法

```cpp
#include <iostream>

// 定義強類型枚舉
enum class Color {
    Red,
    Green,
    Blue,
    Yellow
};

// 定義帶有底層類型的強類型枚舉
enum class Status : uint8_t {
    OK = 0,
    Error = 1,
    Pending = 2,
    Unknown = 255
};

void print_color(Color color) {
    switch (color) {
        case Color::Red:
            std::cout << "Red" << std::endl;
            break;
        case Color::Green:
            std::cout << "Green" << std::endl;
            break;
        case Color::Blue:
            std::cout << "Blue" << std::endl;
            break;
        case Color::Yellow:
            std::cout << "Yellow" << std::endl;
            break;
    }
}

int main() {
    // 使用強類型枚舉
    Color c = Color::Blue;
    print_color(c);
    
    // 需要顯式轉換為整數
    int color_value = static_cast<int>(c);
    std::cout << "Color value: " << color_value << std::endl;
    
    // 顯式從整數轉換為枚舉
    Status s = static_cast<Status>(1);
    
    // 檢查狀態
    if (s == Status::Error) {
        std::cout << "Status is Error" << std::endl;
    }
    
    // 獲取底層類型的值
    uint8_t status_code = static_cast<uint8_t>(s);
    std::cout << "Status code: " << static_cast<int>(status_code) << std::endl;
    
    return 0;
}
```

### 前向宣告與底層類型

```cpp
#include <iostream>
#include <type_traits>

// 前向宣告
enum class Direction : int;
enum class FileMode : unsigned char;

// 在其他地方完整定義
enum class Direction : int {
    North,
    East,
    South,
    West
};

enum class FileMode : unsigned char {
    Read = 0x01,
    Write = 0x02,
    Append = 0x04,
    Binary = 0x08
};

int main() {
    // 檢查底層類型
    std::cout << "Direction underlying type is int: " 
              << std::is_same<std::underlying_type<Direction>::type, int>::value 
              << std::endl;
    
    std::cout << "FileMode underlying type is unsigned char: " 
              << std::is_same<std::underlying_type<FileMode>::type, unsigned char>::value 
              << std::endl;
    
    // 使用位運算（需要顯式轉換）
    FileMode mode = FileMode::Read;
    FileMode combined = static_cast<FileMode>(
        static_cast<unsigned char>(FileMode::Read) | 
        static_cast<unsigned char>(FileMode::Binary)
    );
    
    // 檢查標誌
    unsigned char mode_bits = static_cast<unsigned char>(combined);
    bool can_read = (mode_bits & static_cast<unsigned char>(FileMode::Read)) != 0;
    bool is_binary = (mode_bits & static_cast<unsigned char>(FileMode::Binary)) != 0;
    
    std::cout << "Can read: " << can_read << std::endl;
    std::cout << "Is binary: " << is_binary << std::endl;
    
    return 0;
}
```

### 與傳統枚舉的比較

```cpp
#include <iostream>

// 傳統枚舉（無作用域）
enum Color {
    Red,    // 進入全局命名空間
    Green,
    Blue
};

// 強類型枚舉
enum class Shape {
    Circle,
    Square,
    Triangle
};

// 另一個傳統枚舉，可能導致命名衝突
enum TrafficLight {
    // Red,    // 錯誤：與 Color::Red 衝突
    Yellow,
    // Green   // 錯誤：與 Color::Green 衝突
};

// 另一個強類型枚舉，不會有命名衝突
enum class Fruit {
    Apple,
    Orange,
    Banana
};

int main() {
    // 傳統枚舉使用
    Color c1 = Red;  // 不需要限定符
    
    // 隱式轉換為整數
    int color_value = c1;  // 有效，不需要顯式轉換
    
    // 強類型枚舉使用
    Shape s1 = Shape::Circle;  // 需要限定符
    
    // 不能隱式轉換為整數
    // int shape_value = s1;  // 錯誤：不允許隱式轉換
    int shape_value = static_cast<int>(s1);  // 正確：顯式轉換
    
    // 類型安全性比較
    if (c1 == 0) {  // 有效，但可能導致邏輯錯誤
        std::cout << "Color is Red (0)" << std::endl;
    }
    
    // if (s1 == 0) {  // 錯誤：不能比較 Shape 和 int
    if (s1 == Shape::Circle) {  // 正確：類型安全的比較
        std::cout << "Shape is Circle" << std::endl;
    }
    
    // 傳統枚舉可以比較不同枚舉類型（可能導致邏輯錯誤）
    if (Red == Yellow) {  // 編譯有效，但邏輯上無意義
        std::cout << "This comparison makes no sense" << std::endl;
    }
    
    // 強類型枚舉不能比較不同枚舉類型
    // if (Shape::Circle == Fruit::Apple) {  // 錯誤：不同類型
    
    return 0;
}
```

### 在類中使用強類型枚舉

```cpp
#include <iostream>
#include <string>
#include <vector>

class Database {
public:
    // 嵌套的強類型枚舉
    enum class ConnectionType {
        Local,
        Remote,
        Cloud
    };
    
    enum class ErrorCode : int {
        None = 0,
        ConnectionFailed = 100,
        QueryFailed = 200,
        DataCorrupted = 300
    };
    
    Database(ConnectionType type) : connection_type_(type) {
        std::cout << "Database created with connection type: " 
                  << static_cast<int>(type) << std::endl;
    }
    
    ErrorCode connect() {
        // 模擬連接邏輯
        if (connection_type_ == ConnectionType::Cloud) {
            last_error_ = ErrorCode::ConnectionFailed;
            return last_error_;
        }
        
        last_error_ = ErrorCode::None;
        return last_error_;
    }
    
    std::string get_error_message() const {
        switch (last_error_) {
            case ErrorCode::None:
                return "No error";
            case ErrorCode::ConnectionFailed:
                return "Failed to establish connection";
            case ErrorCode::QueryFailed:
                return "Query execution failed";
            case ErrorCode::DataCorrupted:
                return "Data integrity check failed";
            default:
                return "Unknown error";
        }
    }
    
private:
    ConnectionType connection_type_;
    ErrorCode last_error_ = ErrorCode::None;
};

int main() {
    // 使用嵌套的強類型枚舉
    Database db(Database::ConnectionType::Cloud);
    
    Database::ErrorCode result = db.connect();
    
    if (result != Database::ErrorCode::None) {
        std::cout << "Error: " << db.get_error_message() << std::endl;
        std::cout << "Error code: " << static_cast<int>(result) << std::endl;
    }
    
    return 0;
}
```

## 使用情境

### 適合使用強類型枚舉的情況

1. **表示離散選項**：當需要表示一組固定的、互斥的選項時：
   ```cpp
   enum class DayOfWeek {
       Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
   };
   ```

2. **API 設計**：為公共 API 提供類型安全的參數選項：
   ```cpp
   enum class LogLevel {
       Debug, Info, Warning, Error, Critical
   };
   
   void log(const std::string& message, LogLevel level);
   ```

3. **狀態機**：表示系統或對象的不同狀態：
   ```cpp
   enum class TaskState {
       Created, Running, Paused, Completed, Failed
   };
   ```

4. **配置選項**：表示配置或設置選項：
   ```cpp
   enum class TextAlignment {
       Left, Center, Right, Justified
   };
   ```

5. **錯誤代碼**：定義結構化的錯誤代碼系統：
   ```cpp
   enum class NetworkError : int {
       None = 0,
       ConnectionRefused = 100,
       Timeout = 101,
       InvalidResponse = 200
   };
   ```

6. **位標誌**（需要額外工作）：
   ```cpp
   enum class Permissions : unsigned {
       None = 0,
       Read = 1 << 0,
       Write = 1 << 1,
       Execute = 1 << 2
   };
   
   // 需要自定義運算符或輔助函數
   Permissions operator|(Permissions a, Permissions b) {
       return static_cast<Permissions>(
           static_cast<unsigned>(a) | static_cast<unsigned>(b)
       );
   }
   ```

### 不適合使用強類型枚舉的情況

1. **需要頻繁與整數互操作**：如果代碼需要頻繁地在枚舉和整數之間轉換：
   ```cpp
   // 如果需要大量這樣的轉換，傳統枚舉可能更方便
   for (int i = 0; i < 4; ++i) {
       Direction dir = static_cast<Direction>(i);  // 使用強類型枚舉需要顯式轉換
   }
   ```

2. **與 C API 交互**：當需要與期望整數常量的 C API 交互時：
   ```cpp
   // C API 期望整數常量
   extern "C" void c_function(int mode);
   
   // 使用傳統枚舉可能更簡單
   enum Mode { Normal, Fast, Safe };
   c_function(Fast);  // 直接傳遞
   
   // 使用強類型枚舉需要顯式轉換
   enum class Mode { Normal, Fast, Safe };
   c_function(static_cast<int>(Mode::Fast));  // 需要轉換
   ```

3. **簡單的常量集合**：對於簡單的、作用域有限的常量集合，使用 `const` 或 `constexpr` 可能更簡單：
   ```cpp
   // 簡單常量可能不需要枚舉
   constexpr int MAX_RETRY = 3;
   constexpr double PI = 3.14159265358979;
   ```

4. **需要連續擴展的集合**：如果集合需要頻繁添加新成員，可能需要考慮其他方案：
   ```cpp
   // 如果經常需要添加新的命令，可能需要更靈活的設計
   // 而不是不斷擴展枚舉
   enum class Command { Open, Close, Save, SaveAs, Print };
   ```

### 最佳實踐

- 優先使用強類型枚舉（enum class）而非傳統枚舉
- 為枚舉指定適當的底層類型，特別是當需要控制大小或與特定 API 交互時
- 在類內部定義與該類密切相關的枚舉，利用命名空間限制作用域
- 使用有意義的枚舉名稱和枚舉值名稱，提高代碼可讀性
- 考慮為強類型枚舉提供輔助函數，簡化常見操作（如轉換為字符串）
- 對於位標誌枚舉，定義適當的運算符重載或輔助函數
- 使用 `switch` 語句處理枚舉值時，考慮添加 `default` 分支或使用 `-Wswitch` 編譯器警告
- 避免為枚舉值指定特定數值，除非有特殊需求（如與外部 API 兼容）
- 使用前向宣告減少編譯依賴
- 考慮提供從枚舉到字符串的轉換函數，便於調試和日誌記錄