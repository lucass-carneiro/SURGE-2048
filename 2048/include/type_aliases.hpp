#ifndef SURGE_2048_TYPE_ALIASES_HPP
#define SURGE_2048_TYPE_ALIASES_HPP

#include "sc_opengl/atoms/pv_ubo.hpp"
#include "sc_opengl/atoms/sprite_database.hpp"
#include "sc_opengl/atoms/text.hpp"
#include "sc_opengl/atoms/texture.hpp"

namespace s2048 {

using pvubo_t = surge::gl_atom::pv_ubo::buffer;

using tdb_t = surge::gl_atom::texture::database;
using sdb_t = surge::gl_atom::sprite_database::database;

struct txd_t {
  surge::gl_atom::text::text_engine ten{};
  surge::gl_atom::text::glyph_cache gc{};
  surge::gl_atom::text::text_buffer txb{};
};

} // namespace s2048

#endif // SURGE_2048_TYPE_ALIASES_HPP