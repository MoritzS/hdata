#include "util.h"

#include <limits>
#include <stdexcept>

namespace std {
    unsigned int stou(std::string const& str, size_t* idx, int base) {
        unsigned long int i = std::stoul(str, idx, base);
        if (i > std::numeric_limits<unsigned int>::max()) {
            throw std::out_of_range("std::stou");
        }
        return i;
    }

    unsigned int stou_safe(std::string const& str, int base) {
        size_t idx;
        unsigned int i = std::stou(str, &idx, base);
        if (idx < str.length()) {
            throw std::invalid_argument(
                "std::stou_safe: string contains invalid characters"
            );
        }
        return i; 
    }
}

bool stream_ui(std::istream& stream, uint32_t& out, int base) {
    std::string s;
    stream >> s;
    try {
        out = stou_safe(s, base);
        return true;
    } catch (std::logic_error) {
        return false;
    }
}
