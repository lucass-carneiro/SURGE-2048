#include "debug_window.hpp"

#include "player/logging.hpp"

void s2048::debug_window::create(GLFWwindow *window) noexcept {
  log_info("Initializing debug window");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  /* ImGuiIO &io =  ImGui::GetIO();
  (void)io;
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard
  Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable
  Gamepad Controls
  */

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 460");
}

void s2048::debug_window::destroy() noexcept {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void s2048::debug_window::draw(bool &show, GLFWwindow *window) noexcept {
  if (show) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    main_window(window);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
}

void s2048::debug_window::main_window(GLFWwindow *) noexcept {
  if (ImGui::BeginMainMenuBar()) {
    ImGui::EndMainMenuBar();
  }
}