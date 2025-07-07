# C++11 範圍迴圈

## 核心概念

範圍迴圈（Range-based for loop）是 C++11 引入的一種新的迴圈語法，提供了一種簡潔、易讀且不易出錯的方式來遍歷容器或其他可迭代序列中的元素。這種語法消除了顯式迭代器的使用需求，減少了樣板程式碼，並提高了程式碼的可讀性和安全性。

範圍迴圈的基本形式為 `for (element_declaration : range_expression)`，其中：
- `element_declaration` 是每次迭代中從範圍中提取的元素的宣告
- `range_expression` 是要迭代的序列（容器、陣列或其他可迭代物件）

在底層，範圍迴圈使用 `begin()` 和 `end()` 函數（或成員函數）來獲取迭代器，然後使用這些迭代器遍歷序列。這意味著任何提供了適當 `begin()` 和 `end()` 函數的類型都可以與範圍迴圈一起使用。

範圍迴圈的主要優點包括：
- 語法簡潔，減少了樣板程式碼
- 消除了常見的迭代錯誤，如邊界檢查錯誤
- 提高了程式碼的可讀性
- 與標準庫容器和自定義容器無縫協作
- 可以與 `auto` 關鍵字結合使用，進一步簡化程式碼

## 語法

```cpp
// 基本語法
for (element_declaration : range_expression) {
    // 迴圈體
}

// 使用 auto 自動推導類型
for (auto element : container) {
    // 迴圈體
}

// 使用引用避免複製
for (auto& element : container) {
    // 可修改元素
}

// 使用常量引用避免複製但不修改元素
for (const auto& element : container) {
    // 不可修改元素
}

// 使用結構化綁定（C++17）
for (auto [key, value] : map) {
    // 直接訪問鍵和值
}
```

## 與相似概念的比較

| 特性 | 範圍迴圈 | 傳統 for 迴圈 | 迭代器迴圈 | std::for_each |
|------|----------|--------------|------------|---------------|
| 語法簡潔性 | 高 | 中 | 低 | 中 |
| 適用於自定義類型 | 是（需實現 begin/end） | 是（需實現索引） | 是（需實現迭代器） | 是（需實現迭代器） |
| 索引訪問 | 否（直接） | 是 | 否（間接） | 否 |
| 修改元素 | 是（通過引用） | 是 | 是 | 是（通過函數） |
| 性能開銷 | 低 | 極低 | 低 | 低到中 |
| 可讀性 | 高 | 中 | 中 | 中到高 |
| 錯誤傾向 | 低 | 高 | 中 | 低 |
| 適用於陣列 | 是 | 是 | 是（較複雜） | 是 |

## 使用範例

### 基本用法

```cpp
#include <iostream>
#include <vector>
#include <string>

int main() {
    // 遍歷 vector
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    std::cout << "Numbers: ";
    for (int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // 遍歷陣列
    int array[] = {10, 20, 30, 40, 50};
    
    std::cout << "Array elements: ";
    for (int value : array) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // 遍歷字串
    std::string text = "Hello";
    
    std::cout << "Characters: ";
    for (char c : text) {
        std::cout << c << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 使用引用修改元素

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // 使用引用修改元素
    std::cout << "Doubling each number:" << std::endl;
    for (auto& num : numbers) {
        num *= 2;  // 修改原始元素
    }
    
    // 使用常量引用遍歷（避免複製但不修改）
    std::cout << "Modified numbers: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";  // 輸出: 2 4 6 8 10
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 遍歷關聯容器

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, int> scores = {
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };
    
    // 遍歷 map
    std::cout << "Student scores:" << std::endl;
    for (const auto& pair : scores) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // 使用結構化綁定（C++17）
    std::cout << "\nUsing structured binding (C++17):" << std::endl;
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
    
    return 0;
}
```

### 自定義類型與範圍迴圈

```cpp
#include <iostream>
#include <vector>

// 自定義容器類別
class IntContainer {
private:
    std::vector<int> data;
    
public:
    // 添加元素
    void add(int value) {
        data.push_back(value);
    }
    
    // 提供 begin() 和 end() 函數以支持範圍迴圈
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    
    // 常量版本
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

int main() {
    IntContainer container;
    container.add(10);
    container.add(20);
    container.add(30);
    
    // 使用範圍迴圈遍歷自定義容器
    std::cout << "Container elements: ";
    for (int value : container) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 使用初始化列表

```cpp
#include <iostream>
#include <initializer_list>

