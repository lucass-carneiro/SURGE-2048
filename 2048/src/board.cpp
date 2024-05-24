#include "board.hpp"

#include "player/window.hpp"

auto s2048::board::load(GLFWwindow *, tdb_t &tdb, sdb_t &sdb) noexcept
    -> std::optional<surge::error> {

  using namespace surge;
  using namespace surge::atom;

  tdb.reset();
  sdb.reinit();

  // Board
  texture::create_info ci{};
  ci.filtering = renderer::texture_filtering::anisotropic;
  tdb.add(ci, "resources/board.png");

  return {};
}

auto s2048::board::unload(tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error> {
  tdb.reset();
  sdb.reinit();

  return {};
}

void s2048::board::update(double, GLFWwindow *window, const tdb_t &tdb, sdb_t &sdb) noexcept {
  using namespace surge;
  using namespace surge::atom;

  sdb.reset();

  // Texture handles
  static const auto bckg_handle{tdb.find("resources/board.png").value_or(0)};

  // Background model
  const auto [ww, wh] = window::get_dims(window);
  const auto bckg_model{sprite::place(glm::vec2{0.0f}, glm::vec2{ww, wh}, 0.1f)};
  sdb.add(bckg_handle, bckg_model, 1.0);
}

void s2048::board::draw(sdb_t &sdb) noexcept { sdb.draw(); }