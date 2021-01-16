#ifndef FIOCCA_VIEW_CYCLE_HPP_
#define FIOCCA_VIEW_CYCLE_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<input_range View>
class cycle_view : public view_interface<cycle_view<View> > {
public:
  cycle_view() = default;
  constexpr cycle_view(View&& view) : view_(forward<View>(view)) { }

  template<typename _iterator_t> struct _sentinel_impl;
  struct _iterator_base {
    using cyc_view_t = cycle_view;
    using views_iter_t = iterator_t<View>;
    using deref_t = decltype(*declval<iterator_t<View> >());
    using const_deref_t = ext::const_trait_t<
      decltype(*declval<iterator_t<View> >())>;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = typename iterator_traits<
      iterator_t<View> >::iterator_category;
    using value_type = range_value_t<View>;
    using difference_type = range_difference_t<View>;

    // Default constructors.
    _iterator_base() = default;
    template<typename Iterator>
    constexpr _iterator_base(const cycle_view& cyc_view, Iterator&& iter)
        : current_iter_ { forward<Iterator>(iter) },
          cyc_view_(addressof(cyc_view)) {  }

    friend constexpr bool operator==(
      const _iterator_base& lhs, const _iterator_base& rhs)
    requires equality_comparable<views_iter_t> {
      return lhs.current_iter_ == rhs.current_iter_;
    }

  protected:
    friend cycle_view;
    template<typename _iterator_t> friend struct _sentinel_impl;

    constexpr void _increment_impl() {
      if (auto& it = current_iter_; ++it == ranges::end(cyc_view_->view_))
        it = ranges::begin(cyc_view_->view_);
    }
    constexpr void _decrement_impl() {
      if (auto& it = current_iter_; --it == ext::head(cyc_view_->view_))
        it = ext::back(cyc_view_->view_);
    }

    views_iter_t current_iter_ { };
    const cycle_view* cyc_view_ { nullptr };
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
    constexpr deref_t operator*() const { return *(this->current_iter_); }
    constexpr _iterator_impl& operator++() {
      this->_increment_impl(); return *this;
    }
    constexpr _iterator_impl operator++(int)
    requires forward_range<View> {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _iterator_impl& operator--()
    requires bidirectional_range<View> {
      this->_decrement_impl(); return *this;
    }
    constexpr _iterator_impl operator--(int)
    requires bidirectional_range<View> {
      auto tmp = *this; --*this; return tmp;
    }
  };

  // Type alias for normal (forward) and const iterators.
  // The template implementation classes aims at reducing duplication.
  using _iterator = _iterator_impl<typename _iterator_base::deref_t>;
  using _const_iterator = _iterator_impl<typename _iterator_base::const_deref_t>;

  template<typename _iterator_t>
  struct _sentinel_impl {
    _sentinel_impl() = default;
    constexpr explicit _sentinel_impl(const cycle_view& cyc_view)
        : end_(ranges::end(cyc_view.view_)),
          cyc_view_(addressof(cyc_view)) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _iterator_t& iterator, const _sentinel_impl& sentinel) {
      return sentinel._eq(iterator);
    }

    // Trace the previous iterator (the last element in the range)
    // according to the given sentinel. This function would be enabled if and
    // only if each subview is bidirectional.
    constexpr _iterator_t prev() const
    requires bidirectional_range<View> {
      return _iterator_t { *cyc_view_, std::ext::prev(end_) };
    }

  private:
    constexpr bool _eq(const _iterator_t& iterator) const {
      // Check whether a given iterator arrives at the ending.
      return iterator.current_iter_ == end_;
    }

    sentinel_t<View> end_;
    const cycle_view* cyc_view_ { nullptr };
  };

  // Type alias for normal and const sentinels.
  // The template implementation classes aims at reducing duplication.
  using _sentinel = _sentinel_impl<_iterator>;
  using _const_sentinel = _sentinel_impl<_const_iterator>;

  constexpr _iterator begin() {
    return _iterator { *this, ranges::begin(view_) };
  }
  constexpr auto end() {
    if constexpr (common_range<View>)
      return _iterator { *this, ranges::end(view_) };
    else
      return _sentinel { *this };
  }

  constexpr _const_iterator cbegin() const {
    return _const_iterator { *this, ranges::cbegin(view_) };
  }
  constexpr auto cend() const {
    if constexpr (common_range<View>)
      return _iterator { *this, ranges::cend(view_) };
    else
      return _const_sentinel { *this };
  }

private:
  View view_;
};

// Specialize enable_borrowed_range to true for cv-unqualified
// program-defined types which model borrowed_range.
template<input_range... Views>
inline constexpr bool
enable_borrowed_range<cycle_view<Views...> > = true;

// Template deduction guide.
template<input_range Range>
cycle_view(Range) -> cycle_view<views::all_t<Range> >;

namespace views {

inline constexpr __adaptor::_RangeAdaptorClosure cycle
  = []<viewable_range Range>(Range&& range) {
    return cycle_view { forward<Range>(range) };
  };

} // namespace views

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_CYCLE_HPP_