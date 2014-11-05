#pragma once

#include "types.hpp"
#include "util.hpp"

#include "config.hpp"
#include "items.hpp"
#include "entity.hpp"
#include "messages.hpp"
#include "tile_sheet.hpp"
#include "keyboard.hpp"

namespace bkrl {

//==============================================================================
//!
//==============================================================================
enum class filetype {
    invalid, config, locale, item, message, texmap, entity, keymap
};

//==============================================================================
//!
//==============================================================================
class data_definitions {
public:  
    BK_NOCOPY(data_definitions);
    data_definitions();

    void set_language(lang_id const lang);

    filetype get_file_type(json::cref value);

    filetype get_strings_type(json::cref value);

    void load_locale(json::cref value);

    void load_config(json::cref value);

    void load_texmap(json::cref value);

    void load_item(json::cref value);

    void load_entity(json::cref value);

    void load_keymap(json::cref value);

    void load_files();

    void get_file_names();

    config             const& get_config()   const { return config_; }
    keymap             const& get_keymap()   const { return keymap_; }
    tile_map           const& get_tilemap()  const { return tilemap_; }
    message_map        const& get_messages() const { return messages_; }
    item_definitions   const& get_items()    const { return item_defs_; }
    entity_definitions const& get_entities() const { return entity_defs_; }
private:
    std::vector<path_string> files_;

    config             config_;
    keymap             keymap_;
    tile_map           tilemap_;
    message_map        messages_;
    item_definitions   item_defs_;
    entity_definitions entity_defs_;
};

} //namespace bkrl
