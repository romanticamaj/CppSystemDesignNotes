# C++11 智能指針

## 核心概念

智能指針（Smart Pointers）是 C++11 標準庫引入的一組類模板，用於自動化記憶體管理，解決 C++ 中常見的記憶體洩漏和懸掛指針（dangling pointers）問題。它們提供了類似於原始指針（raw pointers）的語法，但增加了自動資源管理的功能，遵循 RAII（Resource Acquisition Is Initialization，資源獲取即初始化）原則。

C++11 標準庫提供了三種主要的智能指針：
1. **std::unique_ptr**：獨占所有權模型，不允許複製，但可以移動
2. **std::shared_ptr**：共享所有權模型，使用引用計數
3. **std::weak_ptr**：弱引用，配合 std::shared_ptr 使用，不增加引用計數

這些智能指針解決了 C++ 中手動記憶體管理的許多問題：
- 防止記憶體洩漏（忘記釋放記憶體）
- 防止重複釋放（同一塊記憶體被釋放多次）
- 防止懸掛指針（指向已釋放記憶體的指針）
- 提供清晰的所有權語義

智能指針的工作原理是在其解構函數中自動釋放所管理的資源。當智能指針離開作用域或被重新賦值時，如果沒有其他指針引用該資源（對於 shared_ptr），則資源會被自動釋放。

## 語法

```cpp
// unique_ptr
std::unique_ptr<T> ptr1(new T());                 // 顯式建構
std::unique_ptr<T> ptr2 = std::make_unique<T>();  // C++14 推薦用法
auto ptr3 = std::make_unique<T>();                // C++14 使用 auto

// shared_ptr
std::shared_ptr<T> ptr4(new T());                 // 顯式建構
std::shared_ptr<T> ptr5 = std::make_shared<T>();  // 推薦用法
auto ptr6 = std::make_shared<T>();                // 使用 auto

// weak_ptr
std::weak_ptr<T> weak1 = ptr5;                    // 從 shared_ptr 建構

// 存取指針
T& ref = *ptr1;                                   // 解引用
T* raw_ptr = ptr1.get();                          // 獲取原始指針
ptr1->member_function();                          // 成員存取

// 重置/釋放
ptr1.reset();                                     // 釋放資源
ptr1.reset(new T());                              // 釋放舊資源，獲取新資源

// 所有權轉移 (unique_ptr)
std::unique_ptr<T> ptr7 = std::move(ptr1);        // 移動所有權

// 共享所有權 (shared_ptr)
std::shared_ptr<T> ptr8 = ptr5;                   // 增加引用計數

// 檢查 weak_ptr
if (!weak1.expired()) {                           // 檢查引用對象是否存在
    std::shared_ptr<T> ptr9 = weak1.lock();       // 獲取 shared_ptr
}
```

## 與相似概念的比較

| 特性 | std::unique_ptr | std::shared_ptr | std::weak_ptr | 原始指針 | std::auto_ptr (C++17 已棄用) |
|------|----------------|----------------|--------------|----------|---------------------------|
| 所有權模型 | 獨占 | 共享 | 無所有權 | 無定義 | 獨占（但有缺陷） |
| 可複製 | 否 | 是 | 是 | 是 | 是（但轉移所有權） |
| 可移動 | 是 | 是 | 是 | 是 | 是 |
| 引用計數 | 無 | 是 | 無（使用 shared_ptr 的計數） | 無 | 無 |
| 記憶體開銷 | 小 | 中（控制塊） | 小 | 無 | 小 |
| 性能開銷 | 極小 | 小 | 小 | 無 | 小 |
| 線程安全性 | 移動操作非線程安全 | 引用計數操作線程安全 | 與 shared_ptr 相同 | 無 | 非線程安全 |
| 循環引用問題 | 無 | 可能有 | 用於打破循環引用 | 無 | 無 |
| 陣列支持 | 是 | 是（但不推薦） | 是 | 是 | 否（有缺陷） |
| 自定義刪除器 | 是（無額外開銷） | 是（可能有開銷） | 繼承自 shared_ptr | 無 | 否 |

## 使用範例

