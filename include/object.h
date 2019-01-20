#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <variant>
#include <iostream>

namespace amps
{
    using number_t = uint64_t;
    using var_t    = std::variant<bool,
                                  number_t,
                                  std::string>;

    enum class vobject_types : uint8_t
    {
        NUMBER,
        STRING,
        BOOL,
    };

    inline std::ostream& operator<<(std::ostream &os, const vobject_types &obj)
    {
        switch (obj) {
            case vobject_types::BOOL:
                os << "bool";
                break;

            case vobject_types::NUMBER:
                os << "number";
                break;

            case vobject_types::STRING:
                os << "string";
                break;
        }

        return os;
    }

    class vobject
    {
        var_t data_;

    public:
        vobject()
        {
        }

        vobject(var_t value) :
            data_(value)
        {
        }

        vobject(vobject &&)            = default;
        vobject(const vobject &)       = default;
        ~vobject()                     = default;

        vobject &operator=(vobject &)  = default;
        vobject &operator=(vobject &&) = default;

        bool get_bool_or(bool alt) const;
        number_t get_number_or(number_t alt) const;
        vobject_types get_type() const;
        std::string get_string_or(const std::string &alt) const;
        std::string to_string() const;
    };

    inline bool vobject::get_bool_or(bool alt) const
    {
        auto ret = std::get_if<bool>(&data_);
        if (ret == nullptr) {
            return alt;
        }

        return *ret;
    }

    inline number_t vobject::get_number_or(number_t alt) const
    {
        auto ret = std::get_if<number_t>(&data_);
        if (ret == nullptr) {
            return alt;
        }

        return *ret;
    }

    inline std::string vobject::get_string_or(const std::string &alt) const
    {
        auto ret = std::get_if<std::string>(&data_);
        if (ret == nullptr) {
            return alt;
        }

        return *ret;
    }

    inline vobject_types vobject::get_type() const
    {
        if (std::holds_alternative<bool>(data_)) {
            return vobject_types::BOOL;
        }
        else if (std::holds_alternative<number_t>(data_)) {
            return vobject_types::NUMBER;
        }
        else {
            return vobject_types::STRING;
        }
    }

    inline std::string vobject::to_string() const
    {
        switch (get_type()) {
            case vobject_types::BOOL:
                return get_bool_or(false) ? "true" : "false";

            case vobject_types::NUMBER:
                return std::to_string(get_number_or(0));

            case vobject_types::STRING:
                return get_string_or("");
        }

        return "<null>";
    }
}

#endif // OBJECT_H

