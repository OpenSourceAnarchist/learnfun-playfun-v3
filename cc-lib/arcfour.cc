
#include "arcfour.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

using uint8 = uint8_t;
static_assert (sizeof(uint8) == 1, "Char must be one byte.");

static void Initialize(const std::array<uint8, 256> &kk,
                       std::array<uint8, 256> &ss) {
  uint8 i = 0, j = 0;
  for (int n = 256; n--;) {
    j += ss[i] + kk[i];
    uint8 t = ss[i];
    ss[i] = ss[j];
    ss[j] = t;
    i++;
  }
}

ArcFour::ArcFour(const vector<uint8> &v) : ii(0), jj(0) {
  std::array<uint8, 256> kk{};
  for (int i = 0; i < 256; i++) {
    ss[i] = i;
    kk[i] = v[i % v.size()];
  }
  Initialize(kk, ss);
}

ArcFour::ArcFour(const string &s) : ii(0), jj(0) {
  std::array<uint8, 256> kk{};
  for (int i = 0; i < 256; i++) {
    ss[i] = i;
    kk[i] = (uint8)s[i % s.size()];
  }
  Initialize(kk, ss);
}

auto ArcFour::Byte() -> uint8 {
  ii++;
  jj += ss[ii];
  uint8 ti = ss[ii];
  uint8 tj = ss[jj];
  ss[ii] = tj;
  ss[jj] = ti;

  return ss[(ti + tj) & 255];
}

void ArcFour::Discard(int n) {
  while (n--) (void)Byte();
}


