
#include "rle.h"

#include <cstdint>
#include <print>
#include <vector>

#include "base/logging.h"

using uint8 = uint8_t;

// static
constexpr uint8 RLE::DEFAULT_CUTOFF;

// static
auto RLE::Compress(const vector<uint8> &in) -> vector<uint8> {
  return CompressEx(in, 128);
}

// static
auto RLE::Decompress(const vector<uint8> &in) -> vector<uint8> {
  vector<uint8> out;
  CHECK(DecompressEx(in, 128, &out)) << "Decompression failed.";
  return out;
}

// static
auto RLE::CompressEx(const vector<uint8> &in,
                     uint8 run_cutoff) -> vector<uint8> {
  // No idea how big this needs to be until we compress...
  // (There are lower bounds like in.size() / 128 but it's
  // hard to imagine that being useful.
  vector<uint8> out;

  const size_t max_run_length = static_cast<size_t>(run_cutoff) + 1U;
  CHECK_GT(max_run_length, static_cast<size_t>(0));
  // Note that we always encode an antirun of length 1 as a run of
  // length 1 (control byte 0), so it is possible that there may be no
  // antiruns allowed if this evaluates to 1 because run_cutoff is
  // 255.
  const size_t max_antirun_length =
      static_cast<size_t>(255U - run_cutoff) + 1U;

  for (size_t i = 0; i < in.size(); /* in loop */) {
    // Greedy: Grab the longest prefix of bytes that are the same,
    // up to max_run_length.
    const uint8 target = in[i];
    // We already know that we have a run of at least length 1 and
    // that this is legal.
    size_t run_length = 1U;
    while (run_length < max_run_length &&
           i + run_length < in.size() &&
           in[i + run_length] == target) {
      ++run_length;
    }

    CHECK_NE(run_length, 0) << "Bug: Impossible";
    CHECK_LE(run_length, max_run_length) << "Bug: Impossible";

    if (run_length > 1) {
      // printf("at %d, Run of %dx%d\n", i, target, run_length);
      const uint8 control = static_cast<uint8>(run_length - 1U);
      out.push_back(control);
      out.push_back(target);

      i += run_length;
    } else {
      // The next two bytes are not the same, but we don't want to
      // include the second byte in the anti-run unless the one
      // following IT is also different (otherwise it should be part
      // of a run). Increase the size of the anti_run up to our
      // maximum, or until BEFORE we see a pair of bytes that are the
      // same.
      size_t anti_run_length = 1U;
      while (anti_run_length < max_antirun_length &&
         i + anti_run_length + 1 < in.size() &&
         in[i + anti_run_length] !=
         in[i + anti_run_length + 1]) {
        ++anti_run_length;
      }

      CHECK_GT(anti_run_length, 0);
      CHECK_LE(anti_run_length, max_antirun_length);

      if (anti_run_length == 1U) {
	// printf("at %d, singleton of %d\n", i, target);
  CHECK_EQ(run_length, 1U);
	const uint8 control = 0;
	out.push_back(control);
	out.push_back(target);
  ++i;
      } else {
  const uint8 control = static_cast<uint8>(anti_run_length - 1U +
                                                static_cast<size_t>(run_cutoff));
	CHECK_GT(control, run_cutoff);
	out.reserve(out.size() + anti_run_length + 1);
	out.push_back(control);
  for (size_t a = 0; a < anti_run_length; ++a) {
	  out.push_back(in[i]);
    ++i;
	}
      }
    }
  }
  return out;
}

// static 
auto RLE::DecompressEx(const vector<uint8> &in,
                       uint8 run_cutoff,
                       vector<uint8> *out) -> bool {
  // Worst possible encoding is that the output is half the size of
  // the input, though it could also be like 256x longer. PERF: Could
  // make a sub-linear pass to compute this (then remove the reserves
  // below).
  out->clear();
  
  for (size_t i = 0; i < in.size(); /* in loop */) {
    const uint8 control = in[i];
    ++i;
    if (control <= run_cutoff) {
      // If less than the run cutoff, we treat it as a run.
      const size_t run_length = static_cast<size_t>(control) + 1U;
      
      if (i >= in.size()) {
        std::println("Run of length {}, i now {}, in.size() is {}",
             run_length, i, in.size());
        return false;
      }
      
      const uint8 b = in[i];
      ++i;
      out->reserve(out->size() + run_length);
      for (size_t j = 0; j < run_length; ++j)
    	out->push_back(b);

    } else {
      // run_cutoff may be e.g. 100, but we know from the if that the
      // control is strictly greater than the cutoff, so (control -
      // run_cutoff) is strictly greater than 0. We never need an
      // anti-run of length 0 (pointless) or 1 (same as run of 1,
      // represented as 0) so we code starting at 2.
        const size_t antirun_length =
          static_cast<size_t>(control - run_cutoff) + 1U;
      
      if (i + antirun_length >= in.size()) {
        std::println("Antirun of length {}, i now {}, in.size() is {}",
             antirun_length, i, in.size());
        return false;
      }

      out->reserve(out->size() + antirun_length);
      
      for (size_t j = 0; j < antirun_length; ++j) {
    	out->push_back(in[i]);
    	++i;
      }
    }
  }

  return true;
};