// 函數接受初始化列表並使用範圍迴圈處理
void process_values(std::initializer_list<int> values) {
    int sum = 0;
    
    // 使用範圍迴圈遍歷初始化列表
    for (int value : values) {
        sum += value;
    }
    
    std::cout << "Sum: " << sum << std::endl;
}

int main() {
    // 直接傳遞初始化列表
    process_values({1, 2, 3, 4, 5});  // 輸出: Sum: 15
    
    // 使用範圍迴圈遍歷臨時初始化列表
    std::cout << "Values: ";
    for (int x : {10, 20, 30, 40}) {
        std::cout << x << " ";
    }
    std::cout << std::endl;  // 輸出: Values: 10 20 30 40
    
    return 0;
}
```

## 使用情境

### 適合使用範圍迴圈的情況

1. **遍歷整個容器**：當需要處理容器中的每個元素時：
   ```cpp
   std::vector<int> numbers = {1, 2, 3, 4, 5};
   for (const auto& num : numbers) {
       process(num);
   }
   ```

2. **處理字串中的字符**：
   ```cpp
   std::string text = "Hello";
   for (char c : text) {
       std::cout << static_cast<int>(c) << " ";  // 輸出 ASCII 值
   }
   ```

3. **修改容器中的元素**：
   ```cpp
   std::vector<std::string> words = {"hello", "world"};
   for (auto& word : words) {
       word[0] = std::toupper(word[0]);  // 將首字母大寫
   }
   ```

4. **遍歷關聯容器**：
   ```cpp
   std::map<std::string, int> counts;
   // ... 填充 map
   for (const auto& [word, count] : counts) {  // C++17
       if (count > 10) {
           std::cout << word << " appears " << count << " times\n";
       }
   }
   ```

5. **處理自定義容器**：
   ```cpp
   CustomCollection<Data> collection;
   // ... 填充集合
   for (auto& item : collection) {
       item.update();
   }
   ```

### 不適合使用範圍迴圈的情況

1. **需要索引的情況**：
   ```cpp
   // 不適合範圍迴圈，因為需要索引
   std::vector<int> values = {10, 20, 30};
   for (size_t i = 0; i < values.size(); ++i) {
       std::cout << "Element " << i << ": " << values[i] << std::endl;
   }
   ```

2. **需要特定迭代順序**：
   ```cpp
   // 不適合範圍迴圈，因為需要反向迭代
   std::vector<int> values = {1, 2, 3, 4, 5};
   for (auto it = values.rbegin(); it != values.rend(); ++it) {
       std::cout << *it << " ";  // 從後向前輸出
   }
   ```

3. **需要同時迭代多個容器**：
   ```cpp
   // 不適合範圍迴圈，因為需要同步迭代兩個容器
   std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
   std::vector<int> scores = {95, 87, 92};
   
   for (size_t i = 0; i < names.size(); ++i) {
       std::cout << names[i] << ": " << scores[i] << std::endl;
   }
   ```

4. **需要條件迭代或跳過元素**：
   ```cpp
   // 不適合範圍迴圈，因為需要有條件地跳過元素
   std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   for (auto it = values.begin(); it != values.end(); ++it) {
       if (*it % 2 == 0) {  // 只處理偶數
           std::cout << *it << " ";
       }
   }
   ```

5. **需要在迭代過程中修改容器結構**：
   ```cpp
   // 不適合範圍迴圈，因為需要在迭代過程中刪除元素
   std::vector<int> values = {1, 2, 3, 4, 5};
   for (auto it = values.begin(); it != values.end(); ) {
       if (*it % 2 == 0) {
           it = values.erase(it);  // 刪除偶數
       } else {
           ++it;
       }
   }
   ```

### 最佳實踐

- 優先使用 `const auto&` 來避免不必要的複製，除非需要修改元素
- 對於簡單的內建類型（如 `int`、`double`），可以直接使用值傳遞
- 結合 `auto` 關鍵字使用，簡化程式碼並提高可維護性
- 在 C++17 中，對於關聯容器使用結構化綁定來直接訪問鍵和值
- 確保自定義類型提供適當的 `begin()` 和 `end()` 函數以支持範圍迴圈
- 注意範圍迴圈的限制，在不適合的情況下使用傳統迴圈或迭代器
- 避免在範圍迴圈中修改容器結構（如添加或刪除元素），這可能導致迭代器失效
- 理解範圍迴圈的底層機制，特別是對於自定義類型的迭代行為
- 在性能關鍵的代碼中，確認範圍迴圈的編譯結果是否符合預期
- 考慮使用 C++20 的範圍庫（Ranges library）來擴展範圍迴圈的功能