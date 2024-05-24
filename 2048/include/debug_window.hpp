#ifndef SURGE_MODULE_2048_DEBUG_WINDOW_HPP
#define SURGE_MODULE_2048_DEBUG_WINDOW_HPP

// clang-format off
#include "player/window.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// clang-format on

namespace s2048::debug_window {

void create(GLFWwindow *window) noexcept;
void destroy() noexcept;

void main_window(GLFWwindow *window) noexcept;

void draw(bool &show, GLFWwindow *window) noexcept;

} // namespace s2048::debug_window

#endif // SURGE_MODULE_2048_DEBUG_WINDOW_HPP