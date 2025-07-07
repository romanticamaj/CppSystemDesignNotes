# C++17 標準庫特性

## 核心概念

C++17 不僅帶來了語言層面的改進，還大幅擴充了標準庫，引入了許多新的組件和功能。這些標準庫擴充使得 C++ 程序員能夠更加高效地解決常見問題，減少對第三方庫的依賴，並提高代碼的可移植性和可維護性。

C++17 標準庫的主要擴充包括：

1. **`<filesystem>` 庫**：提供跨平台的文件系統操作功能
2. **`<optional>`, `<variant>`, `<any>`**：提供更安全的數據表示方式
3. **並行算法**：在 `<algorithm>` 和 `<numeric>` 中添加支持並行執行的算法版本
4. **字符串視圖 (`std::string_view`)**：提供字符串的非擁有引用
5. **`std::invoke`**：統一函數調用語法
6. **`std::apply`**：將元組展開為函數參數
7. **`std::from_chars` 和 `std::to_chars`**：高效的數字轉換
8. **`std::shared_mutex` 和 `std::scoped_lock`**：增強的同步原語

這些標準庫擴充使得 C++ 更加現代化，能夠更好地應對當今軟件開發的挑戰，如並行計算、文件處理、錯誤處理等。它們與 C++17 的語言特性相輔相成，共同提升了 C++ 的表達能力和開發效率。

## 主要組件

### std::optional

`std::optional` 是一個包裝器，可以包含一個值或者不包含任何值。它用於表示可能存在也可能不存在的值，是處理可能失敗的操作的優雅方式，避免了使用特殊值（如 `-1` 或 `nullptr`）或異常來表示失敗情況。

```cpp
#include <optional>
#include <string>
#include <iostream>

// 可能失敗的函數
std::optional<std::string> get_nickname(const std::string& name) {
    if (name == "Robert") return "Bob";
    if (name == "William") return "Bill";
    if (name == "Richard") return "Dick";
    return std::nullopt;  // 沒有昵稱
}

int main() {
    auto nickname1 = get_nickname("Robert");
    if (nickname1) {
        std::cout << "昵稱: " << *nickname1 << std::endl;
    }

    auto nickname2 = get_nickname("John");
    if (!nickname2) {
        std::cout << "John 沒有昵稱" << std::endl;
    }

    // 使用 value_or 提供默認值
    std::cout << "Richard 的昵稱: " << get_nickname("Richard").value_or("無昵稱") << std::endl;
    std::cout << "John 的昵稱: " << get_nickname("John").value_or("無昵稱") << std::endl;

    return 0;
}
```

### std::variant

`std::variant` 是一個類型安全的聯合體，可以在同一時間持有多種可能類型中的一種。與 C 風格的聯合體不同，`std::variant` 知道它當前持有的是哪種類型，並提供了安全的訪問方式。

```cpp
#include <variant>
#include <string>
#include <iostream>

int main() {
    // 可以存儲 int, float 或 std::string 的變體
    std::variant<int, float, std::string> v;

    v = 42;  // v 現在包含 int
    std::cout << "v 包含整數: " << std::get<int>(v) << std::endl;

    v = 3.14f;  // v 現在包含 float
    std::cout << "v 包含浮點數: " << std::get<float>(v) << std::endl;

    v = "hello";  // v 現在包含 string
    std::cout << "v 包含字符串: " << std::get<std::string>(v) << std::endl;

    // 檢查當前持有的類型
    if (std::holds_alternative<int>(v)) {
        std::cout << "v 包含整數" << std::endl;
    } else if (std::holds_alternative<float>(v)) {
        std::cout << "v 包含浮點數" << std::endl;
    } else if (std::holds_alternative<std::string>(v)) {
        std::cout << "v 包含字符串" << std::endl;
    }

    // 使用 std::visit 處理所有可能的類型
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>)
            std::cout << "v 包含整數: " << arg << std::endl;
        else if constexpr (std::is_same_v<T, float>)
            std::cout << "v 包含浮點數: " << arg << std::endl;
        else if constexpr (std::is_same_v<T, std::string>)
            std::cout << "v 包含字符串: " << arg << std::endl;
    }, v);

    return 0;
}
```

### std::any

`std::any` 可以存儲任何可複製構造的類型的單個值，提供了一種類型安全的方式來處理未知類型的值。與 `void*` 不同，`std::any` 記住了存儲值的類型，並提供了類型安全的訪問方式。

