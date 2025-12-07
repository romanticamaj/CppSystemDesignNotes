# Binary Search Operations

二分搜尋相關操作，適用於已排序範圍。

## lower_bound

回傳第一個 >= value 的迭代器

```cpp
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt it;
    typename std::iterator_traits<ForwardIt>::difference_type count, step;
    count = std::distance(first, last);

    while (count > 0) {
        it = first;
        step = count / 2;
        std::advance(it, step);
        if (*it < value) {
            first = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}
```

## upper_bound

回傳第一個 > value 的迭代器

```cpp
template<class ForwardIt, class T>
ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt it;
    typename std::iterator_traits<ForwardIt>::difference_type count, step;
    count = std::distance(first, last);

    while (count > 0) {
        it = first;
        step = count / 2;
        std::advance(it, step);
        if (!(value < *it)) {
            first = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}
```

## binary_search

測試值是否存在於已排序範圍

```cpp
template<class ForwardIt, class T>
bool binary_search(ForwardIt first, ForwardIt last, const T& value) {
    first = std::lower_bound(first, last, value);
    return (first != last && !(value < *first));
}
```

## equal_range

取得等值元素的子範圍 [lower_bound, upper_bound)

```cpp
template<class ForwardIt, class T>
std::pair<ForwardIt, ForwardIt>
equal_range(ForwardIt first, ForwardIt last, const T& value) {
    return {std::lower_bound(first, last, value),
            std::upper_bound(first, last, value)};
}
```

更高效的實現會同時計算兩個邊界，避免重複搜尋。