### std::unique_ptr 基本用法

```cpp
#include <iostream>
#include <memory>
#include <string>

class Resource {
private:
    std::string name;
    
public:
    Resource(const std::string& n) : name(n) {
        std::cout << "Resource " << name << " acquired\n";
    }
    
    ~Resource() {
        std::cout << "Resource " << name << " released\n";
    }
    
    void use() const {
        std::cout << "Using resource " << name << "\n";
    }
};

// 函數返回 unique_ptr
std::unique_ptr<Resource> create_resource(const std::string& name) {
    return std::unique_ptr<Resource>(new Resource(name));
    // C++14: return std::make_unique<Resource>(name);
}

// 函數接受 unique_ptr（通過移動）
void consume_resource(std::unique_ptr<Resource> res) {
    res->use();
    // res 離開作用域時自動釋放
}

int main() {
    // 創建和使用 unique_ptr
    {
        std::unique_ptr<Resource> res1(new Resource("res1"));
        res1->use();
        
        // 使用 reset 更換管理的對象
        res1.reset(new Resource("res2"));  // res1 被釋放
        
        // 獲取原始指針（不轉移所有權）
        Resource* raw_ptr = res1.get();
        raw_ptr->use();
        
        // 釋放所有權
        res1.reset();  // res2 被釋放
        
        // 檢查是否為空
        if (!res1) {
            std::cout << "res1 is empty\n";
        }
    }
    
    // 所有權轉移
    {
        auto res3 = create_resource("res3");
        res3->use();
        
        // 移動所有權
        auto res4 = std::move(res3);  // res3 現在為空
        
        if (!res3) {
            std::cout << "res3 is empty after move\n";
        }
        
        res4->use();
        
        // 將所有權轉移給函數
        consume_resource(std::move(res4));  // res4 現在為空
        
        if (!res4) {
            std::cout << "res4 is empty after consumption\n";
        }
    }
    
    // 管理陣列
    {
        std::unique_ptr<int[]> array(new int[10]);
        for (int i = 0; i < 10; ++i) {
            array[i] = i * i;
        }
        
        std::cout << "array[5] = " << array[5] << "\n";  // 輸出 25
    }  // 陣列自動釋放
    
    return 0;
}
```

### std::shared_ptr 基本用法

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Resource {
private:
    std::string name;
    
public:
    Resource(const std::string& n) : name(n) {
        std::cout << "Resource " << name << " acquired\n";
    }
    
    ~Resource() {
        std::cout << "Resource " << name << " released\n";
    }
    
    void use() const {
        std::cout << "Using resource " << name << "\n";
    }
};

// 函數返回 shared_ptr
std::shared_ptr<Resource> create_shared_resource(const std::string& name) {
    return std::make_shared<Resource>(name);
}

// 函數接受 shared_ptr（通過複製）
void use_resource(std::shared_ptr<Resource> res) {
    std::cout << "Resource use count: " << res.use_count() << "\n";
    res->use();
}

int main() {
    // 創建和共享 shared_ptr
    {
        // 創建 shared_ptr
        auto res1 = std::make_shared<Resource>("shared");
        std::cout << "Initial use count: " << res1.use_count() << "\n";  // 輸出 1
        
        // 共享所有權
        {
            auto res2 = res1;  // 複製 shared_ptr，增加引用計數
            std::cout << "After sharing: " << res1.use_count() << "\n";  // 輸出 2
            
            // 兩個指針指向同一個對象
            res2->use();
        }  // res2 離開作用域，引用計數減少
        
        std::cout << "After res2 is gone: " << res1.use_count() << "\n";  // 輸出 1
        
        // 傳遞給函數
        use_resource(res1);  // 函數內部引用計數為 2
        
        // 存儲在容器中
        std::vector<std::shared_ptr<Resource>> resources;
        resources.push_back(res1);
        resources.push_back(create_shared_resource("another"));
        
        std::cout << "After adding to vector: " << res1.use_count() << "\n";  // 輸出 2
        
        // 清空 shared_ptr
        res1.reset();
        std::cout << "After reset: " << (res1 ? "not empty" : "empty") << "\n";  // 輸出 empty
        
        // 資源仍然存在於 vector 中
        resources[0]->use();
    }  // vector 離開作用域，所有資源被釋放
    
    // 自定義刪除器
    {
        auto custom_deleter = [](Resource* p) {
            std::cout << "Custom deleting resource\n";
            delete p;
        };
        
        std::shared_ptr<Resource> res_custom(new Resource("custom"), custom_deleter);
        res_custom->use();
    }  // 使用自定義刪除器釋放資源
    
    return 0;
}
```

### std::weak_ptr 與循環引用

```cpp
#include <iostream>
#include <memory>
#include <string>

