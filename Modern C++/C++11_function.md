# C++11 std::function

## 核心概念

`std::function` 是 C++11 引入的一個通用的多態函數包裝器，它可以存儲、複製和調用任何可調用目標（函數、lambda 表達式、綁定表達式或其他函數對象），以及通過指針調用的成員函數。`std::function` 是一個函數對象類型，它是對 C++ 中現有的可調用實體的一種類型安全的包裝。

`std::function` 的主要特點包括：

1. **類型擦除**：可以包裝不同類型的可調用對象，只要它們的調用簽名兼容
2. **統一接口**：提供統一的方式來存儲和調用不同類型的可調用對象
3. **值語義**：具有值語義，可以被複製、移動和賦值
4. **可空性**：可以不包含任何可調用對象（空狀態）
5. **類型安全**：在編譯期間檢查調用簽名的兼容性

`std::function` 在許多場景中非常有用，例如：
- 回調函數
- 事件處理
- 策略模式
- 命令模式
- 延遲計算
- 函數組合

## 語法

```cpp
// 定義 std::function 對象
std::function<return_type(parameter_types...)> func;

// 示例：定義一個接受 int 參數並返回 bool 的函數對象
std::function<bool(int)> is_positive;

// 賦值不同類型的可調用對象
// 1. 普通函數
bool check_positive(int x) { return x > 0; }
is_positive = check_positive;

// 2. Lambda 表達式
is_positive = [](int x) { return x > 0; };

// 3. 函數對象（仿函數）
struct PositiveChecker {
    bool operator()(int x) const { return x > 0; }
};
is_positive = PositiveChecker();

// 4. 成員函數（需要使用 std::bind 或 lambda）
class Number {
public:
    bool isPositive() const { return value > 0; }
    int value;
};
Number num{42};
is_positive = [&num](int) { return num.isPositive(); };
// 或者
is_positive = std::bind(&Number::isPositive, &num);

// 調用 std::function 對象
bool result = is_positive(42);  // 返回 true

// 檢查 std::function 是否為空
if (is_positive) {
    // 非空，可以安全調用
} else {
    // 為空，調用會拋出 std::bad_function_call 異常
}

// 清空 std::function
is_positive = nullptr;
```

## 與相似概念的比較

| 特性 | std::function | 函數指針 | 函數對象 | Lambda 表達式 | std::bind |
|------|--------------|---------|----------|--------------|-----------|
| 類型擦除 | 是 | 否 | 否 | 否 | 是 |
| 存儲狀態 | 是 | 否 | 是 | 是 | 是 |
| 內聯優化 | 較差 | 較好 | 較好 | 較好 | 較差 |
| 可空性 | 是 | 是 | 否 | 否 | 否 |
| 語法複雜度 | 中等 | 低 | 高 | 低 | 高 |
| 類型安全 | 高 | 中等 | 高 | 高 | 高 |
| 運行時開銷 | 較高 | 低 | 低 | 低 | 較高 |
| 可組合性 | 高 | 低 | 中等 | 中等 | 高 |
| C++11 引入 | 是 | 否（基本特性） | 否（基本特性） | 是 | 是（新版本） |

## 使用範例

### 基本用法

```cpp
#include <iostream>
#include <functional>
#include <string>

// 普通函數
bool is_even(int n) {
    return n % 2 == 0;
}

// 函數對象
struct IsOdd {
    bool operator()(int n) const {
        return n % 2 == 1;
    }
};

// 帶有狀態的函數對象
class DivisibleBy {
public:
    DivisibleBy(int d) : divisor(d) {}
    
    bool operator()(int n) const {
        return n % divisor == 0;
    }
    
private:
    int divisor;
};

int main() {
    // 使用普通函數初始化
    std::function<bool(int)> func1 = is_even;
    std::cout << "42 is even: " << func1(42) << std::endl;
    
    // 使用函數對象初始化
    std::function<bool(int)> func2 = IsOdd();
    std::cout << "42 is odd: " << func2(42) << std::endl;
    
    // 使用帶有狀態的函數對象初始化
    std::function<bool(int)> func3 = DivisibleBy(3);
    std::cout << "42 is divisible by 3: " << func3(42) << std::endl;
    
    // 使用 lambda 表達式初始化
    std::function<bool(int)> func4 = [](int n) { return n > 0; };
    std::cout << "42 is positive: " << func4(42) << std::endl;
    
    // 使用帶有捕獲的 lambda 表達式初始化
    int threshold = 30;
    std::function<bool(int)> func5 = [threshold](int n) { return n > threshold; };
    std::cout << "42 is greater than " << threshold << ": " << func5(42) << std::endl;
    
    // 檢查是否為空
    std::function<void()> empty_func;
    if (!empty_func) {
        std::cout << "empty_func is empty" << std::endl;
    }
    
    // 賦值為 nullptr
    func1 = nullptr;
    if (!func1) {
        std::cout << "func1 is now empty" << std::endl;
    }
    
    // 調用空函數會拋出異常
    try {
        func1(42);
    } catch (const std::bad_function_call& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### 回調函數

```cpp
#include <iostream>
#include <functional>
#include <vector>
#include <string>

