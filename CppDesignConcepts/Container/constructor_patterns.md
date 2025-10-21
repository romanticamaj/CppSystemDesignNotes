# STL 容器建構函式設計模式分析

## 🔵 設計原則

1. **一致性**: 相同語義在不同容器中行為一致
2. **正交性**: 不同建構方式滿足不同需求，互不重疊
3. **效率**: 避免不必要的複製和記憶體分配

---

## 📊 建構函式完整分類

### 1. 預設建構 (Default)

```cpp
std::vector<int> v;
std::string s;
```

- **支援**: 所有容器 ✅
- **複雜度**: O(1)

### 2. 複製建構 (Copy)

```cpp
std::vector<int> v2(v1);
std::vector<int> v3 = v1;
```

- **支援**: 所有容器 ✅
- **複雜度**: O(n)，深拷貝

### 3. 移動建構 (Move, C++11)

```cpp
std::vector<int> v2(std::move(v1));
```

- **支援**: 所有容器 ✅
- **複雜度**: O(1)，資源轉移

### 4. 填充建構 (Fill)

```cpp
std::vector<int> v(10, 42);    // 10 個元素，值為 42
std::string s(10, '*');        // "**********"
```

- **支援**: 僅序列容器 ✅ (vector, deque, list, string)
- **不支援**: array (固定大小), 關聯容器 (元素唯一性), 無序容器
- **複雜度**: O(n)

### 5. 範圍建構 (Range)

```cpp
std::vector<int> v(source.begin(), source.end());
std::set<int> s(vec.begin(), vec.end());
```

- **支援**: 所有容器 ✅ (除了 array)
- **複雜度**: O(n) 或 O(n log n)

### 6. 初始化列表 (Initializer List, C++11)

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
std::set<int> s = {3, 1, 4};           // 自動排序
std::array<int, 3> arr = {1, 2, 3};
```

- **支援**: 所有容器 ✅
- **複雜度**: O(n) 或 O(n log n)

### 7. 容器特定建構

**std::string**:

```cpp
std::string sub(str, 5, 10);   // 子字串：從位置 5，長度 10
std::string s("Hello", 3);     // C-string：前 3 個字元 "Hel"
```

**std::array**:

```cpp
std::array<int, 5> arr = {1, 2, 3, 4, 5};  // 聚合初始化
std::array<int, 5> arr{};                  // 零初始化
```

- **聚合初始化** (Aggregate Initialization): 用於初始化聚合類型（無自訂建構函式、無私有/保護成員、無虛函式的類別或結構），直接用 `{}` 初始化所有成員

**關聯容器** (set, map):

```cpp
std::set<int, std::greater<int>> s({1, 2, 3});  // 自訂比較器
```

**無序容器** (unordered_set, unordered_map):

```cpp
std::unordered_set<int> s(100);  // 指定 bucket 數量
```

---

## 📋 容器支援矩陣

| 建構函式   | vector | deque | list | string | array | set/map | unordered |
| ---------- | ------ | ----- | ---- | ------ | ----- | ------- | --------- |
| 預設       | ✅     | ✅    | ✅   | ✅     | ✅    | ✅      | ✅        |
| 複製       | ✅     | ✅    | ✅   | ✅     | ✅    | ✅      | ✅        |
| 移動       | ✅     | ✅    | ✅   | ✅     | ✅    | ✅      | ✅        |
| 填充       | ✅     | ✅    | ✅   | ✅     | ❌    | ❌      | ❌        |
| 範圍       | ✅     | ✅    | ✅   | ✅     | ❌    | ✅      | ✅        |
| 初始化列表 | ✅     | ✅    | ✅   | ✅     | ✅    | ✅      | ✅        |

**設計原理**:

- **array 不支援填充/範圍**: 大小固定，編譯期指定
- **關聯/無序容器不支援填充**: 元素唯一性約束，填充相同值無意義

---

## ⚠️ 常見陷阱

### 1. `()` vs `{}` 語義差異

```cpp
std::vector<int> v1(10, 5);   // 填充：10 個元素，值為 5
std::vector<int> v2{10, 5};   // 列表：2 個元素 {10, 5}

std::string s1(10, 'A');      // "AAAAAAAAAA"
std::string s2{10, 'A'};      // "\nA" (2 個字元：ASCII 10 和 'A')
```

**規則**: `{}` 優先匹配 `initializer_list`，`()` 呼叫傳統建構函式

### 2. std::array 的特殊性

```cpp
// ✅ 正確：聚合初始化
std::array<int, 5> arr = {1, 2, 3, 4, 5};

// ❌ 錯誤：無自訂建構函式
std::array<int, 5> arr(10, 0);              // 編譯錯誤
std::array<int, 5> arr(v.begin(), v.end()); // 編譯錯誤
```

**原因**: `std::array` 是聚合類型 (aggregate type)，零開銷抽象，不提供自訂建構函式

**深入解析**：關於 `std::array` 為何不提供 fill/range constructor 的完整設計理念分析，請參閱：[std::array 設計理念深度解析](./array_design_philosophy.md)

**替代方案**：

```cpp
// 編譯期初始化
std::array<int, 5> arr = {42, 42, 42, 42, 42};

// 運行期填充
std::array<int, 5> arr;
arr.fill(42);
```

### 3. 關聯容器無填充建構

```cpp
// ❌ 不存在：填充相同值違反唯一性
// std::set<int> s(10, 42);  // 編譯錯誤

// ✅ 替代方案
std::vector<int> temp(10, 42);
std::multiset<int> ms(temp.begin(), temp.end());  // 允許重複
```

---

## 💡 最佳實踐

### 1. 明確表達意圖

```cpp
// 填充：重複元素用 ()
std::vector<int> buffer(1024, 0);

// 列表：明確元素用 {}
std::vector<int> data{1, 2, 3, 4, 5};

// 範圍：從其他容器
std::set<int> unique(vec.begin(), vec.end());
```

### 2. 效能最佳化

```cpp
// ❌ 低效：多次重新分配
std::vector<int> v;
for (int i = 0; i < 1000; ++i)
    v.push_back(0);

// ✅ 高效：一次分配
std::vector<int> v(1000, 0);
```

### 3. 泛型程式設計

```cpp
// 利用一致的介面
template<typename Container>
Container createFilled(size_t n, typename Container::value_type val) {
    return Container(n, val);
}

auto v = createFilled<std::vector<int>>(100, 42);
auto d = createFilled<std::deque<double>>(50, 3.14);
auto s = createFilled<std::string>(20, '*');
```

---

## 🎓 設計智慧總結

1. **通用建構** (預設、複製、移動、範圍、初始化列表) 在所有容器保持一致
2. **填充建構** 僅適用於序列容器 (動態大小 + 可重複元素)
3. **特定建構** 反映容器獨特性質 (string 子字串、關聯容器比較器)
4. **std::array** 是特例：固定大小 → 聚合初始化

### 選擇建構函式的決策樹

```
從現有容器/範圍構造？
├─ 是 → 範圍建構或複製/移動
└─ 否 → 需要多個相同元素？
    ├─ 是 → 填充建構 (序列容器)
    └─ 否 → 明確元素列表？
        ├─ 是 → 初始化列表
        └─ 否 → 預設建構
```

---

## 🔗 相關主題

- `emplace` 系列函式的就地建構
- Allocator-aware 容器
- CTAD (C++17) 類別模板參數推導
- Ranges (C++20) 現代範圍建構
