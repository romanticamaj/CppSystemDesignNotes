# C++11 std::random

## 核心概念

`std::random` 是 C++11 引入的隨機數生成庫，它提供了一套完整、靈活且高效的隨機數生成工具。這個庫的設計遵循了現代隨機數生成的最佳實踐，將隨機數生成過程分為兩個關鍵部分：

1. **隨機數引擎（Random Number Engines）**：產生原始隨機位序列的算法
2. **隨機數分佈（Random Number Distributions）**：將引擎產生的原始隨機數轉換為特定統計分佈的算法

這種分離設計使得開發者可以獨立選擇適合的引擎和分佈，從而滿足不同應用場景的需求。相比於 C 標準庫中的 `rand()` 函數，C++11 的隨機數庫提供了更高的隨機性質量、更好的性能、更多的分佈類型以及更強的可重現性。

## 語法

### 隨機數引擎

```cpp
// 常用的隨機數引擎
std::mt19937 engine;  // Mersenne Twister 32位版本
std::mt19937_64 engine64;  // Mersenne Twister 64位版本
std::minstd_rand engine_simple;  // 最小標準線性同余引擎
std::ranlux24 engine_luxury;  // 高品質隨機數引擎

// 初始化引擎
std::random_device rd;  // 用於獲取真隨機種子
std::mt19937 engine(rd());  // 使用隨機設備初始化
std::mt19937 engine_seeded(42);  // 使用固定種子初始化

// 生成原始隨機數
auto random_value = engine();  // 產生一個隨機整數
```

### 隨機數分佈

```cpp
// 均勻分佈
std::uniform_int_distribution<int> dist_int(1, 6);  // 整數均勻分佈 [1, 6]
std::uniform_real_distribution<double> dist_real(0.0, 1.0);  // 浮點數均勻分佈 [0.0, 1.0)

// 其他常見分佈
std::normal_distribution<double> dist_normal(0.0, 1.0);  // 正態分佈 (均值=0.0, 標準差=1.0)
std::bernoulli_distribution dist_bernoulli(0.5);  // 伯努利分佈 (p=0.5)
std::binomial_distribution<int> dist_binomial(10, 0.5);  // 二項分佈 (n=10, p=0.5)
std::poisson_distribution<int> dist_poisson(5.0);  // 泊松分佈 (λ=5.0)

// 使用分佈生成隨機數
int dice_roll = dist_int(engine);  // 生成 1-6 的隨機整數
double random_probability = dist_real(engine);  // 生成 [0,1) 範圍的隨機浮點數
```

### 引擎和分佈的組合使用

```cpp
// 完整的隨機數生成過程
std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> dist(5.0, 2.0);

double random_value = dist(gen);  // 從正態分佈 N(5.0, 2.0) 生成隨機數
```

## 與相似概念的比較

| 特性 | std::random | C 風格 rand() | Boost.Random | 系統特定 API |
|------|-------------|--------------|-------------|-------------|
| 隨機性質量 | 高 | 低 | 高 | 視實現而定 |
| 週期長度 | 非常長 (2^19937-1 for mt19937) | 較短 (RAND_MAX 通常為 2^15-1 或 2^31-1) | 非常長 | 視實現而定 |
| 分佈類型 | 豐富 | 僅均勻分佈 | 豐富 | 有限 |
| 可重現性 | 強 | 有限 | 強 | 弱 |
| 執行效率 | 高 | 中等 | 高 | 視實現而定 |
| 線程安全 | 需手動處理 | 否 | 需手動處理 | 視實現而定 |
| 跨平台一致性 | 高 | 低 | 高 | 低 |
| 標準庫一部分 | 是 (C++11) | 是 (C) | 否 | 否 |

## 使用範例

### 基本用法：擲骰子模擬

```cpp
#include <iostream>
#include <random>
#include <map>
#include <iomanip>

int main() {
    // 創建隨機數引擎
    std::random_device rd;  // 獲取真隨機種子
    std::mt19937 gen(rd());  // 使用 Mersenne Twister 引擎
    
    // 創建均勻分佈 [1, 6]
    std::uniform_int_distribution<int> dist(1, 6);
    
    // 模擬擲骰子 10000 次
    std::map<int, int> counts;
    for (int i = 0; i < 10000; ++i) {
        ++counts[dist(gen)];
    }
    
    // 輸出結果
    std::cout << "擲骰子 10000 次的結果：" << std::endl;
    for (int i = 1; i <= 6; ++i) {
        std::cout << i << ": " << std::string(counts[i] / 100, '*') << " " 
                  << counts[i] << " (" 
                  << std::fixed << std::setprecision(2) 
                  << counts[i] / 100.0 << "%)" << std::endl;
    }
    
    return 0;
}
```

