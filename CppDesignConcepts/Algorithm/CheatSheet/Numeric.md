# Numeric Operations

數值演算法，來自 `<numeric>` 標頭。

## accumulate

累加範圍內元素

```cpp
template<class InputIt, class T>
T accumulate(InputIt first, InputIt last, T init) {
    for (; first != last; ++first)
        init = init + *first;
    return init;
}

template<class InputIt, class T, class BinaryOp>
T accumulate(InputIt first, InputIt last, T init, BinaryOp op) {
    for (; first != last; ++first)
        init = op(init, *first);
    return init;
}
```

## reduce

類似 accumulate，但順序不定（C++17，支援並行）

```cpp
template<class InputIt, class T, class BinaryOp>
T reduce(InputIt first, InputIt last, T init, BinaryOp op);
```

## partial_sum

計算部分和

```cpp
template<class InputIt, class OutputIt>
OutputIt partial_sum(InputIt first, InputIt last, OutputIt d_first) {
    if (first == last) return d_first;

    typename std::iterator_traits<InputIt>::value_type sum = *first;
    *d_first = sum;

    while (++first != last) {
        sum = sum + *first;
        *++d_first = sum;
    }
    return ++d_first;
}
```

## inclusive_scan / exclusive_scan

類似 partial_sum，但支援並行（C++17）

```cpp
template<class InputIt, class OutputIt, class T, class BinaryOp>
OutputIt exclusive_scan(InputIt first, InputIt last,
                        OutputIt d_first, T init, BinaryOp op);

template<class InputIt, class OutputIt, class BinaryOp>
OutputIt inclusive_scan(InputIt first, InputIt last,
                        OutputIt d_first, BinaryOp op);
```

## iota

以遞增值填充範圍（C++11）

```cpp
template<class ForwardIt, class T>
void iota(ForwardIt first, ForwardIt last, T value) {
    while (first != last) {
        *first++ = value;
        ++value;
    }
}
```

## inner_product

計算內積

```cpp
template<class InputIt1, class InputIt2, class T>
T inner_product(InputIt1 first1, InputIt1 last1,
                InputIt2 first2, T init) {
    while (first1 != last1) {
        init = init + (*first1) * (*first2);
        ++first1;
        ++first2;
    }
    return init;
}

template<class InputIt1, class InputIt2, class T,
         class BinaryOp1, class BinaryOp2>
T inner_product(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                T init, BinaryOp1 op1, BinaryOp2 op2) {
    while (first1 != last1) {
        init = op1(init, op2(*first1, *first2));
        ++first1;
        ++first2;
    }
    return init;
}
```

## adjacent_difference

計算相鄰差

```cpp
template<class InputIt, class OutputIt>
OutputIt adjacent_difference(InputIt first, InputIt last, OutputIt d_first) {
    if (first == last) return d_first;

    typename std::iterator_traits<InputIt>::value_type acc = *first;
    *d_first = acc;

    while (++first != last) {
        auto val = *first;
        *++d_first = val - acc;
        acc = std::move(val);
    }
    return ++d_first;
}
```

## gcd / lcm

最大公因數與最小公倍數（C++17）

```cpp
template<class M, class N>
constexpr std::common_type_t<M, N> gcd(M m, N n) {
    return n == 0 ? m : gcd(n, m % n);
}

template<class M, class N>
constexpr std::common_type_t<M, N> lcm(M m, N n) {
    return (m != 0 && n != 0) ? (m / gcd(m, n)) * n : 0;
}
```
