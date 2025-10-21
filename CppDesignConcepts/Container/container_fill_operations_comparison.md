# Container 填充操作完整比較：Fill Constructor, assign(), std::fill, array::fill()

## 1. 核心概念對比

C++ 提供四種主要的容器填充方式，各有不同的設計哲學和適用場景：

| 操作                     | 語法範例                            | 適用時機     | 元素處理方式               | 支援容器                       |
| :----------------------- | :---------------------------------- | :----------- | :------------------------- | :----------------------------- |
| **Fill Constructor**     | `vector<int> v(10, 42)`             | 創建新容器   | Copy construction          | 序列容器 (vector, deque, list) |
| **`assign()` 方法**      | `v.assign(10, 42)`                  | 重置既有容器 | Destruction + construction | 序列容器                       |
| **`std::fill` 算法**     | `std::fill(v.begin(), v.end(), 42)` | 覆寫既有元素 | Assignment operator        | 任何範圍 (需元素已存在)        |
| **`array::fill()` 方法** | `arr.fill(42)`                      | 覆寫固定容器 | Assignment operator        | 僅 `std::array`                |

---

## 2. 詳細行為分析

### 2.1 Fill Constructor：創建時填充

```cpp
// 創建包含 10 個值為 42 的 vector
std::vector<int> vec(10, 42);
```

**內部機制**：

1. 分配能容納 10 個元素的記憶體
2. 從提供的值 `42` 創建一個臨時對象
3. 對每個位置進行 **copy construction**（複製構造）

**特點**：

- 僅在容器創建時可用
- 一次性分配記憶體，效率高
- 所有元素通過 copy constructor 構造

### 2.2 `assign()`：完全重置

```cpp
std::vector<int> vec = {1, 2, 3};
vec.assign(5, 99);  // vec 變為 {99, 99, 99, 99, 99}
```

**內部機制**：

1. **總是銷毀所有現有元素**（調用析構函數）
2. 判斷容量：
   - 若 `count ≤ capacity()`：在既有記憶體上構造新元素（重用記憶體）
   - 若 `count > capacity()`：釋放舊記憶體，分配新記憶體
3. 創建臨時對象，對每個位置進行 **copy construction**

**特點**：

- 舊數據完全丟失（與 `resize()` 的關鍵差異）
- 可能觸發記憶體重新分配
- 基本異常安全保證（若拋出異常，容器處於有效但未定義狀態）

### 2.3 `std::fill`：範圍賦值

```cpp
std::vector<int> vec(5);  // {0, 0, 0, 0, 0}
std::fill(vec.begin(), vec.end(), 42);  // {42, 42, 42, 42, 42}
```

**內部機制**：

1. 對範圍內的每個**已存在元素**使用 **assignment operator** (`operator=`)
2. 不改變容器的 `size()` 或 `capacity()`
3. 不涉及構造或析構

**特點**：

- 最靈活：可填充任何範圍，不限於整個容器
- **前提條件**：範圍內的元素必須已存在（不會擴展容器）
- 適合部分填充：`std::fill(vec.begin(), vec.begin() + 3, 99)`

### 2.4 `array::fill()`：固定容器專用

```cpp
std::array<int, 5> arr = {1, 2, 3, 4, 5};
arr.fill(0);  // arr 變為 {0, 0, 0, 0, 0}
```

**內部機制**：

1. 對所有元素使用 **assignment operator**
2. 大小固定，不涉及記憶體分配

**特點**：

- 僅適用於 `std::array`（大小編譯期確定）
- 等價於 `std::fill(arr.begin(), arr.end(), value)`
- 提供簡潔語法，語義明確

---

## 3. 性能與效率考量

### 3.1 構造 vs 賦值開銷

```cpp
struct HeavyObject {
    std::vector<int> data;
    HeavyObject(int size) : data(size, 0) {}  // 構造開銷大
    HeavyObject& operator=(const HeavyObject& other) {  // 賦值可能重用記憶體
        data = other.data;
        return *this;
    }
};

// 1. Fill constructor：10 次 copy construction
std::vector<HeavyObject> v1(10, HeavyObject(100));

// 2. assign()：先銷毀舊元素，再 10 次 copy construction
v1.assign(10, HeavyObject(100));

// 3. std::fill：10 次 assignment（可能重用內部 buffer）
std::vector<HeavyObject> v2(10, HeavyObject(50));
std::fill(v2.begin(), v2.end(), HeavyObject(100));
```

**性能結論**：

