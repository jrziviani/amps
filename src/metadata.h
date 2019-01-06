#ifndef METADATA_H
#define METADATA_H

#include <iostream>
#include <string>
#include <vector>

#include "token.h"

namespace volt
{
    enum class metatype : uint8_t
    {
        TEXT,
        CODE,
        ECHO,
    };

    struct metarange
    {
        size_t start;
        size_t end;
    };

    struct metadata
    {
        metatype type;
        metarange range;
        std::string data;
        std::vector<token_t> tokens;
    };

    static std::ostream& operator<<(std::ostream &os, const metadata &tpl)
    {
        os << "$" << std::to_string(static_cast<uint8_t>(tpl.type)) << "$"
           << tpl.range.start << "$"
           << tpl.range.end << "$"
           << tpl.data << "$";
        return os;
    }
}

#endif // METADATA_H
