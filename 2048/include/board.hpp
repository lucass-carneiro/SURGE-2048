#ifndef SURGE_2048_BOARD_HPP
#define SURGE_2048_BOARD_HPP

#include "player/container_types.hpp"
#include "player/error_types.hpp"
#include "type_aliases.hpp"

#include <optional>

namespace s2048::board {

struct pieces {
  surge::vector<surge::u8> current_slots{};
  surge::vector<surge::u8> target_slots{};
  surge::vector<GLuint64> curr_val_handle{};
  surge::vector<glm::vec2> current_positions{};
};

auto load(GLFWwindow *window, tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error>;
auto unload(tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error>;

void update(double dt, GLFWwindow *window, const tdb_t &tdb, sdb_t &sdb, txd_t &txd) noexcept;
void draw(sdb_t &sdb, txd_t &txd) noexcept;

auto val_to_texture_handle(const tdb_t &tdb, surge::u8 value) noexcept -> GLuint64;
auto slot_to_coords(surge::u8 slot) noexcept -> glm::vec2;

void update_pieces(double dt, sdb_t &sdb) noexcept;

auto add_piece(const tdb_t &tdb, surge::u8 slot, surge::u8 value) noexcept -> bool;
void add_random_piece(const tdb_t &tdb) noexcept;
void reset_board() noexcept;

} // namespace s2048::board

#endif // SURGE_2048_BOARD_HPP