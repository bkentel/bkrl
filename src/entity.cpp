#include "entity.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

////////////////////////////////////////////////////////////////////////////////
// item_definitions
////////////////////////////////////////////////////////////////////////////////
struct bkrl::entity_definition {
    using dist_t = random::random_dist;

    static path_string tile_filename;
    static tex_point_i tile_size;

    entity_def_id id;      //!< entity id
    utf8string    id_string;
    dist_t        items;   //!< number of carried items
    tex_point_i   tile_position = tex_point_i {0, 0};
    argb8         tile_color = argb8 {255, 255, 255, 255};
    dist_t        health;  //!< health
};

bkrl::path_string bkrl::entity_definition::tile_filename {};
bkrl::tex_point_i bkrl::entity_definition::tile_size     {0, 0};

//==============================================================================
//!
//==============================================================================
class bkrl::detail::entity_definitions_impl {
public:
    using definition = entity_definition;
    using locale     = entity_locale;
    using cref = bkrl::json::cref;  

    static utf8string const undefined_name;
    static utf8string const undefined_text;
    static locale     const undefined_locale;

    static tex_point_i player_tile;

    ////////////////////////////////////////////////////////////////////////////
    entity_definitions_impl() {
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_definitions(cref data) {
        rule_ent_root(data);
    }
    
    //--------------------------------------------------------------------------
    void rule_ent_root(cref value) {
        jc::get_filetype(value, jc::filetype_entity);

        rule_ent_tiles(value);
        rule_ent_player_tile(value);
        rule_ent_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_ent_tiles(cref value) {
        //TODO
        definition::tile_filename = jc::get_filename(value);
        definition::tile_size = jc::get_positive_int_pair<tex_coord_i>(
            value[jc::field_tile_size]
        );
    }

    //--------------------------------------------------------------------------
    void rule_ent_player_tile(cref value) {
        auto const w = entity_definition::tile_size.x;
        auto const h = entity_definition::tile_size.y;
        
        auto const p = jc::get_positive_int_pair<tex_coord_i>(value[jc::field_player]);
        player_tile.x = w * p.x;
        player_tile.y = h * p.y;
    }

    //--------------------------------------------------------------------------
    void rule_ent_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        definitions_.reserve(array.size());

        for (auto&& def : array) {
            rule_ent_definition(def);
        }
    }

    //--------------------------------------------------------------------------
    void rule_ent_definition(cref value) {
        rule_end_id(value);
        rule_ent_items(value);
        rule_ent_color(value);
        rule_ent_tile(value);
        rule_ent_health(value);

        definitions_.emplace(cur_def_.id, std::move(cur_def_));
    }

    void rule_end_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = entity_def_id {slash_hash32(str)};
        cur_def_.id_string = str.to_string();
    }

    //--------------------------------------------------------------------------
    void rule_ent_tile(cref value) {
        auto const w = entity_definition::tile_size.x;
        auto const h = entity_definition::tile_size.y;

        auto const p = jc::get_tile_index(value);
        
        cur_def_.tile_position.x = w * p.x;
        cur_def_.tile_position.y = h * p.y;
    }

    //--------------------------------------------------------------------------
    void rule_ent_color(cref value) {
        auto const color = json::require_array(value[jc::field_color], 3, 3);
        
        cur_def_.tile_color = make_color(
            json::require_int<uint8_t>(color[0])
          , json::require_int<uint8_t>(color[1])
          , json::require_int<uint8_t>(color[2])
          , 255
        );
    }

    //--------------------------------------------------------------------------
    void rule_ent_items(cref value) {
        cur_def_.items = jc::get_random(value[jc::field_items]);
    }

    //--------------------------------------------------------------------------
    void rule_ent_health(cref value) {
        cur_def_.health = jc::get_random(value[jc::field_health]);
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_locale(cref data) {
        rule_loc_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_loc_root(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_entity);
        if (!locale) {
            BK_TODO_FAIL();
        }
        
        cur_lang_ = *locale;

        rule_loc_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_loc_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        cur_loc_map_.reserve(array.size());

        for (auto&& def : array) {
            rule_loc_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_map_));
    }

