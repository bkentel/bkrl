#include "engine_client.hpp"

namespace bkrl {

//void set_textures(grid_storage& grid, texture_map const& map) {
//    auto const w = grid.width();
//    auto const h = grid.height();
//
//    auto const floor_rule = [&](grid_index x, grid_index y) {
//        return texture_type::floor;
//    };
//
//    auto const wall_rule = [&](grid_index x, grid_index y) {
//        auto const n = check_grid_block5(grid, x, y, attribute::type, tile_type::wall);
//
//        switch (n) {
//        case (1<<2) : return texture_type::wall_none;
//
//        case (1<<0)|(1<<2) : return texture_type::wall_n;
//        case (1<<1)|(1<<2) : return texture_type::wall_w;
//        case (1<<2)|(1<<3) : return texture_type::wall_e;
//        case (1<<2)|(1<<4) : return texture_type::wall_s;
//
//        case (1<<0)|(1<<2)|(1<<4) : return texture_type::wall_ns;
//        case (1<<1)|(1<<2)|(1<<3) : return texture_type::wall_ew;
//        case (1<<2)|(1<<3)|(1<<4) : return texture_type::wall_se;
//        case (1<<1)|(1<<2)|(1<<4) : return texture_type::wall_sw;
//        case (1<<0)|(1<<2)|(1<<3) : return texture_type::wall_ne;
//        case (1<<0)|(1<<1)|(1<<2) : return texture_type::wall_nw;
//
//        case (1<<0)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_nse;
//        case (1<<0)|(1<<1)|(1<<2)|(1<<4) : return texture_type::wall_nsw;
//        case (1<<0)|(1<<1)|(1<<2)|(1<<3) : return texture_type::wall_new;
//        case (1<<1)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_sew;
//
//        case (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_nsew;
//        }
//
//        return texture_type::wall_nsew;
//    };
//
//    auto const get_texture = [&](grid_index x, grid_index y) {
//        auto const type = grid.get(attribute::type, x, y);
//
//        switch (type) {
//        case tile_type::wall : return wall_rule(x, y);
//        case tile_type::floor : return floor_rule(x, y);
//        }
//
//        return texture_type::invalid;
//    };
//
//    for (grid_index y = 0; y < h; ++y) {
//        for (grid_index x = 0; x < w; ++x) {
//            auto const tex_type  = get_texture(x, y);
//            auto const tex_index = map[tex_type];
//            auto const tex = texture_info {tex_type, tex_index};
//            grid.set(attribute::texture, x, y, tex);
//        }
//    }
//}

//////



} //namespace bkrl

///////
//
//class room : public bkrl::grid_storage {
//public:
//    explicit room(bkrl::grid_region const bounds)
//      : grid_storage {bounds}
//      , location_    {{bounds.left, bounds.top}}
//    {
//    }
//
//    bkrl::grid_region bounds() const {
//        auto const l = location_.x;
//        auto const t = location_.y;
//        auto const r = l + width();
//        auto const b = t + height();
//
//        return bkrl::grid_region {l, t, r, b};
//    }
//
//    room& translate(int dx, int dy) {
//        location_ = bkrl::translate(location_, dx, dy);
//        return *this;
//    }
//private:
//     bkrl::grid_point location_;
//};
//
//////



