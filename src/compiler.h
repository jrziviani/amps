#ifndef COMPILER_H
#define COMPILER_H

#include "types.h"
#include "error.h"
#include "context.h"

#include <vector>
#include <string>

namespace volt
{
    class parser_iterator;

    class compiler
    {
        struct branch
        {
            token_types type;
            bool taken;
        };

        token_types jump_;
        error &error_;
        context context_;
        std::vector<branch> branches_;

    private:
        void jump_to(token_types type);

        bool parse_expression(parser_iterator &it);
        bool parse_logical(parser_iterator &it);
        bool parse_equality(parser_iterator &it);
        bool parse_comparison(parser_iterator &it);
        bool parse_addition(parser_iterator &it);
        bool parse_multiplication(parser_iterator &it);
        bool parse_unary(parser_iterator &it);
        bool parse_primary(parser_iterator &it);

        bool run_statement(parser_iterator &it);
        bool run_print(parser_iterator &it);
        bool run_for(parser_iterator &it);
        bool run_endfor(parser_iterator &it);
        bool run_if(parser_iterator &it);
        bool run_else(parser_iterator &it);
        bool run_elif(parser_iterator &it);
        bool run_endif(parser_iterator &it);
        bool run_insert(parser_iterator &it);

        object compute(token_types oper);
        object compute_unary(token_types oper);
        object compute_numbers(number_t a,
                               number_t b,
                               token_types oper);
        object compute_strings(std::string a,
                               std::string b,
                               token_types oper);

    public:
        compiler(error &err);
        void generate(const metainfo &metainfo, const user_map &usermap);
    };

    class parser_iterator
    {
        friend class compiler;

        const tokens &tokens_;
        size_t cursor_;

        parser_iterator(const tokens &tks) :
            tokens_(tks),
            cursor_(0)
        {
        }

        bool advance()
        {
            if (cursor_ > tokens_.size() - 1) {
                return false;
            }

            ++cursor_;
            return true;
        }

        bool is_eot() const
        {
            if (cursor_ >= tokens_.size()) {
                return true;
            }

            return false;
        }

        token_t look() const
        {
            return tokens_[cursor_];
        }

        token_t look_back() const
        {
            return tokens_[cursor_ - 1];
        }

        void next()
        {
            if (is_eot()) {
                return;
            }

            advance();
        }

        void skip_all()
        {
            while (!is_eot()) {
                advance();
            }
        }

        bool match(token_types type)
        {
            if (!is_eot() && tokens_[cursor_].type() == type) {
                advance();
                return true;
            }

            return false;
        }
    };
}

#endif // COMPILER_H
