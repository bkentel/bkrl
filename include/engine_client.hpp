//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Game engine client.
//##############################################################################
#pragma once

#include <memory>

#include "types.hpp"

namespace bkrl {

class renderer;
class config;

//==============================================================================
//! engine_client
//==============================================================================
class engine_client {
public:
    engine_client(config const& conf);
    ~engine_client();

    void on_command(command_type const cmd);

    void render(renderer& r);
private:
    struct impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
