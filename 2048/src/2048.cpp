#include "2048.hpp"

#include "pieces.hpp"
#include "type_aliases.hpp"
#include "ui.hpp"

#ifdef SURGE_BUILD_TYPE_Debug
#  include "debug_window.hpp"
#endif

#include "sc_glm_includes.hpp"

namespace globals {

static s2048::tdb_t tdb{};      // NOLINT
static s2048::pvubo_t pv_ubo{}; // NOLINT
static s2048::sdb_t sdb{};      // NOLINT

static s2048::txd_t txd{}; // NOLINT

static s2048::pieces::pieces_data pd{};       // NOLINT
static s2048::pieces::piece_id_queue_t spc{}; // NOLINT

static s2048::state_queue stq{}; // NOLINT

static surge::u32 current_score{0}; // NOLINT
static surge::u32 best_score{0};    // NOLINT

#ifdef SURGE_BUILD_TYPE_Debug
static bool show_debug_window{true}; // NOLINT
#endif

} // namespace globals

extern "C" SURGE_MODULE_EXPORT auto gl_on_load(surge::window::window_t w) -> int {
  using namespace s2048;
  using namespace surge;

  // Texture database
  globals::tdb = gl_atom::texture::database::create(128);

  // Sprite database
  gl_atom::sprite_database::database_create_info sdb_ci{128, 3};
  auto sdb{gl_atom::sprite_database::create(sdb_ci)};
  if (!sdb) {
    log_error("Unable to create sprite database");
    return static_cast<int>(sdb.error());
  }
  globals::sdb = *sdb;

  // Text Engine
  const auto ten_result{gl_atom::text::text_engine::create()};
  if (ten_result) {
    globals::txd.ten = *ten_result;
  } else {
    log_error("Unable to create text engine");
    return static_cast<int>(ten_result.error());
  }

  const auto load_face_result{
      globals::txd.ten.load_face("resources/dejavu_sans_bold.ttf", "dejavu_sans_bold")};
  if (load_face_result.has_value()) {
    log_error("Unable to load resources/dejavu_sans_bold.ttf");
    return static_cast<int>(*load_face_result);
  }

  // Glyph Caches
  const auto face{globals::txd.ten.get_face("dejavu_sans_bold")};
  if (!face) {
    log_error("Font dejavu_sans_bold not found in cache");
    return static_cast<int>(error::freetype_null_face);
  }

  const auto glyph_cache{gl_atom::text::glyph_cache::create(*face)};
  if (!glyph_cache) {
    log_error("Unable to create glyph cache for dejavu_sans_bold");
    return static_cast<int>(glyph_cache.error());
  }

  globals::txd.gc = *glyph_cache;
  globals::txd.gc.make_resident();

  // Text Buffer
  const auto text_buffer{gl_atom::text::text_buffer::create(540)};
  if (!text_buffer) {
    log_error("Unable to create text buffer");
    return static_cast<int>(text_buffer.error());
  }
  globals::txd.txb = *text_buffer;

  // Initialize global 2D projection matrix and view matrix
  const auto dims{window::get_dims(w)};
  const auto projection{glm::ortho(0.0f, dims[0], dims[1], 0.0f, 0.0f, 1.0f)};
  const auto view{glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f))};

  // PV UBO
  globals::pv_ubo = gl_atom::pv_ubo::buffer::create();
  globals::pv_ubo.update_all(&projection, &view);

  // Load game resources
  // All textures
  gl_atom::texture::create_info ci{};
  ci.filtering = gl_atom::texture::texture_filtering::anisotropic;
  globals::tdb.add(ci, "resources/board.png", "resources/button_press.png",
                   "resources/button_release.png", "resources/pieces_2.png",
                   "resources/pieces_4.png", "resources/pieces_8.png", "resources/pieces_16.png",
                   "resources/pieces_32.png", "resources/pieces_64.png", "resources/pieces_128.png",
                   "resources/pieces_256.png", "resources/pieces_512.png",
                   "resources/pieces_1024.png", "resources/pieces_2048.png");

  // Init piece ID queue
  for (surge::u8 i = 0; i < 16; i++) {
    globals::pd.ids.push_back(i);
  }

  // Init state stack
  globals::stq.push_back(game_state::idle);

  // Reserve memory for hash maps
  globals::pd.positions.reserve(16);

  globals::pd.current_values.reserve(16);
  globals::pd.target_values.reserve(16);

  globals::pd.current_slots.reserve(16);
  globals::pd.target_slots.reserve(16);

  // Create initial pieces
  pieces::create_random(globals::pd);
  pieces::create_random(globals::pd);

  // Debug window
