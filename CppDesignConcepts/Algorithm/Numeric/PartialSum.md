# `std::partial_sum` Design Analysis

## Core Concept

`std::partial_sum` computes the partial sums (prefix sums) of a range of elements. Each element in the output range contains the sum of all elements from the beginning of the input range up to and including the current position. This is also known as a cumulative sum or running total.

## Common Use Cases

- Computing prefix sums for range sum queries (very common in LeetCode)
- Dynamic programming problems involving cumulative values
- Financial calculations (running totals, cumulative profits)
- Image processing (integral images)
- Efficient range sum calculations in O(1) time after O(n) preprocessing

## C++ Example

```cpp
#include <numeric>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::vector<int> prefix_sums(numbers.size());

    // Compute prefix sums
    std::partial_sum(numbers.begin(), numbers.end(), prefix_sums.begin());

    std::cout << "Original: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 1 2 3 4 5

    std::cout << "Prefix sums: ";
    for (int n : prefix_sums) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 1 3 6 10 15

    // Using custom operation (multiplication instead of addition)
    std::vector<int> prefix_products(numbers.size());
    std::partial_sum(numbers.begin(), numbers.end(), prefix_products.begin(),
                     std::multiplies<int>());

    std::cout << "Prefix products: ";
    for (int n : prefix_products) std::cout << n << " ";
    std::cout << std::endl;
    // Output: 1 2 6 24 120

    // Range sum query example using prefix sums
    // To get sum from index i to j: prefix_sums[j] - prefix_sums[i-1]
    int range_sum = prefix_sums[3] - prefix_sums[0]; // sum from index 1 to 3
    std::cout << "Sum from index 1 to 3: " << range_sum << std::endl; // Output: 9

    // Special case: sum from index 0 to j (simplified - no subtraction needed)
    int sum_from_start = prefix_sums[3]; // sum from index 0 to 3
    std::cout << "Sum from index 0 to 3: " << sum_from_start << std::endl; // Output: 10

    return 0;
}
```

## Iterator Design Concept

### Third Parameter: Output Iterator

The third parameter of `partial_sum` uses **OutputIterator** design, embodying the input-output separation principle:

```cpp
template< class InputIt, class OutputIt >
OutputIt partial_sum(InputIt first, InputIt last, OutputIt d_first);
```

**Design Advantages:**

- **Flexibility** - Can choose in-place modification or write to a new container
- **Type Decoupling** - Input and output can use different container types
- **Stream Output** - Supports direct output to `ostream_iterator`

```cpp
std::vector<int> nums = {1, 2, 3};
std::vector<long long> prefix(3);  // Different type

// Write to a different type container
std::partial_sum(nums.begin(), nums.end(), prefix.begin());
```

For detailed design analysis, see: [`output_iterator_pattern.md`](../Iterator/output_iterator_pattern.md)

## Complexity Analysis

- **Time Complexity:** O(n) - where n is the number of elements in the range
- **Space Complexity:** O(1) additional space (if output range is provided), O(n) if creating new container

## Common Pitfalls and Best Practices

### Overflow Risk

When using `std::partial_sum`, accumulation may cause integer overflow:

```cpp
// Dangerous: may overflow
std::vector<int> nums = {1000000000, 1000000000, 1000000000};
std::vector<int> prefix(3);
std::partial_sum(nums.begin(), nums.end(), prefix.begin());
// prefix[2] will overflow!

// Safe: use a larger integer type
std::vector<int> nums = {1000000000, 1000000000, 1000000000};
std::vector<long long> prefix(3);  // Use long long
std::partial_sum(nums.begin(), nums.end(), prefix.begin());
// prefix: {1000000000, 2000000000, 3000000000}
```

**LeetCode Best Practice:**

```cpp
// Template: overflow-safe prefix sum
std::vector<int> nums = {/* ... */};
std::vector<long long> prefix(nums.size());  // Always use long long
std::partial_sum(nums.begin(), nums.end(), prefix.begin());
```

## LeetCode Applications

This algorithm is fundamental for many LeetCode problems:

- Range Sum Query problems
- Subarray sum problems
- 2D matrix prefix sum problems
- Problems involving cumulative calculations

## Related Algorithms

- `std::accumulate` - computes single sum of all elements
- `std::inclusive_scan` (C++17) - similar to partial_sum but more flexible
- `std::exclusive_scan` (C++17) - computes prefix sums excluding current element
