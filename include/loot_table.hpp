//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "assert.hpp"
#include "identifier.hpp"
#include "random_forward.hpp"
#include "json_forward.hpp"
#include "string.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////

class loot_table_definitions;
class loot_table;

namespace detail { class loot_table_definitions_impl; }
namespace detail { class loot_table_parser_impl; }

//==============================================================================
//! data defining the result of a successful roll.
//==============================================================================
struct loot_rule_data_t {
    enum class id_t : uint8_t {
        item_ref  //!< the rule refers to an item definition.
      , table_ref //!< the rule refers to a loot table definition.
    };

    using string_t = std::array<char, 31>;

    hash_t   id;              //!< id of the resulting item or table.
    uint16_t count_lo;        //!< the lower bound on the quantity generated.
    uint16_t count_hi;        //!< the upper bound on the quantity generated.
    string_t id_debug_string; //!< snipped of the string used to generate the id hash.
    id_t     id_type;         //!< indicated either an item or table result.
};

//==============================================================================
//! collection of named loot tables.
//==============================================================================
class loot_table_definitions {
public:
    ~loot_table_definitions();

    loot_table_definitions();
    loot_table_definitions(loot_table_definitions&&);
    loot_table_definitions& operator=(loot_table_definitions&&);

    //--------------------------------------------------------------------------
    void load_definitions(json::cref data);

    //--------------------------------------------------------------------------
    loot_table const& operator[](loot_table_def_id id) const;
private:
    std::unique_ptr<detail::loot_table_definitions_impl> impl_;
};

//==============================================================================
//! the definition for a single loot table.
//==============================================================================
class loot_table {
public:
    enum class roll_t {
        roll_all   //!< all entries are rolled for success / failure.
      , choose_one //!< one weighted entry is chosen.
    };

    using defs_t = loot_table_definitions const&;
    using rule_t = loot_rule_data_t const&;

    using write_t = std::function<void (item_def_id, uint16_t)>;

    //--------------------------------------------------------------------------
    loot_table(loot_table&&)                 = default;
    loot_table& operator=(loot_table&&)      = default;
    loot_table(loot_table const&)            = delete;
    loot_table& operator=(loot_table const&) = delete;

    //--------------------------------------------------------------------------
    loot_table();

    //--------------------------------------------------------------------------
    template <typename Data, typename Rules>
    loot_table(
        string_ref const  id_string
      , roll_t     const  type
      , Data       const& data
      , Rules      const& rules
    )
      : id_string_(std::begin(id_string), std::end(id_string))
      , id_ {slash_hash32(id_string)}
      , type_ {type}
    {
        auto const size_data  = data.size();
        auto const size_rules = rules.size();

        BK_ASSERT_DBG(
            (type == roll_t::roll_all)   && (size_data == size_rules * 2)
         || (type == roll_t::choose_one) && (size_data == size_rules * 1)
        );

        append_to(data,  roll_data_);
        append_to(rules, rules_);

        tranform_data_();
    }

    //--------------------------------------------------------------------------
    void generate(random_t& gen, defs_t defs, write_t const& write) const;

    //--------------------------------------------------------------------------
    void set_id(string_ref const id_str, loot_table_def_id const id);

    //--------------------------------------------------------------------------
    loot_table_def_id id() const noexcept { return id_; }

    //--------------------------------------------------------------------------
    string_ref id_string() const noexcept { return id_string_; }

    //--------------------------------------------------------------------------
    roll_t type() const noexcept { return type_; }
private:
    //--------------------------------------------------------------------------
    void tranform_data_();

    //--------------------------------------------------------------------------
    void roll_all_(random_t& gen, defs_t defs, write_t const& write) const;
    
    //--------------------------------------------------------------------------
    void choose_one_(random_t& gen, defs_t defs, write_t const& write) const;

    //--------------------------------------------------------------------------
    void roll_one_(random_t& gen, defs_t defs, rule_t rule, write_t const& write) const;

    //--------------------------------------------------------------------------
    void roll_one_item_(random_t& gen, rule_t rule, write_t const& write) const;

    //--------------------------------------------------------------------------
    void roll_one_table_(random_t& gen, defs_t defs, rule_t rule, write_t const& write) const;

    //--------------------------------------------------------------------------
    std::vector<uint16_t>         roll_data_;
    std::vector<loot_rule_data_t> rules_;
    utf8string                    id_string_;
    loot_table_def_id             id_;
    roll_t                        type_;
};

//==============================================================================
//! json parser for loot tables.
//==============================================================================
class loot_table_parser {
public:
    ~loot_table_parser();
    loot_table_parser();

    loot_table parse(json::cref data);
private:
    std::unique_ptr<detail::loot_table_parser_impl> impl_;
};

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
