#ifndef _HDATA_UTIL_H
#define _HDATA_UTIL_H

#include <string>

namespace std {
    unsigned int stou(std::string const& str, size_t* idx = nullptr, int base = 10);
    unsigned int stou_safe(std::string const& str, int base = 10);
}

#endif
