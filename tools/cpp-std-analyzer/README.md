# C++ STL API 分析 Agent

這個 agent 專門用於分析 C++ 標準庫（STL）的 API 設計，產出深度心法文件。

## 目的

當需要整理某個 C++ STL component（容器、算法、工具類）時，這個 agent 能：

1. **從 API 設計角度分析**：不只是使用方法，而是解釋「為何這樣設計」
2. **產出 LeetCode 實戰建議**：聚焦於競賽和刷題場景
3. **提供速查表**：TL;DR 格式，快速查閱關鍵 API
4. **對比設計決策**：分析不同設計方案的權衡

## 輸出文件特色

參考 [`CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md`](../../CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md)

- **深度**：API 設計原理，不只是用法
- **量級**：精簡但完整，約 300-400 行
- **細度**：關鍵參數、返回值設計、效能分析

## 使用方式

### 方式 1: 透過 Roo Code Mode（推薦）

Mode 已配置在專案根目錄的 [`.roomodes`](../../.roomodes) 文件中。

1. 重啟 Roo Code 或重新載入專案
2. 切換到 mode：`/mode cpp-std-analyzer` 或選擇「🔬 C++ STL Analyzer」
3. 提供要分析的 STL component 名稱
4. Agent 會產出心法文件

### 方式 2: 直接使用 Prompt

複製 [`system_prompt.md`](./system_prompt.md) 的內容，然後：

```
請分析 std::vector 的 API 設計，產出類似 cpp_string_api_reference.md 的心法文件
```

## 適用範圍

- **容器**：vector, deque, list, map, unordered_map, set, etc.
- **算法**：sort, binary_search, partition, transform, etc.
- **工具類**：optional, variant, any, tuple, etc.
- **智慧指標**：shared_ptr, unique_ptr, weak_ptr
- **其他**：string_view, span, ranges, etc.

## 文件結構

每份產出的文件會包含：

1. **TL;DR 速查表** - 常用 API 一覽
2. **設計分析章節** - 每個主要 API 的設計思維
3. **對比表格** - 不同方法的權衡
4. **LeetCode 實戰** - 競賽場景建議
5. **設計原則總結** - 核心哲學

## 範例

已完成的分析：
- [`cpp_string_api_reference.md`](../../CppDesignConcepts/Algorithm/String/cpp_string_api_reference.md) - std::string API 設計分析

## 文件模板

參考 [`template.md`](./template.md) 了解標準結構。

## 技術文檔

- **Mode 配置規格：** [`MODE_SPEC.md`](./MODE_SPEC.md) - Roo Code Mode 的 YAML 配置詳解
- **System Prompt：** [`system_prompt.md`](./system_prompt.md) - 完整的 agent 分析指引
- **文件模板：** [`template.md`](./template.md) - 標準文件結構
- **快速開始：** [`QUICKSTART.md`](./QUICKSTART.md) - 使用指南