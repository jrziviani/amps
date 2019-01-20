#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <optional>
#include <functional>

#define TOKENS              \
    X(IDENTIFIER)           \
    X(VARIABLE)             \
    X(STRING)               \
    X(NUMBER)               \
    X(PAIR)                 \
    X(BOOLEAN)              \
    X(TRUE)                 \
    X(FALSE)                \
    X(NIL)                  \
    X(AND)                  \
    X(OR)                   \
    X(NOT)                  \
    X(EQ)                   \
    X(NE)                   \
    X(LT)                   \
    X(LE)                   \
    X(GT)                   \
    X(GE)                   \
    X(PLUS)                 \
    X(MINUS)                \
    X(SLASH)                \
    X(STAR)                 \
    X(PERCENT)              \
    X(ASSIGN)               \
    X(COMMA)                \
    X(IF)                   \
    X(ELIF)                 \
    X(ELSE)                 \
    X(ENDIF)                \
    X(FOR)                  \
    X(IN)                   \
    X(ENDFOR)               \
    X(LEFT_PAREN)           \
    X(RIGHT_PAREN)          \
    X(LEFT_BRACKET)         \
    X(RIGHT_BRACKET)        \
    X(RANGE)                \
    X(PRINT)                \
    X(SIZE)                 \
    X(EXCEPT)               \
    X(INSERT)

#define SINGLE_TOKEN        \
    X('(', LEFT_PAREN)      \
    X(')', RIGHT_PAREN)     \
    X('[', LEFT_BRACKET)    \
    X(']', RIGHT_BRACKET)   \
    X(',', COMMA)           \
    X('+', PLUS)            \
    X('-', MINUS)           \
    X('*', STAR)            \
    X('/', SLASH)           \
    X('%', PERCENT)         \
    X('=', ASSIGN)

#define KEYWORDS            \
    X("true", TRUE)         \
    X("false", FALSE)       \
    X("boolean", BOOLEAN)   \
    X("null", NIL)          \
    X("and", AND)           \
    X("or", OR)             \
    X("not", NOT)           \
    X("eq", EQ)             \
    X("ne", NE)             \
    X("lt", LT)             \
    X("le", LE)             \
    X("gt", GT)             \
    X("ge", GE)             \
    X("if", IF)             \
    X("elif", ELIF)         \
    X("else", ELSE)         \
    X("endif", ENDIF)       \
    X("for", FOR)           \
    X("in", IN)             \
    X("endfor", ENDFOR)     \
    X("range", RANGE)       \
    X("print", PRINT)       \
    X("size", SIZE)         \
    X("insert", INSERT)

namespace amps
{
    enum class token_types : uint8_t
    {
    #define X(name) name,
        TOKENS
    #undef X

        EOT,
    };

    inline std::string get_token_name(token_types t)
    {
        switch (t) {
        #define X(name) case token_types::name: \
            return std::string(#name);
            TOKENS
        #undef X

            case token_types::EOT:
                return "END";

            default:
                return "undefined";
        }
    }

    inline std::ostream& operator<<(std::ostream &os, const token_types &type)
    {
        os << get_token_name(type);
        return os;
    }

    class token_t
    {
        using value_t = std::optional<std::string>;

        token_types type_;
        value_t value_;

    public:
        token_t();
        token_t(token_types type);
        token_t(token_types type, value_t value);
        token_t(const token_t&)         = default;
        token_t(token_t &&)             = default;
        ~token_t()                      = default;

        token_t &operator=(const token_t &)   = default;
        token_t &operator=(token_t &&)  = default;

        std::string to_string() const;
        token_types type() const;
        value_t value() const;

        size_t hash() const;
    };

    inline size_t token_t::hash() const
    {
        std::hash<std::string> hash_string;
        return hash_string(to_string());
    }

    inline std::string token_t::to_string() const
    {
        std::string ret = "token: " + get_token_name(type_) +
                          ", object: " + value_.value_or("<null>");
        return ret;
    }

    inline token_types token_t::type() const
    {
        return type_;
    }

    inline token_t::value_t token_t::value() const
    {
        return value_;
    }
}

#endif // TOKEN_H
