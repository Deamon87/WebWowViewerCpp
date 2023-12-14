#ifndef ARENA_ALLOC_HH
#define ARENA_ALLOC_HH
#include <cstddef>
#include <type_traits>

namespace arena
{
namespace detail
{
struct Lock
{
  Lock ();
  ~Lock ();
};
char * allocate (std::size_t n, std::size_t alignment, const char *hint);
void deallocate (char *p, std::size_t n);
char * reallocate (char *p, std::size_t from_n, std::size_t to_n,
                   std::size_t alignment, const char *hint);
std::size_t default_region_size ();
}

/**
 * A region-based allocator wrapping ‘std::allocator’.
 *
 * The allocator is stateless, that is, all instances of the given allocator
 * are interchangeable, compare equal and can deallocate memory allocated by
 * any other instance of the same allocator type.
 *
 * The allocator satisfies allocator completeness requirements.
 */
template <class T>
struct Allocator
{
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  Allocator () { }
  template <class U = T> Allocator (const Allocator<U> &) { }

  /**
   * @brief allocates uninitialized storage
   *
   * The pointer ‘hint’ may be used to provide locality of reference: the
   * allocator, if possible, will place the new allocation in the same
   * region as ‘hint’.
   *
   * If ‘n’ is zero, a null pointer is returned.
   *
   * @param n - the number of objects to allocate storage for
   * @param hint - pointer to a nearby memory location
   * @return Pointer to the first element of an array of ‘n’ objects of type ‘T’
   *         whose elements have not been constructed yet
   */
  [[nodiscard]] T *
  allocate (std::size_t n, const T *hint = nullptr)
  {
    if (n == 0)
      return nullptr;
    const detail::Lock lock {};
    return (reinterpret_cast<T *>
            (detail::allocate (n * sizeof (T), alignof (T),
                               reinterpret_cast<const char *> (hint))));
  }

  /**
   * @brief deallocates storage
   *
   * If ‘p’ is a null pointer, no action occurrs.
   *
   * @param p - pointer obtained from the allocator
   * @param n - number of objects allocated
   */
  void
  deallocate (T *p, std::size_t n)
  {
    if (p == nullptr)
      return;
    const detail::Lock lock {};
    detail::deallocate (reinterpret_cast<char *> (p), n * sizeof (T));
  }

  /**
   * @brief expands or shrinks previously allocated storage
   *
   * Reallocats the given allocation.
   *
   * The reallocation is done by either:
   *   a) expanding or contracting the existing allocation pointed to by ‘p’,
   *      if possible. The contents of the array remain unchanged up to the
   *      lesser of the new and old sizes. If the allocation is expanded, the
   *      contents of the new part of the array are undefined.
   *   b) doing nothing, if the new size is less than or equal to the old size.
   *   c) allocating a new array that can hold ‘to_n’ objects, copying ‘from_n’
   *      objects, and deallocating the old allocation.
   *
   * If ‘p’ is a null pointer, the behavior is the same as calling
   * ‘allocate (to_n, hint)’.
   *
   * If ‘to_n’ is zero, a null pointer is returned.
   *
   * The pointer ‘hint’ may be used to provide locality reference, if a new
   * allocation occurrs: the allocator, if possible, will place the new
   * allocation in the same region as ‘hint’.
   *
   * @param p - pointer obtained from the allocator
   * @param from_n - number of object allocated
   * @param to_n - number of objects to allocate storage for
   * @param hint - pointer to a nearby memory location
   * @return Pointer to the first element of an array of ‘to_nn’ objects of
   *         type ‘T’. The lesser of ‘from_n’ and ‘to_n’ objects are
   *         initialized, the rest are not yet constructed. The returned
   *         pointer must be deallocated with @ref deallocate(), the original
   *         pointer is invalidated and should no longer be used (even if
   *         reallocation was in-place).
   */
  [[nodiscard]] T *
  reallocate (T *p, std::size_t from_n, std::size_t to_n, const T *hint = nullptr)
  {
    const detail::Lock lock {};
    return (reinterpret_cast<T *>
            (detail::reallocate (reinterpret_cast<char *> (p),
                                 from_n * sizeof (T), to_n * sizeof (T),
                                 alignof (T),
                                 reinterpret_cast<const char *> (hint))));
  }

};

template <class T>
inline bool
operator== (const Allocator<T> &, const Allocator<T> &)
{ return true; }

template <class T>
inline bool
operator!= (const Allocator<T> &, const Allocator<T> &)
{ return false; }

}

#endif // !ARENA_ALLOC_HH

