
#ifndef __PNGSAVE_H
#define __PNGSAVE_H

#include <string>

struct PngSave {

  /* save PNG to the specified filename. rgba should be
     width * height * 4 r,g,b,a quadruplets, in that order
     (regardless of endianness).
     returns true on success. */
  static auto SaveAlpha(const std::string &filename,
                        int width, int height,
                        const unsigned char *rgba) -> bool;

  // XXX no-alpha version
};

#endif
