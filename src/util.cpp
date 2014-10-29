#include "util.hpp"
#include "types.hpp"
#include "hash.hpp"

#include <cstring>
#include <cstdio>

#include <utf8.h>

using namespace bkrl;

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

namespace {

struct file_deleter {
    void operator()(FILE* const ptr) const noexcept {
        auto const result = std::fclose(ptr);
        if (result == EOF) {
            BK_TODO_FAIL();
        }
    }
};

using unique_file = std::unique_ptr<FILE, file_deleter>;

#if BOOST_COMP_MSVC
unique_file open_file(path_string_ref const filename) {
    FILE* ptr = nullptr;

    auto const result = _wfopen_s(&ptr, filename.data(), L"rb");
    if (result) {
        BK_TODO_FAIL();
    }

    return unique_file {ptr};
}
#elif !BOOST_COMP_MSVC && BOOST_OS_WINDOWS
unique_file open_file(path_string_ref const filename) {
    utf8string narrow;
    narrow.reserve(filename.size());

    utf8::utf16to8(std::cbegin(filename), std::cend(filename), std::back_inserter(narrow));

    auto const result = std::fopen(narrow.data(), "rb");
    if (result == nullptr) {
        BK_TODO_FAIL();
    }

    return unique_file {result};
}
#endif

} //namespace

utf8string bkrl::read_file(path_string_ref const filename) {
    constexpr auto max_size = (1 << 20); //1 MiB

    auto const file   = open_file(filename);
    auto const handle = file.get();

    if (std::fseek(handle, 0, SEEK_END)) {
        BK_TODO_FAIL();
    }

    auto const pos = std::ftell(handle);
    if (pos == EOF) {
        BK_TODO_FAIL();
    }

    if (std::fseek(handle, 0, SEEK_SET)) {
        BK_TODO_FAIL();
    }

    auto const size = static_cast<size_t>(pos);
    if (size > max_size) {
        BK_TODO_FAIL();
    }

    //HACK TODO safe?
    auto result = utf8string(size, 0);
    auto const read = std::fread(const_cast<char*>(result.data()), 1, size, handle);

    return result;
}
