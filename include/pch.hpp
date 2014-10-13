#pragma once

////////////////////////////////////////////////////////////////////////////////
// Standard Headers
////////////////////////////////////////////////////////////////////////////////
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cinttypes>
#include <cstring>

#include <limits>
#include <memory>
#include <type_traits>
#include <functional>
#include <utility>
#include <tuple>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <string>
#include <iterator>
#include <array>
#include <vector>
#include <bitset>
#include <fstream>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Boost
////////////////////////////////////////////////////////////////////////////////
#include <boost/predef.h>
#include <boost/exception/all.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>

////////////////////////////////////////////////////////////////////////////////
// SDL2
////////////////////////////////////////////////////////////////////////////////
#define SDL_MAIN_HANDLED
#include <sdl/SDL.h>

////////////////////////////////////////////////////////////////////////////////
// Json
////////////////////////////////////////////////////////////////////////////////
#include <json11/json11.hpp>

////////////////////////////////////////////////////////////////////////////////
// Freetype2
////////////////////////////////////////////////////////////////////////////////
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
