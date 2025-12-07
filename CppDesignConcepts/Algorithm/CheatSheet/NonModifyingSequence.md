# Non-Modifying Sequence Operations

不修改序列的操作，用於搜尋、計數、比較。

## all_of

測試所有元素是否滿足條件

```cpp
template<class InputIt, class UnaryPred>
bool all_of(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (!p(*first)) return false;
    return true;
}
```

## any_of

測試是否有任一元素滿足條件

```cpp
template<class InputIt, class UnaryPred>
bool any_of(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (p(*first)) return true;
    return false;
}
```

## none_of

測試是否沒有元素滿足條件

```cpp
template<class InputIt, class UnaryPred>
bool none_of(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (p(*first)) return false;
    return true;
}
```

## for_each

對範圍內每個元素執行函式

```cpp
template<class InputIt, class UnaryFunc>
UnaryFunc for_each(InputIt first, InputIt last, UnaryFunc f) {
    for (; first != last; ++first)
        f(*first);
    return f;
}
```

## find

在範圍內尋找值

```cpp
template<class InputIt, class T>
InputIt find(InputIt first, InputIt last, const T& value) {
    for (; first != last; ++first)
        if (*first == value) return first;
    return last;
}
```

## find_if

在範圍內尋找滿足條件的元素

```cpp
template<class InputIt, class UnaryPred>
InputIt find_if(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (p(*first)) return first;
    return last;
}
```

## find_if_not

在範圍內尋找不滿足條件的元素

```cpp
template<class InputIt, class UnaryPred>
InputIt find_if_not(InputIt first, InputIt last, UnaryPred p) {
    for (; first != last; ++first)
        if (!p(*first)) return first;
    return last;
}
```

## count

計算值在範圍內出現次數

```cpp
template<class InputIt, class T>
typename iterator_traits<InputIt>::difference_type
count(InputIt first, InputIt last, const T& value) {
    typename iterator_traits<InputIt>::difference_type n = 0;
    for (; first != last; ++first)
        if (*first == value) ++n;
    return n;
}
```

## count_if

計算滿足條件的元素數量

```cpp
template<class InputIt, class UnaryPred>
typename iterator_traits<InputIt>::difference_type
count_if(InputIt first, InputIt last, UnaryPred p) {
    typename iterator_traits<InputIt>::difference_type n = 0;
    for (; first != last; ++first)
        if (p(*first)) ++n;
    return n;
}
```

## equal

測試兩個範圍是否相等

```cpp
template<class InputIt1, class InputIt2>
bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
    for (; first1 != last1; ++first1, ++first2)
        if (!(*first1 == *first2)) return false;
    return true;
}
```

## mismatch

回傳兩個範圍第一個不同的位置

```cpp
template<class InputIt1, class InputIt2>
std::pair<InputIt1, InputIt2>
mismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
    while (first1 != last1 && *first1 == *first2) {
        ++first1;
        ++first2;
    }
    return {first1, first2};
}
```

## adjacent_find

尋找相鄰相等的元素

```cpp
template<class ForwardIt>
ForwardIt adjacent_find(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt next = first;
    ++next;
    for (; next != last; ++next, ++first)
        if (*first == *next) return first;
    return last;
}
```

## search

在範圍內搜尋子序列

```cpp
template<class ForwardIt1, class ForwardIt2>
ForwardIt1 search(ForwardIt1 first, ForwardIt1 last,
                  ForwardIt2 s_first, ForwardIt2 s_last) {
    for (;; ++first) {
        ForwardIt1 it = first;
        for (ForwardIt2 s_it = s_first;; ++it, ++s_it) {
            if (s_it == s_last) return first;
            if (it == last) return last;
            if (!(*it == *s_it)) break;
        }
    }
}
```
