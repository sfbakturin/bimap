# [`bimap`](https://en.wikipedia.org/wiki/Bidirectional_map)

[This](lib/bimap.h) is a educational version of the bidirectional map. `bimap` is a data structure that stores a set of pairs and efficiently performs key-by-value lookup. Unlike [`std::map`](https://en.cppreference.com/w/cpp/container/map), `bimap` can be looked up on both the left (*left*) and right (*right*) elements of pairs.

`bimap` is parameterized by 2 types (*left* and *right*) and 2 comparators that define an order on these types.

Example of usage:

```cpp
bimap<int, char> bm;
bm.insert(42, 'h');
char found_right = bm.at_left(42); // `found_right` == 'h'
int found_left = bm.at_right('h'); // `found_left` == 42
```
