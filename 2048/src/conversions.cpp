#include "board.hpp"

auto s2048::board::val_to_texture_handle(const tdb_t &tdb, surge::u8 value) noexcept -> GLuint64 {
  static const auto piece_2_handle{tdb.find("resources/pieces_2.png").value_or(0)};
  static const auto piece_4_handle{tdb.find("resources/pieces_4.png").value_or(0)};
  static const auto piece_8_handle{tdb.find("resources/pieces_8.png").value_or(0)};
  static const auto piece_16_handle{tdb.find("resources/pieces_16.png").value_or(0)};
  static const auto piece_32_handle{tdb.find("resources/pieces_32.png").value_or(0)};
  static const auto piece_64_handle{tdb.find("resources/pieces_64.png").value_or(0)};
  static const auto piece_128_handle{tdb.find("resources/pieces_128.png").value_or(0)};
  static const auto piece_256_handle{tdb.find("resources/pieces_256.png").value_or(0)};
  static const auto piece_512_handle{tdb.find("resources/pieces_512.png").value_or(0)};
  static const auto piece_1024_handle{tdb.find("resources/pieces_1024.png").value_or(0)};
  static const auto piece_2048_handle{tdb.find("resources/pieces_2048.png").value_or(0)};

  switch (value) {
  case 2:
    return piece_2_handle;
  case 4:
    return piece_4_handle;
  case 8:
    return piece_8_handle;
  case 16:
    return piece_16_handle;
  case 32:
    return piece_32_handle;
  case 64:
    return piece_64_handle;
  case 128:
    return piece_128_handle;
  case 256:
    return piece_256_handle;
  case 512:
    return piece_512_handle;
  case 1024:
    return piece_1024_handle;
  case 2048:
    return piece_2048_handle;
  default:
    return 0;
  }
}

auto s2048::board::slot_to_coords(surge::u8 slot) noexcept -> glm::vec2 {
  switch (slot) {
  case 0:
    return glm::vec2{15.0f, 315.0f};
  case 1:
    return glm::vec2{136.0f, 315.0f};
  case 2:
    return glm::vec2{257.0f, 315.0f};
  case 3:
    return glm::vec2{378.0f, 315.0f};
  case 4:
    return glm::vec2{15.0f, 436.0f};
  case 5:
    return glm::vec2{136.0f, 436.0f};
  case 6:
    return glm::vec2{257.0f, 436.0f};
  case 7:
    return glm::vec2{378.0f, 436.0f};
  case 8:
    return glm::vec2{15.0f, 557.0f};
  case 9:
    return glm::vec2{136.0f, 557.0f};
  case 10:
    return glm::vec2{257.0f, 557.0f};
  case 11:
    return glm::vec2{378.0f, 557.0f};
  case 12:
    return glm::vec2{15.0f, 678.0f};
  case 13:
    return glm::vec2{136.0f, 678.0f};
  case 14:
    return glm::vec2{257.0f, 678.0f};
  case 15:
    return glm::vec2{378.0f, 678.0f};
  default:
    return glm::vec2{0.0f};
  }
}