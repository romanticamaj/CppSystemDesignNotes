# Merge & Set Operations

合併與集合操作，適用於已排序範圍。

## merge

合併兩個已排序範圍

```cpp
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt merge(InputIt1 first1, InputIt1 last1,
               InputIt2 first2, InputIt2 last2,
               OutputIt d_first) {
    for (; first1 != last1; ++d_first) {
        if (first2 == last2)
            return std::copy(first1, last1, d_first);
        if (*first2 < *first1) {
            *d_first = *first2;
            ++first2;
        } else {
            *d_first = *first1;
            ++first1;
        }
    }
    return std::copy(first2, last2, d_first);
}
```

## inplace_merge

原地合併連續已排序範圍 [first, middle) 和 [middle, last)

```cpp
template<class BidirIt>
void inplace_merge(BidirIt first, BidirIt middle, BidirIt last);

template<class BidirIt, class Compare>
void inplace_merge(BidirIt first, BidirIt middle, BidirIt last, Compare comp);
```

## includes

測試已排序範圍是否包含另一個已排序範圍

```cpp
template<class InputIt1, class InputIt2>
bool includes(InputIt1 first1, InputIt1 last1,
              InputIt2 first2, InputIt2 last2) {
    for (; first2 != last2; ++first1) {
        if (first1 == last1 || *first2 < *first1)
            return false;
        if (!(*first1 < *first2))
            ++first2;
    }
    return true;
}
```

## set_union

已排序範圍的聯集

```cpp
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt set_union(InputIt1 first1, InputIt1 last1,
                   InputIt2 first2, InputIt2 last2,
                   OutputIt d_first) {
    for (; first1 != last1; ++d_first) {
        if (first2 == last2)
            return std::copy(first1, last1, d_first);
        if (*first2 < *first1) {
            *d_first = *first2++;
        } else {
            if (!(*first1 < *first2))
                ++first2;
            *d_first = *first1++;
        }
    }
    return std::copy(first2, last2, d_first);
}
```

## set_intersection

已排序範圍的交集

```cpp
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt set_intersection(InputIt1 first1, InputIt1 last1,
                          InputIt2 first2, InputIt2 last2,
                          OutputIt d_first) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 < *first2) {
            ++first1;
        } else {
            if (!(*first2 < *first1))
                *d_first++ = *first1++;
            ++first2;
        }
    }
    return d_first;
}
```

## set_difference

已排序範圍的差集（在第一個範圍但不在第二個）

```cpp
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt set_difference(InputIt1 first1, InputIt1 last1,
                        InputIt2 first2, InputIt2 last2,
                        OutputIt d_first) {
    while (first1 != last1) {
        if (first2 == last2)
            return std::copy(first1, last1, d_first);
        if (*first1 < *first2) {
            *d_first++ = *first1++;
        } else {
            if (!(*first2 < *first1))
                ++first1;
            ++first2;
        }
    }
    return d_first;
}
```

## set_symmetric_difference

已排序範圍的對稱差集（在其中一個範圍但不在兩者都有）

```cpp
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt set_symmetric_difference(InputIt1 first1, InputIt1 last1,
                                  InputIt2 first2, InputIt2 last2,
                                  OutputIt d_first) {
    while (first1 != last1) {
        if (first2 == last2)
            return std::copy(first1, last1, d_first);
        if (*first1 < *first2) {
            *d_first++ = *first1++;
        } else if (*first2 < *first1) {
            *d_first++ = *first2++;
        } else {
            ++first1;
            ++first2;
        }
    }
    return std::copy(first2, last2, d_first);
}
```
