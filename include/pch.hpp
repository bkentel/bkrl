#pragma once

////////////////////////////////////////////////////////////////////////////////
// Standard Headers
////////////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <cinttypes>
#include <cassert>

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
#include <array>
#include <vector>

#include <bitset>

#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// Boost
////////////////////////////////////////////////////////////////////////////////
#include <boost/exception/all.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/string_ref.hpp>

////////////////////////////////////////////////////////////////////////////////
// SDL2
////////////////////////////////////////////////////////////////////////////////
#define SDL_MAIN_HANDLED
#include <sdl/SDL.h>

////////////////////////////////////////////////////////////////////////////////
// Json
////////////////////////////////////////////////////////////////////////////////
#include "json11/json11.hpp"

////////////////////////////////////////////////////////////////////////////////
// Freetype2
////////////////////////////////////////////////////////////////////////////////
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

////////////////////////////////////////////////////////////////////////////////
// Unit testing
////////////////////////////////////////////////////////////////////////////////
#if defined(BK_TEST)
#   include "catch/catch.hpp"
#endif