/////////////////
//class bsp_generator {
//    struct node_t {
//        node_t(bkrl::grid_region const region)
//          : region {region}
//        {
//        }
//
//        bool is_leaf()  const { return child_index == 0; }
//        bool is_empty() const { return room == nullptr; }
//
//        bkrl::grid_region region;
//        room*             room = nullptr;
//        unsigned          child_index = 0;
//    };
//public:
//    struct params_t {
//        unsigned min_region_w = 4;
//        unsigned min_region_h = 4;
//        unsigned max_region_w = 20;
//        unsigned max_region_h = 20;
//
//        unsigned split_chance = 50;
//    };
//
//    params_t params_;
//
//    using room_callback = std::function<void (bkrl::grid_region bounds)>;
//
//
//    void generate(bkrl::random::generator& gen) {
//        nodes_.clear();
//        nodes_.emplace_back(bkrl::grid_region {0, 0, 100, 100});
//        split(gen);
//    }
//
//    void split(bkrl::random::generator& gen) {
//        auto const split_all = [&](size_t const beg, size_t const end) {
//            for (auto i = beg; i < end; ++i) {
//                auto const did_split = split(gen, nodes_[i]);
//                if (did_split) {
//                    nodes_[i].child_index = nodes_.size() - 2; //added 2 elements
//                }
//            }
//
//            return nodes_.size();
//        };
//
//        // TODO could improve this for clarity?
//        // split while there are unvisited nodes
//        auto beg = 0;
//        auto end = nodes_.size();
//
//        while (true) {
//            auto const new_end = split_all(beg, end);
//
//            if (new_end - end == 0) {
//                break;
//            }
//
//            beg = end;
//            end = new_end;
//        }
//    }
//
//    bool split(bkrl::random::generator& gen, node_t const node) {
//        auto const w = node.region.width();
//        auto const h = node.region.height();
//
//        if (w < 20 && h < 20) {
//            auto const do_split = bkrl::random::uniform_range(gen, 0, 100) < 50;
//            if (!do_split) {
//                return false;
//            }
//        }
//
//        if (node.room) {
//            BK_TODO_FAIL();
//        }
//
//        auto const distribution = [&](unsigned const lo, unsigned const hi) {
//            return bkrl::random::uniform_range(gen, lo, hi);
//        };
//
//        //TODO clean up
//        auto const ratio = (w > h)
//          ? (static_cast<float>(w) / h)
//          : (static_cast<float>(h) / w);
//
//        auto const which = (ratio > 1.6f)
//          ? (w > h)
//          : (bkrl::random::uniform_range(gen, 0, 100) < 50);
//
//        auto const children = which
//            ? split_vertical(distribution, node.region, 4)
//            : split_horizontal(distribution, node.region, 4);
//
//        if (!std::get<0>(children)) {
//            //BK_TODO_FAIL();
//            return false;
//        }
//
//        nodes_.emplace_back(std::get<1>(children));
//        nodes_.emplace_back(std::get<2>(children));
//
//        return true;
//    }
////private:
//
//
//    std::vector<node_t> nodes_;
//};
//
/////////////////
//
//namespace generate {
//
//class simple_room {
//public:
//    room generate(bkrl::random::generator& gen, bkrl::grid_region const bounds) {
//        room result {bounds};
//
//        bkrl::random::uniform_int dist;
//
//        auto const w = dist.generate(gen, 4u, bounds.width());
//        auto const h = dist.generate(gen, 4u, bounds.height());
//
//        auto const slack_w = bounds.width()  - w;
//        auto const slack_h = bounds.height() - h;
//
//        auto const left   = dist.generate(gen, 0u, slack_w);
//        auto const top    = dist.generate(gen, 0u, slack_h);
//        auto const right  = left + w;
//        auto const bottom = top + h;
//
//        for (bkrl::grid_index yi = 0; yi < h; ++yi) {
//            for (bkrl::grid_index xi = 0; xi < w; ++xi) {
//                auto const x = xi + left;
//                auto const y = yi + top;
//
//                if (xi == 0 || yi == 0 || xi == w - 1 || yi == h - 1) {
//                    result.set(bkrl::attribute::type, x, y, bkrl::tile_type::wall);
//                } else {
//                    result.set(bkrl::attribute::type, x, y, bkrl::tile_type::floor);
//                }
//            }
//        }
//
//        //tmp
//        result.set(bkrl::attribute::type, left + 2, top + 2, bkrl::tile_type::wall);
//
//        return result;
//    }
//private:
//
//};
//
//
//} //namespace generate

//////////


using renderer = bkrl::sdl_renderer;

//////


//class entity {
//public:
//    entity()
//      : position {{0, 0}}
//    {
//    }
//
//    bkrl::grid_point position;
//};


using application = bkrl::sdl_application;

////



class engine_server {
};


int main(int, char**) {
    bkrl::engine_client client;

    return 0;
}
