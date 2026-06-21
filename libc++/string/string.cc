#include <cassert>
#include <cstddef>
#include <string>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Heap.h>
#else
extern "C" void *malloc(size_t size);
extern "C" void free(void *ptr);
#endif

void string::checkNotNull(const char *p) noexcept
{
  if (!p) {
    for (;;) {
      __asm__("hlt");
    }
  }
}

char *string::allocate(size_type cap) noexcept
{
#ifdef __IS_DOORS_KERNEL
  char *p = static_cast<char *>(Heap::alloc(cap + 1));
#else
  char *p = static_cast<char *>(malloc(cap + 1));
#endif
  if (!p) {
    for (;;) {
      __asm__("hlt");
    }
  }
  return p;
}

void string::deallocate(char *p) noexcept
{
#ifdef __IS_DOORS_KERNEL
  Heap::free(p);
#else
  free(p);
#endif
}

void string::setSSO(const char *s, size_type n) noexcept
{
  memcpy(sso_, s, n);
  sso_[n] = '\0';
  data_ = sso_;
  size_ = n;
}

void string::setHeap(char *p, size_type n, size_type cap) noexcept
{
  data_ = p;
  size_ = n;
  capacity_ = cap;
  data_[n] = '\0';
}

string::string() noexcept : data_(sso_), size_(0), capacity_(SSO_CAPACITY)
{
  sso_[0] = '\0';
}

string::string(const char *s) noexcept
{
  checkNotNull(s);
  size_type n = strlen(s);
  if (n <= SSO_CAPACITY) {
    setSSO(s, n);
    capacity_ = SSO_CAPACITY;
  }
  else {
    char *p = allocate(n);
    memcpy(p, s, n + 1);
    setHeap(p, n, n);
  }
}

string::string(const char *s, size_type count) noexcept
{
  checkNotNull(s);
  if (count <= SSO_CAPACITY) {
    setSSO(s, count);
    capacity_ = SSO_CAPACITY;
  }
  else {
    char *p = allocate(count);
    memcpy(p, s, count);
    setHeap(p, count, count);
  }
}

string::string(size_type count, char ch) noexcept
{
  if (count <= SSO_CAPACITY) {
    memset(sso_, ch, count);
    sso_[count] = '\0';
    data_ = sso_;
    size_ = count;
    capacity_ = SSO_CAPACITY;
  }
  else {
    char *p = allocate(count);
    memset(p, ch, count);
    setHeap(p, count, count);
  }
}

string::string(const string &other) noexcept
{
  if (other.isSSO()) {
    setSSO(other.sso_, other.size_);
    capacity_ = SSO_CAPACITY;
  }
  else {
    char *p = allocate(other.capacity_);
    memcpy(p, other.data_, other.size_ + 1);
    setHeap(p, other.size_, other.capacity_);
  }
}

string::string(string &&other) noexcept
  : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
{
  if (other.isSSO()) {
    memcpy(sso_, other.sso_, other.size_ + 1);
    data_ = sso_;
  }
  other.data_ = other.sso_;
  other.size_ = 0;
  other.sso_[0] = '\0';
}

string::~string() noexcept
{
  if (isHeap()) {
    deallocate(data_);
  }
}

string &string::operator=(const string &other) noexcept
{
  if (this == &other) {
    return *this;
  }
  if (other.isSSO()) {
    if (isHeap()) {
      deallocate(data_);
    }
    setSSO(other.sso_, other.size_);
    capacity_ = SSO_CAPACITY;
  }
  else {
    if (isHeap() && capacity_ >= other.size_) {
      memcpy(data_, other.data_, other.size_ + 1);
      size_ = other.size_;
      data_[size_] = '\0';
    }
    else {
      if (isHeap()) {
        deallocate(data_);
      }
      char *p = allocate(other.capacity_);
      memcpy(p, other.data_, other.size_ + 1);
      setHeap(p, other.size_, other.capacity_);
    }
  }
  return *this;
}

