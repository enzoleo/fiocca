# Extension to Views

This directory contains some self-implemented views that are not included in `C++20`. They are actually very useful in a lot of scenarios and perhaps most of them will be included in future standard library. The implementation here is very preliminary and not precise, so there might exist very siginificant and stupid bugs or poor designs.

## Remarks

Some improvements are required.

- The `prev` methods of all `reverse_iterator` classes.
- The `cartesian_product_view` seems to have wrong implementation of `rbegin` method.
- The `crbegin` and `crend` methods for const `reverse_iterator` access.
