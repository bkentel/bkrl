//##############################################################################
//! @author Brandon Kentel
//!
//! Project wide base for all exception types.
//##############################################################################
#pragma once

#include <exception>
#include <boost/exception/all.hpp>

namespace bkrl {

struct exception_base : virtual std::exception, virtual boost::exception {};

} //namespace bkrl
