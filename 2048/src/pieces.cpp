#include "pieces.hpp"

#include "player/logging.hpp"

#include <algorithm>
#include <random>

#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
#  include <tracy/Tracy.hpp>
#endif

namespace globals {

// clang-format off
static const std::array<glm::vec2, 16> slot_coords {
  glm::vec2{15.0f , 315.0f},
  glm::vec2{136.0f, 315.0f},
  glm::vec2{257.0f, 315.0f},
  glm::vec2{378.0f, 315.0f},

  glm::vec2{15.0f , 436.0f},
  glm::vec2{136.0f, 436.0f},
  glm::vec2{257.0f, 436.0f},
  glm::vec2{378.0f, 436.0f},

  glm::vec2{15.0f , 557.0f},
  glm::vec2{136.0f, 557.0f},
  glm::vec2{257.0f, 557.0f},
  glm::vec2{378.0f, 557.0f},

  glm::vec2{15.0f , 678.0f},
  glm::vec2{136.0f, 678.0f},
  glm::vec2{257.0f, 678.0f},
  glm::vec2{378.0f, 678.0f}
};
// clang-format on

static const auto slot_delta{slot_coords[1][0] - slot_coords[0][0]};

} // namespace globals

auto s2048::pieces::create_piece(pieces_data &pd, surge::u16 value, surge::u8 slot) noexcept
    -> surge::u8 {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::create_piece");
#endif

  // Gen ID
  const auto id{pd.ids.front()};
  pd.ids.pop_front();

  // Store components
  pd.positions[id] = globals::slot_coords[slot];

  pd.current_values[id] = value;
  pd.target_values[id] = value;

  pd.current_slots[id] = slot;
  pd.target_slots[id] = slot;

  return id;
}

void s2048::pieces::delete_piece(pieces_data &pd, surge::u8 piece_id) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::delete_piece");
#endif

  if (std::find(pd.ids.begin(), pd.ids.end(), piece_id) != pd.ids.end()) {
    log_debug("Unable to remove piece id %u because it is already non existant", piece_id);
  } else {
    pd.positions.erase(piece_id);

    pd.current_values.erase(piece_id);
    pd.target_values.erase(piece_id);

    pd.current_slots.erase(piece_id);
    pd.target_slots.erase(piece_id);

    pd.ids.push_back(piece_id);
  }
}

template <typename T, typename U> static auto has_value(const T value, const U &collection)
    -> bool {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::has_value");
#endif

  for (const auto &i : collection) {
    if (i.second == value) {
      return true;
    }
  }
  return false;
}

auto s2048::pieces::create_random(pieces_data &pd) noexcept -> surge::u8 {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::create_random");
#endif

  log_debug("Adding random piece");

  const auto &slots{pd.current_slots};

  if (slots.size() == 16) {
    log_debug("pieces::create_random failed because the board is full. Returning piece ID 16");
    return 16;
  }

  static std::minstd_rand engine{std::random_device{}()};
  static std::uniform_int_distribution<unsigned short> slot_dist{0, 15};
  static std::uniform_int_distribution<unsigned short> exp_dist{1, 2};

  const auto random_value{static_cast<surge::u8>(1 << exp_dist(engine))};

  // Find free slot
  auto random_slot{slot_dist(engine)};

  while (has_value(random_slot, slots)) {
    random_slot = static_cast<surge::u8>(slot_dist(engine));
  }

  return create_piece(pd, random_value, random_slot);
}

auto s2048::pieces::idle(const pieces_data &pd) noexcept -> bool {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::idle");
#endif

  auto &slots{pd.current_slots};
  auto &target_slots{pd.target_slots};

  for (const auto &s : slots) {
    if (s.second != target_slots.at(s.first)) {
      return false;
    }
  }

  return true;
}

