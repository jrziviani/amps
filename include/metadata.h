#ifndef METADATA_H
#define METADATA_H

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
        COMMENT,
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
}

#endif // METADATA_H
