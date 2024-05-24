#include "2048.hpp"

#include "board.hpp"
#include "type_aliases.hpp"

#ifdef SURGE_BUILD_TYPE_Debug
#  include "debug_window.hpp"
#endif

#include "player/error_types.hpp"
#include "player/logging.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace globals {

static GLuint text_shader{0}; // NOLINT

static s2048::tdb_t tdb{};      // NOLINT
static s2048::pvubo_t pv_ubo{}; // NOLINT
static s2048::sdb_t sdb{};      // NOLINT

static s2048::txd_t txd{}; // NOLINT

#ifdef SURGE_BUILD_TYPE_Debug
static bool show_debug_window{true}; // NOLINT
#endif

} // namespace globals

extern "C" SURGE_MODULE_EXPORT auto on_load(GLFWwindow *window) noexcept -> int {
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
      globals::txd.ten.load_face("resources/ClearSans-Bold.ttf", "clear_sans")};
  if (load_face_result.has_value()) {
    log_error("Unable to load resources/ClearSans-Bold.ttf");
    return static_cast<int>(*load_face_result);
  }

  // Glyph Caches
  const auto face{globals::txd.ten.get_face("clear_sans")};
  if (!face) {
    log_error("Font clear_sans not found in cache");
    return static_cast<int>(error::freetype_null_face);
  }

  const auto gc_result{text::glyph_cache::create(*face)};
  if (!gc_result) {
    log_error("Unable to create glyph cache for daveau_regular");
    return static_cast<int>(gc_result.error());
  }

  globals::txd.gc = *gc_result;
  globals::txd.gc.make_resident();

  // Text Buffer
  globals::txd.txb = surge::atom::text::text_buffer::create(540);

  // Initialize global 2D projection matrix and view matrix
  const auto [ww, wh] = surge::window::get_dims(window);
  const auto projection{glm::ortho(0.0f, ww, wh, 0.0f, 0.0f, 1.0f)};
  const auto view{glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f))};

  // PV UBO
  globals::pv_ubo = pv_ubo::buffer::create();
  globals::pv_ubo.update_all(&projection, &view);

  // Text shader
  const auto text_shader{
      surge::renderer::create_shader_program("shaders/text.vert", "shaders/text.frag")};
  if (!text_shader) {
    return static_cast<int>(text_shader.error());
  }
  globals::text_shader = *text_shader;

  // First state
  s2048::board::load(window, globals::tdb, globals::sdb);

  // Debug window
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::create(window);
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto on_unload(GLFWwindow *window) noexcept -> int {
  s2048::board::unload(globals::tdb, globals::sdb);

  surge::renderer::destroy_shader_program(globals::text_shader);

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

  s2048::board::draw(globals::sdb);

  // Debug UI pass
#ifdef SURGE_BUILD_TYPE_Debug
  s2048::debug_window::draw(globals::show_debug_window, window);
#endif

  return 0;
}

extern "C" SURGE_MODULE_EXPORT auto update(GLFWwindow *window, double dt) noexcept -> int {
  s2048::board::update(dt, window, globals::tdb, globals::sdb);

  return 0;
}

extern "C" SURGE_MODULE_EXPORT void keyboard_event(GLFWwindow *, int key, int, int action,
                                                   int) noexcept {
  if (key == GLFW_KEY_F6 && action == GLFW_RELEASE) {
    globals::show_debug_window = !globals::show_debug_window;
    log_info("%s debug window", globals::show_debug_window ? "Showing" : "Hiding");
  }
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