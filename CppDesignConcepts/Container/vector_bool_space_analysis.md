# `vector<bool>` 空間消耗分析

## 🧠 核心設計：Bit-Packing 特例化

`vector<bool>` 是 C++ STL 唯一的**部分特化（partial specialization）**容器，採用 bit-packing 將每個 `bool` 壓縮為 **1 bit**（而非 1 byte），犧牲部分容器語義換取空間效率。

## 空間消耗計算

### 基本公式

```cpp
Space = ⌈N / 8⌉ bytes  // N = element count
```

### 實際案例對比

| 容器類型         | 每元素大小 | 1M 元素空間  | 備註           |
| ---------------- | ---------- | ------------ | -------------- |
| `vector<int>`    | 4 bytes    | ≈ 4 MB       | 標準整數       |
| `vector<char>`   | 1 byte     | ≈ 1 MB       | 最小 byte 單位 |
| `vector<bool>`   | **1 bit**  | ≈ **125 KB** | bit-packed     |
| `std::bitset<N>` | 1 bit      | ⌈N/8⌉ bytes  | 固定大小       |

**關鍵觀察**：32-bit 全域 bitmap（`2^32` bits）需要 **512 MiB** 連續空間。

## ⚠️ 設計代價

```cpp
vector<bool> v(10);

// ❌ 不是真正的 bool 容器
bool* ptr = v.data();  // 編譯錯誤！

// ⚠️ operator[] 回傳 proxy 物件（不是 bool&）
auto x = v[0];  // x 是 std::_Bit_reference，非 bool&
bool& ref = v[0];  // 編譯錯誤！

// ⚠️ 效能損耗：需要 bit mask/shift 操作
v[i] = true;  // 等價於: word |= (1 << offset)
```

## 適用場景分析

### ✅ 適合使用 `vector<bool>`

1. **小範圍 key 空間** (如 0..10^6)

   ```cpp
   class MyHashSet {
       vector<bool> bitmap;  // ~125 KB for 1M range
   public:
       MyHashSet(): bitmap(1'000'001, false) {}
       void add(int key) { bitmap[key] = true; }
       bool contains(int key) { return bitmap[key]; }
   };
   ```

2. **高密度數據**：大部分位會被使用
3. **需要動態大小**：比 `bitset<N>` 更靈活

### ❌ 不適合場景

1. **稀疏數據 + 大範圍**

   - 32-bit 全域：512 MiB 幾乎全部浪費
   - 改用 hash set 或 Roaring Bitmap

2. **需要真正的 `bool&`**

   - 多執行緒無鎖場景
   - 需要指標穩定性

3. **頻繁 bit 操作成本敏感**
   - 考慮 `std::bitset` 或手寫位操作

## 替代方案對比

### Hash Set 空間分析

```cpp
// std::unordered_set<int> 實作（separate chaining）
// 每個 int 元素實際消耗：
// - 桶子指標攤分：≈ 8 bytes
// - 節點開銷：24~40 bytes
//   ├─ next pointer: 8
//   ├─ cached hash: 8
//   ├─ value (int): 4 + padding
//   └─ malloc overhead: 8~16
// 總計：≈ 32~48 bytes/element
```

**空間效率**：

- `vector<bool>`：1M elements = **125 KB**
- `unordered_set<int>`：1M elements ≈ **32~48 MB**

### 何時選擇 Hash Set？

- **稀疏數據**：元素數 << key 範圍
- **未知範圍**：無法預先配置 bitmap
- **需要迭代器穩定性**

### 進階方案：Roaring Bitmap

適合「稀疏但成塊」的 32-bit 範圍：

- 混合 array/bitset/RLE 壓縮
- 空間 << 512 MiB（通常數十 KB~MB）
- 支援快速集合運算

## 實務建議

### 1. 範圍已知且小（< 10^7）

```cpp
vector<bool> bitmap(MAX_KEY + 1);  // 直接用 bitmap
```

### 2. 稀疏大範圍

```cpp
unordered_set<int> s;  // 或 flat_hash_set（更快）
```

### 3. 需要集合運算 + 大範圍

```cpp
#include <roaring/roaring.h>
roaring::Roaring r;
```

### 4. 自訂分頁 Bitmap（on-demand 配置）

```cpp
struct PagedBitmap {
    static constexpr uint32_t PAGE_SIZE = 1 << 16;  // 64K bits
    unordered_map<uint32_t, unique_ptr<uint64_t[]>> pages;

    void set(uint32_t x) {
        uint32_t pg = x >> 16, off = x & 0xFFFF;
        auto& p = pages[pg];
        if (!p) p = make_unique<uint64_t[]>(PAGE_SIZE / 64);
        p[off / 64] |= 1ull << (off % 64);
    }
};
```

## ⏱️ 效能特性

| 操作         | `vector<bool>` | `unordered_set` |
| ------------ | -------------- | --------------- |
| 插入/查詢    | O(1) bit ops   | O(1) avg, hash  |
| 空間         | N/8 bytes      | ~40N bytes      |
| Cache 友善性 | ✅ 高（連續）  | ❌ 低（鏈結）   |
| 迭代器穩定   | rehash 時失效  | insert 不影響   |

## ✅ 關鍵要點

- [ ] `vector<bool>` 是特例化，**不是標準容器**（無 `bool&`）
- [ ] 1 bit/element = N/8 bytes（比 byte 容器省 8 倍）
- [ ] 適用小範圍高密度；大範圍稀疏改用 hash set
- [ ] 32-bit 全域 bitmap = **512 MiB**（幾乎總是浪費）
- [ ] 需要真正容器語義？用 `vector<char>` 或 `deque<bool>`