string &string::operator=(string &&other) noexcept
{
  if (this == &other) {
    return *this;
  }
  if (isHeap()) {
    deallocate(data_);
  }
  data_ = other.data_;
  size_ = other.size_;
  capacity_ = other.capacity_;
  if (other.isSSO()) {
    memcpy(sso_, other.sso_, other.size_ + 1);
    data_ = sso_;
  }
  other.data_ = other.sso_;
  other.size_ = 0;
  other.sso_[0] = '\0';
  return *this;
}

string &string::operator=(const char *s) noexcept
{
  return assign(s);
}

string &string::assign(const string &other) noexcept
{
  return *this = other;
}

string &string::assign(const char *s) noexcept
{
  checkNotNull(s);
  size_type n = strlen(s);
  if (n <= SSO_CAPACITY) {
    if (isHeap()) {
      deallocate(data_);
    }
    setSSO(s, n);
    capacity_ = SSO_CAPACITY;
  }
  else {
    if (isHeap() && capacity_ >= n) {
      memcpy(data_, s, n + 1);
      size_ = n;
    }
    else {
      if (isHeap()) {
        deallocate(data_);
      }
      char *p = allocate(n);
      memcpy(p, s, n + 1);
      setHeap(p, n, n);
    }
  }
  return *this;
}

char &string::operator[](size_type pos) noexcept
{
  return data_[pos];
}

char &string::at(size_type pos) noexcept
{
  assert(pos < size_);
  return data_[pos];
}

const char &string::at(size_type pos) const noexcept
{
  assert(pos < size_);
  return data_[pos];
}

char *string::heapData() noexcept
{
  return isHeap() ? data_ : nullptr;
}

const char *string::heapData() const noexcept
{
  return isHeap() ? data_ : nullptr;
}

void string::reserve() noexcept
{
  shrink_to_fit();
}

void string::reserve(size_type newCap) noexcept
{
  if (newCap <= capacity()) return;
  grow(newCap);
}

void string::shrink_to_fit() noexcept
{
  if (isSSO()) return;
  if (size_ == capacity_) return;
  if (size_ <= SSO_CAPACITY) {
    char tmp[SSO_CAPACITY + 1];
    memcpy(tmp, data_, size_);
    tmp[size_] = '\0';
    deallocate(data_);
    setSSO(tmp, size_);
    capacity_ = SSO_CAPACITY;
  }
  else {
    char *newData = allocate(size_);
    memcpy(newData, data_, size_);
    newData[size_] = '\0';
    deallocate(data_);
    setHeap(newData, size_, size_);
  }
}

void string::clear() noexcept
{
  if (isHeap()) {
    deallocate(data_);
    data_ = sso_;
    capacity_ = SSO_CAPACITY;
  }
  size_ = 0;
  sso_[0] = '\0';
}

void string::pop_back() noexcept
{
  if (size_ > 0) {
    --size_;
    data_[size_] = '\0';
  }
}

string &string::insert(size_type pos, const string &str) noexcept
{
  if (pos > size_) {
    pos = size_;
  }
  reserveFor(size_ + str.size_);
  if (pos < size_) {
    memmove(data_ + pos + str.size_, data_ + pos, size_ - pos);
  }
  memcpy(data_ + pos, str.data_, str.size_);
  size_ += str.size_;
  data_[size_] = '\0';
  return *this;
}

string &string::insert(size_type pos, const char *s) noexcept
{
  return insert(pos, string(s));
}

string &string::insert(size_type pos, size_type count, char ch) noexcept
{
  if (pos > size_) {
    pos = size_;
  }
  reserveFor(size_ + count);
  if (pos < size_) {
    memmove(data_ + pos + count, data_ + pos, size_ - pos);
  }
  memset(data_ + pos, ch, count);
  size_ += count;
  data_[size_] = '\0';
  return *this;
}

