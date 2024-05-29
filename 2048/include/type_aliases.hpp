#ifndef SURGE_2048_TYPE_ALIASES_HPP
#define SURGE_2048_TYPE_ALIASES_HPP

#include "player/pv_ubo.hpp"
#include "player/sprite.hpp"
#include "player/text.hpp"
#include "player/texture.hpp"

namespace s2048 {

using pvubo_t = surge::atom::pv_ubo::buffer;

using tdb_t = surge::atom::texture::database;
using sdb_t = surge::atom::sprite::database;

struct txd_t {
  surge::atom::text::text_engine ten{};
  surge::atom::text::glyph_cache gc{};
  surge::atom::text::text_buffer txb{};
};

} // namespace s2048

#endif // SURGE_2048_TYPE_ALIASES_HPP