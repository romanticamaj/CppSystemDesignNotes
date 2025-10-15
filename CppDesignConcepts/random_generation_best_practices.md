# C++ 隨機數生成：傳統 vs 現代方法

## 核心差異對照

### 傳統方法 (C-style)

```cpp
#include <cstdlib>
#include <ctime>

std::srand(std::time(nullptr)); // 只做一次
int value = std::rand(); // 範圍: [0, RAND_MAX] (至少 32767)
```

**問題**：

- 品質差：線性同餘生成器 (LCG)，週期短
- 不可移植：實作因編譯器而異
- 難以控制分佈：需手動取模 `% n`，有偏態問題
- 全域狀態：非執行緒安全

### 現代方法 (C++11+)

```cpp
#include <random>

std::mt19937 rng(std::random_device{}()); // 初始化一次
std::uniform_int_distribution<int> dist(1, 100);
int value = dist(rng);
```

**優勢**：

- 品質高：Mersenne Twister (MT19937)，週期 2^19937-1
- 可預測性：相同 seed 保證相同序列
- 精確分佈：uniform/normal/binomial 等數學正確
- 物件化：可多個獨立生成器

## 常見錯誤：重複 Seeding

### ❌ 錯誤範例

```cpp
int rollDice() {
    std::srand(std::time(nullptr)); // 每次呼叫都 seed
    return std::rand() % 6 + 1;
}

// 問題：同一秒內連續呼叫會得到相同結果
for (int i = 0; i < 10; ++i) {
    std::cout << rollDice() << " "; // 可能輸出: 3 3 3 3 3...
}
```

**原因**：`time(nullptr)` 精度是秒，短時間內重複 seed 會重置生成器狀態。

### ✅ 正確做法

#### 方案 1：全域初始化一次

```cpp
// 在 main() 或全域範圍
std::srand(std::time(nullptr)); // 只做一次

int rollDice() {
    return std::rand() % 6 + 1; // 直接使用
}
```

#### 方案 2：物件化（推薦）

```cpp
class DiceRoller {
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
public:
    DiceRoller()
        : rng(std::random_device{}()), dist(1, 6) {} // 建構子 seed 一次

    int roll() { return dist(rng); }
};

// 使用
DiceRoller roller;
for (int i = 0; i < 10; ++i) {
    std::cout << roller.roll() << " "; // 真正隨機
}
```

## 使用習慣建議

### 快速原型/競賽

```cpp
// main() 開頭
std::srand(std::time(nullptr));

// 隨處使用
int x = std::rand() % 100;
```

適用於不要求高品質隨機的場景。

### 生產環境/面試

```cpp
// 類別成員或區域靜態變數
class Simulation {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<> dist{0.0, 1.0};

public:
    double sample() { return dist(rng); }
};

// 或使用區域 static
int randomInt(int min, int max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}
```

### 可重現性測試

```cpp
std::mt19937 rng(42); // 固定 seed
// 每次執行都會得到相同的隨機序列
```

## 面試重點

1. **Seeding 只做一次**：在建構子或靜態初始化，不在迴圈/函式內
2. **優先使用 `<random>`**：展現現代 C++ 知識
3. **明確分佈需求**：避免 `% n` 偏態，使用 `uniform_int_distribution`
4. **執行緒安全**：每個執行緒應有獨立的 RNG 物件

## 快速對照表

| 特性       | `rand()`                 | `<random>`                |
| ---------- | ------------------------ | ------------------------- |
| 輸出範圍   | `[0, RAND_MAX]` (≥32767) | 依生成器 (MT19937: 32 位) |
| 品質       | 差                       | 優 (MT19937)              |
| 可移植性   | 弱                       | 強                        |
| 分佈精確性 | 需手動處理               | 內建多種分佈              |
| 執行緒安全 | ❌                       | ✅ (物件隔離)             |
| 可重現性   | 有限                     | 完全可控                  |
| 適用場景   | 快速原型                 | 生產環境/模擬             |
