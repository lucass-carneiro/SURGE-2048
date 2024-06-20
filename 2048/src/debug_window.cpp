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

void s2048::debug_window::draw(bool &show, GLFWwindow *window, const tdb_t &tdb, const sdb_t &sdb,
                               const pieces::pieces_data &pd, const pieces::piece_id_queue_t &spc,
                               const state_queue &stq) noexcept {
  if (show) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    main_window(window, tdb, sdb, pd, spc, stq);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
}

static void tdb_window(bool *open, const s2048::tdb_t &tdb) noexcept {
  if (!ImGui::Begin("Texture Database", open, 0)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  constexpr ImGuiTableFlags flags{ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV
                                  | ImGuiTableFlags_BordersH
                                  | ImGuiTableFlags_HighlightHoveredColumn};

  if (ImGui::BeginTable("tdb_table", 4, flags)) {
    const auto &ids{tdb.get_ids()};
    const auto &handles{tdb.get_handles()};
    const auto &hashes{tdb.get_hashes()};

    ImGui::TableSetupColumn("Element");
    ImGui::TableSetupColumn("Id");
    ImGui::TableSetupColumn("Handle");
    ImGui::TableSetupColumn("Hash");
    ImGui::TableHeadersRow();

    for (surge::usize i = 0; i < tdb.size(); i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("%lu", i);
      ImGui::TableNextColumn();

      ImGui::Text("%u", ids[i]);
      ImGui::TableNextColumn();

      ImGui::Text("%lu", handles[i]);
      ImGui::TableNextColumn();

      ImGui::Text("%lu", hashes[i]);
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

static void sdb_window(bool *open, const s2048::sdb_t &sdb) noexcept {
  if (!ImGui::Begin("Sprite Database", open, 0)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  constexpr ImGuiTableFlags flags{ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV
                                  | ImGuiTableFlags_BordersH
                                  | ImGuiTableFlags_HighlightHoveredColumn};

  if (ImGui::CollapsingHeader("Texture Handles")) {
    if (ImGui::BeginTable("sdb_table_texture_handle", 2, flags)) {
      ImGui::TableSetupColumn("Size");
      ImGui::TableSetupColumn("Write Buffer");
      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.texture_handles.size());
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.texture_handles.get_write_buffer());

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Model Matrices")) {
    if (ImGui::BeginTable("sdb_table_model_matrices", 2, flags)) {
      ImGui::TableSetupColumn("Size");
      ImGui::TableSetupColumn("Write Buffer");
      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.models.size());
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.models.get_write_buffer());

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Alphas")) {
    if (ImGui::BeginTable("sdb_table_alphas", 2, flags)) {
      ImGui::TableSetupColumn("Size");
      ImGui::TableSetupColumn("Write Buffer");
      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.alphas.size());
      ImGui::TableNextColumn();

      ImGui::Text("%lu", sdb.alphas.get_write_buffer());

      ImGui::EndTable();
    }
  }

  ImGui::End();
}

static void pd_window(bool *open, const s2048::pieces::pieces_data &pd) noexcept {
  if (!ImGui::Begin("Pieces Data", open, 0)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  constexpr ImGuiTableFlags flags{ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV
                                  | ImGuiTableFlags_BordersH
                                  | ImGuiTableFlags_HighlightHoveredColumn};

  if (ImGui::CollapsingHeader("Available IDs")) {
    if (ImGui::BeginTable("pd.ids_table", 2, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &id : pd.ids) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", id);
        i++;
      }

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Positions")) {
    if (ImGui::BeginTable("pd.ids_table", 3, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Position");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.positions) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.first);
        ImGui::TableNextColumn();

        ImGui::Text("(%.0f, %.0f)", p.second[0], p.second[1]);

        i++;
      }

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Current Values")) {
    if (ImGui::BeginTable("pd.current_values", 3, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Current Value");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.current_values) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.first);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.second);

        i++;
      }

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Target Values")) {
    if (ImGui::BeginTable("pd.target_values", 3, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Target Value");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.target_values) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.first);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.second);

        i++;
      }

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Current Slots")) {
    if (ImGui::BeginTable("pd.current_slots", 3, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Current Slot");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.current_slots) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.first);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.second);

        i++;
      }

      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Target Slots")) {
    if (ImGui::BeginTable("pd.target_slots", 3, flags)) {
      ImGui::TableSetupColumn("Element");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Target Slot");
      ImGui::TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.target_slots) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("%lu", i);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.first);
        ImGui::TableNextColumn();

        ImGui::Text("%u", p.second);

        i++;
      }

      ImGui::EndTable();
    }
  }

  ImGui::End();
}

static void spc_window(bool *open, const s2048::pieces::piece_id_queue_t &) noexcept {
  if (!ImGui::Begin("Piece ID Queue", open, 0)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  ImGui::End();
}

static void stq_window(bool *open, const s2048::state_queue &) noexcept {
  if (!ImGui::Begin("State Queue", open, 0)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  ImGui::End();
}

void s2048::debug_window::main_window(GLFWwindow *window, const tdb_t &tdb, const sdb_t &sdb,
                                      const pieces::pieces_data &pd,
                                      const pieces::piece_id_queue_t &spc,
                                      const state_queue &stq) noexcept {
  static bool tdb_window_open{false};
  static bool sdb_window_open{false};
  static bool pd_window_open{false};
  static bool spc_window_open{false};
  static bool stq_window_open{false};

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("GPU Data")) {
      if (ImGui::MenuItem("Texture Database")) {
        tdb_window_open = true;
      }

      if (ImGui::MenuItem("Sprite Database")) {
        sdb_window_open = true;
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Module")) {
      if (ImGui::MenuItem("Quit")) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Pieces")) {
      if (ImGui::MenuItem("Pieces Data")) {
        pd_window_open = true;
      }

      if (ImGui::MenuItem("Piece ID Queue")) {
        spc_window_open = true;
      }

      if (ImGui::MenuItem("State Queue")) {
        stq_window_open = true;
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  if (tdb_window_open) {
    tdb_window(&tdb_window_open, tdb);
  }

  if (sdb_window_open) {
    sdb_window(&sdb_window_open, sdb);
  }

  if (pd_window_open) {
    pd_window(&pd_window_open, pd);
  }

  if (spc_window_open) {
    spc_window(&spc_window_open, spc);
  }

  if (stq_window_open) {
    stq_window(&stq_window_open, stq);
  }
}