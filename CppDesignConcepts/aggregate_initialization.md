# Aggregate Initialization (聚合初始化)

## 1. 核心概念

**Aggregate Initialization** 是 C++ 中一種特殊的初始化語法，允許直接用大括號 `{}` 按順序初始化**聚合類型 (aggregate type)** 的所有成員，無需調用建構函式。

```cpp
struct Point {
    int x;
    int y;
};

Point p = {10, 20};  // Aggregate initialization: x=10, y=20
```

---

## 2. Aggregate Type (聚合類型) 定義

一個類型是 **aggregate** 必須滿足以下所有條件：

### C++17 標準

1. **No user-declared constructors** (無用戶聲明的建構函式)
2. **No private or protected non-static data members** (無私有或保護的非靜態資料成員)
3. **No virtual functions** (無虛函式)
4. **No virtual, private, or protected base classes** (無虛基類、私有或保護基類)

### C++20 放寬規則

- 允許 **deleted or defaulted constructors** (刪除或預設的建構函式)
- 允許使用 `= default` 或 `= delete` 的建構函式

```cpp
// ✅ C++20: 仍是 aggregate
struct Aggregate {
    int x;
    Aggregate() = default;  // 允許 defaulted constructor
};

Aggregate a = {42};  // 有效的 aggregate initialization
```

---

## 3. Aggregate Initialization 語法與行為

### 3.1 基本語法

```cpp
struct Data {
    int a;
    double b;
    char c;
};

// 完整初始化
Data d1 = {1, 3.14, 'x'};

// 省略 = 號 (C++11)
Data d2{2, 2.718, 'y'};

// 部分初始化 (Partial Initialization)
Data d3 = {5};  // a=5, b=0.0, c='\0' (其餘成員值初始化)

// 零初始化 (Zero Initialization)
Data d4 = {};   // a=0, b=0.0, c='\0'
Data d5{};      // 同上
```

**關鍵規則**：
- 按**聲明順序**初始化成員
- 未提供的成員執行 **value initialization** (值初始化)：
  - 基本類型 (int, double) → 0
  - 指標 → `nullptr`
  - 類別類型 → 調用預設建構函式

### 3.2 Nested Aggregates (嵌套聚合)

```cpp
struct Point {
    int x, y;
};

struct Rectangle {
    Point top_left;
    Point bottom_right;
};

// 嵌套初始化需要雙層大括號
Rectangle rect = {{0, 0}, {100, 50}};

// C++11 允許省略內層大括號 (但不建議)
Rectangle rect2 = {0, 0, 100, 50};  // 可行但不清晰
```

### 3.3 Array Aggregates (陣列聚合)

```cpp
int arr1[5] = {1, 2, 3, 4, 5};  // 完整初始化
int arr2[5] = {1, 2};           // {1, 2, 0, 0, 0}
int arr3[5] = {};               // {0, 0, 0, 0, 0}

// std::array 是 aggregate
std::array<int, 3> arr4 = {10, 20, 30};
std::array<int, 3> arr5{};  // {0, 0, 0}
```

---

## 4. `std::array` 與 Aggregate Initialization

### 4.1 為什麼 `std::array` 是 Aggregate？

`std::array` 的簡化實作：

```cpp
template<typename T, std::size_t N>
struct array {
    T elements[N];  // 內部就是 C 風格陣列

    // 無自定義建構函式
    // 所有成員都是 public
    // 無虛函式

    // 僅提供成員函式 (不影響 aggregate 特性)
    T& operator[](size_t i) { return elements[i]; }
    size_t size() const { return N; }
    // ...
};
```

**設計理念**：
- `std::array` 是 C 陣列的**零開銷封裝 (zero-overhead wrapper)**
- 保持 aggregate 特性確保編譯期優化和簡單內存布局

### 4.2 `std::array` 不支援 Fill Constructor

```cpp
// ❌ 編譯錯誤：std::array 無此建構函式
std::array<int, 5> arr(10, 42);

// ✅ 只能用 aggregate initialization
std::array<int, 5> arr = {42, 42, 42, 42, 42};

// ✅ 或在運行期用 fill() 方法
std::array<int, 5> arr;
arr.fill(42);
```

**對比 `std::vector`**：

| 特性                  | `std::vector`         | `std::array`            |
| :-------------------- | :-------------------- | :---------------------- |
| **類型**              | Non-aggregate         | **Aggregate**           |
| **大小**              | 動態（運行期）        | 固定（編譯期）          |
| **建構函式**          | 有自定義建構函式      | 無（僅 defaulted）      |
| **Fill constructor**  | ✅ `vector(10, 42)`   | ❌ 不支援               |
| **初始化語法**        | Constructor or `{}`   | **Aggregate init** `{}` |