string &string::erase(size_type pos, size_type count) noexcept
{
  if (pos > size_) {
    return *this;
  }
  if (count == npos || pos + count > size_) {
    count = size_ - pos;
  }
  if (pos + count < size_) {
    memmove(data_ + pos, data_ + pos + count, size_ - pos - count);
  }
  size_ -= count;
  data_[size_] = '\0';
  return *this;
}

void string::push_back(char ch) noexcept
{
  reserveFor(size_ + 1);
  data_[size_++] = ch;
  data_[size_] = '\0';
}

string &string::append(const string &str) noexcept
{
  return insert(size_, str);
}

string &string::append(const char *s) noexcept
{
  return insert(size_, string(s));
}

string &string::operator+=(const string &str) noexcept
{
  return append(str);
}

string &string::operator+=(char ch) noexcept
{
  push_back(ch);
  return *this;
}

void string::resize(size_type n, char ch) noexcept
{
  if (n < size_) {
    size_ = n;
    data_[size_] = '\0';
  }
  else if (n > size_) {
    reserveFor(n);
    memset(data_ + size_, ch, n - size_);
    size_ = n;
    data_[size_] = '\0';
  }
}

void string::resize(size_type n) noexcept
{
  resize(n, '\0');
}

void string::swap(string &other) noexcept
{
  string temp(static_cast<string &&>(*this));
  *this = static_cast<string &&>(other);
  other = static_cast<string &&>(temp);
}

string &string::replace(size_type pos, size_type count, const string &str) noexcept
{
  erase(pos, count);
  insert(pos, str);
  return *this;
}

string &string::replace(size_type pos, size_type count, const char *s) noexcept
{
  return replace(pos, count, string(s));
}

string &string::replace(size_type pos, size_type count, const char *s, size_type count2) noexcept
{
  checkNotNull(s);
  if (pos > size_) {
    pos = size_;
  }
  if (count == npos || pos + count > size_) {
    count = size_ - pos;
  }
  reserveFor(size_ - count + count2);
  if (pos + count < size_) {
    memmove(data_ + pos + count2, data_ + pos + count, size_ - pos - count);
  }
  memcpy(data_ + pos, s, count2);
  size_ = size_ - count + count2;
  data_[size_] = '\0';
  return *this;
}

string::size_type string::copy(char *s, size_type count, size_type pos) const noexcept
{
  checkNotNull(s);
  if (pos >= size_) {
    return 0;
  }
  if (count > size_ - pos) {
    count = size_ - pos;
  }
  memcpy(s, data_ + pos, count);
  return count;
}

string string::substr(size_type pos, size_type count) const noexcept
{
  if (pos >= size_) {
    return string();
  }
  if (count == npos || pos + count > size_) {
    count = size_ - pos;
  }
  return string(data_ + pos, count);
}

