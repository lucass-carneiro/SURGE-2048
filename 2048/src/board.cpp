#include "board.hpp"

#include "player/window.hpp"
#include "ui.hpp"

#include <array>

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

  // New game bttn
  tdb.add(ci, "resources/button_press.png");
  tdb.add(ci, "resources/button_release.png");

  return {};
}

auto s2048::board::unload(tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error> {
  tdb.reset();
  sdb.reinit();

  return {};
}

void s2048::board::update(double, GLFWwindow *window, const tdb_t &tdb, sdb_t &sdb,
                          txd_t &txd) noexcept {
  using std::snprintf;
  using namespace surge;
  using namespace surge::atom;

  sdb.reset();
  txd.txb.reset();

  static u16 score{0};
  static u16 best{0};

  // Texture handles
  static const auto bckg_handle{tdb.find("resources/board.png").value_or(0)};
  static const auto new_game_press{tdb.find("resources/button_press.png").value_or(0)};
  static const auto new_game_release{tdb.find("resources/button_release.png").value_or(0)};

  // Background model
  const auto [ww, wh] = window::get_dims(window);
  const auto bckg_model{sprite::place(glm::vec2{0.0f}, glm::vec2{ww, wh}, 0.1f)};
  sdb.add(bckg_handle, bckg_model, 1.0);

  // UI code
  static s2048::ui::ui_state uist{window, -1, -1};

  // New game bttn
  s2048::ui::draw_data dd{glm::vec2{358.0f, 66.0f}, glm::vec2{138.0f, 40.0f}, 0.2f, 1.0f};
  s2048::ui::button_skin skins{new_game_release, new_game_release, new_game_press};
  if (s2048::ui::button(__COUNTER__, uist, dd, sdb, skins)) {
    if (score > best) {
      best = score;
    }
    score = 0;
  }

  // Score
  std::array<char, 5> score_buffer{};
  std::fill(score_buffer.begin(), score_buffer.end(), 0);
  snprintf(score_buffer.data(), score_buffer.size(), "%u", score);
  txd.txb.push(glm::vec3{360.0f, 48.0f, 0.2f}, glm::vec2{0.15f}, txd.gc, score_buffer.data());

  std::fill(score_buffer.begin(), score_buffer.end(), 0);
  snprintf(score_buffer.data(), score_buffer.size(), "%u", score);
  txd.txb.push(glm::vec3{432.0f, 48.0f, 0.2f}, glm::vec2{0.15f}, txd.gc, score_buffer.data());
}

void s2048::board::draw(sdb_t &sdb, txd_t &txd) noexcept {
  sdb.draw();
  txd.txb.draw(txd.draw_color);
}