### 生成不同分佈的隨機數

```cpp
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>

// 輔助函數：計算平均值和標準差
void calculate_stats(const std::vector<double>& values, double& mean, double& stddev) {
    mean = 0.0;
    for (double value : values) {
        mean += value;
    }
    mean /= values.size();
    
    stddev = 0.0;
    for (double value : values) {
        stddev += (value - mean) * (value - mean);
    }
    stddev = std::sqrt(stddev / values.size());
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    const int num_samples = 100000;
    
    // 1. 均勻整數分佈
    std::uniform_int_distribution<int> uniform_int(1, 100);
    std::vector<double> uniform_int_samples;
    for (int i = 0; i < num_samples; ++i) {
        uniform_int_samples.push_back(uniform_int(gen));
    }
    
    // 2. 均勻實數分佈
    std::uniform_real_distribution<double> uniform_real(0.0, 1.0);
    std::vector<double> uniform_real_samples;
    for (int i = 0; i < num_samples; ++i) {
        uniform_real_samples.push_back(uniform_real(gen));
    }
    
    // 3. 正態分佈
    std::normal_distribution<double> normal(0.0, 1.0);
    std::vector<double> normal_samples;
    for (int i = 0; i < num_samples; ++i) {
        normal_samples.push_back(normal(gen));
    }
    
    // 4. 指數分佈
    std::exponential_distribution<double> exponential(1.0);
    std::vector<double> exponential_samples;
    for (int i = 0; i < num_samples; ++i) {
        exponential_samples.push_back(exponential(gen));
    }
    
    // 5. 泊松分佈
    std::poisson_distribution<int> poisson(5.0);
    std::vector<double> poisson_samples;
    for (int i = 0; i < num_samples; ++i) {
        poisson_samples.push_back(poisson(gen));
    }
    
    // 計算並輸出統計信息
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "分佈類型\t\t平均值\t\t標準差\t\t最小值\t\t最大值" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    double mean, stddev;
    
    calculate_stats(uniform_int_samples, mean, stddev);
    std::cout << "均勻整數 [1,100]\t" << mean << "\t" << stddev << "\t" 
              << *std::min_element(uniform_int_samples.begin(), uniform_int_samples.end()) << "\t\t"
              << *std::max_element(uniform_int_samples.begin(), uniform_int_samples.end()) << std::endl;
    
    calculate_stats(uniform_real_samples, mean, stddev);
    std::cout << "均勻實數 [0,1)\t\t" << mean << "\t" << stddev << "\t" 
              << *std::min_element(uniform_real_samples.begin(), uniform_real_samples.end()) << "\t"
              << *std::max_element(uniform_real_samples.begin(), uniform_real_samples.end()) << std::endl;
    
    calculate_stats(normal_samples, mean, stddev);
    std::cout << "正態 N(0,1)\t\t" << mean << "\t" << stddev << "\t" 
              << *std::min_element(normal_samples.begin(), normal_samples.end()) << "\t"
              << *std::max_element(normal_samples.begin(), normal_samples.end()) << std::endl;
    
    calculate_stats(exponential_samples, mean, stddev);
    std::cout << "指數 λ=1\t\t" << mean << "\t" << stddev << "\t" 
              << *std::min_element(exponential_samples.begin(), exponential_samples.end()) << "\t"
              << *std::max_element(exponential_samples.begin(), exponential_samples.end()) << std::endl;
    
    calculate_stats(poisson_samples, mean, stddev);
    std::cout << "泊松 λ=5\t\t" << mean << "\t" << stddev << "\t" 
              << *std::min_element(poisson_samples.begin(), poisson_samples.end()) << "\t\t"
              << *std::max_element(poisson_samples.begin(), poisson_samples.end()) << std::endl;
    
    return 0;
}
```

### 可重現的隨機序列

