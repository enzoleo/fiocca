#ifndef FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_
#define FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<typename ...Views>
requires ext::variadic_views<Views...>
class cartesian_product_view {
public:
  static constexpr auto dim() { return sizeof...(Views); }
  cartesian_product_view() = default;
  constexpr explicit cartesian_product_view(Views&&... views)
      : views_{ forward<Views>(views)... } {  }
  
  template<typename _iterator_t> struct _sentinel_impl;
  struct _reverse_sentinel;

  struct _iterator_base {
    using cp_view_t = cartesian_product_view;
    using views_iter_t = tuple<iterator_t<Views>...>;
    template<typename View>
    using deref_value_t = decltype(*declval<iterator_t<View> >());

    // Type aliases for derefence tuple types.
    using deref_t = tuple<deref_value_t<Views>...>;
    using const_deref_t = tuple<ext::const_trait_t<deref_value_t<Views> >...>;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = typename iterator_traits<
      tuple_element_t<0, views_iter_t> >::iterator_category;
    using value_type = tuple<range_value_t<Views>...>;
    using difference_type = common_type_t<range_difference_t<Views>...>;

    // Default constructors.
    _iterator_base() = default;
    template<typename ...Iterators>
    requires (sizeof...(Iterators) == cp_view_t::dim())
    constexpr _iterator_base(const cartesian_product_view& cp_view, Iterators&&... iters)
        : current_iter_ { forward<Iterators>(iters)... },
          cp_view_(addressof(cp_view)) {  }

    friend constexpr bool operator==(
      const _iterator_base& lhs, const _iterator_base& rhs)
    requires (equality_comparable<iterator_t<Views> > && ...) {
      return lhs.current_iter_ == rhs.current_iter_;
    }

  protected:
    friend cartesian_product_view;
    template<typename _iterator_t> friend struct _sentinel_impl;

    template<size_t index>
    constexpr void _increment_impl() {
      constexpr auto N = index - 1;
      auto& view = get<N>(const_cast<cp_view_t*>(cp_view_)->views_);
      if (auto& it = get<N>(current_iter_);
          ++it == ranges::end(view)) {
        if constexpr (N == 0) return;
        else {
          it = ranges::begin(view);
          _increment_impl<N>();
        }
      } else return;
    }

    template<size_t index>
    constexpr void _decrement_impl() {
      constexpr auto N = index - 1;
      auto& view = get<N>(const_cast<cp_view_t*>(cp_view_)->views_);
      if (auto& it = get<N>(current_iter_);
          --it == ext::head(view)) {
        if constexpr (N == 0) return;
        else {
          it = ext::back(view);
          _decrement_impl<N>();
        }
      } else return;
    }

    views_iter_t current_iter_ { };
    const cartesian_product_view* cp_view_ { nullptr };
  };

  template<typename deref_t>
  struct _iterator_impl : public _iterator_base {
    using iterator_category = _iterator_base::iterator_category;
    using value_type = _iterator_base::value_type;
    using difference_type = _iterator_base::difference_type;

    // Inherit the constructors of iterator base class.
    using _iterator_base::_iterator_base;

    // Note the operator* overloading has to be const to fit the
    // adaptors. The range-based for loop never calls const version.
    constexpr auto operator*() const {
      // Have to consider both reference and non-reference types.
      auto _visit_impl =
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return deref_t { *get<Ns>(this->current_iter_)... };
        }; // Return type deref_t!
      return _visit_impl(make_index_sequence<dim()>());
    }

    constexpr _iterator_impl& operator++() {
      this->template _increment_impl<sizeof...(Views)>();
      return *this;
    }
    constexpr _iterator_impl operator++(int)
    requires (forward_range<Views> && ...) {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _iterator_impl& operator--()
    requires ext::variadic_bidirectional_ranges<Views...> {
      this->template _decrement_impl<sizeof...(Views)>();
      return *this;
    }
    constexpr _iterator_impl operator--(int)
    requires ext::variadic_bidirectional_ranges<Views...> {
      auto tmp = *this; --*this; return tmp;
    }
  };

  // Type alias for normal (forward) and const iterators.
  // The template implementation classes aims at reducing duplication.
  using _iterator = _iterator_impl<typename _iterator_base::deref_t>;
  using _const_iterator = _iterator_impl<typename _iterator_base::const_deref_t>;

  struct _reverse_iterator {
    using cp_view_t = cartesian_product_view;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = _iterator::iterator_category;
    using value_type = _iterator::value_type;
    using difference_type = _iterator::difference_type;

    // Default constructors.
    _reverse_iterator() = default;
    constexpr explicit _reverse_iterator(_iterator iter)
        : current_(iter) { }
    
