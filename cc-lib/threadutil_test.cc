
#include "threadutil.h"

#include <print>
#include <vector>
#include <string>

#include "base/stringprintf.h"
#include "base/logging.h"

using namespace std;

auto Square(int i) -> int {
  return i * i;
}

template<class T>
static void CheckSameVec(const vector<T> &a,
			 const vector<T> &b) {
  CHECK(a.size() == b.size()) << "\n" << a.size() << " != " << b.size();
  for (size_t i = 0; i < a.size(); ++i) {
    CHECK(a[i] == b[i]) << "\n" << a[i] << " != " << b[i];
  }
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {
  {
    vector<int> v = { 3, 2, 1 };
    vector<int> vs = ParallelMap(v, Square, 25);
    CHECK((vector<int>{9, 4, 1}) == vs);
    CHECK(vs == UnParallelMap(v, Square, 25));

    for (int i = 0; i < 10; i++) {
      CheckSameVec(UnParallelMap(v, Square, i),
		   ParallelMap(v, Square, i));
    }
  }

  {
    vector<string> v;
    for (int i = 0; i < 100; i++)
      v.push_back(StringPrintf("hello %d", i));

    auto F = [](const string &s) -> string { return s + " world"; };
    
    for (int i = 0; i < 20; i++) {
      CheckSameVec(UnParallelMap(v, F, i), ParallelMap(v, F, i));
    }
    
  }
  
  std::println("OK.");
  return 0;
}
