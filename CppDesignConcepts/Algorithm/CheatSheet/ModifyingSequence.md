# Modifying Sequence Operations

修改序列的操作，用於複製、移動、轉換、填充、替換、移除。

## copy

複製範圍

```cpp
template<class InputIt, class OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first)
        *d_first = *first;
    return d_first;
}
```

## copy_if

複製滿足條件的元素

```cpp
template<class InputIt, class OutputIt, class UnaryPred>
OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPred p) {
    for (; first != last; ++first)
        if (p(*first)) {
            *d_first = *first;
            ++d_first;
        }
    return d_first;
}
```

## copy_n

複製 n 個元素

```cpp
template<class InputIt, class Size, class OutputIt>
OutputIt copy_n(InputIt first, Size count, OutputIt result) {
    for (Size i = 0; i < count; ++i, ++first, ++result)
        *result = *first;
    return result;
}
```

## copy_backward

從後向前複製範圍

```cpp
template<class BidirIt1, class BidirIt2>
BidirIt2 copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    while (first != last)
        *(--d_last) = *(--last);
    return d_last;
}
```

## move

移動範圍

```cpp
template<class InputIt, class OutputIt>
OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first)
        *d_first = std::move(*first);
    return d_first;
}
```

## move_backward

從後向前移動範圍

```cpp
template<class BidirIt1, class BidirIt2>
BidirIt2 move_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    while (first != last)
        *(--d_last) = std::move(*(--last));
    return d_last;
}
```

## transform

轉換範圍（一元）

```cpp
template<class InputIt, class OutputIt, class UnaryOp>
OutputIt transform(InputIt first, InputIt last, OutputIt d_first, UnaryOp op) {
    for (; first != last; ++first, ++d_first)
        *d_first = op(*first);
    return d_first;
}
```

轉換範圍（二元）

```cpp
template<class InputIt1, class InputIt2, class OutputIt, class BinaryOp>
OutputIt transform(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                   OutputIt d_first, BinaryOp op) {
    for (; first1 != last1; ++first1, ++first2, ++d_first)
        *d_first = op(*first1, *first2);
    return d_first;
}
```

## fill

以值填充範圍

```cpp
template<class ForwardIt, class T>
void fill(ForwardIt first, ForwardIt last, const T& value) {
    for (; first != last; ++first)
        *first = value;
}
```

## fill_n

以值填充 n 個元素

```cpp
template<class OutputIt, class Size, class T>
OutputIt fill_n(OutputIt first, Size count, const T& value) {
    for (Size i = 0; i < count; ++i, ++first)
        *first = value;
    return first;
}
```

## generate

以函式生成值填充範圍

```cpp
template<class ForwardIt, class Generator>
void generate(ForwardIt first, ForwardIt last, Generator g) {
    for (; first != last; ++first)
        *first = g();
}
```

## generate_n

以函式生成值填充 n 個元素

```cpp
template<class OutputIt, class Size, class Generator>
OutputIt generate_n(OutputIt first, Size count, Generator g) {
    for (Size i = 0; i < count; ++i, ++first)
        *first = g();
    return first;
}
```

## replace

替換範圍內的值

```cpp
template<class ForwardIt, class T>
void replace(ForwardIt first, ForwardIt last,
             const T& old_value, const T& new_value) {
    for (; first != last; ++first)
        if (*first == old_value)
            *first = new_value;
}
```

## replace_if

替換滿足條件的值

```cpp
template<class ForwardIt, class UnaryPred, class T>
void replace_if(ForwardIt first, ForwardIt last, UnaryPred p, const T& new_value) {
    for (; first != last; ++first)
        if (p(*first))
            *first = new_value;
}
```

## swap_ranges

交換兩個範圍的元素

```cpp
template<class ForwardIt1, class ForwardIt2>
ForwardIt2 swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2) {
    for (; first1 != last1; ++first1, ++first2)
        std::iter_swap(first1, first2);
    return first2;
}
```

## iter_swap

交換兩個迭代器指向的元素

```cpp
template<class ForwardIt1, class ForwardIt2>
void iter_swap(ForwardIt1 a, ForwardIt2 b) {
    std::swap(*a, *b);
}
```

## remove

移除範圍內的值（回傳新邏輯結尾）

```cpp
template<class ForwardIt, class T>
ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
    first = std::find(first, last, value);
    if (first != last)
        for (ForwardIt i = first; ++i != last;)
            if (!(*i == value))
                *first++ = std::move(*i);
    return first;
}
```

## remove_if

移除滿足條件的元素

```cpp
template<class ForwardIt, class UnaryPred>
ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPred p) {
    first = std::find_if(first, last, p);
    if (first != last)
        for (ForwardIt i = first; ++i != last;)
            if (!p(*i))
                *first++ = std::move(*i);
    return first;
}
```

## unique

移除連續重複元素（回傳新邏輯結尾）

```cpp
template<class ForwardIt>
ForwardIt unique(ForwardIt first, ForwardIt last) {
    if (first == last) return last;
    ForwardIt result = first;
    while (++first != last)
        if (!(*result == *first) && ++result != first)
            *result = std::move(*first);
    return ++result;
}
```

## reverse

反轉範圍

```cpp
template<class BidirIt>
void reverse(BidirIt first, BidirIt last) {
    while (first != last && first != --last)
        std::iter_swap(first++, last);
}
```

## rotate

旋轉範圍，使 middle 成為新的起點

```cpp
template<class ForwardIt>
ForwardIt rotate(ForwardIt first, ForwardIt middle, ForwardIt last) {
    if (first == middle) return last;
    if (middle == last) return first;

    ForwardIt write = first;
    ForwardIt next_read = first;

    for (ForwardIt read = middle; read != last; ++write, ++read) {
        if (write == next_read)
            next_read = read;
        std::iter_swap(write, read);
    }
    rotate(write, next_read, last);
    return write;
}
```

## shuffle

隨機打亂範圍（C++11）

```cpp
template<class RandomIt, class URBG>
void shuffle(RandomIt first, RandomIt last, URBG&& g) {
    using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
    using distr_t = std::uniform_int_distribution<diff_t>;
    using param_t = typename distr_t::param_type;

    distr_t D;
    for (diff_t i = last - first - 1; i > 0; --i)
        std::swap(first[i], first[D(g, param_t(0, i))]);
}
```
