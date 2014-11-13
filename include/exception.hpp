//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project wide base for all exception types.
//##############################################################################
#pragma once

#include <exception>
#include <boost/exception/all.hpp>

#include "string.hpp"

namespace bkrl {

struct exception_base : virtual std::exception, virtual boost::exception {};

namespace error {

struct data_error : virtual exception_base {};
struct parsing_error : virtual data_error {};

using parsing_rule = boost::error_info<struct parsing_rule_tag, string_ref>;
using parsing_expected_type = boost::error_info<struct parsing_rule_tag, string_ref>;
using parsing_actual_type = boost::error_info<struct parsing_rule_tag, string_ref>;

} //namespace error

} //namespace bkrl