- 對於複雜類型，`std::fill` 的賦值可能比 `assign()` 的「銷毀+構造」更高效（取決於 `operator=` 實作）
- 對於 POD 類型，編譯器可能將三者優化為 `memset` 或 `memcpy`

### 3.2 記憶體分配策略

| 操作             | 記憶體分配行為                                              |
| :--------------- | :---------------------------------------------------------- |
| Fill constructor | 一次性分配精確大小，無浪費                                  |
| `assign()`       | **總是銷毀所有元素**；`count ≤ capacity()` 時重用記憶體，否則重新分配 |
| `std::fill`      | 不涉及分配（僅賦值）                                        |
| `array::fill()`  | 不涉及分配（棧上固定記憶體）                                |

---

## 4. 實際應用場景

### 4.1 創建預分配緩衝區

```cpp
// ✅ 最佳：使用 fill constructor
std::vector<double> audio_buffer(1024, 0.0);
```

### 4.2 重置容器狀態

```cpp
std::vector<int> scores = {85, 92, 78};

// ✅ 使用 assign()：完全替換內容
scores.assign(10, 0);  // 新回合，10 個玩家，初始分數 0
```

### 4.3 部分填充

```cpp
std::vector<int> data(100);

// ✅ 使用 std::fill：填充前 50 個元素
std::fill(data.begin(), data.begin() + 50, -1);
```

### 4.4 固定大小陣列初始化

```cpp
std::array<char, 256> lookup_table;

// ✅ 使用 array::fill()：語義清晰
lookup_table.fill('\0');
```

---

## 5. 常見陷阱與解決方案

### 陷阱 1：誤用 `std::fill` 擴展容器

```cpp
std::vector<int> vec;  // 空容器

// ❌ 錯誤：fill 不會擴展容器
std::fill(vec.begin(), vec.end(), 42);  // 無效，vec 仍為空

// ✅ 正確方法 1：先 resize
vec.resize(10);
std::fill(vec.begin(), vec.end(), 42);

// ✅ 正確方法 2：直接使用 assign
vec.assign(10, 42);
```

### 陷阱 2：誤解 `assign()` 與 `resize()` 的差異

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// ❌ 意圖：保留前 3 個元素並填充
vec.assign(3, 99);  // 結果：{99, 99, 99}（舊數據丟失）

// ✅ 正確：使用 resize（保留數據）+ fill
vec.resize(3);           // {1, 2, 3}
std::fill(vec.begin(), vec.end(), 99);  // {99, 99, 99}
```

### 陷阱 3：對非 POD 類型的效率誤判

```cpp
struct Resource {
    std::unique_ptr<int[]> buffer;
    Resource() : buffer(new int[1000]) {}
};

std::vector<Resource> v1(100);  // 100 次構造

// ❌ 低效：先銷毀 100 個對象，再重新構造 100 個
v1.assign(100, Resource());

// ✅ 若只需重置，考慮直接操作元素
for (auto& r : v1) {
    r = Resource();  // 單次賦值，可能重用記憶體
}
```

---

## 6. 決策樹：選擇正確的填充方法

```
需要創建新容器？
├─ 是 → Fill Constructor
│   └─ std::vector<int> vec(10, 42);
│
└─ 否（操作既有容器）
    ├─ 需要完全替換內容？
    │   ├─ 是 → assign()
    │   │   └─ vec.assign(10, 99);
    │   │
    │   └─ 否（保留部分數據）→ resize() + std::fill
    │       └─ vec.resize(10); std::fill(vec.begin(), vec.end(), 0);
    │
    └─ 需要填充部分範圍或特定範圍？
        └─ std::fill(vec.begin() + 2, vec.end(), 0);
```

**特殊情況**：

- 若容器是 `std::array` → 使用 `arr.fill(value)` 或 `std::fill(arr.begin(), arr.end(), value)`

---

## 7. 總結

| 方法                 | 最佳用途                    | 記憶點                 |
| :------------------- | :-------------------------- | :--------------------- |
| **Fill Constructor** | 創建預填充的新容器          | "從零開始，一步到位"   |
| **`assign()`**       | 完全重置容器內容            | "推倒重來，不留舊痕"   |
| **`std::fill`**      | 覆寫既有元素或部分範圍      | "靈活填充，不改大小"   |
| **`array::fill()`**  | 重置固定大小的 `std::array` | "固定容器專用快捷方式" |

**關鍵原則**：

1. **創建時優先 fill constructor**（效率最高）
2. **完全重置用 `assign()`**（語義明確）
3. **部分填充或靈活範圍用 `std::fill`**（最通用）
4. **`std::array` 用 `fill()` 方法**（簡潔直觀）