namespace arena
{

#if ((defined (_GLIBCXX_STRING) \
      || defined (_LIBCPP_STRING) \
      || defined (_STRING_)) \
     && !defined (ARENA_HAS_STRING_DEF))
#define ARENA_HAS_STRING_DEF
template <class CharT, class TraitsT = std::char_traits<CharT>>
using basic_string = std::basic_string<CharT, TraitsT, Allocator<CharT>>;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
#if __cplusplus >= 202002L
using u8string = basic_string<char8_t>;
#endif
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;
#endif

#if ((defined (_GLIBCXX_SSTREAM) \
      || defined (_LIBCPP_SSTREAM) \
      || defined (_SSTREAM_)) \
     && !defined (ARENA_HAS_SSTREAM_DEF))
#define ARENA_HAS_SSTREAM_DEF
template <class CharT, class TraitsT = std::char_traits<CharT>>
using basic_stringstream = std::basic_stringstream<CharT, TraitsT, Allocator<CharT>>;
using stringstream = basic_stringstream<char>;
using wstringstream = basic_stringstream<wchar_t>;
// non-standard:
#if __cplusplus >= 202002L
using u8stringstream = basic_stringstream<char8_t>;
#endif
using u16stringstream = basic_stringstream<char16_t>;
using u32stringstream = basic_stringstream<char32_t>;
#endif

#if ((defined (_GLIBCXX_DEQUE) \
      || defined (_LIBCPP_DEQUE) \
      || defined (_DEQUE_)) \
     && !defined (ARENA_HAS_DEQUE_DEF))
#define ARENA_HAS_DEQUE_DEF
template <class T>
using deque = std::deque<T, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_VECTOR) \
      || defined (_LIBCPP_VECTOR) \
      || defined (_VECTOR_)) \
     && !defined (ARENA_HAS_VECTOR_DEF))
#define ARENA_HAS_VECTOR_DEF
template <class T>
using vector = std::vector<T, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_FORWARD_LIST) \
      || defined (_LIBCPP_FORWARD_LIST) \
      || defined (_FORWARD_LIST_)) \
     && !defined (ARENA_HAS_FORWARD_LIST_DEF))
#define ARENA_HAS_FORWARD_LIST_DEF
template <class T>
using forward_list = std::forward_list<T, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_LIST) \
      || defined (_LIBCPP_LIST) \
      || defined (_LIST_)) \
     && !defined (ARENA_HAS_LIST_DEF))
#define ARENA_HAS_LIST_DEF
template <class T>
using list = std::list<T, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_SET) \
      || defined (_LIBCPP_SET) \
      || defined (_SET_)) \
     && !defined (ARENA_HAS_SET_DEF))
#define ARENA_HAS_SET_DEF
template <class T, class Compare = std::less<T>>
using set = std::set<T, Compare, Allocator<T>>;

template <class T, class Compare=std::less<T>>
using multiset = std::multiset<T, Compare, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_MAP) \
      || defined (_LIBCPP_MAP) \
      || defined (_MAP_)) \
     && !defined (ARENA_HAS_MAP_DEF))
#define ARENA_HAS_MAP_DEF
template <class Key, class Value, class Compare = std::less<Key>>
using map = std::map<Key, Value, Compare, Allocator<std::pair<const Key, Value>>>;

template <class Key, class Value, class Compare = std::less<Key>>
using multimap = std::multimap<Key, Value, Compare, Allocator<std::pair<const Key, Value>>>;
#endif

#if ((defined (_GLIBCXX_UNORDERED_SET)\
      || defined (_LIBCPP_UNORDERED_SET) \
      || defined (_UNORDERED_SET_)) \
     && !defined (ARENA_HAS_UNORDERED_SET_DEF))
#define ARENA_HAS_UNORDERED_SET_DEF
template <class T, class Hash = std::hash<T>, class TEqual = std::equal_to<T>>
using unordered_set = std::unordered_set<T, Hash, TEqual, Allocator<T>>;

template <class T, class Hash = std::hash<T>, class TEqual = std::equal_to<T>>
using unordered_multiset = std::unordered_multiset<T, Hash, TEqual, Allocator<T>>;
#endif

#if ((defined (_GLIBCXX_UNORDERED_MAP) \
      || defined (_LIBCPP_UNORDERED_MAP) \
      || defined (_UNORDERED_MAP_)) \
     && !defined (ARENA_HAS_UNORDERED_MAP_DEF))
#define ARENA_HAS_UNORDERED_MAP_DEF
template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, Value, Hash, KeyEqual, Allocator<std::pair<const Key, Value>>>;

template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
using unordered_multimap = std::unordered_multimap<Key, Value, Hash, KeyEqual, Allocator<std::pair<const Key, Value>>>;
#endif

}

