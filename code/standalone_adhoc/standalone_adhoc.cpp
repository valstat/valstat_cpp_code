// https://godbolt.org/z/Woes1jrdx
// (c) by dbj@dbj.org
// valstat idea ad-hoc application to the
// dynamic buffer of trivially copyable types
// by no means finished either in design or in implementation

// being strict we do not need assert when we have valstat
// #include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FX(F, X) printf("\n%4d : %32s : " F, __LINE__, (#X), (X))

#include <type_traits>
#include <utility>

// https://github.com/valstat
// this is global valstat type, it is not type embeded in a user type
// this valstat is for dealing with errno
struct errno_valstat final {
  int errno_;        // 0 == no error
  const char *stat;  // message or 0 if no error
};

// Some ad hoc "not a vector" type

// C++20 (and later) code
#if __cplusplus >= 202002L
template <typename T, size_t MAX_CAP = 0xFFFF>
requires  // requires-clause (ad-hoc constraint)
    std::is_trivially_copyable_v<T>
#else   // C++ < 20
template <typename T, size_t MAX_CAP = 0xFFFF,
          std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
#endif  // C++ < 20
class not_a_vector {
  using value_type = T;
  using type = not_a_vector;
  //
  //   static_assert(std::is_trivially_copyable_v<T>,
  //                 "\n\nnot_a_vector<>T -- realloc can handle only trivially
  //                 " "copiable types!\n\n");
  //
  // the problem is static_assert does operate on instances not types
  // that is left in to show the problem with using static_assert
  // to try and constrain a type

  constexpr static size_t initial_capacity_ = MAX_CAP;
  constexpr static size_t capacity_increment_ = 2;
  size_t size_ = 0;
  size_t capacity_ = initial_capacity_;
  T *arr_ = nullptr;

  // again thanks to valstat we do not exit on assert
  // if size requested is too big
  errno_valstat resize_(std::size_t newSize) {

    if (1 > newSize) return {EINVAL, "Bad argument: size < 1"};
    if ( newSize > this->capacity_)  return {EINVAL, "newSize > this->capacity_"};
    
  // NOTE: hack ahead, we do not realloc as capacity == MAX_CAP
    if (this->is_empty()) {
      this->arr_ = static_cast<T *>(::calloc( this->initial_capacity_, sizeof(T) ));
    if (this->arr_ == nullptr)
      return {ENOMEM, "No enough memory"};
    }
    this->size_ = newSize;
    return {0 /* OK */, nullptr};
  }

 public:
  /// at strategic places we return VALSTAT(tm)
  /// https://github.com/valstat
  /// index operator (bellow) is the best place to understand why
  struct valstat {
    /// CAVEAT EMPTOR: this is pointer to interals
    /// thus "use it or loose it"
    /// if this is null some error has happened
    T *val;
    /// this is error message AND signal there was
    /// no error if it is null
    const char *stat;
  };

  size_t size() const noexcept { return this->size_; }
  size_t capacity() const noexcept { return capacity_; }

  /// each valstat is made of two fields
  /// CRUCIAL: user must be able to test each field for 'emptiness' too
  /// there are two valstats in this class
  /// this one returns the class instanace by value
  /// this class has the bool empty() method
  struct valstat_make_rezult final {
    type val;            /// empty default not_a_vector<T>
    const char *stat{};  /// if not null means error stat-us
  };

  /// making is allowed through a factory method only
  /// argument is an native array
  /// note: making != constructing
  template <size_t N>
  static valstat_make_rezult make(const T (&arr)[N]) {
    type retval_;  // empty vector !

    /// on error we move out the empty not_a_vector and error message
    if (N > MAX_CAP) return {std::move(retval_), "requested Size > MAX_CAP"};

    auto [err_, msg_] = retval_.resize_(N);

    if (err_ != 0)
      /// resize has failed, we pass out the error message
      /// with the empty value
      return {std::move(retval_), msg_};

    /// copy the array param to the retval
    memcpy(retval_.arr_, arr, N);
    retval_.size_ = N;

    /// moving out
    return {
        std::move(retval_),  /// we deliberately barred copying
        nullptr              /// no error status
    };
  }

  // making from pointers to arrays is a problematic proposal
  template <size_t N>
  static type make(const T (*arr)[N]) = delete;

  // making from arrays of pointers is a problematic proposal too
  template <size_t N>
  static type make(const T *(&arr)[N]) = delete;

