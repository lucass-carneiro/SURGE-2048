#ifndef SURGE_2048_TYPE_ALIASES_HPP
#define SURGE_2048_TYPE_ALIASES_HPP

#include "surge_core.hpp"

namespace s2048 {

using pvubo_t = surge::gl_atom::pv_ubo::buffer;

using tdb_t = surge::gl_atom::texture::database;
using sdb_t = surge::gl_atom::sprite::database;

struct txd_t {
  surge::gl_atom::text_engine ten{};
  surge::gl_atom::glyph_cache gc{};
  surge::gl_atom::text_buffer txb{};
};

} // namespace s2048

#endif // SURGE_2048_TYPE_ALIASES_HPP