# Algorithm: `std::count`

## 1. Core Concept

`std::count` counts the number of elements in a range that are equal to a specific value. It's a simple linear search algorithm that iterates through a container and increments a counter each time it finds a matching element.

**Key characteristics:**

- Uses `operator==` for comparison: `*iterator == value`
- Signature: `count(first, last, const T& value)`
- Accepts both l-values and r-values for the value parameter (const reference binding)

## 2. Common Use Cases

- Count occurrences of a specific value in an array or vector
- Validate data by counting expected elements
- Statistical analysis of datasets
- Frequency analysis in algorithms
- Common in LeetCode problems involving counting elements

## 3. C++ Example

### Basic Usage

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 2, 4, 2, 5};

    // Using r-value (literal)
    int count1 = std::count(numbers.begin(), numbers.end(), 2);  // Output: 3

    // Using l-value (variable)
    int target = 2;
    int count2 = std::count(numbers.begin(), numbers.end(), target);  // Output: 3

    // Using r-value (expression)
    int count3 = std::count(numbers.begin(), numbers.end(), 1 + 1);  // Output: 3

    return 0;
}
```

### Custom Types (Requires `operator==`)

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

struct Point {
    int x, y;

    // Must overload operator== for std::count
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

int main() {
    std::vector<Point> points = {{1,1}, {2,2}, {1,1}, {3,3}};
    Point target = {1, 1};

    int count = std::count(points.begin(), points.end(), target);
    std::cout << "Count of (1,1): " << count << std::endl;  // Output: 2

    return 0;
}
```

## 4. Implementation Details

### Comparison Mechanism

`std::count` uses `operator==` for equality comparison:

- **Built-in types**: Direct comparison (`int`, `char`, `double`, etc.)
- **Custom types**: Must overload `operator==`
- **Alternative**: Use `std::count_if` with custom predicate if `operator==` is not suitable

### L-value vs R-value Parameters

The value parameter is `const T& value`, which accepts both:

- **L-value** (named variable): Binds directly to const reference → no copy
- **R-value** (temporary/literal): Const reference extends lifetime → no extra copy

Both cases are **equally efficient** - no performance difference.

## 5. Complexity Analysis

- **Time Complexity:** O(n) - where n is the number of elements in the range
- **Space Complexity:** O(1) - constant space, only uses a counter variable

## Related Algorithms

- `std::count_if` - counts elements that satisfy a predicate
- `std::find` - finds the first occurrence of a value