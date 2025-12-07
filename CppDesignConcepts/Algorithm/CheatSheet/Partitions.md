# Partition Operations

分割操作，將範圍分成滿足與不滿足條件的兩部分。

## partition

將範圍分成兩部分：滿足條件的在前，不滿足的在後

```cpp
template<class ForwardIt, class UnaryPred>
ForwardIt partition(ForwardIt first, ForwardIt last, UnaryPred p) {
    first = std::find_if_not(first, last, p);
    if (first == last) return first;

    for (auto i = std::next(first); i != last; ++i) {
        if (p(*i)) {
            std::iter_swap(i, first);
            ++first;
        }
    }
    return first;
}
```

回傳分割點（第一個不滿足條件的元素）。

## stable_partition

穩定分割，保持各組內元素的相對順序

```cpp
template<class BidirIt, class UnaryPred>
BidirIt stable_partition(BidirIt first, BidirIt last, UnaryPred p);
```

複雜度：若有額外記憶體則 O(n)，否則 O(n log n)

## is_partitioned

測試範圍是否已分割（C++11）

```cpp
template<class InputIt, class UnaryPred>
bool is_partitioned(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (!p(*first))
            break;
    for (; first != last; ++first)
        if (p(*first))
            return false;
    return true;
}
```

## partition_point

取得已分割範圍的分割點（C++11）

```cpp
template<class ForwardIt, class UnaryPred>
ForwardIt partition_point(ForwardIt first, ForwardIt last, UnaryPred p) {
    auto n = std::distance(first, last);
    while (n > 0) {
        auto half = n / 2;
        auto mid = std::next(first, half);
        if (p(*mid)) {
            first = std::next(mid);
            n -= half + 1;
        } else {
            n = half;
        }
    }
    return first;
}
```

複雜度：O(log n)（使用二分搜尋）

## partition_copy

分割並複製到兩個不同的輸出（C++11）

```cpp
template<class InputIt, class OutputIt1, class OutputIt2, class UnaryPred>
std::pair<OutputIt1, OutputIt2>
partition_copy(InputIt first, InputIt last,
               OutputIt1 d_first_true, OutputIt2 d_first_false,
               UnaryPred p) {
    for (; first != last; ++first) {
        if (p(*first)) {
            *d_first_true = *first;
            ++d_first_true;
        } else {
            *d_first_false = *first;
            ++d_first_false;
        }
    }
    return {d_first_true, d_first_false};
}
```
