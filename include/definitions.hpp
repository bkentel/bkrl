#pragma once

#include "types.hpp"

#include "config.hpp"
#include "item.hpp"
#include "entity.hpp"
#include "messages.hpp"
#include "tile_sheet.hpp"
#include "keyboard.hpp"

#include "json.hpp"

#include <boost/filesystem.hpp>

namespace bkrl {

using lang_id = bkrl::tagged_type<uint32_t, struct lang_id_tag>;

#define BK_MAKE_LANG_CODE3(a, b, c) bkrl::lang_id { \
  static_cast<uint32_t>((a & 0xFF) << 16) \
| static_cast<uint32_t>((b & 0xFF) <<  8) \
| static_cast<uint32_t>((c & 0xFF) <<  0) \
}

#define BK_MAKE_LANG_CODE2(a, b) BK_MAKE_LANG_CODE3(0, a, b)

class data_definitions {
public:
    enum class filetype {
        invalid, config, locale, item, message, texmap, entity, keymap
    };
    
    static lang_id get_language(json::cref value) {
        auto const lang_string = json::require_string(value[json::common::field_language]);
        if (lang_string.length() != 2) {
            BK_TODO_FAIL();
        }

        return BK_MAKE_LANG_CODE2(lang_string[0], lang_string[1]);
    }

    BK_NOCOPY(data_definitions);
    
    data_definitions()
    {
        get_file_names();
        load_files();
    }

    template <typename T>
    static auto get_current(T& map, lang_id const lang) {
        //using result_t = decltype(&(std::begin(map)->second));

        if (map.size() == 0) {
            BK_TODO_FAIL();
        }

        auto const it = map.find(lang);
        if (it == std::end(map)) { //TODO add warning
            return &(std::begin(map)->second);
        }

        return &(it->second);
    }

    void set_language(lang_id const lang) {
        messages_current_loc_ = get_current(messages_, lang);

        entity_defs_.set_locale(lang);
        item_defs_.set_locale(lang);
    }

    filetype get_file_type(json::cref value) {
        static auto const hash_config  = slash_hash32(json::common::filetype_config);
        static auto const hash_locale  = slash_hash32(json::common::filetype_locale);
        static auto const hash_texmap  = slash_hash32(json::common::filetype_tilemap);
        static auto const hash_item    = slash_hash32(json::common::filetype_item);
        static auto const hash_entitiy = slash_hash32(json::common::filetype_entity);
        static auto const hash_keymap  = slash_hash32(json::common::filetype_keymap);

        auto const type = json::common::get_filetype(value);
        auto const hash = slash_hash32(type);

        if      (hash == hash_config)  { return filetype::config; }
        else if (hash == hash_locale)  { return filetype::locale; }
        else if (hash == hash_texmap)  { return filetype::texmap; }
        else if (hash == hash_item)    { return filetype::item; }
        else if (hash == hash_entitiy) { return filetype::entity; }
        else if (hash == hash_keymap)  { return filetype::keymap; }

        return filetype::invalid;
    }

    filetype get_strings_type(json::cref value) {
        static auto const hash_messages = slash_hash32(json::common::stringtype_messages);
        static auto const hash_items    = slash_hash32(json::common::filetype_item);
        static auto const hash_entities = slash_hash32(json::common::filetype_entity);

        auto const type = json::require_string(value[json::common::field_stringtype]);
        auto const hash = slash_hash32(type);

        if      (hash == hash_messages) { return filetype::message; }
        else if (hash == hash_items)    { return filetype::item; }
        else if (hash == hash_entities) { return filetype::entity; }

        return filetype::invalid;
    }

    void load_locale(json::cref value) {
        auto const type = get_strings_type(value);
        auto const lang = get_language(value);

        if (type == filetype::message) {
            auto it = messages_.find(lang);
            if (it == std::end(messages_)) {
                messages_.emplace(lang, message_map {value});
            } else {
                it->second.reload(value);
            }
        } else if (type == filetype::entity) {
            entity_defs_.load_locale(value); //TODO reload
        } else if (type == filetype::item) {
            item_defs_.load_locale(value); //TODO reload
        }
    }

    void load_config(json::cref value) {
        config_ = bkrl::load_config(value);
    }

    void load_texmap(json::cref value) {
        tilemap_ = bkrl::load_tilemap(value);
    }

    void load_item(json::cref value) {      
        item_defs_.load_definitions(value);
    }

    void load_entity(json::cref value) {
        entity_defs_.load_definitions(value);
    }

    void load_keymap(json::cref value) {
        keymap_ = keymap {value};
    }

    void load_files() {
        for (auto const& filename : files_) {
            auto const value = json::common::from_file(filename);
            auto const type = get_file_type(value);

            switch (type) {
            case filetype::config  : load_config(value);  break;
            case filetype::locale  : load_locale(value);  break;
            case filetype::texmap  : load_texmap(value);  break;
            case filetype::item    : load_item(value);    break;
            case filetype::entity  : load_entity(value);  break;
            case filetype::keymap  : load_keymap(value);  break;
            }
        }
    }

    void get_file_names() {
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

    config  const& get_config()  const { return config_; }
    keymap  const& get_keymap()  const { return keymap_; }
    tilemap const& get_tilemap() const { return tilemap_; }

    message_map const& get_messages() const { return *messages_current_loc_; }

    item_definitions   const& get_items()    const { return item_defs_; }
    entity_definitions const& get_entities() const { return entity_defs_; }
private:
    lang_id language_ = BK_MAKE_LANG_CODE2('e','n');

    std::vector<path_string> files_;

    template <typename T>
    using map_t = boost::container::flat_map<lang_id, T>;

    config  config_;
    keymap  keymap_;
    tilemap tilemap_;

    map_t<message_map> messages_;
    message_map*       messages_current_loc_ = nullptr;

    item_definitions   item_defs_;
    entity_definitions entity_defs_;
};


} //namespace bkrl