class Person {
private:
    std::string name;
    std::shared_ptr<Person> partner;  // 會導致循環引用
    // std::weak_ptr<Person> partner;  // 正確的做法，避免循環引用
    
public:
    Person(const std::string& n) : name(n) {
        std::cout << "Person " << name << " created\n";
    }
    
    ~Person() {
        std::cout << "Person " << name << " destroyed\n";
    }
    
    void set_partner(const std::shared_ptr<Person>& p) {
        partner = p;
    }
    
    const std::string& get_name() const {
        return name;
    }
    
    void display_partner() const {
        if (partner) {
            std::cout << name << "'s partner is " << partner->name << "\n";
        } else {
            std::cout << name << " has no partner\n";
        }
    }
};

// 正確版本，使用 weak_ptr 避免循環引用
class PersonCorrect {
private:
    std::string name;
    std::weak_ptr<PersonCorrect> partner;  // 使用 weak_ptr
    
public:
    PersonCorrect(const std::string& n) : name(n) {
        std::cout << "PersonCorrect " << name << " created\n";
    }
    
    ~PersonCorrect() {
        std::cout << "PersonCorrect " << name << " destroyed\n";
    }
    
    void set_partner(const std::shared_ptr<PersonCorrect>& p) {
        partner = p;
    }
    
    void display_partner() const {
        if (auto p = partner.lock()) {  // 嘗試獲取 shared_ptr
            std::cout << name << "'s partner is " << p->name << "\n";
        } else {
            std::cout << name << "'s partner no longer exists\n";
        }
    }
};

void demonstrate_cycle() {
    std::cout << "Demonstrating memory leak with cycle:\n";
    
    // 創建兩個 Person 對象
    auto alice = std::make_shared<Person>("Alice");
    auto bob = std::make_shared<Person>("Bob");
    
    // 建立循環引用
    alice->set_partner(bob);
    bob->set_partner(alice);
    
    alice->display_partner();
    bob->display_partner();
    
    std::cout << "Leaving scope, but objects won't be destroyed due to cycle\n";
}  // alice 和 bob 的引用計數都是 1，不會被釋放 -> 記憶體洩漏

void demonstrate_weak_ptr() {
    std::cout << "\nDemonstrating proper cleanup with weak_ptr:\n";
    
    // 創建兩個 PersonCorrect 對象
    auto alice = std::make_shared<PersonCorrect>("Alice");
    auto bob = std::make_shared<PersonCorrect>("Bob");
    
    // 建立關係，但不形成循環引用
    alice->set_partner(bob);
    bob->set_partner(alice);
    
    alice->display_partner();
    bob->display_partner();
    
    std::cout << "Leaving scope, objects will be properly destroyed\n";
}  // alice 和 bob 正確釋放

int main() {
    demonstrate_cycle();
    demonstrate_weak_ptr();
    
    return 0;
}
```

### 自定義刪除器與特殊資源管理

```cpp
#include <iostream>
#include <memory>
#include <fstream>
#include <cstdio>

// 使用自定義刪除器管理 FILE*
void manage_c_file() {
    // 自定義刪除器，關閉 FILE*
    auto file_deleter = [](FILE* fp) {
        if (fp) {
            std::cout << "Closing FILE*\n";
            fclose(fp);
        }
    };
    
    // 創建管理 FILE* 的 unique_ptr
    std::unique_ptr<FILE, decltype(file_deleter)> file(fopen("example.txt", "w"), file_deleter);
    
    if (file) {
        fprintf(file.get(), "Hello, custom deleter!");
        std::cout << "Wrote to file\n";
    }
}  // file 自動關閉

