# Sorting Operations

排序相關操作。

## sort

排序範圍（不穩定，平均 O(n log n)）

```cpp
template<class RandomIt>
void sort(RandomIt first, RandomIt last);

template<class RandomIt, class Compare>
void sort(RandomIt first, RandomIt last, Compare comp);
```

典型實現使用 introsort（quicksort + heapsort + insertion sort）。

## stable_sort

穩定排序，保持相等元素的相對順序

```cpp
template<class RandomIt>
void stable_sort(RandomIt first, RandomIt last);

template<class RandomIt, class Compare>
void stable_sort(RandomIt first, RandomIt last, Compare comp);
```

典型實現使用 merge sort，需要額外空間。

## partial_sort

部分排序，使前 middle - first 個元素為最小的且已排序

```cpp
template<class RandomIt>
void partial_sort(RandomIt first, RandomIt middle, RandomIt last);

template<class RandomIt, class Compare>
void partial_sort(RandomIt first, RandomIt middle, RandomIt last, Compare comp);
```

複雜度：約 (last - first) * log(middle - first)

## partial_sort_copy

部分排序並複製到另一範圍

```cpp
template<class InputIt, class RandomIt>
RandomIt partial_sort_copy(InputIt first, InputIt last,
                           RandomIt d_first, RandomIt d_last);

template<class InputIt, class RandomIt, class Compare>
RandomIt partial_sort_copy(InputIt first, InputIt last,
                           RandomIt d_first, RandomIt d_last, Compare comp);
```

## nth_element

重新排列，使第 n 個元素為排序後應在的位置

```cpp
template<class RandomIt>
void nth_element(RandomIt first, RandomIt nth, RandomIt last);

template<class RandomIt, class Compare>
void nth_element(RandomIt first, RandomIt nth, RandomIt last, Compare comp);
```

nth 之前的元素都 <= *nth，nth 之後的元素都 >= *nth。平均 O(n)。

## is_sorted

檢查是否已排序

```cpp
template<class ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last) {
    if (first != last) {
        ForwardIt next = first;
        while (++next != last) {
            if (*next < *first)
                return false;
            first = next;
        }
    }
    return true;
}
```

## is_sorted_until

回傳第一個破壞排序的元素位置

```cpp
template<class ForwardIt>
ForwardIt is_sorted_until(ForwardIt first, ForwardIt last) {
    if (first != last) {
        ForwardIt next = first;
        while (++next != last) {
            if (*next < *first)
                return next;
            first = next;
        }
    }
    return last;
}
```
