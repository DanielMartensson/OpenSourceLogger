#include "TimeConverter.h"
#include "boost/date_time/posix_time/posix_time.hpp"


std::string currentISO8601Time() {
	// Compute microseconds
	boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
	return "'" + to_iso_extended_string(t) + "'";
}
