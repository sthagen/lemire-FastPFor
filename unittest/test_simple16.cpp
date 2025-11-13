#include <vector>

#include "simple16.h"
#include "util.h"

namespace FastPForLib {

TEST(Simple16Test, DecodesWithUnknownLength) {
  Simple16<false> codec;
  std::vector<uint32_t> in;
  for (uint32_t i = 0; i < 128; ++i) {
    in.push_back(i);
  }
  verifyUnknownInputLengthDecode(codec, in);
}
} // namespace FastPForLib
