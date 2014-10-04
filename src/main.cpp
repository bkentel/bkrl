#include "engine_client.hpp"
#include "config.hpp"

namespace bkrl {

//struct cached_glyph {
//    codepoint_t codepoint;
//    uint32_t    index;
//
//    opaque_handle<texture> texture;
//
//    uint16_t tex_x;
//    uint16_t tex_y;
//
//    uint8_t width;
//    uint8_t height;
//    uint8_t left;
//    uint8_t top;
//
//    uint8_t advance;   
//    uint8_t reserved[3];
//};
//
//cached_glyph load_glyph(codepoint_t const cp) {
//    cached_glyph result {};
//    
//    result.codepoint = cp;
//    result.width  = 20;
//    result.height = 20;
//
//    return result;
//}
//
//class static_glyph_cache {
//public:
//    using key_t   = unicode::block_value;
//    using value_t = std::vector<cached_glyph>;
//
//    static value_t init_block(unicode::block_value const block) {
//        auto const beg = value_of(block.first);
//        auto const end = value_of(block.last);
//        auto const len = block.size();
//
//        value_t result;
//        result.reserve(len);
//
//        for (auto i = beg; i <= end; ++i) {
//            result.emplace_back(load_glyph(i));
//        }
//
//        return result;
//    }
//
//    static_glyph_cache() = default;
//
//    static_glyph_cache(std::initializer_list<unicode::block_value> blocks) {
//        map_.reserve(blocks.size());
//
//        for (auto const& block : blocks) {
//            map_.emplace(block, init_block(block));
//        }
//    }
//
//    void add_block(unicode::block_value const block) {
//        map_.emplace(block, init_block(block));
//    }
//
//    optional<cached_glyph const&> operator[](unicode::codepoint const cp) const {
//        auto const it = map_.find(unicode::block_value {cp});
//
//        if (it == std::cend(map_)) {
//            return {};
//        }
//
//        auto const& block  = it->first;
//        auto const& glyphs = it->second;
//
//        BK_ASSERT_DBG(block.contains(cp));
//
//        auto const i = block.offset(cp);
//        return glyphs[i];
//    }
//
//    ipoint2 update_texture_coords(int const w, int const h) {
//        using x_type = decltype(cached_glyph::tex_x);
//        using y_type = decltype(cached_glyph::tex_y);
//
//        static_assert(std::is_same<x_type, y_type>::value, "");
//        
//        using type = x_type;
//
//        static auto const max = std::numeric_limits<type>::max();
//
//        BK_ASSERT_SAFE(w > 0 && w <= max);
//        BK_ASSERT_SAFE(h > 0 && h <= max);
//
//        auto const tw = static_cast<type>(w);
//        auto const th = static_cast<type>(h);
//
//        type cur_x = 0;
//        type cur_y = 0;
//        type row_h = 0;
//
//        auto const update_glyph = [&cur_x, &cur_y, &row_h, tw, th](cached_glyph& glyph) {
//            type const gw = glyph.width;
//            type const gh = glyph.height;
//
//            if (cur_x + gw > tw) {
//                cur_x  = 0;
//                cur_y += row_h;
//                row_h  = 0;
//            }
//
//            if ((cur_x + gw > tw) || (cur_y + gh > th)) {
//                BK_TODO_FAIL();
//            }
//            
//            glyph.tex_x = cur_x;
//            glyph.tex_y = cur_y;
//
//            row_h  = std::max(row_h, gh);
//            cur_x += gw;
//        };
//
//        for (auto& block : map_) {
//            for (auto& glyph : block.second) {
//                update_glyph(glyph);
//            }
//        }
//
//        //move to the next row
//        return {0, cur_y + row_h};
//    }
//
//    void render() {
//    
//    }
//private:
//
//
//    boost::container::flat_map<key_t, value_t> map_;
//};
//
//class glyph_cache {
//public:
//    template <typename Key, typename Value>
//    using map_t = boost::container::flat_map<Key, Value>;
//
//    using codepoint = bkrl::unicode::codepoint;
//    using index_t   = int;
//    
//    using value_t = bkrl::cached_glyph;
//
//    //--------------------------------------------------------------------------
//    glyph_cache() {
//        constexpr auto tex_w = 1024;
//        constexpr auto tex_h = 1024;
//        constexpr auto cell_size = 20;
//
//        static_.add_block(unicode::basic_latin    {});
//        static_.add_block(unicode::basic_japanese {});
//
//        auto const p = static_.update_texture_coords(tex_w, tex_h);
//
//        auto const cells = (tex_w / cell_size) * ((tex_h - p.y) / cell_size);
//        BK_ASSERT_SAFE(cells > 0);
//
//        limit_ = static_cast<size_t>(cells);
//
//        cached_.reserve(limit_);
//        map_.reserve(limit_);
//
//        for (index_t i = 0; i < cells; ++i) {
//            lru_.push_back(i);
//        }
//    }
//
//    //--------------------------------------------------------------------------
//    index_t oldest() const {
//        std::cout << "evict\n";
//
//        return lru_.back();
//    }
//
//    //--------------------------------------------------------------------------
//    index_t next_available() {
//        auto const size = cached_.size();
//
//        auto const i = (size >= limit_)
//          ? oldest()
//          : (cached_.resize(size + 1), size);
//
//        freshen(i);
//
//        return i;
//    }
//
//    //--------------------------------------------------------------------------
//    value_t load_value(codepoint const cp) const {
//        std::cout << "load\n";
//
//        value_t result {};
//
//        result.codepoint = bkrl::value_of(cp);
//
//        return result;
//    }
//
//    //--------------------------------------------------------------------------
//    void freshen(index_t const i) {
//        auto const beg = std::begin(lru_);
//        auto const end = std::end(lru_);
//
//        auto const it = std::find(beg, end, i);
//        if (it == beg) {
//            //i is already the mru value.
//            return;
//        } else if (it == end) {
//            //should never happed
//            BK_TODO_FAIL();
//        }
//
//
//        //make i the mru value
//        auto const value = *it;
//
//        lru_.erase(it);
//        lru_.push_front(value);
//    }
//
//    //--------------------------------------------------------------------------
//    value_t const& insert(codepoint const cp) {
//        std::cout << "insert\n";
//
//        auto const i = next_available();
//        auto& cached = cached_[i];
//
//        auto const old_cp = unicode::codepoint {cached.codepoint};
//        map_.erase(old_cp);
//        map_.emplace(cp, i);
//
//        cached = load_value(cp);
//
//        return cached;
//    }
//
//    //--------------------------------------------------------------------------
//    value_t const& operator[](codepoint const cp) {
//        auto const static_value = static_[cp];
//        if (static_value) {
//            return *static_value;
//        }
//
//        auto const it = map_.find(cp);
//        if (it != map_.cend()) {
//            std::cout << "hit\n";
//
//            auto const i = it->second;
//            freshen(i);
//            return cached_[i];
//        }
//
//        return insert(cp);
//    }
//private:
//    size_t                    limit_;  //!< cache size
//    static_glyph_cache        static_; //!< statically cached values
//    std::vector<value_t>      cached_; //!< cached values
//    std::deque<index_t>       lru_;    //!< LRU eviction scheme
//    map_t<codepoint, index_t> map_;    //!< map from codepoint -> cache index
//};

} //namespace bkrl

class engine_server {
};


int main(int, char**) {
    //bkrl::glyph_cache test_cache {};

    //auto const data = bkrl::read_file(bkrl::string_ref {"./data/cache_test.txt"});
    //
    //std::vector<uint32_t> utf32;
    //utf8::utf8to32(data.begin(), data.end(), std::back_inserter(utf32));

    //for (auto const cp : utf32) {
    //    test_cache[bkrl::unicode::codepoint {cp}];
    //}

    auto const config = bkrl::load_config("./data/config.def");

    bkrl::engine_client client {config};

    return 0;
}
