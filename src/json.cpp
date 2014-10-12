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

string_ref const common::filetype_locale {"LOCALE"};
string_ref const common::filetype_item   {"ITEM"};
