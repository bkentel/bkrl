//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Game engine client.
//##############################################################################
#pragma once
#include "types.hpp"

namespace bkrl {

class renderer;

//==============================================================================
//! engine_client
//==============================================================================
class engine_client {
public:
    engine_client();
    ~engine_client();

    void on_command(command_type const cmd);

    void render(renderer& r);
private:
    struct impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