```cpp
#include <any>
#include <string>
#include <iostream>
#include <vector>

int main() {
    // 創建 std::any 對象
    std::any a = 42;
    std::cout << "a 包含整數: " << std::any_cast<int>(a) << std::endl;

    a = 3.14;  // 現在 a 包含 double
    std::cout << "a 包含浮點數: " << std::any_cast<double>(a) << std::endl;

    a = std::string("hello");  // 現在 a 包含 string
    std::cout << "a 包含字符串: " << std::any_cast<std::string>(a) << std::endl;

    // 檢查是否有值
    if (a.has_value()) {
        std::cout << "a 包含一個值" << std::endl;
    }

    // 獲取類型信息
    std::cout << "a 的類型 ID: " << a.type().name() << std::endl;

    // 清除值
    a.reset();
    std::cout << "重置後，a 是否有值: " << std::boolalpha << a.has_value() << std::endl;

    // 使用 std::any 存儲不同類型的值
    std::vector<std::any> values;
    values.push_back(42);
    values.push_back(3.14);
    values.push_back(std::string("hello"));
    values.push_back(std::vector<int>{1, 2, 3});

    // 安全地訪問值
    try {
        std::cout << std::any_cast<int>(values[0]) << std::endl;
        std::cout << std::any_cast<double>(values[1]) << std::endl;
        std::cout << std::any_cast<std::string>(values[2]) << std::endl;

        auto& vec = std::any_cast<std::vector<int>&>(values[3]);
        std::cout << "向量大小: " << vec.size() << std::endl;

        // 這將拋出 std::bad_any_cast 異常
        std::cout << std::any_cast<float>(values[1]) << std::endl;
    } catch (const std::bad_any_cast& e) {
        std::cout << "類型轉換錯誤: " << e.what() << std::endl;
    }

    return 0;
}
```

### std::string_view

`std::string_view` 提供了一個字符串的非擁有引用，它不會複製字符串數據，而是簡單地引用現有的字符串。這對於需要讀取但不修改字符串的函數非常有用，可以避免不必要的字符串複製，提高性能。

```cpp
#include <string_view>
#include <string>
#include <iostream>
#include <chrono>

// 使用 string_view 作為參數，避免字符串複製
void analyze_string(std::string_view sv) {
    std::cout << "字符串: \"" << sv << "\"" << std::endl;
    std::cout << "長度: " << sv.length() << std::endl;
    std::cout << "首字符: " << sv.front() << std::endl;
    std::cout << "尾字符: " << sv.back() << std::endl;
}

int main() {
    // 使用字面量創建 string_view
    std::string_view sv1 = "Hello, World!";
    analyze_string(sv1);

    // 從 std::string 創建 string_view
    std::string s = "C++ is awesome";
    std::string_view sv2 = s;
    analyze_string(sv2);

    // string_view 的子串操作不會創建新的字符串
    std::string_view sv3 = sv1.substr(0, 5);  // "Hello"
    analyze_string(sv3);

    // 修改 string_view 的範圍
    std::string_view sv4 = sv1;
    sv4.remove_prefix(7);  // 移除 "Hello, "
    analyze_string(sv4);

    sv4.remove_suffix(1);  // 移除 "!"
    analyze_string(sv4);

    // 注意：string_view 不保證字符串以 null 結尾
    // 不要將 string_view 傳遞給需要 C 風格字符串的 API

    // 比較 string_view 的性能
    const char* long_text = "這是一個非常長的字符串，用於演示 string_view 的性能優勢...";

    // 傳統方式：創建臨時 std::string
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        std::string s(long_text);
        if (s.length() > 0) {
            // 做一些操作
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "使用 std::string: " << duration.count() << " ms" << std::endl;

    // 使用 string_view：沒有複製
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        std::string_view sv(long_text);
        if (sv.length() > 0) {
            // 做一些操作
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "使用 std::string_view: " << duration.count() << " ms" << std::endl;

    return 0;
}
```

### 文件系統庫

C++17 引入了 `<filesystem>` 庫，提供了跨平台的文件系統操作功能，包括路徑操作、文件和目錄的創建、刪除、重命名等。這使得 C++ 程序能夠以可移植的方式處理文件系統操作，而不需要依賴特定平台的 API。

