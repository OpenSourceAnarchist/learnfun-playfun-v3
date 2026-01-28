
#ifndef __TASBOT_H
#define __TASBOT_H

#include "../cc-lib/util.h"
#include "../cc-lib/heap.h"

#include "../cc-lib/base/stringprintf.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#define TASBOT_SAMPLE_RATE 44100

#define DEBUGGING 0

template <class K, class V,
          class Hash = std::hash<K>,
          class Eq = std::equal_to<K>,
          class Alloc = std::allocator<std::pair<const K, V>>>
using hash_map = std::unordered_map<K, V, Hash, Eq, Alloc>;

template <class K,
          class Hash = std::hash<K>,
          class Eq = std::equal_to<K>,
          class Alloc = std::allocator<K>>
using hash_set = std::unordered_set<K, Hash, Eq, Alloc>;

// TODO: Use good logging package.
#define CHECK(condition) \
  while (!(condition)) {                                    \
    fprintf(stderr, "%s:%d. Check failed: %s\n",            \
            __FILE__, __LINE__, #condition                  \
            );                                              \
    abort();                                                \
  }

#define DCHECK(condition) \
  while (DEBUGGING && !(condition)) {                       \
    fprintf(stderr, "%s:%d. DCheck failed: %s\n",           \
            __FILE__, __LINE__, #condition                  \
            );                                              \
    abort();                                                \
  }

#define NOT_COPYABLE(classname) \
  private: \
  classname(const classname &); \
  classname &operator =(const classname &)

using namespace std;

#endif
