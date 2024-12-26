cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# -----------------------------------------
# Project
# -----------------------------------------

project(
  Surge2048
  VERSION 1.0.0
  LANGUAGES CXX
)

# -----------------------------------------
#  Target sources
# -----------------------------------------

set(
  SURGE_MODULE_2048_HEADER_LIST
  "${PROJECT_SOURCE_DIR}/include/pieces.hpp"
  "${PROJECT_SOURCE_DIR}/include/type_aliases.hpp"
  "${PROJECT_SOURCE_DIR}/include/ui.hpp"
  "${PROJECT_SOURCE_DIR}/include/2048.hpp"
)

set(
  SURGE_MODULE_2048_SOURCE_LIST
  "${PROJECT_SOURCE_DIR}/src/pieces.cpp"
  "${PROJECT_SOURCE_DIR}/src/ui.cpp"
  "${PROJECT_SOURCE_DIR}/src/2048.cpp"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND SURGE_MODULE_2048_HEADER_LIST "${PROJECT_SOURCE_DIR}/include/debug_window.hpp")
  list(APPEND SURGE_MODULE_2048_SOURCE_LIST "${PROJECT_SOURCE_DIR}/src/debug_window.cpp")
endif()

# -----------------------------------------
# Module Target
# -----------------------------------------

add_library(Surge2048 MODULE ${SURGE_MODULE_2048_HEADER_LIST} ${SURGE_MODULE_2048_SOURCE_LIST})
target_compile_features(Surge2048 PRIVATE cxx_std_20)
set_target_properties(Surge2048 PROPERTIES PREFIX "")
set_target_properties(Surge2048 PROPERTIES OUTPUT_NAME "2048")
set_target_properties(Surge2048 PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/2048.hpp")
set_target_properties(Surge2048 PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_compile_definitions(Surge2048 PUBLIC COMPILING_SURGE_MODULE_2048)

target_include_directories(
  Surge2048 PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# Enables __VA_OPT__ on msvc
if(SURGE_COMPILER_FLAG_STYLE MATCHES "msvc")
    target_compile_options(Surge2048 PUBLIC /Zc:preprocessor)
endif()

# Use UTF-8 on MSVC
if(SURGE_COMPILER_FLAG_STYLE MATCHES "msvc")
    target_compile_options(Surge2048 PUBLIC /utf-8)
endif()

# Disable min/max macros on msvc
if(SURGE_COMPILER_FLAG_STYLE MATCHES "msvc")
    target_compile_options(Surge2048 PUBLIC /D NOMINMAX)
endif()

# -----------------------------------------
# Compilers flags and options
# -----------------------------------------

if(SURGE_ENABLE_SANITIZERS)
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -fsanitize=address,null,unreachable,undefined)
    target_link_options(Surge2048 PUBLIC -fsanitize=address,null,unreachable,undefined)
  else()
    message(WARNING "Sanitizers don't work on MSVC yet.")
  endif()
endif()

if(SURGE_ENABLE_OPTIMIZATIONS)
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -O2)
    target_link_options(Surge2048 PUBLIC -O2)
  else()
    target_compile_options(Surge2048 PUBLIC /O2)
  endif()
endif()

if(SURGE_ENABLE_TUNING)
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -march=native -mtune=native)
    target_link_options(Surge2048 PUBLIC -march=native -mtune=native)
  else()
    message(WARNING "TODO: Unknow tuning flags for msvc")
  endif()
endif()

if(SURGE_ENABLE_LTO)
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -flto)
    target_link_options(Surge2048 PUBLIC -flto)
  else()
    message(WARNING "TODO: Unknow LTO flag for msvc")    
  endif()
endif()

if(SURGE_ENABLE_FAST_MATH)
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -ffast-math)
    target_link_options(Surge2048 PUBLIC -ffast-math)
  else()
    target_compile_options(Surge2048 PUBLIC /fp:fast)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Generating a Debug build system")
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(
      Surge2048
      PUBLIC
      -Og
      -g3
      -ggdb3
      -fno-omit-frame-pointer
      -Werror
      -Wall
      -Wextra
      -Wpedantic
      -Walloca
      -Wcast-qual
      -Wformat=2
      -Wformat-security
      -Wnull-dereference
      -Wstack-protector
      -Wvla
      -Wconversion
      -Warray-bounds
      -Warray-bounds-pointer-arithmetic
      -Wconditional-uninitialized
      -Wimplicit-fallthrough
      -Wpointer-arith
      -Wformat-type-confusion
      -Wfloat-equal
      -Wassign-enum
      -Wtautological-constant-in-range-compare
      -Wswitch-enum
      -Wshift-sign-overflow
      -Wloop-analysis
      -Wno-switch-enum
    )
    target_link_options(Surge2048 PUBLIC -Og -g3 -ggdb3)
  else()
    target_compile_options(Surge2048 PUBLIC /Wall /MP /MDd)
    target_link_options(Surge2048 PUBLIC /DEBUG:FULL)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Surge2048 PUBLIC debuginfod)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "msvc")
  target_compile_options(Surge2048 PUBLIC /MP /MD)  
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Profile")
  if(SURGE_COMPILER_FLAG_STYLE MATCHES "gcc")
    target_compile_options(Surge2048 PUBLIC -g3 -ggdb3 -fno-omit-frame-pointer)
    target_link_options(Surge2048 PUBLIC -g3 -ggdb3 -fno-omit-frame-pointer -rdynamic)
  else()
    target_compile_options(Surge2048 PUBLIC /MP /MDd)
    target_link_options(Surge2048 PUBLIC /DEBUG:FULL)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Surge2048 PUBLIC debuginfod)
  endif()
endif()

# -----------------------------------------
# Link and build order dependencies
# -----------------------------------------

target_link_libraries(Surge2048 PUBLIC SurgeCore)