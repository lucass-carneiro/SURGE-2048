#ifndef SURGE_MODULE_2048_DEBUG_WINDOW_HPP
#define SURGE_MODULE_2048_DEBUG_WINDOW_HPP

#include "2048.hpp"
#include "pieces.hpp"
#include "type_aliases.hpp"

// clang-format off
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// clang-format on

namespace s2048::debug_window {

void create(GLFWwindow *window) noexcept;
void destroy() noexcept;

void main_window(GLFWwindow *window, const tdb_t &tdb, const sdb_t &sdb,
                 const pieces::pieces_data &pd, const pieces::piece_id_queue_t &spc,
                 const state_queue &stq) noexcept;

void draw(bool &show, GLFWwindow *window, const tdb_t &tdb, const sdb_t &sdb,
          const pieces::pieces_data &pd, const pieces::piece_id_queue_t &spc,
          const state_queue &stq) noexcept;

} // namespace s2048::debug_window

#endif // SURGE_MODULE_2048_DEBUG_WINDOW_HPP