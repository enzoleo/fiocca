#ifndef FIOCCA_VIEW_ZIGZAG_HPP_
#define FIOCCA_VIEW_ZIGZAG_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<typename ZigView, typename ZagView>
requires ( // Zigzag view has to contain bidirectional ranges.
  view<ZigView> && bidirectional_range<ZigView> &&
  view<ZagView> && bidirectional_range<ZagView>
  )
  // Exactly two bidirectional ranges will be included into zigzag view, as
  // one of iterators increases while the other decreases. Besides of that,
  // naturally any zigzag view is reversible.
class zigzag_view {
public:
  zigzag_view() = default;
  constexpr explicit zigzag_view(ZigView&& zig_view, ZagView&& zag_view)
      : zig_view_(forward<ZigView>(zig_view)),
        zag_view_(forward<ZagView>(zag_view)) {  }
  
  template<typename _iterator_t> struct _sentinel_impl;
  struct _reverse_sentinel;

  struct _iterator_base {
    using views_iter_t = pair<iterator_t<ZigView>, iterator_t<ZagView> >;
    template<typename View>
    using deref_value_t = decltype(*declval<iterator_t<View> >());

    // Type aliases for dereference types.
    using deref_t = pair<deref_value_t<ZigView>, deref_value_t<ZagView> >;
    using const_deref_t = pair<
      ext::const_trait_t<deref_value_t<ZigView> >,
      ext::const_trait_t<deref_value_t<ZagView> > >;

    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category =
      typename iterator_traits<iterator_t<ZigView> >::iterator_category;
    using value_type = pair<range_value_t<ZigView>, range_value_t<ZagView> >;
    using difference_type = common_type_t<
      range_difference_t<ZigView>, range_difference_t<ZigView> >;

    // Default constructors.
    _iterator_base() = default;
    template<typename ZigIter, typename ZagIter>
    constexpr _iterator_base( // Construct from two iterators.
        const zigzag_view& zview, ZigIter&& it1, ZagIter&& it2,
        bool turning = true)
        : current_iter_ { forward<ZigIter>(it1), forward<ZagIter>(it2) },
          zview_(addressof(zview)), turning_(turning) {  }

    friend constexpr bool operator==(
      const _iterator_base& lhs, const _iterator_base& rhs)
    requires (equality_comparable<iterator_t<ZigView> > &&
              equality_comparable<iterator_t<ZagView> >) {
      return lhs.current_iter_ == rhs.current_iter_;
    }

  protected:
    friend zigzag_view;
    template<typename _iterator_t> friend struct _sentinel_impl;

    constexpr void _increment_impl() {
      if (turning_) _move_along_dir_forward<0>();
      else          _move_along_dir_forward<1>();
    }

    constexpr void _decrement_impl() {
      if (turning_) _move_along_dir_reverse<1>();
      else          _move_along_dir_reverse<0>();
    }

    template<bool index>
    constexpr auto _move_along_dir_forward() {
      auto view = pair { zview_->zig_view_, zview_->zag_view_ };
      if (auto tmp = get<index>(current_iter_);
          ++tmp == ranges::end(get<index>(view)))
        // Now the zig iterator reaches its boundary, then tend to move
        // the zag iterator to its next one.
        { ++get<1 - index>(current_iter_); turning_ = !turning_; }
      else if (auto tmp = get<1 - index>(current_iter_);
          tmp == ranges::begin(get<1 - index>(view)))
        // Now the zag iterator reaches its boundary.
        { ++get<index>(current_iter_); turning_ = !turning_; }
      else _proceed<1>(); // Forward iteration.
    }

    template<bool index>
    constexpr auto _move_along_dir_reverse() {
      auto view = pair { zview_->zig_view_, zview_->zag_view_ };
      if (auto tmp = get<1 - index>(current_iter_);
          tmp == ranges::begin(get<1 - index>(view)))
        // Now the zag iterator reaches its boundary.
        { --get<index>(current_iter_); turning_ = !turning_; }
      else if (auto tmp = get<index>(current_iter_);
          ++tmp == ranges::end(get<index>(view)))
        // Now the zig iterator reaches its boundary.
        { --get<1 - index>(current_iter_); turning_ = !turning_; }
      else _proceed<0>(); // Backward iteration.
    }

    template<bool forward = true>
    constexpr void _proceed() {
      // The variable determines the direction of our iterator.
      // They should perform in different ways in forward and backward
      // iterations (self increment and decrement).
      bool dir = forward? turning_ : !turning_;
      if (dir) { ++get<0>(current_iter_); --get<1>(current_iter_); }
      else     { --get<0>(current_iter_); ++get<1>(current_iter_); }
    }

    views_iter_t current_iter_ { };
    const zigzag_view* zview_ { nullptr };
    bool turning_ { true };
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
      return deref_t { // Dereference pairs (perfers reference).
        *get<0>(this->current_iter_), *get<1>(this->current_iter_) };
    }

    constexpr _iterator_impl& operator++() {
      this->_increment_impl();
      return *this;
    }
    constexpr _iterator_impl operator++(int) {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _iterator_impl& operator--() {
      this->_decrement_impl();
      return *this;
    }
    constexpr _iterator_impl operator--(int) {
      auto tmp = *this; --*this; return tmp;
    }
  };

  // Type alias for normal (forward) and const iterators.
  // The template implementation classes aims at reducing duplication.
  using _iterator = _iterator_impl<typename _iterator_base::deref_t>;
  using _const_iterator = _iterator_impl<typename _iterator_base::const_deref_t>;

  struct _reverse_iterator {
    // Type aliases for iterators. They are essential to the basic
    // iterator actions and related functions.
    using iterator_category = _iterator::iterator_category;
    using value_type = _iterator::value_type;
    using difference_type = _iterator::difference_type;

