#ifndef TYPES_H
#define TYPES_H

#include "metadata.h"
#include "token.h"
#include "object.h"

#include <variant>
#include <vector>
#include <optional>
#include <string>
#include <unordered_map>

namespace volt
{
    using metainfo   = std::vector<metadata>;
    using tokens     = std::vector<token_t>;

    using user_var   = std::variant<number_t,
                           std::string,
                           std::vector<number_t>,
                           std::vector<std::string>,
                           std::unordered_map<std::string, number_t>,
                           std::unordered_map<std::string, std::string>>;

    using table      = std::unordered_map<std::string, var_t>;
    using user_map   = std::unordered_map<std::string, user_var>;

    using object_t   = vobject;
    using object     = std::optional<object_t>;
    using objects    = std::vector<object_t>;
}

#endif // TYPES_H