    //--------------------------------------------------------------------------
    void rule_loc_definition(cref value) {
        auto const& str = json::require_string(value[jc::field_id]);
        auto const  id  = slash_hash32(str);

        assign(cur_loc_.name, json::default_string(value[jc::field_name], undefined_name));
        assign(cur_loc_.text, json::default_string(value[jc::field_text], undefined_text));

        cur_loc_map_.emplace(id, std::move(cur_loc_));
    }

    ////////////////////////////////////////////////////////////////////////////
    definition const& get_definition(entity_def_id const id) const {
        auto const it = definitions_.find(id);
        if (it == std::end(definitions_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
    
    locale const& get_locale(entity_def_id const id) const {
        auto const it = current_locale_->find(id);
        if (it == std::end(*current_locale_)) {
            return undefined_locale;
        }

        return it->second;
    }

    void set_locale(lang_id const lang) {
        auto const it = locales_.find(lang);
        if (it == std::end(locales_)) {
            BK_TODO_FAIL();
        }

        current_locale_ = &it->second;
    }

    auto get_definitions_size() const {
        return static_cast<int>(definitions_.size());
    }
    
    auto const& get_definition_at(int const index) const {
        BK_ASSERT(index < get_definitions_size());

        auto it = definitions_.begin();
        std::advance(it, index);
        return it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    using locale_map = map_t<entity_def_id, locale>;

    definition cur_def_;
    locale     cur_loc_;
    lang_id    cur_lang_;
    locale_map cur_loc_map_;

    map_t<entity_def_id, definition> definitions_;
    map_t<lang_id,       locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

bkrl::utf8string const bkrl::detail::entity_definitions_impl::undefined_name {"{undefined name}"};
bkrl::utf8string const bkrl::detail::entity_definitions_impl::undefined_text {"{undefined text}"};

bkrl::entity_locale const bkrl::detail::entity_definitions_impl::undefined_locale {
    undefined_name
  , undefined_text
};

bkrl::tex_point_i bkrl::detail::entity_definitions_impl::player_tile {0, 0};

////////////////////////////////////////////////////////////////////////////////
// entity
////////////////////////////////////////////////////////////////////////////////
bkrl::entity_render_info_t
bkrl::entity::render_info(entity_definitions const& defs) const {
    auto const& def = defs.get_definition(id);
    
    return entity_render_info_t {
        def.tile_position
      , def.tile_color
    };
}

bkrl::string_ref bkrl::entity::name(defs_t defs) const {
    return defs.get_locale(id).name;
}

bool bkrl::entity::apply_damage(health_t const delta) {
    auto& hp = data.health;
    return hp.modify(-delta), hp.is_min();
}

bkrl::damage_t bkrl::entity::get_attack_value(random_t& gen, defs_t defs) {
    return {1, damage_type::pierce}; //TODO
}

bkrl::defence_t bkrl::entity::get_defence_value(random_t& gen, defs_t defs, damage_type const type) {
    return {0, type}; //TODO
}

////////////////////////////////////////////////////////////////////////////////
// entity_definitions
////////////////////////////////////////////////////////////////////////////////
bkrl::path_string_ref bkrl::entity_definitions::tile_filename() {
    return entity_definition::tile_filename;
}

//------------------------------------------------------------------------------
bkrl::tex_point_i bkrl::entity_definitions::tile_size() {
    return entity_definition::tile_size;
}

//------------------------------------------------------------------------------
bkrl::tex_point_i bkrl::entity_definitions::player_tile() {
    return detail::entity_definitions_impl::player_tile;
}

//------------------------------------------------------------------------------
bkrl::entity_definitions::~entity_definitions() = default;

//------------------------------------------------------------------------------
bkrl::entity_definitions::entity_definitions()
  : impl_ {std::make_unique<detail::entity_definitions_impl>()}
{
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::load_locale(json::cref data) {
    impl_->load_locale(data);
}

//------------------------------------------------------------------------------
bkrl::entity_definition const&
bkrl::entity_definitions::get_definition(entity_def_id const id) const {
    return impl_->get_definition(id);
}

//------------------------------------------------------------------------------
bkrl::entity_locale const&
bkrl::entity_definitions::get_locale(entity_def_id const id) const {
    return impl_->get_locale(id);
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}

//------------------------------------------------------------------------------
int bkrl::entity_definitions::get_definitions_size() const {
    return impl_->get_definitions_size();
}

//------------------------------------------------------------------------------
bkrl::entity_definition const&
bkrl::entity_definitions::get_definition_at(int const index) const {
    return impl_->get_definition_at(index);
}

////////////////////////////////////////////////////////////////////////////////
// generate_entity
////////////////////////////////////////////////////////////////////////////////
bkrl::entity
bkrl::generate_entity(
    random::generator&        gen
  , entity_definitions const& entity_defs
  , item_definitions   const& item_defs
  , item_store&               items
  , spawn_table        const& table
) {
    //TODO not thread safe; can't be initialized (save / load)
    static auto next_instance_id = uint32_t {0x80000000};

    auto const size  = entity_defs.get_definitions_size();
    auto const index = random::uniform_range(gen, 0, size - 1);

    auto const& def = entity_defs.get_definition_at(index);
    auto const& id  = def.id;

    entity result;

    auto const max_health = static_cast<health_t>(def.health(gen));

    result.id = id;
    result.instance_id = entity_id {next_instance_id++};
    result.data.health = ranged_value<health_t> {max_health};
    result.data.position = {0, 0};

    auto const item_count = def.items(gen);

    if (item_count == 0) {
        return result;
    }

    item_birthplace origin;
    origin.type = item_birthplace::entity;
    origin.id   = id_to_value(id);

    auto const ltable = loot_table {};

    for (int i = 0; i < item_count; ++i) {
        result.data.items.insert(
            generate_item(gen, items, item_defs, ltable, origin)
          , item_defs
          , items
        );
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// player
////////////////////////////////////////////////////////////////////////////////
bkrl::entity_render_info_t
bkrl::player::render_info(entity_definitions const&) const {
    return entity_render_info_t {
        entity_definitions::player_tile()
        , argb8 {255, 255, 255, 255}
    };
}

bkrl::item_stack
bkrl::player::get_equippable(defs_t idefs, items_t istore) const {
    item_stack result;

    for (auto const iid : items()) {
        if (istore[iid].can_equip(idefs)) {
            result.insert(iid, idefs, istore);
        }
    }

    return result;
}

bkrl::equipment::result_t
bkrl::player::equip_item(item_id iid, defs_t defs, items_t istore) {
    auto const try_equip = equip_.equip(iid, defs, istore);
    if (!try_equip.second) {
        return try_equip;
    }

    this->items().remove(iid);

    return try_equip;
}


bkrl::equipment&
bkrl::player::equip() {
    return equip_;
}
bkrl::equipment const&
bkrl::player::equip() const {
    return equip_;
}

bkrl::damage_t
bkrl::player::get_attack_value(random_t& gen, items_t items) {
    auto const opt_weapon = [&]() -> optional<bkrl::item_id> {
        if (auto const main = equip_.in_slot(equip_slot::hand_main)) {
            return main;
        } else if (auto const off = equip_.in_slot(equip_slot::hand_off)) {
            return off;
        }

        return { };
    }();

    //TODO unarmed
    if (!opt_weapon) {
        return damage_t {1, damage_type::blunt};
    }

    auto const weapon_id = *opt_weapon;

    auto const& itm = items[weapon_id];
    auto const  dmg_min = itm.data.weapon.dmg_min; //TODO
    auto const  dmg_max = itm.data.weapon.dmg_max; //TODO
    auto const  dmg_type = itm.data.weapon.dmg_type; //TODO

    auto const dmg = random::uniform_range(gen, dmg_min, dmg_max);

    return damage_t {dmg, dmg_type};
}

bkrl::defence_t
bkrl::player::get_defence_value(random_t& gen, defs_t defs, damage_type type) {
    return defence_t {1, type}; // TODO
}
