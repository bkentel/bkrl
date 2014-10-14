#include "json.hpp"

using bkrl::utf8string;
using bkrl::string_ref;

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

string_ref const common::filetype_locale      {"LOCALE"};
string_ref const common::filetype_item        {"ITEM"};
string_ref const common::filetype_entity      {"ENTITY"};
string_ref const common::filetype_texture_map {"TEXTURE_MAP"};
string_ref const common::filetype_keymap      {"KEYMAP"};
