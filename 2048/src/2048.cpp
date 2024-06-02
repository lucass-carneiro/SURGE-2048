#include "2048.hpp"

#include "pieces.hpp"
#include "type_aliases.hpp"
#include "ui.hpp"

#ifdef SURGE_BUILD_TYPE_Debug
#  include "debug_window.hpp"
#endif

#include "player/error_types.hpp"
#include "player/logging.hpp"
#include "player/window.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

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

extern "C" SURGE_MODULE_EXPORT auto on_load(GLFWwindow *window) noexcept -> int {
  using namespace s2048;
  using namespace surge;
  using namespace surge::atom;

  // Bind callbacks
  const auto bind_callback_stat{s2048::bind_callbacks(window)};
  if (bind_callback_stat != 0) {
    return bind_callback_stat;
  }

  // Texture database
  globals::tdb = texture::database::create(128);

  // Sprite database
  auto sdb{sprite::database::create(128)};
  if (!sdb) {
    log_error("Unable to create sprite database");
    return static_cast<int>(sdb.error());
  }
  globals::sdb = *sdb;

  // Text Engine
  const auto ten_result{text::text_engine::create()};
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

  const auto glyph_cache{text::glyph_cache::create(*face)};
  if (!glyph_cache) {
    log_error("Unable to create glyph cache for dejavu_sans_bold");
    return static_cast<int>(glyph_cache.error());
  }

  globals::txd.gc = *glyph_cache;
  globals::txd.gc.make_resident();

  // Text Buffer
  const auto text_buffer{surge::atom::text::text_buffer::create(540)};
  if (!text_buffer) {
    log_error("Unable to create text buffer");
    return static_cast<int>(text_buffer.error());
  }
  globals::txd.txb = *text_buffer;

  // Initialize global 2D projection matrix and view matrix
  const auto [ww, wh] = surge::window::get_dims(window);
  const auto projection{glm::ortho(0.0f, ww, wh, 0.0f, 0.0f, 1.0f)};
  const auto view{glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f))};

  // PV UBO
  globals::pv_ubo = pv_ubo::buffer::create();
  globals::pv_ubo.update_all(&projection, &view);

  // Load game resources
  // All textures
  texture::create_info ci{};
  ci.filtering = renderer::texture_filtering::anisotropic;
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
  // pieces::create_random(globals::pd);
  pieces::create_piece(globals::pd, 1024, 0);
  pieces::create_piece(globals::pd, 1024, 1);

  // Debug window
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::create(window);
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto on_unload(GLFWwindow *window) noexcept -> int {
  // TODO

  globals::txd.txb.destroy();
  globals::txd.gc.destroy();
  globals::txd.ten.destroy();

  globals::pv_ubo.destroy();
  globals::sdb.destroy();

  globals::tdb.destroy();

  // Unbind callbacks
  const auto unbind_callback_stat{s2048::unbind_callbacks(window)};
  if (unbind_callback_stat != 0) {
    return unbind_callback_stat;
  }

  // Debug window
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::destroy();
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto draw(GLFWwindow *window) noexcept -> int {
  globals::pv_ubo.bind_to_location(2);

  // Sprite and text pass
  globals::sdb.draw();
  globals::txd.txb.draw(glm::vec4{1.0f});

  // Debug UI pass
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::draw(globals::show_debug_window, window);
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto update(GLFWwindow *window, double) noexcept -> int {
  using std::snprintf;
  using namespace surge;
  using namespace s2048;
  using namespace surge::atom;

  // Database resets
  globals::sdb.reset();
  globals::txd.txb.reset();

  // Background Texture handles
  static const auto bckg_handle{globals::tdb.find("resources/board.png").value_or(0)};

  // Background model
  const auto [ww, wh] = window::get_dims(window);
  const auto bckg_model{sprite::place(glm::vec2{0.0f}, glm::vec2{ww, wh}, 0.1f)};
  globals::sdb.add(bckg_handle, bckg_model, 1.0);

  // New Game button
  static ui::ui_state uist{window, -1, -1};
  static const auto new_game_press_handle{
      globals::tdb.find("resources/button_press.png").value_or(0)};
  static const auto new_game_release_handle{
      globals::tdb.find("resources/button_release.png").value_or(0)};

  // New game bttn
  ui::draw_data dd{glm::vec2{358.0f, 66.0f}, glm::vec2{138.0f, 40.0f}, 0.2f, 1.0f};
  ui::button_skin skins{new_game_release_handle, new_game_release_handle, new_game_press_handle};

  if (ui::button(__COUNTER__, uist, dd, globals::sdb, skins)) {
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
      if (!pieces::game_over(globals::pd, globals::txd)) {
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

extern "C" SURGE_MODULE_EXPORT void keyboard_event(GLFWwindow *, int key, int, int action,
                                                   int) noexcept {
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
    log_info("%s debug window", globals::show_debug_window ? "Showing" : "Hiding");
  }
#endif
}

extern "C" SURGE_MODULE_EXPORT void mouse_button_event(GLFWwindow *window, int button, int action,
                                                       int mods) noexcept {
#ifdef SURGE_BUILD_TYPE_Debug
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#endif
}

extern "C" SURGE_MODULE_EXPORT void mouse_scroll_event(GLFWwindow *window, double xoffset,
                                                       double yoffset) noexcept {
#ifdef SURGE_BUILD_TYPE_Debug
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
#endif
}

void s2048::new_game() noexcept {
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
  log_debug("Best score %hu", globals::best_score);

  // Reset piece ID queue
  for (surge::u8 i = 0; i < 16; i++) {
    globals::pd.ids.push_back(i);
  }

  // Init state stack
  globals::stq.push_back(game_state::idle);

  pieces::create_random(globals::pd);
  pieces::create_random(globals::pd);
}

auto s2048::bind_callbacks(GLFWwindow *window) noexcept -> int {
  log_info("Binding interaction callbacks");

  glfwSetKeyCallback(window, keyboard_event);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to bind keyboard event callback");
    return static_cast<int>(surge::error::keyboard_event_unbinding);
  }

  glfwSetMouseButtonCallback(window, mouse_button_event);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to bind mouse button event callback");
    return static_cast<int>(surge::error::mouse_button_event_unbinding);
  }

  glfwSetScrollCallback(window, mouse_scroll_event);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to bind mouse scroll event callback");
    return static_cast<int>(surge::error::mouse_scroll_event_unbinding);
  }

  return 0;
}

auto s2048::unbind_callbacks(GLFWwindow *window) noexcept -> int {
  log_info("Unbinding interaction callbacks");

  glfwSetKeyCallback(window, nullptr);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to unbind keyboard event callback");
  }

  glfwSetMouseButtonCallback(window, nullptr);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to unbind mouse button event callback");
  }

  glfwSetScrollCallback(window, nullptr);
  if (glfwGetError(nullptr) != GLFW_NO_ERROR) {
    log_warn("Unable to unbind mouse scroll event callback");
  }

  return 0;
}