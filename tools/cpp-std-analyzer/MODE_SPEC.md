# Roo Code Mode 配置規格說明

## 配置位置

Roo Code 的 custom mode 可以在兩個地方配置：

1. **全域配置**（所有專案）
   - 路徑：`~/Library/Application Support/Code/User/globalStorage/rooveterinaryinc.roo-cline/settings/custom_modes.yaml`
   - 在 Roo Code 啟動時自動創建

2. **專案配置**（僅此專案）✅ **我們使用這個**
   - 路徑：專案根目錄的 `.roomodes` 文件
   - 專案特定的 mode 會覆蓋全域同名 mode

## 配置格式

必須使用 **YAML 格式**（不是 JSON！）

### 基本結構

```yaml
customModes:
  - slug: mode-name           # 必填：唯一識別碼（小寫、數字、連字號）
    name: Display Name        # 必填：顯示名稱
    description: Short desc   # 選填但推薦：簡短描述（5 個字）
    roleDefinition: >-        # 必填：詳細的 role 描述
      Multi-line description
      of the mode's role
    whenToUse: >-            # 選填但推薦：何時使用此 mode
      Description of when to use
    groups:                   # 必填：允許的工具群組（可為空陣列）
      - read                  # 讀取檔案群組
      - edit                  # 編輯檔案群組
      - browser              # 瀏覽器群組
      - command              # 命令執行群組
      - mcp                  # MCP 工具群組
    customInstructions: >-   # 選填：額外指示
      Additional instructions
```

### 必填欄位

| 欄位 | 類型 | 說明 |
|-----|------|------|
| `slug` | string | 唯一識別碼，只能包含小寫字母、數字、連字號。越短越好 |
| `name` | string | 顯示名稱，會出現在 UI 中 |
| `roleDefinition` | string | 詳細的 role 定義，不能為空 |
| `groups` | array | 允許的工具群組，可以是空陣列 |

### 選填但推薦的欄位

| 欄位 | 類型 | 說明 |
|-----|------|------|
| `description` | string | 簡短描述（建議 5 個字以內） |
| `whenToUse` | string | 說明何時應該使用此 mode，幫助 Orchestrator mode 做決策 |
| `customInstructions` | string | 額外的操作指示 |

## 工具群組 (Groups)

### 基本群組

```yaml
groups:
  - read     # read_file, fetch_instructions, search_files, list_files, list_code_definition_names
  - edit     # apply_diff, write_to_file（可編輯所有檔案）
  - browser  # browser_action
  - command  # execute_command
  - mcp      # use_mcp_tool, access_mcp_resource
```

### 限制檔案類型的編輯

如果只想允許編輯特定類型的檔案：

```yaml
groups:
  - read
  - - edit
    - fileRegex: \.md$
      description: Markdown files only
```

這樣就只能編輯 `.md` 檔案。

## 多行文字處理

YAML 支援多種多行文字語法：

### 方法 1: 使用 `>-`（推薦）

```yaml
roleDefinition: >-
  This is line 1.
  This is line 2.
  
  This is a new paragraph.
```

- `>` 表示摺疊換行（多行合併成一行，用空格分隔）
- `-` 表示去掉結尾的換行

### 方法 2: 使用 `|`

```yaml
roleDefinition: |
  This is line 1.
  This is line 2.
```

- 保留換行符號

### 方法 3: 內嵌 `\n`

```yaml
roleDefinition: "Line 1\nLine 2\n\nDouble line break"
```

## C++ STL Analyzer Mode 配置

我們的配置檔在專案根目錄：[`../../.roomodes`](../../.roomodes)

```yaml
customModes:
  - slug: cpp-std-analyzer
    name: 🔬 C++ STL Analyzer
    description: C++ STL API design analysis expert
    roleDefinition: >-
      You are Roo, an expert in C++ Standard Library (STL) API design analysis...
    whenToUse: >-
      Use this mode when analyzing C++ STL components...
    groups:
      - read
      - - edit
        - fileRegex: \.md$
          description: Markdown documentation files
    customInstructions: >-
      Follow the complete analysis guide in tools/cpp-std-analyzer/system_prompt.md...
```

### 設計考量

1. **檔案限制**：只允許編輯 `.md` 檔案
   - 因為此 mode 是用於產出文件，不應該修改程式碼

2. **工具群組**：只給 `read` 和受限的 `edit`
   - 不需要 `browser`、`command`、`mcp`
   - 保持 mode 的專注性

3. **詳細的 `whenToUse`**：
   - 幫助 Orchestrator mode 判斷何時切換到此 mode
   - 說明適用場景和最佳使用時機

## 使用方式

### 啟動 Mode

```bash
/mode cpp-std-analyzer
```

或在 UI 中選擇「🔬 C++ STL Analyzer」

### 重新載入配置

修改 `.roomodes` 後需要：
1. 重新載入 VSCode 視窗，或
2. 重啟 Roo Code

## 參考資源

- **官方文檔**：透過 `fetch_instructions` 工具查看
  ```
  <fetch_instructions>
  <task>create_mode</task>
  </fetch_instructions>
  ```

- **範例**：查看專案中的 `.roomodes` 文件

## 常見問題

### Q: 為什麼不用 JSON？

**A:** Roo Code 使用 YAML 格式，不支援 JSON。YAML 更適合多行文字配置。

### Q: 修改後沒有生效？

**A:** 需要重新載入 VSCode 或重啟 Roo Code。

### Q: 可以同時有全域和專案配置嗎？

**A:** 可以。專案的 `.roomodes` 會覆蓋全域的同名 mode。

### Q: fileRegex 支援什麼語法？

**A:** 標準的正規表達式。例如：
- `\.md$` - Markdown 檔案
- `\.(cpp|h|hpp)$` - C++ 檔案
- `^src/.*\.ts$` - src 目錄下的 TypeScript 檔案

### Q: 可以有多個 mode 嗎？

**A:** 可以，在 `customModes` 陣列中添加多個 mode 即可。

```yaml
customModes:
  - slug: mode-1
    name: Mode 1
    # ...
  - slug: mode-2
    name: Mode 2
    # ...