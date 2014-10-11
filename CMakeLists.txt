cmake_minimum_required(VERSION 2.8.4)
project(bkrl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

set(SOURCE_FILES
    include/detail/bsp_layout.i.hpp
    include/detail/freetype_text.i.hpp
    include/detail/sdl_application.i.hpp
    include/macros.hpp
    include/algorithm.hpp
    include/assert.hpp
    include/bsp_layout.hpp
    include/command_type.hpp
    include/config.hpp
    include/engine_client.hpp
    include/enum_map.hpp
    include/exception.hpp
    include/font.hpp
    include/generate.hpp
    include/grid.hpp
    include/item.hpp
    include/json.hpp
    include/keyboard.hpp
    include/math.hpp
    include/pch.hpp
    include/player.hpp
    include/random.hpp
    include/renderer.hpp
    include/scancode.hpp
    include/texture_type.hpp
    include/tile_sheet.hpp
    include/tile_type.hpp
    include/types.hpp
    include/util.hpp
    include/iterable.hpp
    include/messages.hpp
    include/scope_exit.hpp
    include/spatial_map.hpp
    include/time.hpp
    lib/catch/catch.hpp
    lib/json11/json11.cpp
    lib/json11/json11.hpp
    lib/json11/LICENSE.txt
    lib/json11/Makefile
    lib/json11/README.md
#    lib/json11/test.cpp
    lib/utf8/checked.h
    lib/utf8/core.h
    lib/utf8/unchecked.h
    lib/utf8.h
    src/assert.cpp
    src/bsp_layout.cpp
    src/config.cpp
    src/engine_client.cpp
    src/enum_map.cpp
    src/font.cpp
    src/generate.cpp
    src/item.cpp
    src/json.cpp
    src/keyboard.cpp
    src/main.cpp
    src/pch.cpp
    src/renderer.cpp
    src/tile_sheet.cpp
    src/util.cpp
    src/math.cpp
    src/messages.cpp
    src/time.cpp
#    test/algorithm.t.cpp
#    test/bsp_layout.t.cpp
#    test/engine_client.t.cpp
#    test/main.t.cpp
#    test/math.t.cpp
)

include_directories(include)
include_directories(lib)

include_directories(SYSYEM D:/lib/include/)
include_directories(SYSYEM D:/lib/include/freetype/)

# set the path to the library folder
link_directories(D:/lib/lib/x86/)

# add_library(sdl2 SHARED IMPORTED)

add_executable(bkrl ${SOURCE_FILES})

# link the libraries to the executable
target_link_libraries (bkrl sdl2_gcc_debug)
target_link_libraries (bkrl freetype)
