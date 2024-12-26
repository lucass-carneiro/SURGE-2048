#ifndef SURGE_MODULE_2048_HPP
#define SURGE_MODULE_2048_HPP

#include "sc_container_types.hpp"
#include "sc_integer_types.hpp"
#include "sc_options.hpp"
#include "sc_window.hpp"

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

#ifdef SURGE_BUILD_TYPE_Debug
auto state_to_str(game_state s) -> const char *;
#endif

using state_queue = surge::deque<game_state>;

void new_game();

} // namespace s2048

extern "C" {

SURGE_MODULE_EXPORT auto gl_on_load(surge::window::window_t w) -> int;

SURGE_MODULE_EXPORT auto gl_on_unload(surge::window::window_t w) -> int;

SURGE_MODULE_EXPORT auto gl_draw(surge::window::window_t w) -> int;

SURGE_MODULE_EXPORT auto gl_update(surge::window::window_t w, double dt) -> int;

SURGE_MODULE_EXPORT void gl_keyboard_event(surge::window::window_t w, int key, int scancode,
                                           int action, int mods);

SURGE_MODULE_EXPORT void gl_mouse_button_event(surge::window::window_t w, int button, int action,
                                               int mods);

SURGE_MODULE_EXPORT void gl_mouse_scroll_event(surge::window::window_t w, double xoffset,
                                               double yoffset);
}

#endif // SURGE_MODULE_2048_HPP