```cpp
#include <iostream>
#include <random>
#include <vector>

void generate_and_print(std::mt19937& gen, const std::string& description) {
    std::uniform_int_distribution<int> dist(1, 100);
    
    std::cout << description << ": ";
    for (int i = 0; i < 5; ++i) {
        std::cout << dist(gen) << " ";
    }
    std::cout << std::endl;
}

int main() {
    // 使用固定種子初始化兩個引擎
    std::mt19937 gen1(42);
    std::mt19937 gen2(42);
    
    // 使用隨機設備初始化第三個引擎
    std::random_device rd;
    std::mt19937 gen3(rd());
    
    // 生成並輸出隨機序列
    generate_and_print(gen1, "引擎 1 (種子=42)，第一次");
    generate_and_print(gen2, "引擎 2 (種子=42)，第一次");
    generate_and_print(gen3, "引擎 3 (隨機種子)，第一次");
    
    std::cout << std::endl;
    
    // 再次生成並輸出隨機序列
    generate_and_print(gen1, "引擎 1 (種子=42)，第二次");
    generate_and_print(gen2, "引擎 2 (種子=42)，第二次");
    generate_and_print(gen3, "引擎 3 (隨機種子)，第二次");
    
    // 重置第一個引擎
    gen1.seed(42);
    std::cout << std::endl;
    generate_and_print(gen1, "引擎 1 (重置為種子=42)");
    
    // 保存和恢復引擎狀態
    std::cout << std::endl << "保存和恢復引擎狀態：" << std::endl;
    
    std::mt19937 gen4(42);
    generate_and_print(gen4, "引擎 4 (種子=42)，初始狀態");
    
    // 生成一些隨機數，改變引擎狀態
    for (int i = 0; i < 10; ++i) {
        gen4();
    }
    
    // 保存當前狀態
    std::stringstream state_stream;
    state_stream << gen4;
    generate_and_print(gen4, "引擎 4，生成 10 個數後");
    
    // 創建新引擎並恢復保存的狀態
    std::mt19937 gen5;
    state_stream >> gen5;
    generate_and_print(gen5, "引擎 5，從引擎 4 的狀態恢復");
    
    return 0;
}
```

### 自定義隨機數生成器

```cpp
#include <iostream>
#include <random>
#include <functional>
#include <string>

// 自定義隨機數生成器類
class RandomGenerator {
private:
    std::mt19937 engine;
    std::uniform_int_distribution<int> int_dist;
    std::uniform_real_distribution<double> real_dist;
    std::normal_distribution<double> normal_dist;
    
public:
    // 使用隨機設備初始化
    RandomGenerator() : engine(std::random_device{}()) {
        reset_distributions();
    }
    
    // 使用指定種子初始化
    RandomGenerator(unsigned int seed) : engine(seed) {
        reset_distributions();
    }
    
    // 重置分佈
    void reset_distributions() {
        int_dist = std::uniform_int_distribution<int>(1, 100);
        real_dist = std::uniform_real_distribution<double>(0.0, 1.0);
        normal_dist = std::normal_distribution<double>(0.0, 1.0);
    }
    
    // 設置新種子
    void seed(unsigned int new_seed) {
        engine.seed(new_seed);
    }
    
    // 生成 [min, max] 範圍內的隨機整數
    int next_int(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(engine);
    }
    
    // 使用預設範圍 [1, 100] 生成隨機整數
    int next_int() {
        return int_dist(engine);
    }
    
    // 生成 [min, max) 範圍內的隨機浮點數
    double next_double(double min, double max) {
        return std::uniform_real_distribution<double>(min, max)(engine);
    }
    
    // 使用預設範圍 [0.0, 1.0) 生成隨機浮點數
    double next_double() {
        return real_dist(engine);
    }
    
    // 從正態分佈生成隨機數
    double next_normal(double mean = 0.0, double stddev = 1.0) {
        return std::normal_distribution<double>(mean, stddev)(engine);
    }
    
    // 生成隨機布爾值，p 是返回 true 的概率
    bool next_bool(double p = 0.5) {
        return std::bernoulli_distribution(p)(engine);
    }
    
    // 從給定集合中隨機選擇一個元素
    template<typename T>
    const T& choice(const std::vector<T>& collection) {
        if (collection.empty()) {
            throw std::invalid_argument("Cannot choose from an empty collection");
        }
        return collection[next_int(0, collection.size() - 1)];
    }
    
    // 生成隨機字符串
    std::string next_string(size_t length, const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            result.push_back(choice(charset));
        }
        
        return result;
    }
    
    // 洗牌算法
    template<typename T>
    void shuffle(std::vector<T>& collection) {
        for (size_t i = collection.size() - 1; i > 0; --i) {
            size_t j = next_int(0, i);
            std::swap(collection[i], collection[j]);
        }
    }
};

int main() {
    // 創建隨機數生成器
    RandomGenerator rng(42);  // 使用固定種子以便結果可重現
    
    // 生成基本隨機數
    std::cout << "隨機整數 [1, 100]: " << rng.next_int() << std::endl;
    std::cout << "隨機整數 [5, 10]: " << rng.next_int(5, 10) << std::endl;
    std::cout << "隨機浮點數 [0, 1): " << rng.next_double() << std::endl;
    std::cout << "隨機浮點數 [5.0, 10.0): " << rng.next_double(5.0, 10.0) << std::endl;
    std::cout << "正態分佈 N(0,1): " << rng.next_normal() << std::endl;
    std::cout << "正態分佈 N(10,2): " << rng.next_normal(10.0, 2.0) << std::endl;
    std::cout << "隨機布爾值 (p=0.5): " << (rng.next_bool() ? "true" : "false") << std::endl;
    std::cout << "隨機布爾值 (p=0.8): " << (rng.next_bool(0.8) ? "true" : "false") << std::endl;
    
    // 生成隨機字符串
    std::cout << "隨機字符串 (長度=10): " << rng.next_string(10) << std::endl;
    std::cout << "隨機數字字符串 (長度=6): " << rng.next_string(6, "0123456789") << std::endl;
    
    // 從集合中隨機選擇
    std::vector<std::string> fruits = {"蘋果", "香蕉", "橙子", "葡萄", "草莓"};
    std::cout << "隨機水果: " << rng.choice(fruits) << std::endl;
    
    // 洗牌演示
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    rng.shuffle(numbers);
    
    std::cout << "洗牌後的數字: ";
    for (int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 蒙特卡羅方法估算 π 值

```cpp
#include <iostream>
#include <random>
#include <iomanip>
#include <cmath>

