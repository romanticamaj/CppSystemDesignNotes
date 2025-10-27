# C++ STL Analyzer - 快速開始

## 方法 1: 使用 Roo Code Mode（推薦）

### Mode 配置

Mode 已經配置在專案根目錄的 [`.roomodes`](../../.roomodes) 文件中（YAML 格式）。

**重要：** Roo Code 會在啟動時自動載入 `.roomodes` 文件。如果你剛創建或修改了這個文件，需要：
- 重新載入 VSCode 視窗，或
- 重啟 Roo Code

### 使用步驟

1. **切換到 STL Analyzer mode**
   ```
   /mode cpp-std-analyzer
   ```

2. **提出分析請求**
   ```
   分析 std::vector 的 API 設計
   ```

3. **Agent 會自動：**
   - 識別核心 API
   - 分析設計決策
   - 產出心法文件
   - 遵循深度、量級、細度標準

### 範例對話

```
User: 分析 std::unordered_map 的 API 設計

Agent: 我會分析 std::unordered_map 的核心 API 設計。

核心 API：
1. 建構與初始化
2. 元素存取（[], at, find）
3. 插入操作（insert, emplace, operator[]）
4. 刪除操作（erase, clear）
5. 容量與雜湊管理
6. 迭代器操作

讓我產出文件...

[產出完整的 markdown 文件]
```

---

## 方法 2: 直接使用 System Prompt

如果不使用 Roo Code Mode，可以手動使用 prompt：

### 步驟

1. **複製 System Prompt**
   - 開啟 [`system_prompt.md`](./system_prompt.md)
   - 複製全部內容

2. **貼到對話開頭**
   ```
   [貼上 system_prompt.md 內容]
   
   現在請分析 std::vector 的 API 設計
   ```

3. **Agent 會按照指引產出文件**

---

## 方法 3: 參考範例自行撰寫

### 閱讀範例

詳細閱讀 [`cpp_string_api_reference.md`](../../CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md)

### 使用模板

複製 [`template.md`](./template.md) 作為起點，填入：
- Component 名稱
- 核心 API
- 設計分析
- LeetCode 建議

---

## 適用的 STL Components

### 容器（Containers）

**順序容器：**
- `std::vector` - 動態陣列
- `std::deque` - 雙端佇列
- `std::list` - 雙向鏈結串列
- `std::forward_list` - 單向鏈結串列
- `std::array` - 固定大小陣列

**關聯容器：**
- `std::set` / `std::multiset`
- `std::map` / `std::multimap`
- `std::unordered_set` / `std::unordered_multiset`
- `std::unordered_map` / `std::unordered_multimap`

**容器適配器：**
- `std::stack`
- `std::queue`
- `std::priority_queue`

### 算法（Algorithms）

**排序相關：**
- `std::sort`
- `std::stable_sort`
- `std::partial_sort`
- `std::nth_element`

**搜尋相關：**
- `std::binary_search`
- `std::lower_bound` / `std::upper_bound`
- `std::equal_range`
- `std::find` / `std::find_if`

**修改相關：**
- `std::transform`
- `std::copy` / `std::move`
- `std::remove` / `std::remove_if`
- `std::unique`
- `std::reverse`
- `std::rotate`

**分割相關：**
- `std::partition`
- `std::stable_partition`

**數值相關：**
- `std::accumulate`
- `std::partial_sum`
- `std::adjacent_difference`

### 工具類（Utilities）

**智慧指標：**
- `std::unique_ptr`
- `std::shared_ptr`
- `std::weak_ptr`

**可選型別：**
- `std::optional`
- `std::variant`
- `std::any`

**其他：**
- `std::pair` / `std::tuple`
- `std::string_view`
- `std::span`
- `std::function`
- `std::move` / `std::forward`

---

## 品質檢查

分析完成後，確認文件包含：

### 必要元素
- ✅ TL;DR 速查表
- ✅ 每個 API 解釋「為何這樣設計」
- ✅ 對比表格（方法比較）
- ✅ LeetCode 實戰建議
- ✅ 常見錯誤 vs 正確模式
- ✅ 核心設計原則

### 長度與深度
- ✅ 約 300-400 行
- ✅ 參數級別的分析
- ✅ 效能分析（時間/空間）
- ✅ 只留關鍵程式碼行

### 風格
- ✅ 技術性、精確
- ✅ 設計視角（不是教學）
- ✅ 有 ❌ vs ✅ 對比

---

## 範例產出

已完成的分析：

1. **std::string**
   - 檔案：[`CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md`](../../CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md)
   - 特色：TL;DR 速查、位置驗證深度分析、轉換函數演進

---

## 常見問題

### Q: 文件太長怎麼辦？

**A:** 只保留 LeetCode 最關鍵的部分
- 刪除冗長範例
- 只留關鍵程式碼行
- 合併相似的 API

### Q: 如何決定分析深度？

**A:** 問這些問題：
- LeetCode 常用嗎？→ 深度分析
- 有設計權衡嗎？→ 對比表格
- 新手易錯嗎？→ 錯誤對比
- 參數複雜嗎？→ 參數級分析

### Q: 某些 API 很少用，要分析嗎？

**A:** 專注在 LeetCode 場景
- 常用的深入分析
- 罕用的簡單帶過或省略
- 目標是實戰心法，不是完整文檔

### Q: 如何處理 C++11/14/17/20 的演進？

**A:** 在總結章節說明
- 列出各版本新增功能
- 解釋設計演進原因
- 標注現代最佳實踐

---

## 下一步

1. **嘗試分析一個簡單的 component**
   - 建議從 `std::array` 或 `std::pair` 開始
   
2. **對比你的產出與範例**
   - 檢查是否符合深度、量級、細度
   
3. **持續改進**
   - 每次分析後檢視品質清單
   - 參考優秀片段改進寫作

---

## 支援

- **範例參考:** [`cpp_string_api_reference.md`](../../CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md)
- **完整指引:** [`system_prompt.md`](./system_prompt.md)
- **文件模板:** [`template.md`](./template.md)