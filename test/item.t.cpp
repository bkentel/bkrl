#include "catch/catch.hpp"
#include "items.hpp"
#include "json.hpp"
#include "random.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace {
////////////////////////////////////////////////////////////////////////////////
static bkrl::utf8string const test_item_defs
{R"(
    { "file_type": "ITEM"
    , "definitions": [
        { "id": "TEST_ITEM0"
        , "type": "weapon"
        , "slot": ["hand_main", "hand_off"]
        }
      , { "id": "TEST_ITEM1"
        , "type": "armor"
        , "slot": ["head"]
        }
      , { "id": "TEST_ITEM2"
        , "type": "potion"
        }
      ]
    }
)"};

static bkrl::utf8string const test_item_loc
{R"(
    { "file_type":   "LOCALE"
    , "string_type": "ITEM"
    , "language":    "en"
    , "definitions": [
        { "id":   "TEST_ITEM0"
        , "name": "test item 0"
        }
      , { "id":   "TEST_ITEM1"
        , "name": "test item 1"
        }
      , { "id":   "TEST_ITEM2"
        , "name": "test item 2"
        }
      ]
    }
)"};

static auto const item_id0 = bkrl::item_def_id {bkrl::slash_hash32("TEST_ITEM0")};
static auto const item_id1 = bkrl::item_def_id {bkrl::slash_hash32("TEST_ITEM1")};
static auto const item_id2 = bkrl::item_def_id {bkrl::slash_hash32("TEST_ITEM2")};

static bkrl::item_definitions make_defs() {
    bkrl::item_definitions result;
    result.load_definitions(bkrl::json::common::from_memory(test_item_defs));
    result.load_locale(bkrl::json::common::from_memory(test_item_loc));

    REQUIRE(result.get_definitions_size() == 3);

    return result;
}
////////////////////////////////////////////////////////////////////////////////
} //namespace
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("item localized names", "[item]") {
    using namespace bkrl;

    auto item_defs = make_defs();

    item_defs.set_locale(BK_MAKE_LANG_CODE2('e','n'));

    REQUIRE(item_defs.get_locale(item_id0).name == "test item 0");
    REQUIRE(item_defs.get_locale(item_id1).name == "test item 1");
    REQUIRE(item_defs.get_locale(item_id2).name == "test item 2");
}

TEST_CASE("equipment sanity check", "[item]") {
    using namespace bkrl;

    auto item_defs = make_defs();
    auto istore    = item_store {};
    auto equip     = equipment {};
    
    auto const id0 = generate_item(item_id0, istore, item_defs);
    auto const id1 = generate_item(item_id1, istore, item_defs);
    auto const id2 = generate_item(item_id2, istore, item_defs);

    REQUIRE(equip.can_equip(id0, item_defs, istore) == true);
    REQUIRE(equip.can_equip(id1, item_defs, istore) == true);
    REQUIRE(equip.can_equip(id2, item_defs, istore) == false);

    equip.equip(id0, item_defs, istore);
    REQUIRE(equip.in_slot(equip_slot::hand_main) == id0);
    REQUIRE(equip.in_slot(equip_slot::hand_off) == id0);

    REQUIRE(equip.can_equip(id0, item_defs, istore) == false);

    REQUIRE(equip.unequip(equip_slot::hand_main, item_defs, istore) == id0);
    REQUIRE(!equip.in_slot(equip_slot::hand_main));
    REQUIRE(!equip.in_slot(equip_slot::hand_off));
    REQUIRE(!equip.unequip(equip_slot::hand_main, item_defs, istore));
    REQUIRE(!equip.unequip(equip_slot::hand_off, item_defs, istore));

    REQUIRE(equip.can_equip(id0, item_defs, istore) == true);
    equip.equip(id0, item_defs, istore);
    REQUIRE(equip.unequip(equip_slot::hand_off, item_defs, istore) == id0);
    REQUIRE(!equip.in_slot(equip_slot::hand_main));
    REQUIRE(!equip.in_slot(equip_slot::hand_off));
}