// 定義一個事件處理器類
class EventHandler {
public:
    // 註冊回調函數
    void registerCallback(const std::string& eventName, std::function<void(const std::string&)> callback) {
        callbacks[eventName] = callback;
    }
    
    // 觸發事件
    void triggerEvent(const std::string& eventName, const std::string& eventData) {
        auto it = callbacks.find(eventName);
        if (it != callbacks.end()) {
            // 調用回調函數
            it->second(eventData);
        } else {
            std::cout << "No callback registered for event: " << eventName << std::endl;
        }
    }
    
private:
    std::map<std::string, std::function<void(const std::string&)>> callbacks;
};

// 一個普通的事件處理函數
void handleClick(const std::string& data) {
    std::cout << "Click event handled with data: " << data << std::endl;
}

class Button {
public:
    Button(const std::string& name) : name(name) {}
    
    void click() const {
        std::cout << "Button " << name << " clicked" << std::endl;
    }
    
private:
    std::string name;
};

int main() {
    EventHandler handler;
    
    // 註冊普通函數作為回調
    handler.registerCallback("click", handleClick);
    
    // 註冊 lambda 表達式作為回調
    handler.registerCallback("hover", [](const std::string& data) {
        std::cout << "Hover event handled with data: " << data << std::endl;
    });
    
    // 註冊帶有狀態的 lambda 表達式作為回調
    int count = 0;
    handler.registerCallback("scroll", [&count](const std::string& data) {
        ++count;
        std::cout << "Scroll event #" << count << " handled with data: " << data << std::endl;
    });
    
    // 註冊成員函數作為回調（使用 lambda 包裝）
    Button submitButton("Submit");
    handler.registerCallback("submit", [&submitButton](const std::string&) {
        submitButton.click();
    });
    
    // 觸發事件
    handler.triggerEvent("click", "Button clicked at (100, 200)");
    handler.triggerEvent("hover", "Mouse over at (150, 250)");
    handler.triggerEvent("scroll", "Scrolled 10 pixels down");
    handler.triggerEvent("scroll", "Scrolled 5 pixels up");
    handler.triggerEvent("submit", "Form data submitted");
    handler.triggerEvent("keypress", "Enter key pressed"); // 沒有註冊的事件
    
    return 0;
}
```

### 策略模式

```cpp
#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>

// 使用 std::function 實現策略模式
class SortingStrategy {
public:
    using CompareFunction = std::function<bool(int, int)>;
    
    SortingStrategy(CompareFunction compareFunc) : compareFunc(compareFunc) {}
    
    void sort(std::vector<int>& data) {
        std::sort(data.begin(), data.end(), compareFunc);
    }
    
private:
    CompareFunction compareFunc;
};

// 打印向量的輔助函數
void printVector(const std::vector<int>& vec, const std::string& label) {
    std::cout << label << ": ";
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    printVector(data, "Original");
    
    // 使用升序策略
    SortingStrategy ascendingStrategy([](int a, int b) { return a < b; });
    ascendingStrategy.sort(data);
    printVector(data, "Ascending");
    
    // 使用降序策略
    SortingStrategy descendingStrategy([](int a, int b) { return a > b; });
    descendingStrategy.sort(data);
    printVector(data, "Descending");
    
    // 使用自定義策略：先按奇偶性排序（偶數在前），然後按大小排序
    SortingStrategy customStrategy([](int a, int b) {
        bool aIsEven = (a % 2 == 0);
        bool bIsEven = (b % 2 == 0);
        
        if (aIsEven && !bIsEven) return true;
        if (!aIsEven && bIsEven) return false;
        
        // 同為奇數或同為偶數時，按升序排序
        return a < b;
    });
    customStrategy.sort(data);
    printVector(data, "Custom (even first, then ascending)");
    
    return 0;
}
```

### 函數組合

```cpp
#include <iostream>
#include <functional>
#include <string>
#include <vector>

