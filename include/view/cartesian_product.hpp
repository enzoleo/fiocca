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
          : views_{ std::move(views)... } { }
  
  struct _sentinel;
  struct _iterator {
    using cp_view_t = cartesian_product_view;
    using views_iter_t = std::tuple<iterator_t<Views>...>;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = typename iterator_traits<
      std::tuple_element_t<0, views_iter_t> >::iterator_category;
    using value_type = std::tuple<range_value_t<Views>...>;
    using difference_type = range_difference_t<
      std::tuple_element_t<0, std::tuple<Views...> > >;

    // Default constructors.
    _iterator() = default;
    template<typename ...Iterators>
    requires (sizeof...(Iterators) == cp_view_t::dim())
    constexpr _iterator(const cartesian_product_view& cp_view, Iterators... iters)
        : current_iter_(std::make_tuple(std::move(iters)...)),
          cp_view_(std::addressof(cp_view)) // Not any overloading!
        {  }

    constexpr auto operator*() const {
      return tuple_visit( // Not any overloading!
          [](auto iter){ return std::ref(*iter); }, current_iter_);
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
    requires (std::equality_comparable<iterator_t<Views> > && ...) {
      return lhs.current_iter_ == rhs.current_iter_;
    }

  private:
    friend cartesian_product_view;
    friend _sentinel;

    template<std::size_t index>
    constexpr void _increment_impl() {
      constexpr auto N = index - 1;
      if (auto& it = std::get<N>(current_iter_);
          ++it == std::ranges::end(std::get<N>(cp_view_->views_))) {
        if constexpr (N == 0) return;
        else {
          it = std::ranges::begin(std::get<N>(cp_view_->views_));
          _increment_impl<N>();
        }
      } else return;
    }

    views_iter_t current_iter_ = views_iter_t();
    const cartesian_product_view* cp_view_ = nullptr;
  };

  struct _sentinel {
    _sentinel() = default;
    constexpr explicit _sentinel(const cartesian_product_view& cp_view)
        : end_(cp_view.sentinel()) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _iterator& iterator, const _sentinel& sentinel) {
      return sentinel._eq(iterator);
    }

  private:
    constexpr bool _eq(const _iterator& iterator) const {
      // Check whether a given iterator arrives at the ending.
      auto _visit_impl = // Template lambda expression.
        [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
          return ((std::get<Ns>(iterator.current_iter_)
            == std::get<Ns>(end_)) || ...);
        };
      // This equality checking is tricky. As we know, any empty sub-range
      // will induce an empty cartesian product range, so any sub-iterator
      // equal to according ending directly induces an ending iterator.
      return _visit_impl(std::make_index_sequence<sizeof...(Views)>());
    }
    std::tuple<sentinel_t<Views>...> end_;
  };

  constexpr auto sentinel() { return _visit(std::ranges::end); }
  constexpr const auto sentinel() const { return _visit(std::ranges::end); }
  constexpr _iterator begin() const {
    auto _visit_impl = // Template lambda expression.
      [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
        return _iterator { *this, std::ranges::begin(std::get<Ns>(views_))... };
      };
    return _visit_impl(std::make_index_sequence<sizeof...(Views)>());
  }
  constexpr _sentinel end() const {
    return _sentinel { *this };
  }

  constexpr auto size() const
  requires variadic_sized_ranges<Views...> {
    auto _visit_impl = // Template lambda expression.
      [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
        return (ssize(std::get<Ns>(views_)) * ...);
      };
    return _visit_impl(std::make_index_sequence<sizeof...(Views)>());
  }

private:
  template<typename Invokable, typename Tuple>
  static constexpr auto tuple_visit(Invokable&& f, Tuple&& tup) {
    auto _visit_impl = // Template lambda expression.
      [&f, &tup]<std::size_t... Ns>(std::index_sequence<Ns...>) {
        return std::tuple { // Capture universal references.
          std::forward<Invokable>(f)(std::get<Ns>(std::forward<Tuple>(tup)))...
        };
      };
    return _visit_impl(std::make_index_sequence<dim()>());
  }

  template<typename Invokable>
  constexpr auto _visit(Invokable&& f) {
    return tuple_visit(std::forward<Invokable>(f), views_);
  }
  template<typename Invokable>
  constexpr const auto _visit(Invokable&& f) const {
    return tuple_visit(std::forward<Invokable>(f), views_);
  }

  std::tuple<Views...> views_;
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
    return cartesian_product_view { std::forward<Ranges>(ranges)... };
  };

} // namespace views

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_CARTESIAN_PRODUCT_HPP_