```cpp
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

void explore_directory(const fs::path& path) {
    std::cout << "探索目錄: " << path << std::endl;

    // 檢查路徑是否存在
    if (!fs::exists(path)) {
        std::cout << "路徑不存在" << std::endl;
        return;
    }

    // 檢查是否是目錄
    if (!fs::is_directory(path)) {
        std::cout << "路徑不是目錄" << std::endl;
        return;
    }

    // 遍歷目錄
    std::cout << "目錄內容:" << std::endl;
    for (const auto& entry : fs::directory_iterator(path)) {
        std::cout << "  " << entry.path().filename();

        if (fs::is_regular_file(entry)) {
            std::cout << " (文件, " << fs::file_size(entry) << " 字節)";
        } else if (fs::is_directory(entry)) {
            std::cout << " (目錄)";
        } else if (fs::is_symlink(entry)) {
            std::cout << " (符號鏈接)";
        }

        std::cout << std::endl;
    }
}

void create_directory_structure() {
    fs::path base_dir = fs::temp_directory_path() / "cpp17_fs_demo";

    std::cout << "創建目錄結構在: " << base_dir << std::endl;

    // 創建目錄
    fs::create_directories(base_dir);

    // 創建一些文件
    std::ofstream(base_dir / "file1.txt") << "這是文件 1 的內容";
    std::ofstream(base_dir / "file2.txt") << "這是文件 2 的內容";

    // 創建子目錄
    fs::create_directory(base_dir / "subdir");
    std::ofstream(base_dir / "subdir" / "file3.txt") << "這是文件 3 的內容";

    // 複製文件
    fs::copy_file(base_dir / "file1.txt", base_dir / "file1_copy.txt");

    // 重命名文件
    fs::rename(base_dir / "file2.txt", base_dir / "file2_renamed.txt");

    // 獲取目錄大小
    std::uintmax_t total_size = 0;
    for (const auto& entry : fs::recursive_directory_iterator(base_dir)) {
        if (fs::is_regular_file(entry)) {
            total_size += fs::file_size(entry);
        }
    }

    std::cout << "目錄總大小: " << total_size << " 字節" << std::endl;

    // 探索創建的目錄
    explore_directory(base_dir);
}

int main() {
    // 獲取當前路徑
    fs::path current_path = fs::current_path();
    std::cout << "當前工作目錄: " << current_path << std::endl;

    // 路徑操作
    fs::path p = "dir/subdir/file.txt";
    std::cout << "路徑: " << p << std::endl;
    std::cout << "文件名: " << p.filename() << std::endl;
    std::cout << "不帶擴展名的文件名: " << p.stem() << std::endl;
    std::cout << "擴展名: " << p.extension() << std::endl;
    std::cout << "父路徑: " << p.parent_path() << std::endl;

    // 路徑拼接
    fs::path base = "/home/user";
    fs::path full = base / "documents" / "file.txt";
    std::cout << "完整路徑: " << full << std::endl;

    // 探索當前目錄
    explore_directory(current_path);

    // 創建和操作目錄結構
    create_directory_structure();

    return 0;
}
```

### 並行算法

C++17 在 `<algorithm>` 和 `<numeric>` 中添加了支持並行執行的算法版本。這些算法接受一個執行策略參數，指定算法應該如何並行化：順序執行、並行執行或並行+向量化執行。

```cpp
#include <algorithm>
#include <execution>
#include <vector>
#include <random>
#include <iostream>
#include <chrono>

int main() {
    // 創建一個大向量並填充隨機數
    const size_t size = 10'000'000;
    std::vector<int> v(size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);

    for (auto& i : v) {
        i = dis(gen);
    }

    // 創建向量的副本用於不同的排序策略
    auto v1 = v;
    auto v2 = v;
    auto v3 = v;

    // 測量順序排序的時間
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, v.begin(), v.end());
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_time = end - start;

    // 測量並行排序的時間
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, v1.begin(), v1.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_time = end - start;

    // 測量並行+向量化排序的時間
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par_unseq, v2.begin(), v2.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_unseq_time = end - start;

    // 測量不指定執行策略的排序時間（默認為 seq）
    start = std::chrono::high_resolution_clock::now();
    std::sort(v3.begin(), v3.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> default_time = end - start;

    // 輸出結果
    std::cout << "排序 " << size << " 個整數的時間:" << std::endl;
    std::cout << "  順序執行 (seq): " << seq_time.count() << " 秒" << std::endl;
    std::cout << "  並行執行 (par): " << par_time.count() << " 秒" << std::endl;
    std::cout << "  並行+向量化 (par_unseq): " << par_unseq_time.count() << " 秒" << std::endl;
    std::cout << "  默認 (無策略): " << default_time.count() << " 秒" << std::endl;

    // 驗證結果是否相同
    bool all_same = std::equal(v.begin(), v.end(), v1.begin()) &&
                    std::equal(v.begin(), v.end(), v2.begin()) &&
                    std::equal(v.begin(), v.end(), v3.begin());

    std::cout << "所有結果相同: " << std::boolalpha << all_same << std::endl;

    // 測試其他並行算法
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);  // 填充 1 到 size

    // 並行 transform
    start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::par, data.begin(), data.end(), data.begin(),
                   [](int x) { return x * x; });
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> transform_time = end - start;

    std::cout << "\n並行 transform 時間: " << transform_time.count() << " 秒" << std::endl;

    // 並行 reduce
    start = std::chrono::high_resolution_clock::now();
    auto sum = std::reduce(std::execution::par, data.begin(), data.end(), 0);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> reduce_time = end - start;

    std::cout << "並行 reduce 時間: " << reduce_time.count() << " 秒" << std::endl;
    std::cout << "總和: " << sum << std::endl;

    return 0;
}
```