// 函數組合器：將多個函數組合成一個函數
template <typename T>
std::function<T(T)> compose(const std::vector<std::function<T(T)>>& functions) {
    return [functions](T value) {
        T result = value;
        // 從後向前應用每個函數
        for (auto it = functions.rbegin(); it != functions.rend(); ++it) {
            result = (*it)(result);
        }
        return result;
    };
}

// 函數鏈式調用：將多個函數按順序應用
template <typename T>
std::function<T(T)> chain(const std::vector<std::function<T(T)>>& functions) {
    return [functions](T value) {
        T result = value;
        // 從前向後應用每個函數
        for (const auto& func : functions) {
            result = func(result);
        }
        return result;
    };
}

int main() {
    // 定義一些簡單的函數
    std::function<int(int)> add5 = [](int x) { return x + 5; };
    std::function<int(int)> multiply2 = [](int x) { return x * 2; };
    std::function<int(int)> square = [](int x) { return x * x; };
    
    // 組合函數：square(multiply2(add5(x)))
    auto composed = compose<int>({add5, multiply2, square});
    
    // 鏈式調用：square(multiply2(add5(x)))
    auto chained = chain<int>({add5, multiply2, square});
    
    // 測試組合函數
    int input = 3;
    int composedResult = composed(input);
    std::cout << "compose(square, multiply2, add5)(" << input << ") = " << composedResult << std::endl;
    // 計算過程：square(multiply2(add5(3))) = square(multiply2(8)) = square(16) = 256
    
    // 測試鏈式調用
    int chainedResult = chained(input);
    std::cout << "chain(add5, multiply2, square)(" << input << ") = " << chainedResult << std::endl;
    // 計算過程：add5(3) = 8, multiply2(8) = 16, square(16) = 256
    
    // 使用 std::function 實現柯里化
    std::function<std::function<int(int)>(int)> adder = [](int a) {
        return [a](int b) { return a + b; };
    };
    
    auto add10 = adder(10);
    std::cout << "add10(5) = " << add10(5) << std::endl;
    
    // 使用 std::function 實現部分應用
    std::function<int(int, int, int)> sum3 = [](int a, int b, int c) { return a + b + c; };
    
    auto sum3_partial = [sum3](int a) {
        return [sum3, a](int b, int c) { return sum3(a, b, c); };
    };
    
    auto sum3_with_10 = sum3_partial(10);
    std::cout << "sum3_with_10(20, 30) = " << sum3_with_10(20, 30) << std::endl;
    
    return 0;
}
```

### 延遲計算

```cpp
#include <iostream>
#include <functional>
#include <vector>
#include <string>

// 延遲計算類
class LazyEvaluation {
public:
    // 添加一個延遲計算的任務
    template <typename Func>
    void addTask(const std::string& name, Func func) {
        tasks.push_back({name, std::function<void()>(func)});
    }
    
    // 執行所有任務
    void executeAll() {
        for (const auto& task : tasks) {
            std::cout << "Executing task: " << task.name << std::endl;
            task.func();
        }
        tasks.clear();
    }
    
    // 執行特定名稱的任務
    void execute(const std::string& name) {
        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            if (it->name == name) {
                std::cout << "Executing task: " << it->name << std::endl;
                it->func();
                tasks.erase(it);
                return;
            }
        }
        std::cout << "Task not found: " << name << std::endl;
    }
    
private:
    struct Task {
        std::string name;
        std::function<void()> func;
    };
    
    std::vector<Task> tasks;
};

// 一個耗時的計算函數
void heavyComputation(int n, const std::string& label) {
    std::cout << "Starting heavy computation " << label << " with n = " << n << std::endl;
    
    // 模擬耗時計算
    long long result = 0;
    for (int i = 0; i < n * 1000000; ++i) {
        result += i % 1000;
    }
    
    std::cout << "Finished heavy computation " << label << ", result = " << result << std::endl;
}

