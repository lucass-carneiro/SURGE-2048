#ifndef SURGE_MODULE_2048_HPP
#define SURGE_MODULE_2048_HPP

// clang-format off
#include "player/options.hpp"
#include "player/container_types.hpp"
#include "player/gl_includes.hpp"
// clang-format on

#if defined(SURGE_COMPILER_Clang) || defined(SURGE_COMPILER_GCC) && COMPILING_SURGE_MODULE_2048
#  define SURGE_MODULE_EXPORT __attribute__((__visibility__("default")))
#elif defined(SURGE_COMPILER_MSVC) && COMPILING_SURGE_MODULE_2048
#  define SURGE_MODULE_EXPORT __declspec(dllexport)
#elif defined(SURGE_COMPILER_MSVC)
#  define SURGE_MODULE_EXPORT __declspec(dllimport)
#else
#  define SURGE_MODULE_EXPORT
#endif

namespace s2048 {

using state_code_t = surge::u8;
enum game_state : state_code_t {
  idle,
  compress_up,
  compress_down,
  compress_left,
  compress_right,
  merge_up,
  merge_down,
  merge_left,
  merge_right,
  piece_removal,
  add_piece,
  check_game_over
};

using state_queue = surge::deque<game_state>;

void new_game() noexcept;

// Callbacks
auto bind_callbacks(GLFWwindow *window) noexcept -> int;
auto unbind_callbacks(GLFWwindow *window) noexcept -> int;

} // namespace s2048

extern "C" {
SURGE_MODULE_EXPORT auto on_load(GLFWwindow *window) noexcept -> int;

SURGE_MODULE_EXPORT auto on_unload(GLFWwindow *window) noexcept -> int;

SURGE_MODULE_EXPORT auto draw(GLFWwindow *window) noexcept -> int;

SURGE_MODULE_EXPORT auto update(GLFWwindow *window, double dt) noexcept -> int;

SURGE_MODULE_EXPORT void keyboard_event(GLFWwindow *window, int key, int scancode, int action,
                                        int mods) noexcept;

SURGE_MODULE_EXPORT void mouse_button_event(GLFWwindow *window, int button, int action,
                                            int mods) noexcept;

SURGE_MODULE_EXPORT void mouse_scroll_event(GLFWwindow *window, double xoffset,
                                            double yoffset) noexcept;
}

#endif // SURGE_MODULE_2048_HPP