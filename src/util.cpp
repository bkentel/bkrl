#include "util.hpp"

#include <fstream>

bkrl::uint64_t bkrl::slash_hash64(char const* s, size_t const len) {
    union {
        uint64_t h;
        uint8_t  u[8];
    };

    h = len ? len : ::strlen(s);

    for (size_t i = 0u; (i < len) && *s; ++i) {
        auto const shift = (h / (i + 1)) % 5;

        u[i % 8] += static_cast<uint8_t>(*s + i + (*s >> shift));
        s++;
    }
    
    return h; //64-bit
}

bkrl::uint32_t bkrl::slash_hash32(char const* s, size_t const len) {
    auto const h = slash_hash64(s, len);
    return static_cast<uint32_t>(h + (h >> 32)); //32-bit
}

std::string bkrl::read_file(std::string const& filename) {
    std::string result;

    std::ifstream in {filename};

    in.seekg(0, std::ios::end);   
    result.reserve(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::ios::beg);

    result.assign(
        std::istreambuf_iterator<char>{in}
      , std::istreambuf_iterator<char>{}
    );

    return result;
}