int main() {
    LazyEvaluation lazy;
    
    std::cout << "Adding tasks..." << std::endl;
    
    // 添加一些延遲計算的任務
    lazy.addTask("task1", []() { heavyComputation(1, "task1"); });
    lazy.addTask("task2", []() { heavyComputation(2, "task2"); });
    lazy.addTask("task3", []() { heavyComputation(1, "task3"); });
    
    std::cout << "Tasks added, but not executed yet." << std::endl;
    
    // 用戶選擇執行特定任務
    std::cout << "Executing task2..." << std::endl;
    lazy.execute("task2");
    
    // 稍後執行所有剩餘任務
    std::cout << "Executing all remaining tasks..." << std::endl;
    lazy.executeAll();
    
    return 0;
}
```

## 使用情境

### 適合使用 std::function 的情況

1. **回調機制**：當需要存儲和調用回調函數時
   ```cpp
   class Button {
   public:
       void setOnClickHandler(std::function<void()> handler) {
           onClick = handler;
       }
       
       void click() {
           if (onClick) {
               onClick();
           }
       }
       
   private:
       std::function<void()> onClick;
   };
   ```

2. **策略模式**：當需要在運行時選擇不同的算法實現時
   ```cpp
   class TextProcessor {
   public:
       void setFormatter(std::function<std::string(const std::string&)> formatter) {
           this->formatter = formatter;
       }
       
       std::string process(const std::string& text) {
           return formatter ? formatter(text) : text;
       }
       
   private:
       std::function<std::string(const std::string&)> formatter;
   };
   ```

3. **事件系統**：當需要註冊和觸發事件處理器時
   ```cpp
   class EventSystem {
   public:
       void addEventListener(const std::string& event, std::function<void(const EventData&)> listener) {
           listeners[event].push_back(listener);
       }
       
       void dispatchEvent(const std::string& event, const EventData& data) {
           for (const auto& listener : listeners[event]) {
               listener(data);
           }
       }
       
   private:
       std::map<std::string, std::vector<std::function<void(const EventData&)>>> listeners;
   };
   ```

4. **命令模式**：當需要封裝操作為對象時
   ```cpp
   class Command {
   public:
       Command(std::function<void()> execute, std::function<void()> undo)
           : execute(execute), undo(undo) {}
       
       void doExecute() { execute(); }
       void doUndo() { undo(); }
       
   private:
       std::function<void()> execute;
       std::function<void()> undo;
   };
   ```

5. **依賴注入**：當需要注入行為而不是具體類型時
   ```cpp
   class Logger {
   public:
       Logger(std::function<void(const std::string&)> logFunc)
           : logFunc(logFunc) {}
       
       void log(const std::string& message) {
           logFunc(message);
       }
       
   private:
       std::function<void(const std::string&)> logFunc;
   };
   ```

### 不適合使用 std::function 的情況

1. **性能關鍵路徑**：當性能是關鍵考慮因素時
   ```cpp
   // 不推薦：在性能關鍵的內部循環中使用 std::function
   void processVector(const std::vector<int>& vec, std::function<int(int)> processor) {
       for (int i = 0; i < 1000000; ++i) {
           for (int value : vec) {
               int result = processor(value); // 每次調用都有開銷
               // ...
           }
       }
   }
   
   // 推薦：使用模板函數避免虛函數調用開銷
   template <typename Processor>
   void processVector(const std::vector<int>& vec, Processor processor) {
       for (int i = 0; i < 1000000; ++i) {
           for (int value : vec) {
               int result = processor(value); // 可能被內聯
               // ...
           }
       }
   }
   ```

2. **簡單的函數轉發**：當只是簡單地轉發函數調用時
   ```cpp
   // 不推薦：不必要地使用 std::function
   class SimpleWrapper {
   public:
       SimpleWrapper(std::function<void()> func) : func(func) {}
       void call() { func(); }
   private:
       std::function<void()> func;
   };
   
   // 推薦：使用模板參數
   template <typename Func>
   class SimpleWrapper {
   public:
       SimpleWrapper(Func func) : func(func) {}
       void call() { func(); }
   private:
       Func func;
   };
   ```

3. **已知具體類型的情況**：當函數類型在編譯期已知時
   ```cpp
   // 不推薦：當類型已知時使用 std::function
   void processData(const std::vector<int>& data, std::function<bool(int)> predicate) {
       // ...
   }
   
   // 推薦：使用具體類型或模板
   void processData(const std::vector<int>& data, bool (*predicate)(int)) {
       // ...
   }
   
   // 或者更通用的模板版本
   template <typename Predicate>
   void processData(const std::vector<int>& data, Predicate predicate) {
       // ...
   }
   ```

4. **嵌入式系統或資源受限環境**：當資源有限時
   ```cpp
   // 不推薦：在資源受限環境中使用 std::function
   void setupSystem() {
       std::function<void(int)> errorHandler = [](int code) {
           // 處理錯誤...
       };
       // ...
   }
   
   // 推薦：使用函數指針或輕量級替代品
   void handleError(int code) {
       // 處理錯誤...
   }
   
   void setupSystem() {
       void (*errorHandler)(int) = handleError;
       // ...
   }
   ```

### 最佳實踐

1. **檢查有效性**：在調用前檢查 `std::function` 是否為空
   ```cpp
   void executeIfValid(const std::function<void()>& func) {
       if (func) {
           func();
       } else {
           std::cout << "Function is not valid" << std::endl;
       }
   }
   ```

2. **使用 auto 簡化語法**：當局部變數類型明顯時
   ```cpp
   // 冗長的寫法
   std::function<int(int, int)> adder = [](int a, int b) { return a + b; };
   
   // 簡潔的寫法
   auto adder = [](int a, int b) { return a + b; };
   ```

3. **避免不必要的類型擦除**：當不需要多態性時使用模板
   ```cpp
   // 使用 std::function 時有類型擦除開銷
   void process(std::function<void(int)> func, int value) {
       func(value);
   }
   
   // 使用模板避免類型擦除開銷
   template <typename Func>
   void process(Func&& func, int value) {
       func(value);
   }
   ```

4. **使用 std::bind 與 std::function 結合**：綁定參數或成員函數
   ```cpp
   class Calculator {
   public:
       int add(int a, int b) { return a + b; }
   };
   
   Calculator calc;
   
   // 綁定成員函數和第一個參數
   std::function<int(int)> add5 = std::bind(&Calculator::add, &calc, 5, std::placeholders::_1);
   
   int result = add5(10); // 調用 calc.add(5, 10)
   ```

5. **使用 std::function 存儲異構回調集合**：當需要存儲不同類型的回調時
   ```cpp
   std::vector<std::function<void()>> callbacks;
   
   // 添加普通函數
   callbacks.push_back([]() { std::cout << "Hello from lambda" << std::endl; });
   
   // 添加成員函數
   class Greeter {
   public:
       void sayHi() { std::cout << "Hi from class" << std::endl; }
   };
   
   Greeter greeter;
   callbacks.push_back([&greeter]() { greeter.sayHi(); });
   
   // 執行所有回調
   for (const auto& callback : callbacks) {
       callback();
   }
   ```

6. **避免循環引用**：當在 lambda 中捕獲 `std::shared_ptr` 時要小心
   ```cpp
   // 可能導致循環引用
   struct Node {
       std::function<void()> callback;
       std::shared_ptr<Node> next;
   };
   
   std::shared_ptr<Node> node = std::make_shared<Node>();
   node->callback = [node]() { // 捕獲了 node，形成循環引用
       std::cout << "Node callback" << std::endl;
   };
   
   // 避免循環引用的方法
   std::weak_ptr<Node> weak_node = node;
   node->callback = [weak_node]() {
       if (auto shared_node = weak_node.lock()) {
           std::cout << "Node callback" << std::endl;
       }
   };
   ```

7. **考慮使用 std::move 優化性能**：當不再需要原始函數對象時
   ```cpp
   std::function<void()> createExpensiveCallback() {
       // 創建一個包含大量捕獲的 lambda
       std::vector<int> largeData(10000, 42);
       return [data = std::move(largeData)]() {
           std::cout << "Data size: " << data.size() << std::endl;
       };
   }
   
   // 使用 std::move 避免複製
   std::function<void()> callback = createExpensiveCallback();
   std::function<void()> movedCallback = std::move(callback);
   
   // callback 現在處於有效但未指定的狀態