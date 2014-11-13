//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Game engine client.
//##############################################################################
#pragma once

#include <memory>

namespace bkrl {

class data_definitions;

//==============================================================================
//! engine_client
//==============================================================================
class engine_client {
public:
    engine_client(data_definitions& defs);
    ~engine_client();
private:
    struct impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
