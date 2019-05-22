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
        OBJECT,
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

            case vobject_types::OBJECT:
                os << "object";
                break;
        }

        return os;
    }

    class vobject
    {
        var_t data_;
        vobject_types ftype_;

    public:
        vobject()
        {
        }

        vobject(var_t value) :
            data_(value)
        {
            if (std::holds_alternative<std::string>(data_)) {
                ftype_ = vobject_types::STRING;
            }
            else if (std::holds_alternative<number_t>(data_)) {
                ftype_ = vobject_types::NUMBER;
            }
            else {
                ftype_ = vobject_types::BOOL;
            }
        }

        vobject(var_t value, vobject_types forced_type) :
            data_(value), ftype_(forced_type)
        {
        }

        vobject(vobject &&)                 = default;
        vobject(const vobject &)            = default;
        ~vobject()                          = default;

        vobject &operator=(const vobject &) = default;
        vobject &operator=(vobject &&)      = default;

        vobject_types get_type() const;
        bool get_bool_or(bool alt) const;
        number_t get_number_or(number_t alt) const;
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
        return ftype_;
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

            case vobject_types::OBJECT:
                return get_string_or("");
        }

        return "<null>";
    }
}

#endif // OBJECT_H

