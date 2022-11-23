#ifndef ENTRY_H_
#define ENTRY_H_

template<typename KeyType, typename ValueType>
struct Entry {
  KeyType key_ {};
  ValueType value_ {};

  Entry() = default;
  Entry(const KeyType &key, const ValueType &value) : key_(key), value_(value) {}
};

#endif // ENTRY_H_