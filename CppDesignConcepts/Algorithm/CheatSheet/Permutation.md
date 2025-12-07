# Permutation & Comparison Operations

排列與比較操作。

## next_permutation

轉換為下一個字典序排列

```cpp
template<class BidirIt>
bool next_permutation(BidirIt first, BidirIt last) {
    if (first == last) return false;
    BidirIt i = last;
    if (first == --i) return false;

    while (true) {
        BidirIt i1, i2;
        i1 = i;
        if (*--i < *i1) {
            i2 = last;
            while (!(*i < *--i2))
                ;
            std::iter_swap(i, i2);
            std::reverse(i1, last);
            return true;
        }
        if (i == first) {
            std::reverse(first, last);
            return false;
        }
    }
}
```

回傳 false 表示已到最後排列，重置為第一個排列。

## prev_permutation

轉換為前一個字典序排列

```cpp
template<class BidirIt>
bool prev_permutation(BidirIt first, BidirIt last) {
    if (first == last) return false;
    BidirIt i = last;
    if (first == --i) return false;

    while (true) {
        BidirIt i1, i2;
        i1 = i;
        if (*i1 < *--i) {
            i2 = last;
            while (!(*--i2 < *i))
                ;
            std::iter_swap(i, i2);
            std::reverse(i1, last);
            return true;
        }
        if (i == first) {
            std::reverse(first, last);
            return false;
        }
    }
}
```

## is_permutation

測試一個範圍是否為另一個的排列（C++11）

```cpp
template<class ForwardIt1, class ForwardIt2>
bool is_permutation(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2) {
    // 找到第一個不匹配的位置
    auto [it1, it2] = std::mismatch(first1, last1, first2);

    if (it1 == last1) return true;

    auto last2 = std::next(first2, std::distance(first1, last1));

    for (auto i = it1; i != last1; ++i) {
        // 跳過已檢查過的元素
        if (std::find(it1, i, *i) != i)
            continue;

        auto count2 = std::count(it2, last2, *i);
        if (count2 == 0 || std::count(i, last1, *i) != count2)
            return false;
    }
    return true;
}
```

## lexicographical_compare

字典序比較兩個範圍

```cpp
template<class InputIt1, class InputIt2>
bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                             InputIt2 first2, InputIt2 last2) {
    for (; (first1 != last1) && (first2 != last2); ++first1, ++first2) {
        if (*first1 < *first2) return true;
        if (*first2 < *first1) return false;
    }
    return (first1 == last1) && (first2 != last2);
}
```

回傳 true 表示第一個範圍字典序小於第二個。

## lexicographical_compare_three_way

三向字典序比較（C++20）

```cpp
template<class InputIt1, class InputIt2, class Cmp>
auto lexicographical_compare_three_way(InputIt1 first1, InputIt1 last1,
                                       InputIt2 first2, InputIt2 last2,
                                       Cmp comp)
    -> decltype(comp(*first1, *first2)) {
    for (; (first1 != last1) && (first2 != last2); ++first1, ++first2) {
        if (auto cmp = comp(*first1, *first2); cmp != 0)
            return cmp;
    }
    return (first1 != last1) ? std::strong_ordering::greater :
           (first2 != last2) ? std::strong_ordering::less :
                               std::strong_ordering::equal;
}
```
