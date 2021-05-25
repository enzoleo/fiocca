#ifndef FIOCCA_VIEW_CYCLE_HPP_
#define FIOCCA_VIEW_CYCLE_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<view _View>
class cycle_view : public view_interface<cycle_view<_View> > {
public:
  cycle_view() = default;
  constexpr cycle_view(_View&& view) : view_(forward<_View>(view)) { }

  template<typename _iterator_t> struct _sentinel_impl;
  struct _iterator_base {
  private:
    static auto _S_iter_concept() {
      if constexpr (random_access_range<_View>)
        return random_access_iterator_tag{};
      else if constexpr (bidirectional_range<_View>)
        return bidirectional_iterator_tag{};
      else if constexpr (forward_range<_View>)
        return forward_iterator_tag{};
      else
        return input_iterator_tag{};
    }
  public:
    using cyc_view_t = cycle_view;
    using views_iter_t = iterator_t<_View>;
    using deref_t = decltype(*declval<iterator_t<_View> >());
    using const_deref_t = ext::const_trait_t<
      decltype(*declval<iterator_t<_View> >())>;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_concept = decltype(_S_iter_concept());
    using iterator_category = typename iterator_traits<
      iterator_t<_View> >::iterator_category;
    using value_type = range_value_t<_View>;
    using difference_type = range_difference_t<_View>;

    // Default constructors.
    _iterator_base() = default;
    template<typename Iterator>
    constexpr _iterator_base(const cycle_view& cyc_view, Iterator&& iter)
        : current_iter_ { forward<Iterator>(iter) },
          cyc_view_(addressof(cyc_view)), ncyc_(0) {  }

    friend constexpr bool operator==(
      const _iterator_base& lhs, const _iterator_base& rhs)
    requires equality_comparable<views_iter_t> {
      return (lhs.current_iter_ == rhs.current_iter_) &&
             (lhs.ncyc_ == rhs.ncyc_);
    }

  protected:
    friend cycle_view;
    template<typename _iterator_t> friend struct _sentinel_impl;

    constexpr void _increment_impl() {
      ++ncyc_; // Increase the cycle number.
      if (auto& it = current_iter_; ++it == ranges::end(cyc_view_->view_))
        it = ranges::begin(cyc_view_->view_);
    }
    constexpr void _decrement_impl() {
      --ncyc_; // Decrease the cycle number.
      if (auto& it = current_iter_; --it == ext::head(cyc_view_->view_))
        it = ext::back(cyc_view_->view_);
    }

    views_iter_t current_iter_ { };
    const cycle_view* cyc_view_ { nullptr };

    // Iterators can locate at different cycles, so this member is
    // quite helpful to determine the equality of two iterators.
    std::intmax_t ncyc_ { 0 };
  };

  template<typename deref_t>
  struct _iterator_impl : public _iterator_base {
    using iterator_concept = _iterator_base::iterator_concept;
    using iterator_category = _iterator_base::iterator_category;
    using value_type = _iterator_base::value_type;
    using difference_type = _iterator_base::difference_type;

    // Inherit the constructors of iterator base class.
    using _iterator_base::_iterator_base;

    // Note the operator* overloading has to be const to fit the
    // adaptors. The range-based for loop never calls const version.
    constexpr deref_t operator*() const { return *(this->current_iter_); }
    constexpr auto operator->() const { return this->current_iter_; }
    constexpr _iterator_impl& operator++() {
      this->_increment_impl(); return *this;
    }
    constexpr _iterator_impl operator++(int)
    requires forward_range<_View> {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _iterator_impl& operator--()
    requires bidirectional_range<_View> {
      this->_decrement_impl(); return *this;
    }
    constexpr _iterator_impl operator--(int)
    requires bidirectional_range<_View> {
      auto tmp = *this; --*this; return tmp;
    }
  };

  // Type alias for normal (forward) and const iterators.
  // The template implementation classes aims at reducing duplication.
  using _iterator = _iterator_impl<typename _iterator_base::deref_t>;
  using _const_iterator = _iterator_impl<typename _iterator_base::const_deref_t>;

  template<typename _iterator_t>
  struct _sentinel_impl : unreachable_sentinel_t {
    _sentinel_impl() = default;
    constexpr explicit _sentinel_impl(const cycle_view& cyc_view)
        : unreachable_sentinel_t(),
          end_(ranges::end(cyc_view.view_)),
          cyc_view_(addressof(cyc_view)) { }

    // Trace the previous iterator (the last element in the range)
    constexpr _iterator_t prev() const
    requires bidirectional_range<_View> {
      return _iterator_t { *cyc_view_, std::ext::prev(end_) };
    }
  private:
    sentinel_t<_View> end_;
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
    if constexpr (common_range<_View>)
      return _iterator { *this, ranges::end(view_) };
    else
      return _sentinel { *this };
  }

  constexpr _const_iterator cbegin() const {
    return _const_iterator { *this, ranges::cbegin(view_) };
  }
  constexpr auto cend() const {
    if constexpr (common_range<_View>)
      return _iterator { *this, ranges::cend(view_) };
    else
      return _const_sentinel { *this };
  }

private:
  _View view_;
};

// Specialize enable_borrowed_range to true for cv-unqualified
// program-defined types which model borrowed_range.
template<input_range _View>
inline constexpr bool
enable_borrowed_range<cycle_view<_View> > = true;

// Template deduction guide.
template<input_range _Range>
cycle_view(_Range&&) -> cycle_view<views::all_t<_Range> >;

namespace views {

struct _Cycle : __adaptor::_RangeAdaptorClosure {
  template<viewable_range _Range>
  constexpr auto operator()(_Range&& __r) const {
    return cycle_view { forward<_Range>(__r) };
  }
};

inline constexpr _Cycle cycle;

} // namespace views

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_CYCLE_HPP_