// 使用 shared_ptr 管理自定義資源
class Database {
public:
    Database(const std::string& name) : db_name(name) {
        std::cout << "Opening database " << db_name << "\n";
        // 實際應用中，這裡會連接到真正的數據庫
    }
    
    ~Database() {
        std::cout << "Closing database " << db_name << "\n";
        // 實際應用中，這裡會關閉數據庫連接
    }
    
    void query(const std::string& sql) {
        std::cout << "Executing on " << db_name << ": " << sql << "\n";
    }
    
private:
    std::string db_name;
};

// 工廠函數，返回數據庫連接
std::shared_ptr<Database> get_database_connection(const std::string& name) {
    // 自定義刪除器，可以執行額外的清理工作
    auto deleter = [](Database* db) {
        std::cout << "Custom cleanup before deleting database\n";
        delete db;
    };
    
    return std::shared_ptr<Database>(new Database(name), deleter);
}

int main() {
    // 管理 C 文件
    manage_c_file();
    
    // 管理數據庫連接
    {
        auto db = get_database_connection("users.db");
        db->query("SELECT * FROM users");
        
        // 共享連接
        auto db2 = db;
        db2->query("SELECT * FROM products");
        
        std::cout << "Database connection use count: " << db.use_count() << "\n";
    }  // 數據庫連接自動關閉
    
    return 0;
}
```

## 使用情境

### 適合使用 std::unique_ptr 的情況

1. **獨占資源所有權**：當資源應該只有一個擁有者時：
   ```cpp
   std::unique_ptr<File> open_file(const std::string& path) {
       return std::make_unique<File>(path);
   }
   ```

2. **工廠函數返回值**：當函數需要創建並返回對象時：
   ```cpp
   std::unique_ptr<Shape> create_shape(ShapeType type) {
       switch (type) {
           case ShapeType::Circle: return std::make_unique<Circle>();
           case ShapeType::Rectangle: return std::make_unique<Rectangle>();
           default: return nullptr;
       }
   }
   ```

3. **避免資源洩漏**：在可能有異常的代碼中安全管理資源：
   ```cpp
   void process_data() {
       auto resource = std::make_unique<Resource>();
       // 即使下面的代碼拋出異常，resource 也會被正確釋放
       process(resource.get());
   }
   ```

4. **類成員變數**：表示類對某資源的獨占所有權：
   ```cpp
   class Engine {
   private:
       std::unique_ptr<FuelPump> fuel_pump_;
   };
   ```

5. **實現 pImpl 模式**：隱藏實現細節：
   ```cpp
   // 頭文件
   class Widget {
   public:
       Widget();
       ~Widget();
       Widget(Widget&&) noexcept;
       Widget& operator=(Widget&&) noexcept;
       
       void do_something();
       
   private:
       class Impl;
       std::unique_ptr<Impl> pimpl_;
   };
   ```

### 適合使用 std::shared_ptr 的情況

1. **共享資源所有權**：當多個對象需要共享資源時：
   ```cpp
   std::shared_ptr<Config> global_config = std::make_shared<Config>();
   
   void init_subsystem() {
       // 共享配置
       subsystem.set_config(global_config);
   }
   ```

2. **緩存或註冊表**：存儲可能被多處代碼訪問的對象：
   ```cpp
   std::map<std::string, std::shared_ptr<Resource>> resource_cache;
   
   std::shared_ptr<Resource> get_resource(const std::string& name) {
       auto it = resource_cache.find(name);
       if (it != resource_cache.end()) {
           return it->second;
       }
       
       auto resource = std::make_shared<Resource>(name);
       resource_cache[name] = resource;
       return resource;
   }
   ```

3. **觀察者模式**：當被觀察對象和觀察者之間需要相互引用時：
   ```cpp
   class Observer {
   public:
       virtual void update(const std::shared_ptr<Subject>& subject) = 0;
   };
   
   class Subject {
   private:
       std::vector<std::shared_ptr<Observer>> observers_;
   };
   ```

4. **異步操作**：確保回調執行時資源仍然有效：
   ```cpp
   void async_process(std::shared_ptr<Data> data) {
       std::thread([data]() {
           // 即使原始 data 被釋放，這裡的 data 仍然有效
           std::this_thread::sleep_for(std::chrono::seconds(1));
           process(data);
       }).detach();
   }
   ```

### 適合使用 std::weak_ptr 的情況

1. **打破循環引用**：避免 shared_ptr 循環引用導致的記憶體洩漏：
   ```cpp
   class Parent {
   public:
       std::vector<std::shared_ptr<Child>> children;
   };
   
   class Child {
   public:
       std::weak_ptr<Parent> parent;  // 使用 weak_ptr 避免循環引用
   };
   ```

2. **緩存**：存儲對象的弱引用，不阻止它們被釋放：
   ```cpp
   std::map<int, std::weak_ptr<Resource>> resource_cache;
   
   std::shared_ptr<Resource> get_resource(int id) {
       auto it = resource_cache.find(id);
       if (it != resource_cache.end()) {
           if (auto resource = it->second.lock()) {
               return resource;  // 資源仍然存在
           }
       }
       
       // 資源不存在或已被釋放，創建新資源
       auto resource = std::make_shared<Resource>(id);
       resource_cache[id] = resource;
       return resource;
   }
   ```

3. **觀察者模式**：觀察者持有被觀察對象的弱引用：
   ```cpp
   class Observer {
   private:
       std::weak_ptr<Subject> subject_;
       
   public:
       void notify() {
           if (auto s = subject_.lock()) {
               // 被觀察對象仍然存在
               s->get_state();
           } else {
               // 被觀察對象已被釋放
               unregister_self();
           }
       }
   };
   ```

4. **防止懸掛指針**：安全地訪問可能已被釋放的對象：
   ```cpp
   class Controller {
   private:
       std::weak_ptr<Model> model_;
       
   public:
       void update() {
           if (auto m = model_.lock()) {
               // 模型仍然存在
               m->update();
           } else {
               // 模型已被釋放
               report_error();
           }
       }
   };
   ```

### 不適合使用智能指針的情況

1. **非堆分配對象**：
   ```cpp
   int value = 42;
   std::unique_ptr<int> ptr(&value);  // 錯誤！不要管理棧上的對象
   ```

2. **陣列（使用 std::vector 或 std::array 代替）**：
   ```cpp
   // 不推薦
   std::shared_ptr<int> array(new int[10]);  // 錯誤：默認刪除器不適用於陣列
   
   // 推薦
   std::vector<int> array(10);
   ```

3. **循環數據結構（除非使用 weak_ptr）**：
   ```cpp
   // 不推薦
   struct Node {
       std::shared_ptr<Node> next;
       std::shared_ptr<Node> prev;  // 可能導致循環引用
   };
   
   // 推薦
   struct Node {
       std::shared_ptr<Node> next;
       std::weak_ptr<Node> prev;  // 使用 weak_ptr 避免循環引用
   };
   ```

4. **需要自定義生命週期的對象**：
   ```cpp
   // 某些情況下，手動管理生命週期可能更合適
   class SpecialResource {
   public:
       void acquire() { /* ... */ }
       void release() { /* ... */ }
   };
   ```

### 最佳實踐

- 優先使用 `std::make_unique` 和 `std::make_shared` 而非直接使用 `new`
- 使用 `std::unique_ptr` 作為默認選擇，只有在確實需要共享所有權時才使用 `std::shared_ptr`
- 避免將原始指針存儲為類成員，除非有特殊需求
- 不要從同一個原始指針創建多個獨立的智能指針
- 使用 `std::weak_ptr` 打破循環引用
- 考慮智能指針的性能影響，特別是在性能關鍵的代碼中
- 為自定義資源實現自定義刪除器
- 避免過早優化，先確保代碼正確性和資源安全
- 理解智能指針的所有權語義，並在 API 設計中明確表達
- 在多執行緒環境中小心使用 `std::shared_ptr`，其引用計數是線程安全的，但對象本身可能不是