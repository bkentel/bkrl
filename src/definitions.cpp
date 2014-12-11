#include "definitions.hpp"

#include <boost/filesystem.hpp>

#include "json.hpp"

namespace jc = bkrl::json::common;

bkrl::data_definitions::data_definitions() {
    get_file_names();
    load_files();
}

void
bkrl::data_definitions::set_language(lang_id const lang) {
    messages_.set_locale(lang);
    entity_defs_.set_locale(lang);
    item_defs_.set_locale(lang);
}

bkrl::filetype
bkrl::data_definitions::get_file_type(json::cref value) {
    //TODO clean this up

    static auto const hash_config  = slash_hash32(jc::filetype_config);
    static auto const hash_locale  = slash_hash32(jc::filetype_locale);
    static auto const hash_texmap  = slash_hash32(jc::filetype_tilemap);
    static auto const hash_item    = slash_hash32(jc::filetype_item);
    static auto const hash_entitiy = slash_hash32(jc::filetype_entity);
    static auto const hash_keymap  = slash_hash32(jc::filetype_keymap);
    static auto const hash_loot    = slash_hash32(jc::filetype_loot);

    auto const type = jc::get_filetype(value);
    auto const hash = slash_hash32(type);

    if      (hash == hash_config)  { return filetype::config; }
    else if (hash == hash_locale)  { return filetype::locale; }
    else if (hash == hash_texmap)  { return filetype::texmap; }
    else if (hash == hash_item)    { return filetype::item; }
    else if (hash == hash_entitiy) { return filetype::entity; }
    else if (hash == hash_keymap)  { return filetype::keymap; }
    else if (hash == hash_loot)    { return filetype::loot_table; }

    return filetype::invalid;
}

bkrl::filetype
bkrl::data_definitions::get_strings_type(json::cref value) {
    static auto const hash_messages = slash_hash32(jc::filetype_messages);
    static auto const hash_items    = slash_hash32(jc::filetype_item);
    static auto const hash_entities = slash_hash32(jc::filetype_entity);

    auto const type = json::require_string(value[jc::field_stringtype]);
    auto const hash = slash_hash32(type);

    if      (hash == hash_messages) { return filetype::message; }
    else if (hash == hash_items)    { return filetype::item; }
    else if (hash == hash_entities) { return filetype::entity; }

    return filetype::invalid;
}

void
bkrl::data_definitions::load_locale(json::cref value) {
    auto const type = get_strings_type(value);

    if      (type == filetype::message) { messages_.load(value); }
    else if (type == filetype::entity)  { entity_defs_.load_locale(value); }
    else if (type == filetype::item)    { item_defs_.load_locale(value); }
    else                                { BK_TODO_FAIL(); }
}

void
bkrl::data_definitions::load_config(json::cref value) {
    config_ = bkrl::load_config(value);
}

void
bkrl::data_definitions::load_texmap(json::cref value) {
    tilemap_.load(value);
}

void
bkrl::data_definitions::load_item(json::cref value) {      
    item_defs_.load_definitions(value);
}

void
bkrl::data_definitions::load_entity(json::cref value) {
    entity_defs_.load_definitions(value);
}

void
bkrl::data_definitions::load_keymap(json::cref value) {
    keymap_ = keymap {value};
}

void
bkrl::data_definitions::load_loot_table(json::cref value) {
    loot_table_defs_.load_definitions(value);
}

void
bkrl::data_definitions::load_files() {
    for (auto const& filename : files_) {
        auto const value = jc::from_file(filename);
        auto const type  = get_file_type(value);

        switch (type) {
        case filetype::config     : load_config(value);     break;
        case filetype::locale     : load_locale(value);     break;
        case filetype::texmap     : load_texmap(value);     break;
        case filetype::item       : load_item(value);       break;
        case filetype::entity     : load_entity(value);     break;
        case filetype::keymap     : load_keymap(value);     break;
        case filetype::loot_table : load_loot_table(value); break;
        }
    }
}

void
bkrl::data_definitions::get_file_names() {
    namespace fs = boost::filesystem;

    auto const p = fs::current_path() / BK_PATH_LITERAL("data");

    if (!fs::exists(p)) {
        BK_TODO_FAIL();
    } else if (!fs::is_directory(p)) {
        BK_TODO_FAIL();
    }
    
    //copy matching file names to files_
    std::for_each(
        fs::recursive_directory_iterator {p}
        , fs::recursive_directory_iterator { }
        , [&](fs::directory_entry const& de) {
            static auto const def_ext = fs::path {BK_PATH_LITERAL(".def")};

            auto const& path = de.path();
            auto const& ext  = path.extension();
                    
            if (ext == def_ext) {
                files_.emplace_back(path.native());
            }
        }
    );
}
