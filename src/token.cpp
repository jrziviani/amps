#include "token.h"

namespace volt
{
    token_t::token_t()
    {
    }

    token_t::token_t(token_types type, value_t value) :
        type_(type),
        value_(value)
    {
    }

    token_t::token_t(token_types type) :
            type_(type),
            value_({})
    {
    }
}
