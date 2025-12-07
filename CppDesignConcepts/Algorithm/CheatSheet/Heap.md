# Heap Operations

堆（max-heap）相關操作。

## make_heap

從範圍建立 max-heap

```cpp
template<class RandomIt>
void make_heap(RandomIt first, RandomIt last);

template<class RandomIt, class Compare>
void make_heap(RandomIt first, RandomIt last, Compare comp);
```

複雜度：O(n)

## push_heap

將 last-1 處的元素推入 heap（假設 [first, last-1) 已是 heap）

```cpp
template<class RandomIt>
void push_heap(RandomIt first, RandomIt last) {
    auto n = last - first;
    auto child = n - 1;

    while (child > 0) {
        auto parent = (child - 1) / 2;
        if (first[parent] < first[child]) {
            std::swap(first[parent], first[child]);
            child = parent;
        } else {
            break;
        }
    }
}
```

複雜度：O(log n)

## pop_heap

將最大元素移到 last-1（原 [first, last) 是 heap，執行後 [first, last-1) 是 heap）

```cpp
template<class RandomIt>
void pop_heap(RandomIt first, RandomIt last) {
    std::swap(*first, *(last - 1));
    --last;
    auto n = last - first;
    auto parent = 0;

    while (true) {
        auto left = 2 * parent + 1;
        if (left >= n) break;
        auto right = left + 1;
        auto largest = parent;

        if (first[left] > first[largest])
            largest = left;
        if (right < n && first[right] > first[largest])
            largest = right;

        if (largest != parent) {
            std::swap(first[parent], first[largest]);
            parent = largest;
        } else {
            break;
        }
    }
}
```

複雜度：O(log n)

## sort_heap

排序 heap 元素（結果為升序）

```cpp
template<class RandomIt>
void sort_heap(RandomIt first, RandomIt last) {
    while (first != last)
        std::pop_heap(first, last--);
}
```

複雜度：O(n log n)

## is_heap

測試範圍是否為 max-heap（C++11）

```cpp
template<class RandomIt>
bool is_heap(RandomIt first, RandomIt last) {
    return std::is_heap_until(first, last) == last;
}
```

## is_heap_until

回傳第一個破壞 heap 性質的元素位置（C++11）

```cpp
template<class RandomIt>
RandomIt is_heap_until(RandomIt first, RandomIt last) {
    auto n = last - first;
    for (decltype(n) child = 1; child < n; ++child) {
        auto parent = (child - 1) / 2;
        if (first[parent] < first[child])
            return first + child;
    }
    return last;
}
```
