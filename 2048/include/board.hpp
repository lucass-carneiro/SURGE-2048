#ifndef SURGE_2048_BOARD_HPP
#define SURGE_2048_BOARD_HPP

#include "player/error_types.hpp"
#include "type_aliases.hpp"

#include <optional>

namespace s2048::board {

auto load(GLFWwindow *window, tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error>;
auto unload(tdb_t &tdb, sdb_t &sdb) noexcept -> std::optional<surge::error>;

void update(double dt, GLFWwindow *window, const tdb_t &tdb, sdb_t &sdb, txd_t &txd) noexcept;
void draw(sdb_t &sdb, txd_t &txd) noexcept;

} // namespace s2048::board

#endif // SURGE_2048_BOARD_HPP