    constexpr _reverse_iterator& operator++()
    requires ext::variadic_bidirectional_ranges<Views...> {
      --current_; return *this;
    }
    constexpr _reverse_iterator operator++(int)
    requires ext::variadic_bidirectional_ranges<Views...> {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _reverse_iterator& operator--()
    requires ext::variadic_bidirectional_ranges<Views...> {
      ++current_; return *this;
    }
    constexpr _iterator operator--(int)
    requires ext::variadic_bidirectional_ranges<Views...> {
      auto tmp = *this; --*this; return tmp;
    }

    constexpr auto operator*() { auto tmp = current_; return *--tmp; }
    constexpr auto base() const noexcept { return current_; }

  protected:
    friend cartesian_product_view;
    friend _reverse_sentinel;
    
    // The underlying iterator of which base() returns a copy.
    _iterator current_;
  };

  template<typename _iterator_t>
  struct _sentinel_impl {
    _sentinel_impl() = default;
    constexpr explicit _sentinel_impl(const cartesian_product_view& cp_view)
        : end_(cp_view._visit([](auto view){ return ranges::end(view); })),
          cp_view_(addressof(cp_view)) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _iterator_t& iterator, const _sentinel_impl& sentinel) {
      return sentinel._eq(iterator);
    }

    // Trace the previous iterator (the last element in the range)
    // according to the given sentinel. This function would be enabled if and
    // only if each subview is bidirectional.
    constexpr _iterator_t prev() const
    requires ext::variadic_bidirectional_ranges<Views...> {
      auto _visit_impl = // Template lambda expression.
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return _iterator_t { *cp_view_, ranges::prev(get<Ns>(end_))... };
        };
      return _visit_impl(make_index_sequence<sizeof...(Views)>());
    }

  private:
    constexpr bool _eq(const _iterator_t& iterator) const {
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
    const cartesian_product_view* cp_view_ { nullptr };
  };

  // Type alias for normal and const sentinels.
  // The template implementation classes aims at reducing duplication.
  using _sentinel = _sentinel_impl<_iterator>;
  using _const_sentinel = _sentinel_impl<_const_iterator>;

  struct _reverse_sentinel {
    _reverse_sentinel() = default;
    constexpr explicit _reverse_sentinel(
      const cartesian_product_view& cp_view)
        : rend_(cp_view._visit([](auto view){ return ranges::begin(view); })),
          cp_view_(addressof(cp_view)) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _reverse_iterator& iterator,
               const _reverse_sentinel& sentinel) {
      return sentinel._eq(iterator);
    }

    constexpr _iterator prev() const
    requires ext::variadic_bidirectional_ranges<Views...> {
      auto _visit_impl = // Template lambda expression.
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return _iterator { *cp_view_, ranges::prev(get<Ns>(rend_))... };
        };
      return _visit_impl(make_index_sequence<sizeof...(Views)>());
    }

  private:
    constexpr bool _eq(const _reverse_iterator& iterator) const {
      // Check whether a given reverse iterator arrives at the ending.
      auto _visit_impl = // Template lambda expression.
        [&]<size_t... Ns>(index_sequence<Ns...>) {
          return ((get<Ns>(iterator.current_.current_iter_)
            == get<Ns>(rend_)) && ...);
        };
      return _visit_impl(make_index_sequence<sizeof...(Views)>());
    }

    tuple<sentinel_t<Views>...> rend_;
    const cartesian_product_view* cp_view_ { nullptr };
  };

  constexpr _iterator begin() {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return _iterator { *this, ranges::begin(get<Ns>(views_))... };
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }
  constexpr _sentinel end() {
    return _sentinel { *this };
  }

  constexpr _const_iterator cbegin() const {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return _const_iterator { *this, ranges::cbegin(get<Ns>(views_))... };
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }
  constexpr _const_sentinel cend() const {
    return _const_sentinel { *this };
  }

  // Note that the accesses to rbegin and rend iterators do not
  // depend on whether the range is bidirectional or not.
  constexpr _reverse_iterator rbegin() {
    return _reverse_iterator { ext::prev(end()) };
  }
  constexpr _reverse_sentinel rend()
  requires ext::variadic_sized_ranges<Views...> {
    return _reverse_sentinel { *this };
  }

  constexpr auto size() const
  requires ext::variadic_sized_ranges<Views...> {
    auto _visit_impl = // Template lambda expression.
      [&]<size_t... Ns>(index_sequence<Ns...>) {
        return (std::ranges::size(get<Ns>(views_)) * ...);
      };
    return _visit_impl(make_index_sequence<sizeof...(Views)>());
  }
  constexpr auto ssize() const
  requires ext::variadic_sized_ranges<Views...> {
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
