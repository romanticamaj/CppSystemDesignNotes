# Min/Max Operations

最大最小值相關操作。

## min

回傳較小值

```cpp
template<class T>
const T& min(const T& a, const T& b) {
    return (b < a) ? b : a;
}

template<class T, class Compare>
const T& min(const T& a, const T& b, Compare comp) {
    return comp(b, a) ? b : a;
}
```

initializer_list 版本（C++11）：

```cpp
template<class T>
T min(std::initializer_list<T> ilist) {
    return *std::min_element(ilist.begin(), ilist.end());
}
```

## max

回傳較大值

```cpp
template<class T>
const T& max(const T& a, const T& b) {
    return (a < b) ? b : a;
}

template<class T, class Compare>
const T& max(const T& a, const T& b, Compare comp) {
    return comp(a, b) ? b : a;
}
```

## minmax

回傳最小和最大值（C++11）

```cpp
template<class T>
std::pair<const T&, const T&> minmax(const T& a, const T& b) {
    return (b < a) ? std::pair<const T&, const T&>(b, a)
                   : std::pair<const T&, const T&>(a, b);
}
```

## min_element

回傳範圍內最小元素的迭代器

```cpp
template<class ForwardIt>
ForwardIt min_element(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt smallest = first;
    ++first;
    for (; first != last; ++first)
        if (*first < *smallest)
            smallest = first;
    return smallest;
}
```

## max_element

回傳範圍內最大元素的迭代器

```cpp
template<class ForwardIt>
ForwardIt max_element(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt largest = first;
    ++first;
    for (; first != last; ++first)
        if (*largest < *first)
            largest = first;
    return largest;
}
```

## minmax_element

回傳範圍內最小和最大元素的迭代器（C++11）

```cpp
template<class ForwardIt>
std::pair<ForwardIt, ForwardIt>
minmax_element(ForwardIt first, ForwardIt last) {
    auto min = first, max = first;
    if (first == last || ++first == last)
        return {min, max};

    if (*first < *min) min = first;
    else max = first;

    while (++first != last) {
        auto i = first;
        if (++first == last) {
            if (*i < *min) min = i;
            else if (!(*i < *max)) max = i;
            break;
        }
        if (*first < *i) {
            if (*first < *min) min = first;
            if (!(*i < *max)) max = i;
        } else {
            if (*i < *min) min = i;
            if (!(*first < *max)) max = first;
        }
    }
    return {min, max};
}
```

## clamp

將值限制在範圍內（C++17）

```cpp
template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

template<class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}
```
