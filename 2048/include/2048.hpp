#ifndef SURGE_MODULE_2048_HPP
#define SURGE_MODULE_2048_HPP

#include "surge_core.hpp"

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
auto state_to_str(game_state s) noexcept -> const char *;
#endif

using state_queue = surge::deque<game_state>;

void new_game() noexcept;
} // namespace s2048

extern "C" {
SURGE_MODULE_EXPORT auto on_load() noexcept -> int;

SURGE_MODULE_EXPORT auto on_unload() noexcept -> int;

SURGE_MODULE_EXPORT auto draw() noexcept -> int;

SURGE_MODULE_EXPORT auto update(double dt) noexcept -> int;

SURGE_MODULE_EXPORT void keyboard_event(int key, int scancode, int action, int mods) noexcept;
SURGE_MODULE_EXPORT void mouse_button_event(int button, int action, int mods) noexcept;
SURGE_MODULE_EXPORT void mouse_scroll_event(double xoffset, double yoffset) noexcept;
}

#endif // SURGE_MODULE_2048_HPP