#ifdef SURGE_BUILD_TYPE_Debug
  imgui::gl::create(imgui::create_config{});
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto gl_on_unload(surge::window::window_t) -> int {
  using namespace surge;

  globals::txd.txb.destroy();
  globals::txd.gc.destroy();
  globals::txd.ten.destroy();

  globals::pv_ubo.destroy();
  gl_atom::sprite_database::destroy(globals::sdb);

  globals::tdb.destroy();

  // Debug window
#ifdef SURGE_BUILD_TYPE_Debug
  imgui::gl::destroy();
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto gl_draw(surge::window::window_t) -> int {
  globals::pv_ubo.bind_to_location(2);

  // Sprite and text pass
  surge::gl_atom::sprite_database::draw(globals::sdb);
  globals::txd.txb.draw(glm::vec4{1.0f});

  // Debug UI pass
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::draw(globals::show_debug_window, globals::tdb, globals::sdb, globals::pd,
                            globals::spc, globals::stq);
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto gl_update(surge::window::window_t w, double) -> int {
  using std::snprintf;
  using namespace surge;
  using namespace s2048;
  using namespace surge::gl_atom;

  // Database resets
  gl_atom::sprite_database::begin_add(globals::sdb);
  globals::txd.txb.reset();

  // Background Texture handles
  static const auto bckg_handle{globals::tdb.find("resources/board.png").value_or(0)};

  // Background model
  const auto dims{window::get_dims(w)};
  const auto bckg_model{sprite_database::place_sprite(glm::vec2{0.0f}, dims, 0.1f)};
  sprite_database::add(globals::sdb, bckg_handle, bckg_model);

  // New Game button
  static ui::ui_state uist{-1, -1};
  static const auto new_game_press_handle{
      globals::tdb.find("resources/button_press.png").value_or(0)};
  static const auto new_game_release_handle{
      globals::tdb.find("resources/button_release.png").value_or(0)};

  // New game bttn
  ui::draw_data dd{glm::vec2{358.0f, 66.0f}, glm::vec2{138.0f, 40.0f}, 0.2f, 1.0f};
  ui::button_skin skins{new_game_release_handle, new_game_release_handle, new_game_press_handle};

  if (ui::button(w, __COUNTER__, uist, dd, globals::sdb, skins)) {
    new_game();
  }

  // Current score value
  std::array<char, 5> score_buffer{};
  std::fill(score_buffer.begin(), score_buffer.end(), 0);
  snprintf(score_buffer.data(), score_buffer.size(), "%u", globals::current_score);

  globals::txd.txb.push_centered(glm::vec3{358.0f, 58.0f, 0.2f}, 0.25, glm::vec2{64.0f, 37.0f},
                                 globals::txd.gc, score_buffer.data());

  // Best score value
  std::fill(score_buffer.begin(), score_buffer.end(), 0);
  snprintf(score_buffer.data(), score_buffer.size(), "%u", globals::best_score);

  globals::txd.txb.push_centered(glm::vec3{432.0f, 58.0f, 0.2f}, 0.25, glm::vec2{64.0f, 37.0f},
                                 globals::txd.gc, score_buffer.data());

  // Game states
  static bool should_add_new_piece{false};

  switch (globals::stq.front()) {

  case game_state::compress_right:
    if (pieces::idle(globals::pd)) {
      pieces::compress_right(globals::pd, should_add_new_piece);
      globals::stq.pop_front();
    }
    break;

  case game_state::merge_right:
    if (pieces::idle(globals::pd)) {
      pieces::merge_right(globals::pd, globals::spc, should_add_new_piece, globals::current_score);
      globals::stq.pop_front();
    }
    break;

  case game_state::compress_left:
    if (pieces::idle(globals::pd)) {
      pieces::compress_left(globals::pd, should_add_new_piece);
      globals::stq.pop_front();
    }
    break;

  case game_state::merge_left:
    if (pieces::idle(globals::pd)) {
      pieces::merge_left(globals::pd, globals::spc, should_add_new_piece, globals::current_score);
      globals::stq.pop_front();
    }
    break;

  case game_state::compress_up:
    if (pieces::idle(globals::pd)) {
      pieces::compress_up(globals::pd, should_add_new_piece);
      globals::stq.pop_front();
    }
    break;

  case game_state::merge_up:
    if (pieces::idle(globals::pd)) {
      pieces::merge_up(globals::pd, globals::spc, should_add_new_piece, globals::current_score);
      globals::stq.pop_front();
    }
    break;

  case game_state::compress_down:
    if (pieces::idle(globals::pd)) {
      pieces::compress_down(globals::pd, should_add_new_piece);
      globals::stq.pop_front();
    }
    break;

  case game_state::merge_down:
    if (pieces::idle(globals::pd)) {
      pieces::merge_down(globals::pd, globals::spc, should_add_new_piece, globals::current_score);
      globals::stq.pop_front();
    }
    break;

  case game_state::piece_removal:
    if (pieces::idle(globals::pd)) {
      pieces::remove_stale(globals::spc, globals::pd);
      pieces::update_exponents(globals::pd);
      globals::stq.pop_front();
    }
    break;

  case game_state::add_piece:
    if (pieces::idle(globals::pd)) {
      if (should_add_new_piece) {
        pieces::create_random(globals::pd);
        should_add_new_piece = false;
      }
      globals::stq.pop_front();
    }
    break;

  case game_state::check_game_over:
    if (pieces::idle(globals::pd)) {
      if (!pieces::game_over(globals::pd, dims[0], dims[1], globals::txd)) {
        globals::stq.pop_front();
      }
    }
    break;

  default:
    break;
  }

  // Update positions and add sprites to draw lists
  pieces::update_positions(globals::pd);
  pieces::add_sprites_to_database(globals::tdb, globals::sdb, globals::pd);

  return 0;
}

extern "C" SURGE_MODULE_EXPORT void gl_keyboard_event(surge::window::window_t, int key, int,
                                                      int action, int) {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::keyboard_event");
#endif

  using namespace s2048;

  // Examine state stack. Only push a move if the board is idle
  if (globals::stq.front() == game_state::idle) {
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
      globals::stq.pop_front();
      globals::stq.push_back(game_state::compress_right);
      globals::stq.push_back(game_state::merge_right);
      globals::stq.push_back(game_state::piece_removal);
      globals::stq.push_back(game_state::add_piece);
      globals::stq.push_back(game_state::check_game_over);
      globals::stq.push_back(game_state::idle);

    } else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
      globals::stq.pop_front();
      globals::stq.push_back(game_state::compress_left);
      globals::stq.push_back(game_state::merge_left);
      globals::stq.push_back(game_state::piece_removal);
      globals::stq.push_back(game_state::add_piece);
      globals::stq.push_back(game_state::check_game_over);
      globals::stq.push_back(game_state::idle);

    } else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
      globals::stq.pop_front();
      globals::stq.push_back(game_state::compress_up);
      globals::stq.push_back(game_state::merge_up);
      globals::stq.push_back(game_state::piece_removal);
      globals::stq.push_back(game_state::add_piece);
      globals::stq.push_back(game_state::check_game_over);
      globals::stq.push_back(game_state::idle);

    } else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
      globals::stq.pop_front();
      globals::stq.push_back(game_state::compress_down);
      globals::stq.push_back(game_state::merge_down);
      globals::stq.push_back(game_state::piece_removal);
      globals::stq.push_back(game_state::add_piece);
      globals::stq.push_back(game_state::check_game_over);
      globals::stq.push_back(game_state::idle);
    }
  }