string::size_type string::find(const string &str, size_type pos) const noexcept
{
  if (pos > size_) {
    return npos;
  }
  if (str.size_ == 0) {
    return pos;
  }
  if (str.size_ > size_ - pos) {
    return npos;
  }
  for (size_type i = pos; i <= size_ - str.size_; ++i) {
    if (memcmp(data_ + i, str.data_, str.size_) == 0) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find(const char *s, size_type pos) const noexcept
{
  return find(string(s), pos);
}

string::size_type string::find(const char *s, size_type pos, size_type count) const noexcept
{
  checkNotNull(s);
  if (pos > size_) {
    return npos;
  }
  if (count == 0) {
    return pos;
  }
  if (count > size_ - pos) {
    return npos;
  }
  for (size_type i = pos; i <= size_ - count; ++i) {
    if (memcmp(data_ + i, s, count) == 0) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find(char ch, size_type pos) const noexcept
{
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    if (data_[i] == ch) {
      return i;
    }
  }
  return npos;
}

string::size_type string::rfind(const string &str, size_type pos) const noexcept
{
  if (str.size_ == 0) {
    return npos;
  }
  if (str.size_ > size_) {
    return npos;
  }
  if (pos > size_ - str.size_) {
    pos = size_ - str.size_;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    if (memcmp(data_ + i, str.data_, str.size_) == 0) {
      return i;
    }
  }
  return npos;
}

string::size_type string::rfind(const char *s, size_type pos) const noexcept
{
  return rfind(string(s), pos);
}

string::size_type string::rfind(const char *s, size_type pos, size_type count) const noexcept
{
  checkNotNull(s);
  if (count == 0) {
    return npos;
  }
  if (count > size_) {
    return npos;
  }
  if (pos > size_ - count) {
    pos = size_ - count;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    if (memcmp(data_ + i, s, count) == 0) {
      return i;
    }
  }
  return npos;
}

string::size_type string::rfind(char ch, size_type pos) const noexcept
{
  if (empty()) {
    return npos;
  }
  if (pos == npos || pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    if (data_[i] == ch) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_first_of(const string &str, size_type pos) const noexcept
{
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    for (size_type j = 0; j < str.size_; ++j) {
      if (data_[i] == str.data_[j]) {
        return i;
      }
    }
  }
  return npos;
}

string::size_type string::find_first_of(const char *s, size_type pos) const noexcept
{
  checkNotNull(s);
  if (pos >= size_) {
    return npos;
  }
  size_type sLen = strlen(s);
  for (size_type i = pos; i < size_; ++i) {
    for (size_type j = 0; j < sLen; ++j) {
      if (data_[i] == s[j]) {
        return i;
      }
    }
  }
  return npos;
}

string::size_type string::find_first_of(const char *s, size_type pos,
                                        size_type count) const noexcept
{
  checkNotNull(s);
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    for (size_type j = 0; j < count; ++j) {
      if (data_[i] == s[j]) {
        return i;
      }
    }
  }
  return npos;
}

string::size_type string::find_first_of(char ch, size_type pos) const noexcept
{
  return find(ch, pos);
}

string::size_type string::find_last_of(const string &str, size_type pos) const noexcept
{
  if (empty()) {
    return npos;
  }
  if (pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    for (size_type j = 0; j < str.size_; ++j) {
      if (data_[i] == str.data_[j]) {
        return i;
      }
    }
  }
  return npos;
}

string::size_type string::find_last_of(const char *s, size_type pos) const noexcept
{
  return find_last_of(string(s), pos);
}

string::size_type string::find_last_of(const char *s, size_type pos, size_type count) const noexcept
{
  checkNotNull(s);
  if (empty()) {
    return npos;
  }
  if (pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    for (size_type j = 0; j < count; ++j) {
      if (data_[i] == s[j]) {
        return i;
      }
    }
  }
  return npos;
}

string::size_type string::find_last_of(char ch, size_type pos) const noexcept
{
  return rfind(ch, pos);
}

string::size_type string::find_first_not_of(const string &str, size_type pos) const noexcept
{
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    bool found = false;
    for (size_type j = 0; j < str.size_; ++j) {
      if (data_[i] == str.data_[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_first_not_of(const char *s, size_type pos) const noexcept
{
  return find_first_not_of(string(s), pos);
}

string::size_type string::find_first_not_of(const char *s, size_type pos,
                                            size_type count) const noexcept
{
  checkNotNull(s);
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    bool found = false;
    for (size_type j = 0; j < count; ++j) {
      if (data_[i] == s[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_first_not_of(char ch, size_type pos) const noexcept
{
  if (pos >= size_) {
    return npos;
  }
  for (size_type i = pos; i < size_; ++i) {
    if (data_[i] != ch) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_last_not_of(const string &str, size_type pos) const noexcept
{
  if (empty()) {
    return npos;
  }
  if (pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    bool found = false;
    for (size_type j = 0; j < str.size_; ++j) {
      if (data_[i] == str.data_[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_last_not_of(const char *s, size_type pos) const noexcept
{
  return find_last_not_of(string(s), pos);
}

string::size_type string::find_last_not_of(const char *s, size_type pos,
                                           size_type count) const noexcept
{
  checkNotNull(s);
  if (empty()) {
    return npos;
  }
  if (pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    bool found = false;
    for (size_type j = 0; j < count; ++j) {
      if (data_[i] == s[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return i;
    }
  }
  return npos;
}

string::size_type string::find_last_not_of(char ch, size_type pos) const noexcept
{
  if (empty()) {
    return npos;
  }
  if (pos >= size_) {
    pos = size_ - 1;
  }
  size_type i = pos + 1;
  while (i > 0) {
    --i;
    if (data_[i] != ch) {
      return i;
    }
  }
  return npos;
}

bool string::contains(const string &str) const noexcept
{
  return find(str) != npos;
}

bool string::contains(char ch) const noexcept
{
  return find(ch) != npos;
}

bool string::starts_with(const string &str) const noexcept
{
  if (str.size_ > size_) {
    return false;
  }
  return memcmp(data_, str.data_, str.size_) == 0;
}

bool string::starts_with(char ch) const noexcept
{
  return !empty() && data_[0] == ch;
}

bool string::ends_with(const string &str) const noexcept
{
  if (str.size_ > size_) {
    return false;
  }
  return memcmp(data_ + size_ - str.size_, str.data_, str.size_) == 0;
}

bool string::ends_with(char ch) const noexcept
{
  return !empty() && data_[size_ - 1] == ch;
}

int string::compare(const string &str) const noexcept
{
  size_type n = size_ < str.size_ ? size_ : str.size_;
  int cmp = memcmp(data_, str.data_, n);
  if (cmp != 0) {
    return cmp;
  }
  if (size_ < str.size_) {
    return -1;
  }
  if (size_ > str.size_) {
    return 1;
  }
  return 0;
}

int string::compare(const char *s) const noexcept
{
  return compare(string(s));
}

strong_ordering string::operator<=>(const string &other) const noexcept
{
  int c = compare(other);
  if (c < 0) {
    return strong_ordering::less;
  }
  if (c > 0) {
    return strong_ordering::greater;
  }
  return strong_ordering::equal;
}

void string::grow(size_type minCap) noexcept
{
  size_type newCap = capacity() * 2;
  if (newCap < minCap) {
    newCap = minCap;
  }
  if (newCap < SSO_CAPACITY + 1) {
    newCap = SSO_CAPACITY + 1;
  }
  char *newData = allocate(newCap);
  if (size_ > 0) memcpy(newData, data_, size_);
  newData[size_] = '\0';
  if (isHeap()) {
    deallocate(data_);
  }
  setHeap(newData, size_, newCap);
}

void string::reserveFor(size_type n) noexcept
{
  if (n <= capacity()) return;
  if (isSSO() && n <= SSO_CAPACITY) return;
  grow(n);
}

bool operator==(const string &a, const string &b) noexcept
{
  return a.size() == b.size() && memcmp(a.data(), b.data(), a.size()) == 0;
}

string operator+(const string &a, const string &b) noexcept
{
  string result(a);
  result += b;
  return result;
}

string operator+(const string &a, const char *b) noexcept
{
  string result(a);
  result += b;
  return result;
}

string operator+(const char *a, const string &b) noexcept
{
  string result(a);
  result += b;
  return result;
}

void swap(string &a, string &b) noexcept
{
  a.swap(b);
}

string::size_type erase(string &s, char c) noexcept
{
  string::size_type count = 0;
  string::size_type j = 0;
  for (string::size_type i = 0; i < s.size(); ++i) {
    if (s[i] != c) {
      if (i != j) {
        s[j] = s[i];
      }
      ++j;
    }
    else {
      ++count;
    }
  }
  if (count > 0) {
    s.resize(j);
  }
  return count;
}
