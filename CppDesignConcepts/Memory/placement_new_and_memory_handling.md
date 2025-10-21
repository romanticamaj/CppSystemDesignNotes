# 🟢 C++ Placement New & Memory Handling 心法大全

## 🟢 目錄

1. Placement New 概念
2. 語法與用法
3. Placement New 為什麼存在
4. alignas 概念
5. Placement New 的記憶體來源
6. `operator new` vs `malloc`
7. new void 為什麼不合法
8. 自訂 operator new/delete
9. Memory Pool 應用
10. 常見陷阱與注意事項
11. 心智模型總結

---

## 1️⃣ Placement New 概念

> **Placement new = 在指定記憶體上直接建構物件**

傳統 `new`：

```cpp
MyClass* p = new MyClass(123);
```

等於：

1. 分配記憶體 (`operator new`)
2. 呼叫 constructor

而 Placement new：

```cpp
new (pointer) MyClass(123);
```

只做**建構**，不分配記憶體。

---

## 2️⃣ 語法與用法

### 語法

```cpp
new (memory_pointer) Type(constructor arguments);
```

### 範例

```cpp
char buffer[sizeof(MyClass)];
MyClass* p = new (buffer) MyClass(42);
```

這不分配記憶體，只用 `buffer` 開始的區塊建構物件。

### 顯式析構

```cpp
p->~MyClass();
```

---

## 3️⃣ Placement New 為什麼存在

**核心用途：**
✅ 高效能記憶體管理（減少 heap allocation）
✅ Memory Pool（記憶體池）
✅ 精確掌控物件生命週期
✅ 對齊需求
✅ 反覆在同一塊記憶體重建物件

常見應用場景：

- 遊戲引擎
- 嵌入式系統
- 自訂 allocator
- Serialization/Deserialization

---

## 4️⃣ `alignas` 概念

**目的：**
顯式指定記憶體對齊（alignment）

### 語法

```cpp
alignas(N) Type var;
```

### 範例

```cpp
alignas(MyClass) char buffer[sizeof(MyClass)];
```

這保證 `buffer` 符合 `MyClass` 的對齊需求。

### 搭配 `alignof`

查詢型別自然對齊：

```cpp
alignof(MyClass); // 例如8
```

---

## 5️⃣ Placement New 的記憶體來源

**關鍵：placement new 完全依賴你提供的記憶體**

### Memory 可以來自

- Stack:

  ```cpp
  alignas(MyClass) char buffer[sizeof(MyClass)];
  ```

- Heap:

  ```cpp
  void* mem = std::malloc(sizeof(MyClass));
  ```

- Static storage:

  ```cpp
  static alignas(MyClass) char buffer[sizeof(MyClass)];
  ```

**在哪裡建構取決於記憶體來源**

---

## 6️⃣ `operator new` vs `malloc`

| 功能             | `malloc`                    | `operator new`                 |
| ---------------- | --------------------------- | ------------------------------ |
| 語法             | `void* p = std::malloc(n);` | `void* p = ::operator new(n);` |
| 分配失敗         | 回傳 `nullptr`              | 拋 `std::bad_alloc`            |
| 對齊             | 平台 C 語言對齊             | C++保證對齊（可覆寫）          |
| Constructor 呼叫 | 不呼叫                      | 不呼叫（只分配記憶體）         |

**用法示例**

```cpp
void* mem = ::operator new(sizeof(MyClass));
MyClass* p = new (mem) MyClass(42);
p->~MyClass();
::operator delete(mem);
```

---

## 7️⃣ 為什麼 `new void` 不合法？

`void` 是不完整型別：

- 沒有大小
- 沒有 constructor/destructor

因此以下皆非法：

```cpp
new void;      // ❌
new void(10);  // ❌
```

若要分配「無型別記憶體」：
✅ 使用 `malloc`
✅ 或 `operator new`

---

## 8️⃣ 自訂 `operator new` / `operator delete`

### 範例

```cpp
class MyClass {
public:
    void* operator new(std::size_t size) {
        std::cout << "Custom operator new\n";
        return ::operator new(size);
    }

    void operator delete(void* p) noexcept {
        std::cout << "Custom operator delete\n";
        ::operator delete(p);
    }
};
```

### 注意

- Placement new **不會呼叫自訂 operator new**
- 它會使用標準的：

  ```cpp
  void* operator new(std::size_t, void* p) noexcept { return p; }
  ```

---

## 9️⃣ Memory Pool 應用

**典型流程：**

1. 一次分配大塊 memory
2. 用 Placement new 在 pool 上建構物件
3. 顯式析構
4. 自行回收 memory

### 範例

```cpp
char pool[sizeof(MyClass) * 100];
MyClass* p = new (pool) MyClass(7);
p->~MyClass();
```

---

## 🔟 常見陷阱與注意事項

✅ 必須手動呼叫 destructor：

```cpp
p->~MyClass();
```

✅ 不要對 Placement new 的物件呼叫 `delete`：

```cpp
delete p; // ❌ 未定義行為
```

✅ 必須確保對齊正確：

```cpp
alignas(MyClass) char buffer[sizeof(MyClass)];
```

✅ Scope 離開，記憶體失效：

```cpp
{
    char buffer[sizeof(MyClass)];
    MyClass* p = new (buffer) MyClass();
} // buffer消失，p成為懸掛指標
```

---

## 🔵 心智模型總結

| 方法                 | 分配記憶體 | 呼叫 constructor | 是否手動析構 | 記憶體來源 |
| -------------------- | ---------- | ---------------- | ------------ | ---------- |
| `new Type()`         | ✅         | ✅               | ❌           | heap       |
| `operator new(size)` | ✅         | ❌               | ❌           | heap       |
| `malloc(size)`       | ✅         | ❌               | ❌           | heap       |
| `new (ptr) Type()`   | ❌         | ✅               | ✅           | 自訂       |

**Placement new 心法：**

> 我已經有空間，我只要「把這個型別放上去」。

---

## 💡 實務建議

- 如果只是要用普通 new/delete，不必接觸 placement new。
- 如果要自己管理記憶體池或優化效能，就要熟練這一套。
- 永遠記得：

  1. 對齊
  2. 顯式析構
  3. 不要 delete placement new 的指標
