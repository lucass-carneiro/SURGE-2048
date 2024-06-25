#include "ui.hpp"

auto s2048::ui::button(surge::i32 id, ui_state &state, draw_data &dd, sdb_t &sdb,
                       const button_skin &bs) noexcept -> bool {
  using namespace surge;
  using namespace surge::gl_atom;

  const glm::vec4 widget_rect{dd.pos[0], dd.pos[1], dd.scale[0], dd.scale[1]};

  const auto mouse_in_widget{point_in_rect(window::get_cursor_pos(), widget_rect)};

  bool bttn_result{false};

  if (id == state.active) {
    if (window::get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
      if (id == state.hot) {
        bttn_result = true;
      }
      state.active = -1;
    }
  } else if (id == state.hot) {
    if (window::get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && mouse_in_widget) {
      state.active = id;
    }
  }

  // "If there is an active item, it doesn't sets us as hot" - Casey
  if (mouse_in_widget && id != state.active) {
    state.hot = id;
  }

  // Display the up or down skin when the button is held
  if (id == state.active) {
    const auto model{sprite::place(dd.pos, dd.scale, dd.z)};
    sdb.add(bs.handle_press, model, dd.alpha);
  } else if (id == state.hot) {
    const auto model{sprite::place(dd.pos, dd.scale, dd.z)};
    sdb.add(bs.handle_select, model, dd.alpha);
  } else {
    const auto model{sprite::place(dd.pos, dd.scale, dd.z)};
    sdb.add(bs.handle_release, model, dd.alpha);
  }

  return bttn_result;
}