    // Default constructors.
    _reverse_iterator() = default;
    constexpr explicit _reverse_iterator(_iterator iter)
        : current_(iter) { }
    
    constexpr _reverse_iterator& operator++() {
      --current_; return *this;
    }
    constexpr _reverse_iterator operator++(int) {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _reverse_iterator& operator--() {
      ++current_; return *this;
    }
    constexpr _iterator operator--(int) {
      auto tmp = *this; --*this; return tmp;
    }

    constexpr auto operator*() { auto tmp = current_; return *--tmp; }
    constexpr auto base() const noexcept { return current_; }

  protected:
    friend zigzag_view;
    friend _reverse_sentinel;
    
    // The underlying iterator of which base() returns a copy.
    _iterator current_;
  };

  template<typename _iterator_t>
  struct _sentinel_impl {
    _sentinel_impl() = default;
    constexpr explicit _sentinel_impl(const zigzag_view& zview)
        : end_(invoke([](auto view) {
            return pair { ranges::end(view.zig_view_), ranges::end(view.zag_view_) };
          }, zview)),
          zview_(addressof(zview)) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _iterator_t& iterator, const _sentinel_impl& sentinel) {
      return sentinel._eq(iterator);
    }

    // Trace the previous iterator (the last element in the range).
    constexpr _iterator_t prev() const
    requires (sized_range<ZigView> && sized_range<ZagView>) {
      auto ts = ranges::size(zview_->zig_view_) + ranges::size(zview_->zag_view_);
      return _iterator_t { // Call prev function in ext namespace!
        *zview_, ext::prev(get<0>(end_)), ext::prev(get<1>(end_)),
        (ts % 2)? false : true };
    }

  private:
    constexpr bool _eq(const _iterator_t& iterator) const {
      // Check whether a given iterator arrives at the ending.
      return (get<0>(iterator.current_iter_) == get<0>(end_)) ||
             (get<1>(iterator.current_iter_) == get<1>(end_));
    }

    pair<sentinel_t<ZigView>, sentinel_t<ZagView> > end_;
    const zigzag_view* zview_ { nullptr };
  };

  // Type alias for normal and const sentinels.
  // The template implementation classes aims at reducing duplication.
  using _sentinel = _sentinel_impl<_iterator>;
  using _const_sentinel = _sentinel_impl<_const_iterator>;

  struct _reverse_sentinel {
    _reverse_sentinel() = default;
    constexpr explicit _reverse_sentinel(const zigzag_view& zview)
        : rend_(invoke([](auto view) {
            // Construct the reverse iterator with the specified base forward
            // iterator. The base of reverse ending is the beginning iterator.
            return pair { ranges::begin(view.zig_view_),
                          ranges::begin(view.zag_view_) };
          }, zview)),
          zview_(addressof(zview)) { }

    friend constexpr bool // Define _eq for friend accessing.
    operator==(const _reverse_iterator& iterator,
               const _reverse_sentinel& sentinel) {
      return sentinel._eq(iterator);
    }

    constexpr _iterator prev() const {
      return _iterator { // Call prev function in ext namespace!
        *zview_, ext::prev(get<0>(rend_)), ext::prev(get<1>(rend_)) };
    }

  private:
    constexpr bool _eq(const _reverse_iterator& iterator) const {
      // Check whether a given iterator arrives at the ending.
      return (get<0>(iterator.current_.current_iter_) == get<0>(rend_)) &&
             (get<1>(iterator.current_.current_iter_) == get<1>(rend_));
    }

    pair<sentinel_t<ZigView>, sentinel_t<ZagView> >  rend_;
    const zigzag_view* zview_ { nullptr };
  };

  constexpr _iterator begin() {
    return _iterator { *this, ranges::begin(zig_view_), ranges::begin(zag_view_) };
  }
  constexpr _sentinel end() { return _sentinel { *this }; }

  constexpr _const_iterator cbegin() {
    return _iterator { *this, ranges::cbegin(zig_view_), ranges::cbegin(zag_view_) };
  }
  constexpr _const_sentinel cend() { return _const_sentinel { *this }; }

  // The reverse begin is only allowed when the two sub-views are finite, or
  // equivalently they are bounded.
  constexpr _reverse_iterator rbegin() {
    return _reverse_iterator { ++ext::prev(end()) };
  }
  constexpr _reverse_sentinel rend()
  requires (sized_range<ZigView> && sized_range<ZagView>) {
    return _reverse_sentinel { *this };
  }

  constexpr auto size() const
  requires (sized_range<ZigView> && sized_range<ZagView>) {
    return ranges::size(zig_view_) * ranges::size(zag_view_);
  }
  constexpr auto ssize() const
  requires (sized_range<ZigView> && sized_range<ZagView>) {
    return ranges::ssize(zig_view_) * ranges::ssize(zag_view_);
  }
  
private:
  ZigView zig_view_;
  ZagView zag_view_;
};

// Specialize enable_borrowed_range to true for cv-unqualified
// program-defined types which model borrowed_range.
template<input_range... Views>
inline constexpr bool
enable_borrowed_range<zigzag_view<Views...> > = true;

// Template deduction guide.
template<input_range... Ranges>
zigzag_view(Ranges&&...) -> zigzag_view<views::all_t<Ranges>...>;

namespace views {

inline constexpr __adaptor::_RangeAdaptor zigzag
  = []<viewable_range... Ranges>(Ranges&&... ranges) {
    return zigzag_view { forward<Ranges>(ranges)... };
  };

} // namespace views

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_ZIGZAG_HPP_