### std::invoke 和 std::apply

`std::invoke` 提供了一種統一的方式來調用可調用對象（函數、函數指針、成員函數指針、函數對象等），而 `std::apply` 則允許將元組的元素作為參數傳遞給函數。

```cpp
#include <functional>
#include <iostream>
#include <tuple>
#include <string>

// 普通函數
int add(int a, int b) {
    return a + b;
}

// 帶有狀態的函數對象
class Multiplier {
private:
    int factor;

public:
    Multiplier(int f) : factor(f) {}

    int operator()(int x) const {
        return x * factor;
    }
};

// 帶有成員函數的類
class Person {
private:
    std::string name;
    int age;

public:
    Person(std::string n, int a) : name(std::move(n)), age(a) {}

    void introduce() const {
        std::cout << "我是 " << name << "，今年 " << age << " 歲。" << std::endl;
    }

    std::string get_greeting(const std::string& prefix) const {
        return prefix + " " + name + "!";
    }
};

int main() {
    // 使用 std::invoke 調用普通函數
    std::cout << "3 + 4 = " << std::invoke(add, 3, 4) << std::endl;

    // 使用 std::invoke 調用函數對象
    Multiplier mult(5);
    std::cout << "7 * 5 = " << std::invoke(mult, 7) << std::endl;

    // 使用 std::invoke 調用 lambda
    auto square = [](int x) { return x * x; };
    std::cout << "6^2 = " << std::invoke(square, 6) << std::endl;

    // 使用 std::invoke 調用成員函數
    Person alice("Alice", 30);
    std::invoke(&Person::introduce, alice);  // 調用 alice.introduce()

    // 使用 std::invoke 調用帶參數的成員函數
    std::string greeting = std::invoke(&Person::get_greeting, alice, "你好");
    std::cout << greeting << std::endl;

    // 使用 std::apply 將元組元素作為參數傳遞給函數
    auto args = std::make_tuple(10, 20);
    int result = std::apply(add, args);
    std::cout << "10 + 20 = " << result << std::endl;

    // 使用 std::apply 和 lambda
    auto print_info = [](const std::string& name, int age, const std::string& city) {
        std::cout << name << " 是 " << age << " 歲，來自 " << city << std::endl;
    };

    auto person_info = std::make_tuple("Bob", 25, "紐約");
    std::apply(print_info, person_info);

    return 0;
}
```

### 其他標準庫改進

C++17 還引入了許多其他標準庫改進，包括：

1. **`std::shared_mutex` 和 `std::scoped_lock`**：增強的同步原語，用於多線程編程
2. **`std::byte`**：表示原始內存的新類型，不同於 `char` 和 `unsigned char`
3. **`std::clamp`**：將值限制在指定範圍內
4. **`std::gcd` 和 `std::lcm`**：計算最大公約數和最小公倍數
5. **`std::sample`**：從序列中隨機採樣
6. **`std::search` 的新重載**：使用搜索器對象進行更高效的搜索
7. **`std::not_fn`**：創建一個函數對象，該對象返回另一個可調用對象的邏輯否定

