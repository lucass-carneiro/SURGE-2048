#include "ui.hpp"

auto s2048::ui::button(surge::window::window_t w, surge::i32 id, ui_state &state, draw_data &dd,
                       sdb_t &sdb, const button_skin &bs) noexcept -> bool {
  using namespace surge;
  using namespace surge::gl_atom;

  const glm::vec4 widget_rect{dd.pos[0], dd.pos[1], dd.scale[0], dd.scale[1]};

  const auto mouse_in_widget{point_in_rect(window::get_cursor_pos(w), widget_rect)};

  bool bttn_result{false};

  if (id == state.active) {
    if (window::get_mouse_button(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
      if (id == state.hot) {
        bttn_result = true;
      }
      state.active = -1;
    }
  } else if (id == state.hot) {
    if (window::get_mouse_button(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && mouse_in_widget) {
      state.active = id;
    }
  }

  // "If there is an active item, it doesn't sets us as hot" - Casey
  if (mouse_in_widget && id != state.active) {
    state.hot = id;
  }

  // Display the up or down skin when the button is held
  if (id == state.active) {
    const auto model{sprite_database::place_sprite(dd.pos, dd.scale, dd.z)};
    gl_atom::sprite_database::add(sdb, bs.handle_press, model,
                                  glm::vec4{1.0f, 1.0f, 1.0f, dd.alpha});
  } else if (id == state.hot) {
    const auto model{sprite_database::place_sprite(dd.pos, dd.scale, dd.z)};
    gl_atom::sprite_database::add(sdb, bs.handle_select, model,
                                  glm::vec4{1.0f, 1.0f, 1.0f, dd.alpha});
  } else {
    const auto model{sprite_database::place_sprite(dd.pos, dd.scale, dd.z)};
    gl_atom::sprite_database::add(sdb, bs.handle_release, model,
                                  glm::vec4{1.0f, 1.0f, 1.0f, dd.alpha});
  }

  return bttn_result;
}