#ifndef SURGE_MODULE_2048_PIECES
#define SURGE_MODULE_2048_PIECES

#include "player/container_types.hpp"
#include "type_aliases.hpp"

#include <array>
#include <glm/matrix.hpp>

namespace s2048::pieces {

using piece_id_queue_t = surge::deque<surge::u8>;
using piece_positions_t = surge::hash_map<surge::u8, glm::vec2>;
using piece_values_t = surge::hash_map<surge::u8, surge::u16>;
using piece_slots_t = surge::hash_map<surge::u8, surge::u8>;

struct pieces_data {
  piece_id_queue_t ids{};

  piece_positions_t positions{};

  piece_values_t current_values{};
  piece_values_t target_values{};

  piece_slots_t current_slots{};
  piece_slots_t target_slots{};
};

enum board_element_type : surge::u8 { row, column };

enum board_element_configuration : surge::u8 {
  // 0 pieces
  OOOO,

  // 1 Piece combos
  XOOO,
  OXOO,
  OOXO,
  OOOX,

  // 2 Piece combos
  XXOO,
  XOXO,
  XOOX,
  OXXO,
  OXOX,
  OOXX,

  // 3 Piece combos
  XXXO,
  XXOX,
  XOXX,
  OXXX,

  // 4 Piece combos
  XXXX
};

struct board_element {
  std::array<surge::u8, 4> data{};
  surge::u8 size{};
  board_element_configuration config{};
};

struct board_address {
  surge::u8 row{};
  surge::u8 col{};
};

auto create_piece(pieces_data &pd, surge::u16 value, surge::u8 slot) noexcept -> surge::u8;
void delete_piece(pieces_data &pd, surge::u8 piece_id) noexcept;

auto create_random(pieces_data &pd) noexcept -> surge::u8;

auto idle(const pieces_data &pd) noexcept -> bool;

auto deflatten_slot(surge::u8 slot) noexcept -> board_address;
auto get_element(const pieces_data &pd, board_element_type type, surge::u16 value) noexcept
    -> board_element;

void compress_right(pieces_data &pd, bool &should_add_new_piece) noexcept;
void merge_right(pieces_data &pd, piece_id_queue_t &stale_pieces, bool &should_add_new_piece,
                 surge::u32 &game_score) noexcept;

void compress_left(pieces_data &pd, bool &should_add_new_piece) noexcept;
void merge_left(pieces_data &pd, piece_id_queue_t &stale_pieces, bool &should_add_new_piece,
                surge::u32 &game_score) noexcept;

void compress_up(pieces_data &pd, bool &should_add_new_piece) noexcept;
void merge_up(pieces_data &pd, piece_id_queue_t &stale_pieces, bool &should_add_new_piece,
              surge::u32 &game_score) noexcept;

void compress_down(pieces_data &pd, bool &should_add_new_piece) noexcept;
void merge_down(pieces_data &pd, piece_id_queue_t &stale_pieces, bool &should_add_new_piece,
                surge::u32 &game_score) noexcept;

void mark_stale(piece_id_queue_t &stale_pieces, surge::u8 piece) noexcept;
void remove_stale(piece_id_queue_t &stale_pieces, pieces_data &pd) noexcept;

auto value_to_texture_handle(const tdb_t &tdb, surge::u16 value) noexcept -> GLuint64;
void add_sprites_to_database(const tdb_t &tdb, sdb_t &sdb, pieces_data &pd) noexcept;

void update_positions(pieces_data &pd) noexcept;
void update_exponents(pieces_data &pd) noexcept;

} // namespace s2048::pieces

#endif // SURGE_MODULE_2048_PIECES