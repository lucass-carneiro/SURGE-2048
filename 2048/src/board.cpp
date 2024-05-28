#include "board.hpp"

#include "player/window.hpp"
#include "ui.hpp"

#include <array>
#include <random>

namespace globals {

static s2048::board::pieces pcs{}; // NOLINT

} // namespace globals

auto s2048::board::load(GLFWwindow *, tdb_t &tdb, sdb_t &sdb) noexcept
    -> std::optional<surge::error> {

  using namespace surge;
  using namespace surge::atom;

  tdb.reset();
  sdb.reinit();

  // All textures
  texture::create_info ci{};
  ci.filtering = renderer::texture_filtering::anisotropic;
  tdb.add(ci, "resources/board.png", "resources/button_press.png", "resources/button_release.png",
          "resources/pieces_2.png", "resources/pieces_4.png", "resources/pieces_8.png",
          "resources/pieces_16.png", "resources/pieces_32.png", "resources/pieces_64.png",
          "resources/pieces_128.png", "resources/pieces_256.png", "resources/pieces_512.png",
          "resources/pieces_1024.png", "resources/pieces_2048.png");

  // First game pieces
  add_random_piece(tdb);
  add_random_piece(tdb);

  return {};
}

auto s2048::board::unload(tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error> {
  tdb.reset();
  sdb.reinit();

  return {};
}

void s2048::board::update(double dt, GLFWwindow *window, const tdb_t &tdb, sdb_t &sdb,
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
  static const auto new_game_press_handle{tdb.find("resources/button_press.png").value_or(0)};
  static const auto new_game_release_handle{tdb.find("resources/button_release.png").value_or(0)};

  // Background model
  const auto [ww, wh] = window::get_dims(window);
  const auto bckg_model{sprite::place(glm::vec2{0.0f}, glm::vec2{ww, wh}, 0.1f)};
  sdb.add(bckg_handle, bckg_model, 1.0);

  // Gameplay code
  update_pieces(dt, sdb);

  // UI code
  static ui::ui_state uist{window, -1, -1};

  // New game bttn
  ui::draw_data dd{glm::vec2{358.0f, 66.0f}, glm::vec2{138.0f, 40.0f}, 0.2f, 1.0f};
  ui::button_skin skins{new_game_release_handle, new_game_release_handle, new_game_press_handle};

  if (ui::button(__COUNTER__, uist, dd, sdb, skins)) {
    if (score > best) {
      best = score;
    }
    score = 0;
    reset_board();
    add_random_piece(tdb);
    add_random_piece(tdb);
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

void s2048::board::update_pieces(double, sdb_t &sdb) noexcept {
  using namespace surge;
  using namespace surge::atom;

  // Fill sprite databases with the pieces
  for (usize i = 0; i < globals::pcs.current_slots.size(); i++) {
    const auto piece_model{
        sprite::place(globals::pcs.current_positions[i], glm::vec2{105.0f}, 0.2f)};
    sdb.add(globals::pcs.curr_val_handle[i], piece_model, 1.0f);
  }
}

auto s2048::board::add_piece(const tdb_t &tdb, surge::u8 slot, surge::u8 value) noexcept -> bool {
  if (std::find(globals::pcs.current_slots.begin(), globals::pcs.current_slots.end(), slot)
      != globals::pcs.current_slots.end()) {
    return false;
  } else {
    globals::pcs.current_slots.push_back(slot);
    globals::pcs.target_slots.push_back(slot);
    globals::pcs.current_positions.push_back(slot_to_coords(slot));
    globals::pcs.curr_val_handle.push_back(val_to_texture_handle(tdb, value));
    return true;
  }
}

void s2048::board::add_random_piece(const tdb_t &tdb) noexcept {
  static std::minstd_rand eng{std::random_device{}()};
  static std::uniform_int_distribution<surge::u8> slot_dist{0, 15};
  static std::uniform_int_distribution<surge::u8> exp_dist{1, 2};
  const auto value{static_cast<surge::u8>(1 << exp_dist(eng))};

  while (!add_piece(tdb, slot_dist(eng), value)) {
    continue;
  }
}

void s2048::board::reset_board() noexcept {
  globals::pcs.current_slots.clear();
  globals::pcs.target_slots.clear();
  globals::pcs.current_positions.clear();
  globals::pcs.curr_val_handle.clear();
}
