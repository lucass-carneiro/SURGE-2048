#include "debug_window.hpp"

#include "surge_core.hpp"

static void pd_window(bool *open, const s2048::pieces::pieces_data &pd) noexcept {
  using namespace surge;

  // Early out if the window is collapsed, as an optimization.
  if (!imgui::begin("Pieces Data", open)) {
    imgui::end();
    return;
  }

  if (imgui::colapsing_header("Available IDs")) {
    if (imgui::begin_table("pd.ids_table", 2)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &id : pd.ids) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", id);
        i++;
      }

      imgui::end_table();
    }
  }

  if (imgui::colapsing_header("Positions")) {
    if (imgui::begin_table("pd.ids_table", 3)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_setup_column("Position");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &p : pd.positions) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", p.first);
        imgui::table_next_column();

        imgui::text("(%.0f, %.0f)", p.second[0], p.second[1]);

        i++;
      }

      imgui::end_table();
    }
  }

  if (imgui::colapsing_header("Current Values")) {
    if (imgui::begin_table("pd.current_values", 3)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_setup_column("Current Value");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &p : pd.current_values) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", p.first);
        imgui::table_next_column();

        imgui::text("%u", p.second);

        i++;
      }

      imgui::end_table();
    }
  }

  if (imgui::colapsing_header("Target Values")) {
    if (imgui::begin_table("pd.target_values", 3)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_setup_column("Target Value");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &p : pd.target_values) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", p.first);
        imgui::table_next_column();

        imgui::text("%u", p.second);

        i++;
      }

      imgui::end_table();
    }
  }

  if (imgui::colapsing_header("Current Slots")) {
    if (imgui::begin_table("pd.current_slots", 3)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_setup_column("Current Slot");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &p : pd.current_slots) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", p.first);
        imgui::table_next_column();

        imgui::text("%u", p.second);

        i++;
      }

      imgui::end_table();
    }
  }

  if (imgui::colapsing_header("Target Slots")) {
    if (imgui::begin_table("pd.target_slots", 3)) {
      imgui::table_setup_column("Element");
      imgui::table_setup_column("ID");
      imgui::table_setup_column("Target Slot");
      imgui::table_headers_row();

      for (surge::usize i = 0; const auto &p : pd.target_slots) {
        imgui::table_next_row();
        imgui::table_next_column();

        imgui::text("%lu", i);
        imgui::table_next_column();

        imgui::text("%u", p.first);
        imgui::table_next_column();

        imgui::text("%u", p.second);

        i++;
      }

      imgui::end_table();
    }
  }

  imgui::end();
}

static void spc_window(bool *open, const s2048::pieces::piece_id_queue_t &spc) noexcept {
  using namespace surge;
  // Early out if the window is collapsed, as an optimization.
  if (!imgui::begin("Stale pieces", open)) {
    imgui::end();
    return;
  }

  if (imgui::begin_table("spc_table", 2)) {
    imgui::table_setup_column("Element");
    imgui::table_setup_column("Stale ID");
    imgui::table_headers_row();

    for (surge::usize i = 0; const auto &id : spc) {
      imgui::table_next_row();
      imgui::table_next_column();

      imgui::text("%lu", i);
      imgui::table_next_column();

      imgui::text("%u", id);

      i++;
    }

    imgui::end_table();
  }

  imgui::end();
}

static void stq_window(bool *open, const s2048::state_queue &stq) noexcept {
  using namespace surge;

  // Early out if the window is collapsed, as an optimization.
  if (!imgui::begin("State Queue", open)) {
    imgui::end();
    return;
  }

  if (imgui::begin_table("state_table", 3)) {
    imgui::table_setup_column("Element");
    imgui::table_setup_column("State ID");
    imgui::table_setup_column("State Name");
    imgui::table_headers_row();

    for (surge::usize i = 0; const auto &id : stq) {
      imgui::table_next_row();
      imgui::table_next_column();

      imgui::text("%lu", i);
      imgui::table_next_column();

      imgui::text("%u", id);
      imgui::table_next_column();

      imgui::text("%s", s2048::state_to_str(static_cast<s2048::game_state>(id)));

      i++;
    }

    imgui::end_table();
  }

  imgui::end();
}

static void main_window(const s2048::tdb_t &tdb, const s2048::sdb_t &sdb,
                        const s2048::pieces::pieces_data &pd,
                        const s2048::pieces::piece_id_queue_t &spc,
                        const s2048::state_queue &stq) noexcept {

  using namespace surge;

  static bool tdb_window_open{false};
  static bool sdb_window_open{false};
  static bool pd_window_open{false};
  static bool spc_window_open{false};
  static bool stq_window_open{false};

  if (imgui::begin_main_menu_bar()) {
    if (imgui::begin_menu("GPU Data")) {
      if (imgui::menu_item("Texture Database")) {
        tdb_window_open = true;
      }

      if (imgui::menu_item("Sprite Database")) {
        sdb_window_open = true;
      }

      imgui::end_menu();
    }

    if (imgui::begin_menu("Pieces")) {
      if (imgui::menu_item("Pieces Data")) {
        pd_window_open = true;
      }

      if (imgui::menu_item("Stale pieces")) {
        spc_window_open = true;
      }

      if (imgui::menu_item("State queue")) {
        stq_window_open = true;
      }

      imgui::end_menu();
    }

    if (imgui::begin_menu("Module")) {
      if (imgui::menu_item("Quit")) {
        window::set_should_close(true);
      }

      imgui::end_menu();
    }

    imgui::end_main_menu_bar();
  }

  if (tdb_window_open) {
    imgui::gl::texture_database_window(&tdb_window_open, tdb);
  }

  if (sdb_window_open) {
    imgui::gl::sprite_database_window(&sdb_window_open, sdb);
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

void s2048::debug_window::draw(bool &show, const tdb_t &tdb, const sdb_t &sdb,
                               const pieces::pieces_data &pd, const pieces::piece_id_queue_t &spc,
                               const state_queue &stq) noexcept {
  using namespace surge;
  if (show) {
    imgui::gl::frame_begin();
    main_window(tdb, sdb, pd, spc, stq);
    imgui::gl::frame_end();
  }
}