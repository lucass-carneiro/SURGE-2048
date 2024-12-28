#include "debug_window.hpp"

#include "sc_opengl/atoms/imgui.hpp"
#include "sc_window.hpp"

static void pd_window(bool *open, const s2048::pieces::pieces_data &pd) noexcept {
  using namespace surge;
  using namespace ImGui;

  // Early out if the window is collapsed, as an optimization.
  if (!Begin("Pieces Data", open)) {
    End();
    return;
  }

  if (CollapsingHeader("Available IDs")) {
    if (BeginTable("pd.ids_table", 2)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &id : pd.ids) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", id);
        i++;
      }

      EndTable();
    }
  }

  if (CollapsingHeader("Positions")) {
    if (BeginTable("pd.ids_table", 3)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableSetupColumn("Position");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.positions) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", p.first);
        TableNextColumn();

        Text("(%.0f, %.0f)", p.second[0], p.second[1]);

        i++;
      }

      EndTable();
    }
  }

  if (CollapsingHeader("Current Values")) {
    if (BeginTable("pd.current_values", 3)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableSetupColumn("Current Value");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.current_values) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", p.first);
        TableNextColumn();

        Text("%u", p.second);

        i++;
      }

      EndTable();
    }
  }

  if (CollapsingHeader("Target Values")) {
    if (BeginTable("pd.target_values", 3)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableSetupColumn("Target Value");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.target_values) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", p.first);
        TableNextColumn();

        Text("%u", p.second);

        i++;
      }

      EndTable();
    }
  }

  if (CollapsingHeader("Current Slots")) {
    if (BeginTable("pd.current_slots", 3)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableSetupColumn("Current Slot");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.current_slots) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", p.first);
        TableNextColumn();

        Text("%u", p.second);

        i++;
      }

      EndTable();
    }
  }

  if (CollapsingHeader("Target Slots")) {
    if (BeginTable("pd.target_slots", 3)) {
      TableSetupColumn("Element");
      TableSetupColumn("ID");
      TableSetupColumn("Target Slot");
      TableHeadersRow();

      for (surge::usize i = 0; const auto &p : pd.target_slots) {
        TableNextRow();
        TableNextColumn();

        Text("%lu", i);
        TableNextColumn();

        Text("%u", p.first);
        TableNextColumn();

        Text("%u", p.second);

        i++;
      }

      EndTable();
    }
  }

  End();
}

static void spc_window(bool *open, const s2048::pieces::piece_id_queue_t &spc) noexcept {
  using namespace surge;
  using namespace ImGui;

  // Early out if the window is collapsed, as an optimization.
  if (!Begin("Stale pieces", open)) {
    End();
    return;
  }

  if (BeginTable("spc_table", 2)) {
    TableSetupColumn("Element");
    TableSetupColumn("Stale ID");
    TableHeadersRow();

    for (surge::usize i = 0; const auto &id : spc) {
      TableNextRow();
      TableNextColumn();

      Text("%lu", i);
      TableNextColumn();

      Text("%u", id);

      i++;
    }

    EndTable();
  }

  End();
}

static void stq_window(bool *open, const s2048::state_queue &stq) noexcept {
  using namespace surge;
  using namespace ImGui;

  // Early out if the window is collapsed, as an optimization.
  if (!Begin("State Queue", open)) {
    End();
    return;
  }

  if (BeginTable("state_table", 3)) {
    TableSetupColumn("Element");
    TableSetupColumn("State ID");
    TableSetupColumn("State Name");
    TableHeadersRow();

    for (surge::usize i = 0; const auto &id : stq) {
      TableNextRow();
      TableNextColumn();

      Text("%lu", i);
      TableNextColumn();

      Text("%u", id);
      TableNextColumn();

      Text("%s", s2048::state_to_str(static_cast<s2048::game_state>(id)));

      i++;
    }

    EndTable();
  }

  End();
}

static void main_window(surge::window::window_t w, const s2048::tdb_t &tdb, const s2048::sdb_t &sdb,
                        const s2048::pieces::pieces_data &pd,
                        const s2048::pieces::piece_id_queue_t &spc,
                        const s2048::state_queue &stq) noexcept {

  using namespace surge;
  using namespace ImGui;

  static bool tdb_window_open{false};
  static bool sdb_window_open{false};
  static bool pd_window_open{false};
  static bool spc_window_open{false};
  static bool stq_window_open{false};

  if (BeginMainMenuBar()) {
    if (BeginMenu("GPU Data")) {
      if (MenuItem("Texture Database")) {
        tdb_window_open = true;
      }

      if (MenuItem("Sprite Database")) {
        sdb_window_open = true;
      }

      EndMenu();
    }

    if (BeginMenu("Pieces")) {
      if (MenuItem("Pieces Data")) {
        pd_window_open = true;
      }

      if (MenuItem("Stale pieces")) {
        spc_window_open = true;
      }

      if (MenuItem("State queue")) {
        stq_window_open = true;
      }

      EndMenu();
    }

    if (BeginMenu("Module")) {
      if (MenuItem("Quit")) {
        window::set_should_close(w, true);
      }

      EndMenu();
    }

    EndMainMenuBar();
  }

  if (tdb_window_open) {
    gl_atom::imgui::texture_database_window(&tdb_window_open, tdb);
  }

  if (sdb_window_open) {
    gl_atom::imgui::sprite_database_window(&sdb_window_open, sdb);
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

void s2048::debug_window::draw(surge::window::window_t w, bool &show, const tdb_t &tdb,
                               const sdb_t &sdb, const pieces::pieces_data &pd,
                               const pieces::piece_id_queue_t &spc,
                               const state_queue &stq) noexcept {
  using namespace surge;
  if (show) {
    gl_atom::imgui::frame_begin();
    main_window(w, tdb, sdb, pd, spc, stq);
    gl_atom::imgui::frame_end();
  }
}