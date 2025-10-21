# Algorithm: `std::sort`

## 1. Core Concept

`std::sort` is a highly optimized sorting algorithm that typically uses a hybrid approach (often Introsort - a combination of quicksort, heapsort, and insertion sort). It sorts elements in ascending order by default, but can accept custom comparison functions for different sorting criteria.

## 2. Common Use Cases

- Sorting arrays and vectors (extremely common in LeetCode)
- Preprocessing data before applying other algorithms
- Sorting with custom comparators (by specific criteria)
- Preparing data for binary search operations
- Solving problems that require sorted input
- Merge intervals, meeting rooms, and scheduling problems

## 3. C++ Example

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

int main() {
    // Basic sorting
    std::vector<int> numbers = {64, 34, 25, 12, 22, 11, 90};
    std::sort(numbers.begin(), numbers.end());
    
    std::cout << "Sorted numbers: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 11 12 22 25 34 64 90
    
    // Sorting in descending order
    std::vector<int> desc_numbers = {64, 34, 25, 12, 22, 11, 90};
    std::sort(desc_numbers.begin(), desc_numbers.end(), std::greater<int>());
    
    std::cout << "Descending: ";
    for (int n : desc_numbers) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 90 64 34 25 22 12 11
    
    // Custom comparator - sort by string length
    std::vector<std::string> words = {"apple", "pie", "washington", "book"};
    std::sort(words.begin(), words.end(), 
              [](const std::string& a, const std::string& b) {
                  return a.length() < b.length();
              });
    
    std::cout << "Sorted by length: ";
    for (const auto& word : words) std::cout << word << " ";
    std::cout << std::endl;
    // Output: pie book apple washington
    
    // Sorting pairs (common in LeetCode)
    std::vector<std::pair<int, int>> pairs = {{3, 1}, {1, 2}, {2, 1}, {1, 1}};
    std::sort(pairs.begin(), pairs.end());
    
    std::cout << "Sorted pairs: ";
    for (const auto& p : pairs) {
        std::cout << "(" << p.first << "," << p.second << ") ";
    }
    std::cout << std::endl;
    // Output: (1,1) (1,2) (2,1) (3,1)
    
    // Partial sorting - only sort first k elements
    std::vector<int> partial = {64, 34, 25, 12, 22, 11, 90};
    std::partial_sort(partial.begin(), partial.begin() + 3, partial.end());
    
    std::cout << "Partial sort (first 3): ";
    for (int n : partial) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 11 12 22 64 34 25 90 (first 3 are smallest)
    
    return 0;
}
```

## 4. Complexity Analysis

- **Time Complexity:** O(n log n) average and worst case
- **Space Complexity:** O(log n) - due to recursion in the underlying algorithm

## LeetCode Applications

`std::sort` is used in countless LeetCode problems:
- Array sorting problems
- Merge intervals
- Meeting rooms scheduling
- Sorting before two-pointer techniques
- Custom sorting criteria problems
- Preprocessing for greedy algorithms

## Related Algorithms

- `std::stable_sort` - maintains relative order of equal elements
- `std::partial_sort` - sorts only the first k elements
- `std::nth_element` - finds the nth smallest element
- `std::is_sorted` - checks if range is sorted
- `std::sort_heap` - sorts a heap