  /// default not_vector is empty!
  not_a_vector() noexcept = default;

  /// default constructed not_vector is empty!
  bool is_empty(void) const noexcept {
    return (size_ == 0) && (arr_ == nullptr);
  }

  ~not_a_vector() noexcept {
    if (this->arr_ != nullptr) {
      std::free(this->arr_);
      this->arr_ = nullptr;
    }
    this->size_ = 0;
    this->capacity_ = this->initial_capacity_;
  }

  // deliberately no copy is allowed
  not_a_vector(not_a_vector const &) noexcept = delete;
  not_a_vector &operator=(not_a_vector const &) noexcept = delete;

  not_a_vector(not_a_vector &&other_) noexcept {
    this->arr_ = other_.arr_;
    this->size_ = other_.size_;
    other_.arr_ = nullptr;
  }

  not_a_vector &operator=(not_a_vector &&other_) &noexcept {
    if (this != &other_) {
      this->arr_ = other_.arr_;
      this->size_ = other_.size_;
      other_.arr_ = nullptr;
    }
    return *this;
  }

  ///////////////////////////////////////////////////////
  /// the utility of a valstat is giving us incredibly
  /// simple and light mechanism to have a meaningfull
  /// return with no exception and no aborts, and no
  /// special complex return types !

  valstat operator[](size_t idx) {
    
    if (idx >= this->size_) {
      // errno = EINVAL;
      // perror("Bad index!");
      // exit(EXIT_FAILURE);
      // instead of above, we use valstat
      return {0, "Bad index!"};
    }

    return {&(this->arr_[idx]), nullptr};
  }

  const T &operator[](size_t idx) const noexcept {
    if (idx > this->size_) {
      return {nullptr, "Bad index!"};
    }
    return {&(this->arr_[idx]), nullptr};
  }

  const T *const data(void) const noexcept { return this->arr_; }

  // append new value
  // return the errno valstat
  errno_valstat push_back(T &&value) noexcept {
    if ((this->size_) < this->capacity_) {
      size_ += 1;
      arr_[this->size_] = value;
    } else {
      errno_valstat resize_rezult_ = resize_(capacity_ * capacity_increment_);
      // some error has happened, thus we just return with the valstat
      if (resize_rezult_.errno_ != 0) return resize_rezult_;
      size_ += 1;
      arr_[this->size_] = value;
    }
    // OK signal
    return {0, nullptr};
  }

  T * begin() noexcept { return this->arr_; }
  T * end() noexcept { return this->arr_ + this->size_; }

  T const * cbegin() const noexcept { return this->arr_; }
  T const * cend() const noexcept { return this->arr_ + this->size_; }

};  // not_a_vector ///////////////////////////////////////////////////////

// move in, print data, move out
decltype(auto) hammer = [](auto &&nv_) {
  for (const auto &  e_ : nv_) printf("%c", e_);
  printf("\n");
  return std::move(nv_);  // no copy
};
   
using nvc = not_a_vector<char>;

int main(void) {
  {
    // we can just ignore the valstat returns
    // and have the simple and stupid code
     nvc v2;


    auto [vec_, stat_] = nvc::make("PAYLOAD");
    // no copy allowed --> v2 = vec_ ;
    // move is allowed
    v2 = hammer(vec_);
    v2 = hammer(v2);

    v2.push_back('A');
    v2.push_back('B');
    v2.push_back('C');
    v2 = hammer(v2);
  }
  printf("\n");
  {
    // or we can enjoy the complex and clever code
    // valstat made possible
    auto [v0, stat_] = not_a_vector<char>::make("ABC");
    v0 = hammer(v0);
    FX("%s", v0[1].val);
    FX("%c", *v0[1].val);
    FX("%s", v0[6].val);   // bad index no abort
    FX("%s", v0[6].stat);  // bad index err message
    {
      printf("\n");
      auto [vsmall, stat_] = not_a_vector<char, 4>::make("ABC");
      vsmall = hammer(vsmall);
      FX("%lu", vsmall.size());
      FX("%lu", vsmall.capacity());
    }
  }

  return 42;
}

// compile time checks cost nothing
// always do this
static_assert(std::is_default_constructible_v<nvc> );
static_assert(std::is_nothrow_default_constructible_v<nvc> );
// and this too?
// static_assert(std::is_trivially_default_constructible_v<nvc> );