#ifdef SURGE_BUILD_TYPE_Debug
  if (key == GLFW_KEY_F6 && action == GLFW_RELEASE) {
    globals::show_debug_window = !globals::show_debug_window;
    log_info("{} debug window", globals::show_debug_window ? "Showing" : "Hiding");
  }
#endif
}

extern "C" SURGE_MODULE_EXPORT void gl_mouse_button_event(surge::window::window_t, int button,
                                                          int action, int mods) {
#ifdef SURGE_BUILD_TYPE_Debug
  surge::imgui::register_mouse_callback(button, action, mods);
#endif
}

extern "C" SURGE_MODULE_EXPORT void gl_mouse_scroll_event(surge::window::window_t, double xoffset,
                                                          double yoffset) {
#ifdef SURGE_BUILD_TYPE_Debug
  surge::imgui::register_mouse_scroll_callback(xoffset, yoffset);
#endif
}

void s2048::new_game() {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::new_game");
#endif

  // Clear all state
  globals::pd.positions.clear();
  globals::pd.current_values.clear();
  globals::pd.target_values.clear();
  globals::pd.current_slots.clear();
  globals::pd.target_slots.clear();
  globals::pd.ids.clear();
  globals::stq.clear();

  if (globals::current_score > globals::best_score) {
    globals::best_score = globals::current_score;
  }

  globals::current_score = 0;
  log_debug("Best score {}", globals::best_score);

  // Reset piece ID queue
  for (surge::u8 i = 0; i < 16; i++) {
    globals::pd.ids.push_back(i);
  }

  // Init state stack
  globals::stq.push_back(game_state::idle);

  pieces::create_random(globals::pd);
  pieces::create_random(globals::pd);
}

#ifdef SURGE_BUILD_TYPE_Debug
auto s2048::state_to_str(game_state s) -> const char * {
  switch (s) {
  case idle:
    return "idle";
  case compress_up:
    return "compress_up";
  case compress_down:
    return "compress_down";
  case compress_left:
    return "compress_left";
  case compress_right:
    return "compress_right";
  case merge_up:
    return "merge_up";
  case merge_down:
    return "merge_down";
  case merge_left:
    return "merge_left";
  case merge_right:
    return "merge_right";
  case piece_removal:
    return "piece_removal";
  case add_piece:
    return "add_piece";
  case check_game_over:
    return "check_game_over";
  default:
    return "unrecognized state";
  }
}
#endif