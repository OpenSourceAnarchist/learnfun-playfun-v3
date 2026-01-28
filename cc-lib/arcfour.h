
/* Alleged RC4 algorithm.
   The RC4 name is trademarked by RSA DSI.
   This implementation is based on the algorithm
   published in Applied Cryptography. */

// Note: I ported this from my SML version and
// never tested it. -tom7

#ifndef __CCLIB_ARCFOUR_H
#define __CCLIB_ARCFOUR_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct ArcFour {
  using uint8 = uint8_t;

  explicit ArcFour(const std::vector<uint8> &v);
  explicit ArcFour(const std::string &s);

  // Get the next byte.
  auto Byte() -> uint8;

  // Discard n bytes from the stream. It is
  // strongly recommended that new uses of
  // arcfour discard at least 1024 bytes after
  // initialization, to prevent against the
  // 2001 attach by Fluhrer, Mantin, and Shamir.
  void Discard(int n);

private:
  uint8 ii, jj;
  std::array<uint8, 256> ss{};
};

#endif