// 使用蒙特卡羅方法估算 π 值
double estimate_pi(int num_samples) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    
    int inside_circle = 0;
    
    for (int i = 0; i < num_samples; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        
        // 檢查點 (x, y) 是否在單位圓內
        if (x*x + y*y <= 1.0) {
            inside_circle++;
        }
    }
    
    // 單位圓在 [-1,1]×[-1,1] 正方形中的面積比例為 π/4
    return 4.0 * inside_circle / num_samples;
}

int main() {
    std::cout << std::fixed << std::setprecision(10);
    
    // 使用不同的樣本數估算 π 值
    std::cout << "估算 π 值：" << std::endl;
    std::cout << "實際 π 值: " << M_PI << std::endl;
    
    for (int samples = 100; samples <= 100000000; samples *= 100) {
        double pi_estimate = estimate_pi(samples);
        double error = std::abs(pi_estimate - M_PI);
        
        std::cout << "樣本數 = " << std::setw(9) << samples 
                  << ", 估算值 = " << pi_estimate 
                  << ", 誤差 = " << error << std::endl;
    }
    
    return 0;
}
```

## 使用情境

### 適合使用 std::random 的情況

1. **模擬與建模**：當需要進行科學模擬或統計建模時
   ```cpp
   // 模擬粒子布朗運動
   std::mt19937 gen(std::random_device{}());
   std::normal_distribution<double> dist(0.0, 1.0);
   
   double x = 0.0, y = 0.0;  // 初始位置
   for (int step = 0; step < 1000; ++step) {
       // 根據正態分佈更新位置
       x += dist(gen);
       y += dist(gen);
   }
   ```

2. **遊戲開發**：生成隨機事件、地圖、敵人等
   ```cpp
   // 生成隨機寶箱內容
   std::mt19937 gen(seed);
   std::discrete_distribution<int> loot_dist({70, 20, 9, 1});  // 普通、稀有、史詩、傳說物品的概率
   
   int item_rarity = loot_dist(gen);  // 0=普通, 1=稀有, 2=史詩, 3=傳說
   ```

3. **密碼學應用**：生成隨機密鑰、鹽值等
   ```cpp
   // 生成隨機密鑰
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_int_distribution<unsigned char> dist(0, 255);
   
   std::vector<unsigned char> key(32);  // 256 位密鑰
   for (auto& byte : key) {
       byte = dist(gen);
   }
   ```

4. **隨機算法**：如隨機化快速排序、隨機森林等
   ```cpp
   // 隨機化快速排序的樞軸選擇
   template<typename T>
   int random_partition(std::vector<T>& arr, int low, int high) {
       static std::mt19937 gen(std::random_device{}());
       std::uniform_int_distribution<int> dist(low, high);
       
       int random_index = dist(gen);
       std::swap(arr[random_index], arr[high]);
       
       // 標準分區過程
       T pivot = arr[high];
       int i = low - 1;
       
       for (int j = low; j < high; j++) {
           if (arr[j] <= pivot) {
               i++;
               std::swap(arr[i], arr[j]);
           }
       }
       
       std::swap(arr[i + 1], arr[high]);
       return i + 1;
   }
   ```

5. **測試數據生成**：創建隨機測試用例
   ```cpp
   // 生成隨機測試數據
   std::mt19937 gen(42);  // 固定種子以便測試可重現
   
   // 生成隨機整數數組
   std::vector<int> generate_test_array(size_t size, int min_val, int max_val) {
       std::uniform_int_distribution<int> dist(min_val, max_val);
       std::vector<int> result(size);
       
       for (auto& val : result) {
           val = dist(gen);
       }
       
       return result;
   }
   ```

### 不適合使用 std::random 的情況

1. **密碼學安全應用**：雖然 `std::random_device` 可能提供密碼學安全的隨機數，但這不是標準保證的
   ```cpp
   // 不要用於高安全性需求
   // 應該使用專門的密碼學庫
   #include <openssl/rand.h>
   
   unsigned char secure_random_bytes[32];
   RAND_bytes(secure_random_bytes, sizeof(secure_random_bytes));
   ```

2. **性能極度敏感的場景**：當需要大量生成隨機數且性能是關鍵因素時
   ```cpp
   // 對於極度性能敏感的場景，可能需要更簡單的生成器
   // 或使用 SIMD 優化的專用庫
   ```

3. **需要特殊分佈的場景**：當需要標準庫未提供的特殊統計分佈時
   ```cpp
   // 對於非標準分佈，可能需要自己實現或使用專門的統計庫
   // 例如 Beta 分佈、Weibull 分佈等
   ```

4. **嵌入式系統或資源受限環境**：當內存或計算資源有限時
   ```cpp
   // 在資源受限環境中，可能需要更輕量級的替代方案
   uint32_t lcg_next(uint32_t& state) {
       state = 1664525 * state + 1013904223;  // 線性同余生成器
       return state;
   }
   ```

### 最佳實踐

1. **選擇合適的引擎**：根據需求選擇適當的隨機數引擎
   ```cpp
   // 對於大多數應用，mt19937 是一個很好的選擇
   std::mt19937 gen(std::random_device{}());
   
   // 對於需要更長週期的應用
   std::mt19937_64 gen64(std::random_device{}());
   
   // 對於簡單應用，可以使用更輕量級的引擎
   std::minstd_rand simple_gen(std::random_device{}());
   ```

2. **正確初始化引擎**：使用好的種子來初始化隨機數引擎
   ```cpp
   // 不好的做法：使用可預測的種子
   std::mt19937 bad_gen(42);  // 除非需要可重現的結果
   
   // 好的做法：使用隨機設備
   std::random_device rd;
   std::mt19937 good_gen(rd());
   
   // 更好的做法：使用多個隨機源
   std::seed_seq seeds{rd(), rd(), rd(), rd(), rd(), rd()};
   std::mt19937 better_gen(seeds);
   ```

3. **重用引擎和分佈**：避免重複創建引擎和分佈對象
   ```cpp
   // 不好的做法：每次需要隨機數時創建新的引擎和分佈
   int get_random_bad() {
       std::mt19937 gen(std::random_device{}());  // 每次都創建新引擎
       std::uniform_int_distribution<int> dist(1, 100);
       return dist(gen);
   }
   
   // 好的做法：重用引擎和分佈
   int get_random_good() {
       static std::mt19937 gen(std::random_device{}());  // 只創建一次
       static std::uniform_int_distribution<int> dist(1, 100);
       return dist(gen);
   }
   ```

4. **處理線程安全**：在多線程環境中正確使