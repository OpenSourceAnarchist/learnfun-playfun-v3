
#include <cstdio>
#include <cstdlib>
#include <print>
#include <string>
#include <cstdint>

#include "heap.h"

using namespace std;

using uint64 = uint64_t;

struct TestValue : public Heapable {
  TestValue(uint64 i) : i(i) {}
  uint64 i;
};

static auto CrapHash(int a) -> uint64 {
  uint64 ret = ~a;
  ret *= 31337;
  ret ^= 0xDEADBEEF;
  ret = (ret >> 17) | (ret << (64 - 17));
  ret -= 911911911911;
  ret *= 65537;
  ret ^= 0xCAFEBABE;
  return ret;
}

auto main () -> int {
  static constexpr int kNumValues = 1000;
  
  Heap<uint64, TestValue> heap;
  
  vector<TestValue> values;
  for (int i = 0; i < kNumValues; i++) {
    values.emplace_back(CrapHash(i));
  }

  for (auto & value : values) {
    heap.Insert(value.i, &value);
  }

  TestValue *last = heap.PopMinimumValue();
  while (!heap.Empty()) {
    TestValue *now = heap.PopMinimumValue();
    std::println(stderr, "{} {}", last->i, now->i);
    if (now->i < last->i) {
      std::println("FAIL: {} {}", last->i, now->i);
      return -1;
    }
    last = now;
  }

  for (size_t i = 0; i < values.size(); ++i) {
    if (values[i].location != -1) {
      std::println("FAIL! {} still in heap at {}", i, values[i].location);
      return -1;
    }
  }
  
  for (size_t i = 0; i < values.size() / 2; ++i) {
    heap.Insert(values[i].i, &values[i]);
  }

  heap.Clear();
  if (!heap.Empty()) {
    std::println("FAIL: Heap not empty after clear?");
    return -1;
  }

  for (size_t i = 0; i < values.size() / 2; ++i) {
    if (values[i].location != -1) {
      std::println("FAIL (B)! {} still in heap at {}", i, values[i].location);
      return -1;
    }
  }
  
  std::println("OK");
  return 0;
}
