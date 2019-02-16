#ifndef COMPILER_H
#define COMPILER_H

#include "types.h"
#include "error.h"
#include "context.h"

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace amps
{
    class parser_iterator;

    struct branch
    {
        token_types type;
        bool taken;
    };

    struct block_cache
    {
        size_t start;
        size_t end;
        size_t resume;
        size_t iterations;
    };

    class compiler
    {
        bool running_cache_;
        std::string result_;
        error &error_;
        size_t current_cache_;
        context context_;
        std::vector<branch> branches_;
        std::unordered_map<size_t, block_cache> cache_;
        std::function<void(const context &,
                           const std::vector<branch> &)> inspect_;

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

        bool run_statement(parser_iterator &it, metainfo &metainfo);
        bool run_print(parser_iterator &it);
        bool run_for(parser_iterator &it);
        bool run_endfor(parser_iterator &it);
        bool run_if(parser_iterator &it);
        bool run_else(parser_iterator &it);
        bool run_elif(parser_iterator &it);
        bool run_endif(parser_iterator &it);
        bool run_insert(parser_iterator &it, metainfo &metainfo);

        object compute(token_types oper, size_t line);
        object compute_unary(token_types oper, size_t line);
        object compute_numbers(number_t a,
                               number_t b,
                               token_types oper, size_t line);
        object compute_strings(std::string a,
                               std::string b,
                               token_types oper);

    public:
        compiler(error &err);
        std::string generate(metainfo &metainfo, const user_map &usermap);

        template <typename F>
        void set_callback(F&& callback)
        {
            inspect_ = std::forward<F>(callback);
        }
    };

    class parser_iterator
    {
        friend class compiler;

        const tokens &tokens_;
        const metarange &range_;
        size_t cursor_;

        parser_iterator(const tokens &tks, const metarange &range) :
            tokens_(tks),
            range_(range),
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

        metarange range() const
        {
            return range_;
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
