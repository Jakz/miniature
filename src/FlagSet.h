#include <type_traits>
#include <numeric>

template <typename T>
class flag_set
{
public:
  using enum_t = T;
  using utype_t = std::underlying_type_t<T>;

private:
  utype_t _flags;

public:
  flag_set() : flag_set(utype_t(0)) { }

  flag_set(const std::initializer_list<enum_t>& initList)
  {
    static_assert(sizeof(flag_set<T>) == sizeof(std::underlying_type_t<T>));
    _flags = std::accumulate(initList.begin(), initList.end(), utype_t(0), [](utype_t x, enum_t y) { return x | utype_t(y); });
  }

  explicit flag_set(utype_t value) : _flags(value) { }

  operator utype_t() const { return _flags; }

  bool operator[](enum_t flag) const { return test(flag); }

  flag_set& set() { _flags = ~utype_t(0); return *this; }

  flag_set& set(enum_t flag, bool val = true)
  {
    _flags = (val ? (_flags | utype_t(flag)) : (_flags & ~utype_t(flag)));
    return *this;
  }

  flag_set& reset()
  {
    _flags = utype_t(0);
    return *this;
  }

  flag_set& reset(enum_t flag)
  {
    _flags &= ~utype_t(flag);
    return *this;
  }

  flag_set& flip()
  {
    _flags = ~_flags;
    return *this;
  }

  flag_set& flip(enum_t flag)
  {
    _flags ^= utype_t(flag);
    return *this;
  }

  size_t count() const
  {
    return (_flags * 0x200040008001ULL & 0x111111111111111ULL) % 0xf;
  }

  constexpr size_t size() const
  {
    return sizeof(enum_t) * 8;
  }

  bool test(enum_t flag) const { return (_flags & utype_t(flag)) > 0; }
  bool any() const { return _flags > 0; }
  bool none() const { return _flags == 0; }

  bool operator&&(enum_t flag) const { return test(flag); }
  flag_set& operator|(enum_t flag) { return set(flag); }

  flag_set& operator|=(enum_t flag) { return set(flag); }
};