```cpp
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <numeric>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>
#include <cstddef>

int main() {
    // std::byte
    std::byte b{42};
    std::cout << "std::byte 值: " << std::to_integer<int>(b) << std::endl;
    std::byte b2 = b << 1;  // 左移操作
    std::cout << "左移後的值: " << std::to_integer<int>(b2) << std::endl;

    // std::clamp
    int value = 75;
    std::cout << "原始值: " << value << std::endl;
    std::cout << "限制在 0-100 範圍內: " << std::clamp(value, 0, 100) << std::endl;
    std::cout << "限制在 0-50 範圍內: " << std::clamp(value, 0, 50) << std::endl;
    std::cout << "限制在 100-200 範圍內: " << std::clamp(value, 100, 200) << std::endl;

    // std::gcd 和 std::lcm
    int a = 24, b = 36;
    std::cout << a << " 和 " << b << " 的最大公約數: " << std::gcd(a, b) << std::endl;
    std::cout << a << " 和 " << b << " 的最小公倍數: " << std::lcm(a, b) << std::endl;

    // std::sample
    std::vector<int> population(100);
    std::iota(population.begin(), population.end(), 1);  // 填充 1 到 100

    std::vector<int> sample(10);
    std::random_device rd;
    std::mt19937 gen(rd());

    std::sample(population.begin(), population.end(), sample.begin(), 10, gen);

    std::cout << "從 1-100 中隨機採樣 10 個數: ";
    for (int x : sample) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    // std::not_fn
    auto is_even = [](int x) { return x % 2 == 0; };
    auto is_odd = std::not_fn(is_even);

    std::cout << "5 是偶數: " << std::boolalpha << is_even(5) << std::endl;
    std::cout << "5 是奇數: " << is_odd(5) << std::endl;

    return 0;
}
```

## 與其他特性的比較

C++17 標準庫特性與之前版本的標準庫相比有以下優勢：

1. **更高的抽象級別**：`std::optional`, `std::variant`, `std::any` 等提供了更高級別的抽象，使代碼更加表達性強、安全且易於維護。

2. **更好的性能**：`std::string_view` 和 `std::from_chars`/`std::to_chars` 等提供了更高效的操作，減少了不必要的內存分配和複製。

3. **更強的並行支持**：並行算法使得利用多核處理器變得更加容易，無需手動管理線程。

4. **更完整的功能集**：文件系統庫填補了 C++ 標準庫中的一個重要空白，使得跨平台文件操作變得更加容易。

5. **更統一的接口**：`std::invoke` 和 `std::apply` 等提供了更統一的函數調用接口，簡化了泛型編程。

## 適用場景

C++17 標準庫特性適用於以下場景：

1. **錯誤處理**：使用 `std::optional` 處理可能失敗的操作，比使用特殊值或異常更加清晰和安全。

2. **多態數據**：使用 `std::variant` 和 `std::any` 處理需要存儲不同類型數據的場景，比如解析 JSON 或 XML 數據。

3. **高性能字符串處理**：使用 `std::string_view` 處理大量字符串讀取操作，避免不必要的字符串複製。

4. **跨平台文件操作**：使用文件系統庫實現可移植的文件和目錄操作，無需依賴特定平台的 API。

5. **並行計算**：使用並行算法處理大量數據，充分利用多核處理器提高性能。

6. **泛型編程**：使用 `std::invoke` 和 `std::apply` 簡化泛型代碼，使其更加統一和可讀。

7. **資源管理**：使用 `std::shared_mutex` 和 `std::scoped_lock` 等同步原語實現更精細的資源訪問控制。

## 最佳實踐

使用 C++17 標準庫特性時，應遵循以下最佳實踐：

1. **優先使用 `std::string_view` 而非 `const std::string&`**：對於只讀字符串參數，使用 `std::string_view` 可以避免不必要的字符串複製，提高性能。

2. **注意 `std::string_view` 的生命週期**：確保 `std::string_view` 引用的字符串在使用期間保持有效。

3. **使用 `std::optional` 表示可能失敗的操作**：避免使用特殊值（如 `-1` 或 `nullptr`）或異常來表示失敗情況。

4. **利用 `std::variant` 的訪問器模式**：使用 `std::visit` 處理 `std::variant` 中的所有可能類型，而不是使用一系列的 `if-else` 或 `switch` 語句。

5. **謹慎使用 `std::any`**：`std::any` 提供了極大的靈活性，但也失去了編譯時類型檢查，應在確實需要時才使用。

6. **選擇合適的執行策略**：根據任務特性和硬件環境選擇合適的並行執行策略，不是所有任務都適合並行化。

7. **使用文件系統庫的錯誤處理機制**：文件系統操作可能失敗，應使用 `std::error_code` 或異常處理可能的錯誤。

## 結論

C++17 標準庫的擴充大大增強了 C++ 的功能和表達能力，使得許多常見任務變得更加簡單和高效。這些新特性與 C++17 的語言特性相輔相成，共同推動了 C++ 向更現代、更安全、更高效的方向發展。

通過合理使用這些標準庫特性，開發者可以寫出更加簡潔、可讀、高效的代碼，減少對第三方庫的依賴，提高代碼的可移植性和可維護性。隨著 C++20 和 C++23 的到來，C++ 標準庫將繼續擴充，為開發者提供更多強大的工具和抽象。
