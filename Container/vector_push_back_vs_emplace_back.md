# std::vector push_back vs emplace_back 使用心法整理

## 🔵 tl;dr (總結)

- `emplace_back(args...)` 的獨特之處在於它可以 **就地構造 (in-place construction)** 物件，直接在 `vector` 的記憶體中根據你給的參數呼叫建構函式，完全避免了臨時物件的產生。
- 在所有其他情況下（例如，傳入一個已存在的物件或一個右值），`emplace_back` 的行為和 `push_back` **完全相同**，都會呼叫對應的複製或移動建構函式。

---

## � 主要差異

| 方法                           | 用途                                         | 行為                                             |
| ------------------------------ | -------------------------------------------- | ------------------------------------------------ |
| `push_back(obj)`               | **把一個現成物件放進去 (lvalue)**            | 呼叫 **copy constructor**                        |
| `push_back(std::move(obj))`    | **把一個現成物件移進去 (rvalue)**            | 呼叫 **move constructor**                        |
| `emplace_back(args...)`        | **就地構造新物件 (直接給 constructor 參數)** | **直接在 vector 裡呼叫 constructor**，無臨時物件 |
| `emplace_back(obj)`            | **把現成物件放進去 (lvalue)**                | 呼叫 **copy constructor** (跟 push_back 一樣)    |
| `emplace_back(std::move(obj))` | **把現成物件移進去 (rvalue)**                | 呼叫 **move constructor** (跟 push_back 一樣)    |

---

## 🟢 實務建議

1. **需要就地構造新物件（避免臨時物件）**  
   ✅ 用 `emplace_back(args...)`  
   範例：

   ```cpp
   vec.emplace_back("hello", 42);
   ```

2. **要放現成物件進去**

   - **保留原物件：**
     ✅ 用 `push_back(obj)` (copy)
   - **不再需要原物件：**
     ✅ 用 `push_back(std::move(obj))` (move)

3. **若團隊習慣全用 `emplace_back()` 也可以**
   但要清楚：

   - 如果傳一個物件，行為與 `push_back()` 一樣
   - 只有「傳 constructor 參數」才是「就地構造」

---

## 🟢 心法總結

- **就地構造新物件？** → `emplace_back(args...)`
- **要把現成物件放進去？**

  - 想要保留：`push_back(obj)`
  - 不再需要：`push_back(std::move(obj))`

- **不確定就用 `push_back()` 可讀性最高**

---

## 🟢 範例一覽

```cpp
std::vector<std::string> v;
std::string s = "apple";

// Copy
v.push_back(s);

// Move
v.push_back(std::move(s));

// In-place construction
v.emplace_back("banana");

// Same as push_back copy
v.emplace_back(s);

// Same as push_back move
v.emplace_back(std::move(s));
```
