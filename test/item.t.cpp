#include "catch/catch.hpp"
#include "item.hpp"

#include "json.hpp"

TEST_CASE("equipment", "[item]") {
    using namespace bkrl;

    utf8string const data {R"(
        { "file_type": "ITEM"
        , "definitions": [
            { "id": "TEST_ITEM"
            , "slot": ["hand_main", "hand_off"]
            }
          ]
        }
    )"};

    item_definitions item_defs;
    item_defs.load_definitions(json::common::from_memory(data));

    random::generator gen {100};

    equipment equip;
    auto original_item = bkrl::generate_item(gen, item_defs.get_definition_at(0));
    
    REQUIRE(equip.can_equip(original_item, item_defs) == true);
    equip.equip(std::move(original_item), item_defs);
    REQUIRE(equip.can_equip(original_item, item_defs) == false);

    REQUIRE(!!equip.in_slot(equip_slot::hand_main, item_defs));
    REQUIRE(!!equip.in_slot(equip_slot::hand_off,  item_defs));

    auto after_item = equip.unequip(equip_slot::hand_main, item_defs);
    REQUIRE(equip.can_equip(after_item, item_defs) == true);
    REQUIRE(!equip.in_slot(equip_slot::hand_main, item_defs));
    REQUIRE(!equip.in_slot(equip_slot::hand_off,  item_defs));
}
