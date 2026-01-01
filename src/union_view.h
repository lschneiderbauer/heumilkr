#ifndef UNION_VIEW_H
#define UNION_VIEW_H

#include <unordered_set>
#include <iterator>
#include <utility>

template<typename T>
class union_view {
public:
  using set_t = std::unordered_set<T>;
  using const_iterator_t = typename set_t::const_iterator;

  union_view(const set_t &a, const set_t &b) noexcept
    : a_(&a), b_(&b) {}

  // nested iterator
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    iterator() noexcept
      : a_(nullptr), b_(nullptr), in_first_(false) {}

    iterator(const set_t *a, const set_t *b,
             const_iterator_t it_a, const_iterator_t end_a,
             const_iterator_t it_b, const_iterator_t end_b) noexcept
      : a_(a), b_(b),
        it_a_(it_a), end_a_(end_a),
        it_b_(it_b), end_b_(end_b),
        in_first_(true)
    {
      if (it_a_ == end_a_) {
        in_first_ = false;
        skip_b_duplicates();
      }
    }

    reference operator*() const noexcept {
      return in_first_ ? *it_a_ : *it_b_;
    }
    pointer operator->() const noexcept {
      return &(operator*());
    }

    iterator& operator++() noexcept {
      if (in_first_) {
        ++it_a_;
        if (it_a_ == end_a_) {
          in_first_ = false;
          skip_b_duplicates();
        }
      } else {
        ++it_b_;
        skip_b_duplicates();
      }
      return *this;
    }

    iterator operator++(int) noexcept {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const iterator &x, const iterator &y) noexcept {
      // both end sentinels
      if (x.a_ == nullptr && y.a_ == nullptr) return true;
      // otherwise compare origin pointers and phase + iterator
      return x.a_ == y.a_ && x.b_ == y.b_ && x.in_first_ == y.in_first_ &&
             (x.in_first_ ? x.it_a_ == y.it_a_ : x.it_b_ == y.it_b_);
    }
    friend bool operator!=(const iterator &x, const iterator &y) noexcept {
      return !(x == y);
    }

  private:
    const set_t *a_;
    const set_t *b_;
    const_iterator_t it_a_;
    const_iterator_t end_a_;
    const_iterator_t it_b_;
    const_iterator_t end_b_;
    bool in_first_;

    void skip_b_duplicates() noexcept {
      if (!b_ || !a_) return;
      while (it_b_ != end_b_ && a_->find(*it_b_) != a_->end()) {
        ++it_b_;
      }
      if (it_b_ == end_b_) {
        // mark as end sentinel
        a_ = nullptr;
        b_ = nullptr;
      }
    }
  };

  iterator begin() const noexcept {
    return iterator(a_, b_, a_->begin(), a_->end(), b_->begin(), b_->end());
  }

  iterator end() const noexcept {
    return iterator(); // default-constructed sentinel
  }

private:
  const set_t *a_;
  const set_t *b_;
};

#endif // HEUMILKR_UNION_VIEW_H