auto s2048::pieces::deflatten_slot(surge::u8 slot) noexcept -> board_address {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::deflatten_slot");
#endif

  switch (slot) {
  case 0:
    return board_address{0, 0};
  case 1:
    return board_address{0, 1};
  case 2:
    return board_address{0, 2};
  case 3:
    return board_address{0, 3};
  case 4:
    return board_address{1, 0};
  case 5:
    return board_address{1, 1};
  case 6:
    return board_address{1, 2};
  case 7:
    return board_address{1, 3};
  case 8:
    return board_address{2, 0};
  case 9:
    return board_address{2, 1};
  case 10:
    return board_address{2, 2};
  case 11:
    return board_address{2, 3};
  case 12:
    return board_address{3, 0};
  case 13:
    return board_address{3, 1};
  case 14:
    return board_address{3, 2};
  case 15:
    return board_address{3, 3};
  default:
    return board_address{16, 16};
  }
}

auto s2048::pieces::get_element(const pieces_data &pd, board_element_type type,
                                surge::u16 value) noexcept -> board_element {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::get_element");
#endif

  board_element element{{16, 16, 16, 16}, 0, board_element_configuration::OOOO};

  // Get data and size
  for (const auto &slot : pd.current_slots) {
    const auto slot_coords{deflatten_slot(slot.second)};

    if (type == board_element_type::row && slot_coords.row == value) {
      element.data.at(slot_coords.col) = slot.first;
      element.size += 1;

    } else if (type == board_element_type::column && slot_coords.col == value) {
      element.data.at(slot_coords.row) = slot.first;
      element.size += 1;
    }
  }

  // Get get configuration
  if (element.size == 0) {
    return element;
  } else if (element.size == 1) {
    if (element.data[0] != 16) {
      element.config = board_element_configuration::XOOO;
    } else if (element.data[1] != 16) {
      element.config = board_element_configuration::OXOO;
    } else if (element.data[2] != 16) {
      element.config = board_element_configuration::OOXO;
    } else {
      element.config = board_element_configuration::OOOX;
    }
  } else if (element.size == 4) {
    element.config = board_element_configuration::XXXX;
  } else if (element.size == 3) {
    if (element.data[0] != 16 && element.data[1] != 16 && element.data[2] != 16) {
      element.config = board_element_configuration::XXXO;
    } else if (element.data[0] != 16 && element.data[1] != 16 && element.data[3] != 16) {
      element.config = board_element_configuration::XXOX;
    } else if (element.data[0] != 16 && element.data[2] != 16 && element.data[3] != 16) {
      element.config = board_element_configuration::XOXX;
    } else {
      element.config = board_element_configuration::OXXX;
    }
  } else if (element.size == 2) {
    if (element.data[0] != 16 && element.data[1] != 16) {
      element.config = board_element_configuration::XXOO;
    } else if (element.data[0] != 16 && element.data[2] != 16) {
      element.config = board_element_configuration::XOXO;
    } else if (element.data[0] != 16 && element.data[3] != 16) {
      element.config = board_element_configuration::XOOX;
    } else if (element.data[1] != 16 && element.data[2] != 16) {
      element.config = board_element_configuration::OXXO;
    } else if (element.data[1] != 16 && element.data[3] != 16) {
      element.config = board_element_configuration::OXOX;
    } else {
      element.config = board_element_configuration::OOXX;
    }
  }

  return element;
}

