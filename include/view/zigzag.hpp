#ifndef FIOCCA_VIEW_ZIGZAG_HPP_
#define FIOCCA_VIEW_ZIGZAG_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<typename ZigView, typename ZagView>
requires (view<ZigView> && view<ZagView>) 
class zigzag_view {
public:
  zigzag_view() = default;
  constexpr explicit zigzag_view(ZigView&& zig_view, ZagView&& zag_view)
      : zig_view_(forward<ZigView>(zig_view)),
        zag_view_(forward<ZagView>(zag_view)) {  }
  
  template<typename _iterator_t> struct _sentinel_impl;
  struct _reverse_sentinel;

  struct _iterator_base {
    using zigzag_view_t = zigzag_view;
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
        typename iterator_traits<ZigView>::iterator_category;
    using value_type = pair<range_value_t<ZigView>, range_value_t<ZagView> >;
    using difference_type = common_type_t<
      range_difference_t<ZigView>, range_difference_t<ZigView> >;

    // Default constructors.
    _iterator_base() = default;
    template<typename ZigIter, typename ZagIter>
    constexpr _iterator_base( // Construct from two iterators.
        const zigzag_view& zview, ZigIter&& it1, ZagIter&& it2)
        : current_iter_ { forward<ZigIter>(it1), forward<ZagIter>(it2) },
          zview_(addressof(zview)) {  }

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
      // TODO: implement increment.
    }

    constexpr void _decrement_impl() {
      // TODO: implement decrement.
    }

    views_iter_t current_iter_ { };
    const zigzag_view* zview_ { nullptr };
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
      // TODO: implement dereference.
    }

    constexpr _iterator_impl& operator++() {
      this->_increment_impl();
      return *this;
    }
    constexpr _iterator_impl operator++(int)
    requires (forward_range<ZigView> && forward_range<ZagView>) {
      auto tmp = *this; ++*this; return tmp;
    }

    constexpr _iterator_impl& operator--()
    requires (bidirectional_range<ZigView> && bidirectional_range<ZagView>) {
      this->_decrement_impl();
      return *this;
    }
    constexpr _iterator_impl operator--(int)
    requires (bidirectional_range<ZigView> && bidirectional_range<ZagView>) {
      auto tmp = *this; --*this; return tmp;
    }
  };

  // Type alias for normal (forward) and const iterators.
  // The template implementation classes aims at reducing duplication.
  using _iterator = _iterator_impl<typename _iterator_base::deref_t>;
  using _const_iterator = _iterator_impl<typename _iterator_base::const_deref_t>;

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
