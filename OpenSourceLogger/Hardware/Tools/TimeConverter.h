#include <string>
#include <boost/asio.hpp>
#include <mysqlx/xdevapi.h>

#ifndef TimeConverter
#define TimeConverter

std::string currentISO8601Time();

#endif // !TimeConverter