---

## 5. Aggregate vs Non-Aggregate

### 5.1 Non-Aggregate 範例

```cpp
// ❌ 有自定義建構函式 → 不是 aggregate
struct NonAggregate1 {
    int x, y;
    NonAggregate1(int a, int b) : x(a), y(b) {}
};

NonAggregate1 na1 = {10, 20};  // 調用建構函式，不是 aggregate initialization

// ❌ 有私有成員 → 不是 aggregate
struct NonAggregate2 {
private:
    int secret;
public:
    int value;
};

// ❌ 有虛函式 → 不是 aggregate
struct NonAggregate3 {
    int x;
    virtual void foo() {}
};
```

### 5.2 檢查是否為 Aggregate

```cpp
#include <type_traits>

struct Agg { int x, y; };
struct NonAgg { NonAgg(int) {} };

static_assert(std::is_aggregate_v<Agg>);      // ✅ 通過
static_assert(std::is_aggregate_v<NonAgg>);   // ❌ 失敗

// C++17 之前需使用
static_assert(std::is_aggregate<Agg>::value);
```

---

## 6. Designated Initializers (C++20)

C++20 引入 **designated initializers**，允許按名稱初始化成員：

```cpp
struct Point {
    int x;
    int y;
    int z;
};

// C++20: 指定成員名稱初始化
Point p = {.x = 10, .y = 20, .z = 30};

// 可省略成員（未指定的成員值初始化）
Point p2 = {.x = 5, .z = 15};  // y=0

// ❌ 必須按聲明順序
Point p3 = {.z = 1, .x = 2};  // 編譯錯誤
```

**限制**：
- 必須按成員**聲明順序**
- 不能混用 positional 和 designated 初始化
- 不能嵌套使用（C++20 限制，C99 允許）

---

## 7. 性能與最佳化

### 7.1 Zero-Overhead Abstraction

```cpp
// 兩者產生相同的機器碼
int c_array[3] = {1, 2, 3};
std::array<int, 3> cpp_array = {1, 2, 3};
```

**編譯器優化**：
- Aggregate initialization 在編譯期完成
- 無建構函式調用開銷
- 無虛函式表 (vtable) 開銷

### 7.2 Constexpr Aggregates

```cpp
struct Config {
    int max_size;
    double threshold;
};

constexpr Config cfg = {1024, 0.95};  // 編譯期常量

// 可用於 constexpr 函式
constexpr int get_max() {
    return cfg.max_size;
}
```

---

## 8. 常見應用場景

### 8.1 配置結構

```cpp
struct AudioConfig {
    int sample_rate;
    int buffer_size;
    int channels;
};

AudioConfig config = {48000, 512, 2};
```

### 8.2 固定大小資料結構

```cpp
struct Vertex {
    float x, y, z;
    float nx, ny, nz;  // normal
};

std::array<Vertex, 3> triangle = {{
    {0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f}
}};
```

### 8.3 Lookup Tables

```cpp
struct Mapping {
    int key;
    const char* value;
};

std::array<Mapping, 3> mappings = {{
    {0, "zero"},
    {1, "one"},
    {2, "two"}
}};
```

---

## 9. 關鍵術語對照

| English                         | 中文               |
| :------------------------------ | :----------------- |
| **Aggregate Initialization**    | 聚合初始化         |
| **Aggregate Type**              | 聚合類型           |
| **Nested Aggregates**           | 嵌套聚合           |
| **Partial Initialization**      | 部分初始化         |
| **Value Initialization**        | 值初始化           |
| **Zero Initialization**         | 零初始化           |
| **Designated Initializers**     | 指定初始化器       |
| **Zero-Overhead Wrapper**       | 零開銷封裝         |
| **User-Declared Constructor**   | 用戶聲明的建構函式 |
| **Non-Static Data Member**      | 非靜態資料成員     |

---

## 10. 總結

**Aggregate Initialization 的核心優勢**：
1. **簡潔語法**：直接用 `{}` 初始化，無需建構函式
2. **零開銷**：編譯期處理，無運行期性能損失
3. **類型安全**：編譯期檢查成員類型和數量
4. **靈活性**：支援部分初始化和嵌套結構

**最佳實踐**：
- 對於簡單的 POD 結構，優先使用 aggregate
- `std::array` 是編譯期固定大小陣列的首選
- 使用 designated initializers (C++20) 提升可讀性
- 避免過度嵌套，保持結構清晰