void s2048::pieces::compress_right(pieces_data &pd, bool &should_add_new_piece) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::compress_right");
#endif

  log_debug("Compressing right");

  auto &target_slots{pd.target_slots};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::row, i)};

    switch (element.config) {
    case board_element_configuration::XOOO:
      target_slots[element.data[0]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOO:
      target_slots[element.data[1]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXO:
      target_slots[element.data[2]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOO:
      target_slots[element.data[0]] = 2 + i * 4;
      target_slots[element.data[1]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXO:
      target_slots[element.data[0]] = 2 + i * 4;
      target_slots[element.data[2]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOOX:
      target_slots[element.data[0]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXO:
      target_slots[element.data[1]] = 2 + i * 4;
      target_slots[element.data[2]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOX:
      target_slots[element.data[1]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXXO:
      target_slots[element.data[0]] = 1 + i * 4;
      target_slots[element.data[1]] = 2 + i * 4;
      target_slots[element.data[2]] = 3 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOX:
      target_slots[element.data[0]] = 1 + i * 4;
      target_slots[element.data[1]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXX:
      target_slots[element.data[0]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    default:
      break;
    }
  }
}

void s2048::pieces::merge_right(pieces_data &pd, piece_id_queue_t &stale_pieces,
                                bool &should_add_new_piece, surge::u32 &game_score) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::merge_right");
#endif

  log_debug("Merging right");

  surge::u16 round_points{0};

  auto &target_slots{pd.target_slots};
  auto &values{pd.current_values};
  auto &target_values{pd.target_values};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::row, i)};

    switch (element.config) {

    case board_element_configuration::OOXX:
      if (values[element.data[2]] == values[element.data[3]]) {
        target_slots[element.data[2]] = 3 + i * 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::OXXX:
      if (values[element.data[2]] == values[element.data[3]]) {
        target_slots[element.data[1]] = 2 + i * 4;
        target_slots[element.data[2]] = 3 + i * 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[1]] = 2 + i * 4;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXX:
      if (values[element.data[2]] == values[element.data[3]]) {
        target_slots[element.data[0]] = 1 + i * 4;
        target_slots[element.data[1]] = 2 + i * 4;
        target_slots[element.data[2]] = 3 + i * 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[0]] = 1 + i * 4;
        target_slots[element.data[1]] = 2 + i * 4;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      } else if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[0]] = 1 + i * 4;
        target_values[element.data[0]] *= 2;
        round_points += target_values[element.data[0]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      }
      break;

    default:
      break;
    }
  }

  game_score += round_points;
}

void s2048::pieces::compress_left(pieces_data &pd, bool &should_add_new_piece) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::compress_left");
#endif

  log_debug("Compressing left");

  auto &target_slots{pd.target_slots};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::row, i)};

    switch (element.config) {
    case board_element_configuration::OXOO:
      target_slots[element.data[1]] = i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXO:
      target_slots[element.data[2]] = i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOOX:
      target_slots[element.data[3]] = i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXO:
      target_slots[element.data[2]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOOX:
      target_slots[element.data[3]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXO:
      target_slots[element.data[1]] = i * 4;
      target_slots[element.data[2]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOX:
      target_slots[element.data[1]] = i * 4;
      target_slots[element.data[3]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXX:
      target_slots[element.data[2]] = i * 4;
      target_slots[element.data[3]] = 1 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOX:
      target_slots[element.data[3]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXX:
      target_slots[element.data[2]] = 1 + i * 4;
      target_slots[element.data[3]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXX:
      target_slots[element.data[1]] = 0 + i * 4;
      target_slots[element.data[2]] = 1 + i * 4;
      target_slots[element.data[3]] = 2 + i * 4;
      should_add_new_piece = true;
      break;

    default:
      break;
    }
  }
}

void s2048::pieces::merge_left(pieces_data &pd, piece_id_queue_t &stale_pieces,
                               bool &should_add_new_piece, surge::u32 &game_score) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::merge_left");
#endif

  log_debug("Merging left");

  surge::u16 round_points{0};

  auto &target_slots{pd.target_slots};
  auto &values{pd.current_values};
  auto &target_values{pd.target_values};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::row, i)};

    switch (element.config) {

    case board_element_configuration::XXOO:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i * 4;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXO:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i * 4;
        target_slots[element.data[2]] = 1 + i * 4;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[2]] = 1 + i * 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXX:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i * 4;
        target_slots[element.data[2]] = 1 + i * 4;
        target_slots[element.data[3]] = 2 + i * 4;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[2]] = 1 + i * 4;
        target_slots[element.data[3]] = 2 + i * 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      } else if (values[element.data[2]] == values[element.data[3]]) {
        target_slots[element.data[3]] = 2 + i * 4;
        target_values[element.data[3]] *= 2;
        round_points += target_values[element.data[3]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      }
      break;

    default:
      break;
    }
  }

  game_score += round_points;
}

