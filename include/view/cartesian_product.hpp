#ifndef FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_
#define FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<typename ...Views>
requires variadic_views<Views...>
class cartesian_product_view {
public:
  static constexpr auto dim() { return sizeof...(Views); }
  cartesian_product_view() = default;
  constexpr explicit cartesian_product_view(Views... views)
          : views_{ move(views)... } {  }
  
  struct _sentinel;
  struct _iterator {
    using cp_view_t = cartesian_product_view;
    using views_iter_t = tuple<iterator_t<Views>...>;
    using dereference_t = tuple<
      decltype(*declval<iterator_t<Views> >())...>;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = typename iterator_traits<
      tuple_element_t<0, views_iter_t> >::iterator_category;
    using value_type = tuple<range_value_t<Views>...>;
    using difference_type = common_type_t<range_difference_t<Views>...>;

    // Default constructors.
    _iterator() = default;
    template<typename ...Iterators>
    requires (sizeof...(Iterators) == cp_view_t::dim())
    constexpr _iterator(const cartesian_product_view& cp_view, Iterators... iters)
        : current_iter_ { move(iters)... },
          cp_view_(addressof(cp_view)) // Not any overloading!
        {  }

    constexpr auto operator*() const {
      // Have to consider both reference and non-reference types.
      auto _visit_impl =
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return dereference_t { *get<Ns>(current_iter_)... };
        }; // Return type dereference_t!
      return _visit_impl(make_index_sequence<dim()>());
    }

    constexpr _iterator& operator++() {
      _increment_impl<sizeof...(Views)>();
      return *this;
    }
    constexpr _iterator operator++(int)
    requires (forward_range<Views> && ...) {
      auto tmp = *this; ++*this;
      return tmp;
    }

    friend constexpr bool operator==(const _iterator& lhs, const _iterator& rhs)
    requires (equality_comparable<iterator_t<Views> > && ...) {
      return lhs.current_iter_ == rhs.current_iter_;
    }

  private:
    friend cartesian_product_view;
    friend _sentinel;

    template<size_t index>
    constexpr void _increment_impl() {
      constexpr auto N = index - 1;
      if (auto& it = get<N>(current_iter_);
          ++it == ranges::end(get<N>(cp_view_->views_))) {
        if constexpr (N == 0) return;
        else {
          it = ranges::begin(get<N>(cp_view_->views_));
          _increment_impl<N>();
        }
      } else return;
    }

    views_iter_t current_iter_ { };
    const cartesian_product_view* cp_view_ { nullptr };
  };

  struct _sentinel {
    _sentinel() = default;
    template<typename Visitor>
    constexpr explicit _sentinel( // Specify sentinel via a visitor.
      const cartesian_product_view& cp_view, Visitor&& vis)
        : end_(cp_view._visit(std::forward<Visitor>(vis))) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _iterator& iterator, const _sentinel& sentinel) {
      return sentinel._eq(iterator);
    }

    constexpr auto prev() const
    requires variadic_bidirectional_ranges<Views...> {
      auto _visit_impl = // Template lambda expression.
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return tuple { std::ranges::prev(get<Ns>(end_))... };
        };
      return _visit_impl(make_index_sequence<sizeof...(Views)>());
    }

  private:
    constexpr bool _eq(const _iterator& iterator) const {
      // Check whether a given iterator arrives at the ending.
      auto _visit_impl = // Template lambda expression.
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return ((get<Ns>(iterator.current_iter_)
            == get<Ns>(end_)) || ...);
        };
      // This equality checking is tricky. As we know, any empty sub-range
      // will induce an empty cartesian product range, so any sub-iterator
      // equal to according ending directly induces an ending iterator.
      return _visit_impl(make_index_sequence<sizeof...(Views)>());
    }
    tuple<sentinel_t<Views>...> end_;
  };

  constexpr _iterator begin() const {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return _iterator { *this, ranges::begin(get<Ns>(views_))... };
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }
  constexpr _sentinel end() const {
    return _sentinel { *this, ranges::end };
  }
  constexpr _sentinel rend() const
  requires variadic_sized_ranges<Views...> {
    return _sentinel { *this,
      [](auto view){ return ranges::prev(ranges::begin(view)); } };
  }

  constexpr auto size() const
  requires variadic_sized_ranges<Views...> {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return (std::ranges::size(get<Ns>(views_)) * ...);
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }
  constexpr auto ssize() const
  requires variadic_sized_ranges<Views...> {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return (std::ranges::ssize(get<Ns>(views_)) * ...);
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }

private:
  template<typename Invokable, typename Tuple>
  static constexpr auto tuple_visit(Invokable&& f, Tuple&& tup) {
    auto _visit_impl = // Template lambda expression.
      [&f, &tup]<size_t... Ns>(index_sequence<Ns...>) {
        return tuple { // Capture universal references.
          forward<Invokable>(f)(get<Ns>(forward<Tuple>(tup)))...
        }; // Always return non-reference type!
      };
    return _visit_impl(make_index_sequence<dim()>());
  }

  template<typename Invokable>
  constexpr auto _visit(Invokable&& f) {
    return tuple_visit(forward<Invokable>(f), views_);
  }
  template<typename Invokable>
  constexpr const auto _visit(Invokable&& f) const {
    return tuple_visit(forward<Invokable>(f), views_);
  }

  tuple<Views...> views_;
};

// Specialize enable_borrowed_range to true for cv-unqualified
// program-defined types which model borrowed_range.
template<input_range... Views>
inline constexpr bool
enable_borrowed_range<cartesian_product_view<Views...> > = true;

// Template deduction guide.
template<input_range... Ranges>
cartesian_product_view(Ranges&&...)
    -> cartesian_product_view<views::all_t<Ranges>...>;

namespace views {

inline constexpr __adaptor::_RangeAdaptor cartesian_product
  = []<viewable_range... Ranges>(Ranges&&... ranges) {
    return cartesian_product_view { forward<Ranges>(ranges)... };
  };

} // namespace views

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_
