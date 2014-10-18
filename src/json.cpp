#include "json.hpp"

#if BOOST_OS_WINDOWS
#   include <utf8.h>
#endif

using namespace bkrl;

namespace common = bkrl::json::common;

utf8string const common::field_filetype    {"file_type"};
utf8string const common::field_stringtype  {"string_type"};
utf8string const common::field_language    {"language"};
utf8string const common::field_definitions {"definitions"};
utf8string const common::field_id          {"id"};
utf8string const common::field_name        {"name"};
utf8string const common::field_text        {"text"};
utf8string const common::field_sort        {"sort"};
utf8string const common::field_mappings    {"mappings"};
utf8string const common::field_filename    {"file_name"};
utf8string const common::field_tile_size   {"tile_size"};

string_ref const common::filetype_config  {"CONFIG"};
string_ref const common::filetype_locale  {"LOCALE"};
string_ref const common::filetype_item    {"ITEM"};
string_ref const common::filetype_entity  {"ENTITY"};
string_ref const common::filetype_tilemap {"TILEMAP"};
string_ref const common::filetype_keymap  {"KEYMAP"};

string_ref const common::stringtype_messages  {"MESSAGE"};

#if BOOST_OS_WINDOWS
path_string common::get_filename(json::cref value) {
    auto const str = require_string(value[field_filename]);
    
    path_string result;
    result.reserve(str.length()); //TODO a bit wasteful

    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(result));

    return result;
}
#else
path_string common::get_filename(json::cref value) {
    auto const str = require_string(value[field_filename]);
    return str.to_string();
}
#endif