void s2048::pieces::compress_up(pieces_data &pd, bool &should_add_new_piece) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::compress_up");
#endif

  log_debug("Compressing up");

  auto &target_slots{pd.target_slots};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::column, i)};

    switch (element.config) {

    case board_element_configuration::OXOO:
      target_slots[element.data[1]] = i;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXO:
      target_slots[element.data[2]] = i;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOOX:
      target_slots[element.data[3]] = i;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXO:
      target_slots[element.data[2]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOOX:
      target_slots[element.data[3]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXO:
      target_slots[element.data[1]] = i;
      target_slots[element.data[2]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOX:
      target_slots[element.data[1]] = i;
      target_slots[element.data[3]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXX:
      target_slots[element.data[2]] = i;
      target_slots[element.data[3]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOX:
      target_slots[element.data[3]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXX:
      target_slots[element.data[2]] = i + 4;
      target_slots[element.data[3]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXX:
      target_slots[element.data[1]] = i;
      target_slots[element.data[2]] = i + 4;
      target_slots[element.data[3]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    default:
      break;
    }
  }
}

void s2048::pieces::merge_up(pieces_data &pd, piece_id_queue_t &stale_pieces,
                             bool &should_add_new_piece, surge::u32 &game_score) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::merge_up");
#endif

  log_debug("Merging up");

  surge::u16 round_points{0};

  auto &target_slots{pd.target_slots};
  auto &values{pd.current_values};
  auto &target_values{pd.target_values};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::column, i)};

    switch (element.config) {

    case board_element_configuration::XXOO:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXO:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i;
        target_slots[element.data[2]] = i + 4;
        target_values[element.data[1]] += 1;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[2]] = i + 4;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXX:
      if (values[element.data[0]] == values[element.data[1]]) {
        target_slots[element.data[1]] = i;
        target_slots[element.data[2]] = i + 4;
        target_slots[element.data[3]] = i + 4 * 2;
        target_values[element.data[1]] *= 2;
        round_points += target_values[element.data[1]];
        mark_stale(stale_pieces, element.data[0]);
        should_add_new_piece = true;
      } else if (values[element.data[1]] == values[element.data[2]]) {
        target_slots[element.data[2]] = i + 4;
        target_slots[element.data[3]] = i + 4 * 2;
        target_values[element.data[2]] *= 2;
        round_points += target_values[element.data[2]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      } else if (values[element.data[2]] == values[element.data[3]]) {
        target_slots[element.data[3]] = i + 4 * 2;
        target_values[element.data[3]] *= 2;
        round_points += target_values[element.data[3]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      }
      break;

    default:
      break;
    }
  }

  game_score += round_points;
}

void s2048::pieces::compress_down(pieces_data &pd, bool &should_add_new_piece) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::compress_down");
#endif

  log_debug("Compressing down");

  auto &target_slots{pd.target_slots};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::column, i)};

    switch (element.config) {

    case board_element_configuration::XOOO:
      target_slots[element.data[0]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOO:
      target_slots[element.data[1]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OOXO:
      target_slots[element.data[2]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXOX:
      target_slots[element.data[1]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOOX:
      target_slots[element.data[0]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    case board_element_configuration::OXXO:
      target_slots[element.data[1]] = i + 4 * 2;
      target_slots[element.data[2]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOO:
      target_slots[element.data[0]] = i + 4 * 2;
      target_slots[element.data[1]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXO:
      target_slots[element.data[0]] = i + 4 * 2;
      target_slots[element.data[2]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XOXX:
      target_slots[element.data[0]] = i + 4;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXOX:
      target_slots[element.data[0]] = i + 4;
      target_slots[element.data[1]] = i + 4 * 2;
      should_add_new_piece = true;
      break;

    case board_element_configuration::XXXO:
      target_slots[element.data[0]] = i + 4;
      target_slots[element.data[1]] = i + 4 * 2;
      target_slots[element.data[2]] = i + 4 * 3;
      should_add_new_piece = true;
      break;

    default:
      break;
    }
  }
}

void s2048::pieces::merge_down(pieces_data &pd, piece_id_queue_t &stale_pieces,
                               bool &should_add_new_piece, surge::u32 &game_score) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("mod_2048::pieces::merge_down");
#endif

  log_debug("Merging down");

  surge::u16 round_points{0};

  auto &target_slots{pd.target_slots};
  auto &exponents{pd.current_values};
  auto &target_exponents{pd.target_values};

  for (surge::u8 i = 0; i < 4; i++) {
    const auto element{get_element(pd, board_element_type::column, i)};

    switch (element.config) {

    case board_element_configuration::OOXX:
      if (exponents[element.data[2]] == exponents[element.data[3]]) {
        target_slots[element.data[2]] = i + 4 * 3;
        target_exponents[element.data[2]] *= 2;
        round_points += target_exponents[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::OXXX:
      if (exponents[element.data[2]] == exponents[element.data[3]]) {
        target_slots[element.data[1]] = i + 4 * 2;
        target_slots[element.data[2]] = i + 4 * 3;
        target_exponents[element.data[2]] *= 2;
        round_points += target_exponents[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      } else if (exponents[element.data[1]] == exponents[element.data[2]]) {
        target_slots[element.data[1]] = i + 4 * 2;
        target_exponents[element.data[1]] *= 2;
        round_points += target_exponents[element.data[1]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      }
      break;

    case board_element_configuration::XXXX:
      if (exponents[element.data[2]] == exponents[element.data[3]]) {
        target_slots[element.data[0]] = i + 4;
        target_slots[element.data[1]] = i + 4 * 2;
        target_slots[element.data[2]] = i + 4 * 3;
        target_exponents[element.data[2]] *= 2;
        round_points += target_exponents[element.data[2]];
        mark_stale(stale_pieces, element.data[3]);
        should_add_new_piece = true;
      } else if (exponents[element.data[1]] == exponents[element.data[2]]) {
        target_slots[element.data[0]] = i + 4;
        target_slots[element.data[1]] = i + 4 * 2;
        target_exponents[element.data[1]] *= 2;
        round_points += target_exponents[element.data[1]];
        mark_stale(stale_pieces, element.data[2]);
        should_add_new_piece = true;
      } else if (exponents[element.data[0]] == exponents[element.data[1]]) {
        target_slots[element.data[0]] = i + 4;
        target_exponents[element.data[0]] *= 2;
        round_points += target_exponents[element.data[0]];
        mark_stale(stale_pieces, element.data[1]);
        should_add_new_piece = true;
      }
      break;

    default:
      break;
    }
  }

  game_score += round_points;
}

void s2048::pieces::mark_stale(piece_id_queue_t &stale_pieces, surge::u8 piece) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::mark_stale");
#endif
  stale_pieces.push_back(piece);
}

void s2048::pieces::remove_stale(piece_id_queue_t &stale_pieces, pieces_data &pd) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::remove_stale");
#endif
  log_debug("Removing stale pieces");

  while (stale_pieces.size() != 0) {
    delete_piece(pd, stale_pieces.front());
    stale_pieces.pop_front();
  }
}

auto s2048::pieces::value_to_texture_handle(const tdb_t &tdb, surge::u16 value) noexcept
    -> GLuint64 {
  static const auto handle_2{tdb.find("resources/pieces_2.png").value_or(0)};
  static const auto handle_4{tdb.find("resources/pieces_4.png").value_or(0)};
  static const auto handle_8{tdb.find("resources/pieces_8.png").value_or(0)};
  static const auto handle_16{tdb.find("resources/pieces_16.png").value_or(0)};
  static const auto handle_32{tdb.find("resources/pieces_32.png").value_or(0)};
  static const auto handle_64{tdb.find("resources/pieces_64.png").value_or(0)};
  static const auto handle_128{tdb.find("resources/pieces_128.png").value_or(0)};
  static const auto handle_256{tdb.find("resources/pieces_256.png").value_or(0)};
  static const auto handle_512{tdb.find("resources/pieces_512.png").value_or(0)};
  static const auto handle_1024{tdb.find("resources/pieces_1024.png").value_or(0)};
  static const auto handle_2048{tdb.find("resources/pieces_2048.png").value_or(0)};

  switch (value) {
  case 2:
    return handle_2;
  case 4:
    return handle_4;
  case 8:
    return handle_8;
  case 16:
    return handle_16;
  case 32:
    return handle_32;
  case 64:
    return handle_64;
  case 128:
    return handle_128;
  case 256:
    return handle_256;
  case 512:
    return handle_512;
  case 1024:
    return handle_1024;
  case 2048:
    return handle_2048;
  default:
    return 0;
  }
}

void s2048::pieces::add_sprites_to_database(const tdb_t &tdb, sdb_t &sdb,
                                            pieces_data &pd) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::add_sprites_to_database()");
#endif
  using namespace surge;
  using namespace surge::atom;

  for (const auto &pos_elm : pd.positions) {
    const auto [id, pos] = pos_elm;
    const auto handle{value_to_texture_handle(tdb, pd.current_values[id])};
    const auto model{sprite::place(pos, glm::vec2{105.0f}, 0.2f)};
    sdb.add(handle, model, 1.0f);
  }
}

void s2048::pieces::update_positions(pieces_data &pd) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::update_positions");
#endif

  using std::abs, std::sqrt;

  // These values must be fine tuned together
  const float v{(globals::slot_delta * 0.125f)}; // Divide by dt
  constexpr const float threshold{2.5f};

  auto &positions{pd.positions};

  auto &slots{pd.current_slots};
  auto &target_slots{pd.target_slots};

  for (const auto &s : slots) {
    auto piece_id{s.first};

    auto src_slot{s.second};
    auto tgt_slot{target_slots.at(piece_id)};

    // Move pieces
    if (tgt_slot != src_slot) {
      const auto curr_pos{positions.at(piece_id)};
      const auto tgt_slot_pos{globals::slot_coords[tgt_slot]};

      const auto delta_r{tgt_slot_pos - curr_pos};
      const auto delta_r_length{sqrt(glm::dot(delta_r, delta_r))};

      // Stopping condition
      if (abs(delta_r_length) < threshold) {
        slots.at(piece_id) = tgt_slot;
        positions.at(piece_id) = tgt_slot_pos;
      } else {
        const auto n_r{delta_r / delta_r_length};
        const auto r_next{curr_pos + v * n_r}; // Multiply by dt. dt cancels
        positions.at(piece_id) = r_next;
      }
    }
  }
}

void s2048::pieces::update_exponents(pieces_data &pd) noexcept {
#if defined(SURGE_BUILD_TYPE_Profile) && defined(SURGE_ENABLE_TRACY)
  ZoneScopedN("s2048::pieces::update_exponents");
#endif

  auto &values{pd.current_values};
  auto &target_values{pd.target_values};

  for (const auto &e : values) {
    auto piece_id{e.first};

    auto src_val{e.second};
    auto tgt_val{target_values[piece_id]};

    if (src_val != tgt_val) {
      values[piece_id] = tgt_val;
    }
  }
}