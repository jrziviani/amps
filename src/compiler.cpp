#include "compiler.h"

#include <iostream>

using namespace std;

namespace volt
{
    compiler::compiler(error &err) :
        error_(err)
    {
    }

    void compiler::generate(const metainfo &metainfo, const user_map &usermap)
    {
        const size_t &counter = context_.get_counter();

        // add user variables in the global scope
        context_.environment_setup(usermap);

        for (; counter < metainfo.size(); context_.jump_to(counter + 1)) {
            if (metainfo[counter].type == metatype::TEXT) {
                if (metainfo[counter].data.size() > 0) {
                    if (branches_.size() == 0 ||
                       (branches_.size() > 0 && branches_.back().taken)) {
                        cout << metainfo[counter].data;
                    }
                }
                continue;
            }

            parser_iterator it(metainfo[counter].tokens);

            while (!it.is_eot()) {
                if (!run_statement(it)) {
                    error_.log("Expression", metainfo[counter].data,
                               "cannot be parsed");
                    context_.stack_clear();
                    break;
                }
            }
        }
    }

    bool compiler::run_statement(parser_iterator &it)
    {
        switch (it.look().type()) {
            case token_types::PRINT:
                return run_print(it);

            case token_types::FOR:
                return run_for(it);

            case token_types::ENDFOR:
                return run_endfor(it);

            case token_types::IF:
                return run_if(it);

            case token_types::ELIF:
                return run_elif(it);

            case token_types::ELSE:
                return run_else(it);

            case token_types::ENDIF:
                return run_endif(it);

            case token_types::INSERT:
                return run_insert(it);

            default:
                return false;
        }
    }

    bool compiler::run_print(parser_iterator &it)
    {
        if (branches_.size() > 0 && !branches_.back().taken) {
            it.skip_all();
            return true;
        }

        it.next();
        if (!parse_expression(it)) {
            cout << "<null>" << endl;
            return false;
        }

        auto result = context_.stack_pop();
        if (result == nullopt) {
            error_.log("print statement cannot evaluate its parameters");
            cout << "<null>" << endl;
            return false;
        }

        auto type = result.value().get_type();
        if (type == vobject_types::STRING) {
            cout << result.value().get_string_or("<null>");
        }
        else if (type == vobject_types::NUMBER) {
            cout << static_cast<int64_t>(result.value().get_number_or(0));
        }
        else {
            cout << boolalpha << result.value().get_bool_or(false);
        }

        return true;
    }

    bool compiler::run_for(parser_iterator &it)
    {
        it.next();

        if (branches_.size() > 0 && !branches_.back().taken) {
            it.skip_all();
            branches_.push_back(branch{token_types::FOR, false});
            return true;
        }

        if (!it.match(token_types::IDENTIFIER)) {
            error_.log("loop statement requires an identifier");
            return false;
        }

        string id_or_key = it.look_back().value().value_or("");
        if (context_.environment_is_key_defined(id_or_key)) {
            error_.log("variable",
                       id_or_key,
                       "already exists, name must be unique");
            return false;
        }

        string value = "";
        if (it.match(token_types::COMMA)) {
            if (it.match(token_types::IDENTIFIER)) {
                value = it.look_back().value().value_or("");
            }
            else {
                error_.log("expected an identifier after ','");
                return false;
            }
        }

        if (!it.match(token_types::IN)) {
            error_.log("expect 'in' operator after identifier");
            return false;
        }

        if (it.match(token_types::RANGE) && value.size() == 0) {
            if (!it.match(token_types::LEFT_PAREN)) {
                error_.log("expect '('");
                return false;
            }

            for (unsigned int i = 0; i < 3; i++) {
                if (!parse_unary(it)) {
                    return false;
                }

                if (i < 2 && !it.match(token_types::COMMA)) {
                    error_.log("expect ','");
                    return false;
                }
            }

            if (!it.match(token_types::RIGHT_PAREN)) {
                error_.log("expected closing ')'");
                return false;
            }

            int64_t step  = context_.stack_pop_number_or(0);
            int64_t end   = context_.stack_pop_number_or(0);
            int64_t start = context_.stack_pop_number_or(0);

            if (step == 0 || start == end ||
                (step > 0 && start > end) ||
                (step < 0 && start < end)) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            vector<int64_t> range;
            for (; ((step < 0 && start > end) || start < end); start += step) {
                range.emplace_back(start);
            }

            context_.environment_add_or_update(string("range" + id_or_key),
                                               range);
            context_.environment_add_or_update(id_or_key, (range.size() > 0) ? range.at(0) : 0);

            context_.stack_push(object_t(string("range" + id_or_key)));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(number_t(0)));
            context_.stack_push(object_t(context_.get_counter()));
            branches_.push_back(branch{token_types::FOR, true});
        }
        else if (it.look().type() == token_types::IDENTIFIER && value.size() == 0) {
            string vect = it.look().value().value_or("");
            it.next();

            if (!context_.environment_is_key_defined(vect)) {
                error_.log("variable", vect, "is not defined");
                return false;
            }

            if (context_.environment_get_size(vect) == 0) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            context_.environment_add_or_update(vect, id_or_key, 0);
            context_.stack_push(object_t(vect));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(number_t(0)));
            context_.stack_push(object_t(context_.get_counter()));
            branches_.push_back(branch{token_types::FOR, true});
        }
        else if (it.look().type() == token_types::IDENTIFIER && value.size() > 0) {
            string tbl = it.look().value().value_or("");
            it.next();

            if (!context_.environment_is_key_defined(tbl)) {
                error_.log("variable", tbl, "is not defined");
                return false;
            }

            if (context_.environment_get_size(tbl) == 0) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            number_t index = 0;
            index = context_.environment_add_or_update(tbl, id_or_key, value, index);
            context_.stack_push(object_t(tbl));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(index));
            context_.stack_push(object_t(context_.get_counter()));
            branches_.push_back(branch{token_types::FOR, true});
        }
        else {
            error_.log("invalid loop");
            return false;
        }

        if (inspect_) {
            inspect_(context_, branches_);
        }

        return true;
    }

    bool compiler::run_endfor(parser_iterator &it)
    {
        it.next();

        if (branches_.size() > 0 && !branches_.back().taken) {
            if (inspect_) {
                inspect_(context_, branches_);
            }
            branches_.pop_back();
            return true;
        }

        if (branches_.size() == 0 ||
            (branches_.size() > 0 &&
            branches_.back().type != token_types::FOR)) {
            error_.log("expected ENDFOR");
            return false;
        }

        // get loop parameters
        number_t counter = context_.stack_pop_number_or(0);
        number_t index   = context_.stack_pop_number_or(0);
        string value     = context_.stack_pop_string_or("");
        string id        = context_.stack_pop_string_or("");
        string key       = context_.stack_pop_string_or("");

        if (++index >= context_.environment_get_size(key)) {
            context_.environment_erase(id);
            branches_.pop_back();
            return true;
        }

        if (value.size() > 0) {
            index = context_.environment_add_or_update(key, id, value, index);
        }
        else {
            context_.environment_add_or_update(key, id, index);
        }

        // update the data and push them onto the stack
        context_.stack_push(object_t(key));
        context_.stack_push(object_t(id));
        context_.stack_push(object_t(value));
        context_.stack_push(object_t(index));
        context_.stack_push(object_t(counter));

        // restart the block execution
        context_.jump_to(counter);

        if (inspect_) {
            inspect_(context_, branches_);
        }

        return true;
    }

    bool compiler::run_if(parser_iterator &it)
    {
        it.next();

        if (branches_.size() > 0 && !branches_.back().taken) {
            it.skip_all();
            branches_.push_back(branch{token_types::IF, false});
            return true;
        }

        if (!parse_expression(it)) {
            error_.log("if cannot be parsed");
            return false;
        }

        bool ret = context_.stack_pop_bool_or(false);
        branches_.push_back(branch{token_types::IF, ret});

        return true;
    }

    bool compiler::run_else(parser_iterator &it)
    {
        it.next();

        if (branches_.size() > 0 && branches_.back().taken) {
            branches_.back().taken = false;
            return true;
        }

        if (branches_.back().type != token_types::IF) {
            error_.log("expected ENDIF, ELSE, or ELIF");
            return false;
        }

        branches_.back().taken = true;

        return true;
    }

    bool compiler::run_elif(parser_iterator &it)
    {
        if (branches_.size() > 0 && branches_.back().taken) {
            it.skip_all();
            return true;
        }

        if (branches_.back().type != token_types::IF) {
            error_.log("expected ENDIF, ELSE, or ELIF");
            return false;
        }

        branches_.pop_back();
        return run_if(it);
    }

    bool compiler::run_endif(parser_iterator &it)
    {
        it.next();

        if (branches_.size() > 0 && !branches_.back().taken) {
            branches_.pop_back();
            return true;
        }

        if (branches_.back().type != token_types::IF) {
            error_.log("expected ENDIF, ELSE, or ELIF");
            return false;
        }
        branches_.pop_back();

        return true;
    }

    bool compiler::run_insert(parser_iterator &it)
    {
        if (branches_.size() > 0 && !branches_.back().taken) {
            it.skip_all();
            return true;
        }

        return true;
    }

    bool compiler::parse_expression(parser_iterator &it)
    {
        return parse_logical(it);
    }

    bool compiler::parse_logical(parser_iterator &it)
    {
        if (!parse_equality(it)) {
            return false;
        }

        while (it.match(token_types::AND) ||
               it.match(token_types::OR)) {
            token_t oper = it.look_back();
            if (!parse_equality(it)) {
                return false;
            }

            auto result = compute(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }

    bool compiler::parse_equality(parser_iterator &it)
    {
        if (!parse_comparison(it)) {
            return false;
        }

        while (it.match(token_types::EQ)  ||
               it.match(token_types::NE)) {
            token_t oper = it.look_back();
            if (!parse_comparison(it)) {
                return false;
            }

            auto result = compute(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }


    bool compiler::parse_comparison(parser_iterator &it)
    {
        if (!parse_addition(it)) {
            return false;
        }

        while (it.match(token_types::GT) ||
               it.match(token_types::GE) ||
               it.match(token_types::LT) ||
               it.match(token_types::LE)) {
            token_t oper = it.look_back();
            if (!parse_addition(it)) {
                return false;
            }

            auto result = compute(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }

    bool compiler::parse_addition(parser_iterator &it)
    {
        if (!parse_multiplication(it)) {
            return false;
        }

        while (it.match(token_types::MINUS) ||
               it.match(token_types::PLUS)) {
            token_t oper = it.look_back();
            if (!parse_multiplication(it)) {
                return false;
            }

            auto result = compute(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }

    bool compiler::parse_multiplication(parser_iterator &it)
    {
        if (!parse_unary(it)) {
            return false;
        }

        while (it.match(token_types::STAR) ||
               it.match(token_types::SLASH) ||
               it.match(token_types::PERCENT)) {
            token_t oper = it.look_back();
            if (!parse_unary(it)) {
                return false;
            }

            auto result = compute(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }

    bool compiler::parse_unary(parser_iterator &it)
    {
        if (it.match(token_types::NOT) ||
            it.match(token_types::MINUS)) {
            token_t oper = it.look_back();
            if (!parse_unary(it)) {
                return false;
            }

            auto result = compute_unary(oper.type());
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }
        else {
            if (!parse_primary(it)) {
                return false;
            }
        }

        return true;
    }

    bool compiler::parse_primary(parser_iterator &it)
    {
        if (it.match(token_types::NUMBER)) {
            number_t value = stoul(it.look_back().value().value_or("0"));
            context_.stack_push(object_t(value));
            return true;
        }
        else if (it.match(token_types::STRING)) {
            string value = it.look_back().value().value_or("");
            context_.stack_push(object_t(value));
            return true;
        }
        else if (it.match(token_types::IDENTIFIER)) {
            string id = it.look_back().value().value_or("");
            if (!context_.environment_is_key_defined(id)) {
                return true;
            }

            if (it.match(token_types::LEFT_BRACKET)) {
                context_.stack_push(object_t(id));

                if (!parse_primary(it)) {
                    return false;
                }

                vobject_types tp = context_.stack_top_type();
                if (tp == vobject_types::STRING) {
                    std::string index = context_.stack_pop_string_or("");
                    std::string id = context_.stack_pop_string_or("");
                    context_.stack_push_from_environment(id, index);
                }
                else if (tp == vobject_types::NUMBER) {
                    number_t index = context_.stack_pop_number_or(0);
                    std::string id = context_.stack_pop_string_or("");
                    context_.stack_push_from_environment(id, index);
                }
                else {
                    context_.stack_pop();
                    context_.stack_push(object_t(std::string("<null>")));
                }

                if (!it.match(token_types::RIGHT_BRACKET)) {
                    error_.log("expect closing ']'");
                    return false;
                }

                return true;
            }
            else {
                context_.stack_push_from_environment(id);
            }
            return true;
        }
        else if (it.match(token_types::LEFT_PAREN)) {
            if (!parse_expression(it)) {
                return false;
            }

            if (!it.match(token_types::RIGHT_PAREN)) {
                error_.log("expected closing ')'");
                return false;
            }

            return true;
        }
        else {
            error_.log("unexpected token found: ",
                       it.look().type(),
                       it.look().value().value_or("<null>"));

            return false;
        }
    }

    object compiler::compute_numbers(number_t a,
                                     number_t b,
                                     token_types oper)
    {
        switch(oper) {
            case token_types::MINUS:
                return object_t(a - b);

            case token_types::PLUS:
                return object_t(a + b);

            case token_types::SLASH:
                if (b == 0) {
                    error_.log("cannot divide by 0");
                    return nullopt;
                }
                return object_t(a / b);

            case token_types::PERCENT:
                if (b == 0) {
                    error_.log("cannot divide by 0");
                    return nullopt;
                }
                return object_t(a % b);

            case token_types::STAR:
                return object_t(a * b);

            case token_types::EQ:
                return object_t(a == b);

            case token_types::NE:
                return object_t(a != b);

            case token_types::GT:
                return object_t(a > b);

            case token_types::GE:
                return object_t(a >= b);

            case token_types::LT:
                return object_t(a < b);

            case token_types::LE:
                return object_t(a <= b);

            default:
                return nullopt;
        }
    }

    object compiler::compute_strings(string a,
                                     string b,
                                     token_types oper)
    {
        switch(oper) {
            case token_types::PLUS:
                return object_t(a + b);

            case token_types::EQ:
                return object_t(a == b);

            case token_types::NE:
                return object_t(a != b);

            case token_types::GT:
                return object_t(a > b);

            case token_types::GE:
                return object_t(a >= b);

            case token_types::LT:
                return object_t(a < b);

            case token_types::LE:
                return object_t(a <= b);

            default:
                return nullopt;
        }
    }

    object compiler::compute(token_types oper)
    {
        auto vb = context_.stack_pop();
        auto va = context_.stack_pop();

        if (va == nullopt || vb == nullopt) {
            return nullopt;
        }

        auto va_type = va.value().get_type();
        auto vb_type = vb.value().get_type();

        if (va_type == vobject_types::NUMBER && vb_type == vobject_types::NUMBER) {
            return compute_numbers(va.value().get_number_or(0),
                                   vb.value().get_number_or(0),
                                   oper);
        }
        else if (va_type == vobject_types::STRING && vb_type == vobject_types::STRING) {
            return compute_strings(va.value().get_string_or(""),
                                   vb.value().get_string_or(""),
                                   oper);
        }
        else if (va_type == vobject_types::BOOL && vb_type == vobject_types::BOOL) {
            bool a = va.value().get_bool_or(false);
            bool b = vb.value().get_bool_or(false);

            switch(oper) {
                case token_types::EQ:
                    return object_t(a == b);

                case token_types::NE:
                    return object_t(a != b);

                case token_types::AND:
                    return object_t(a && b);

                case token_types::OR:
                    return object_t(a || b);

                default:
                    return object_t(false);
            }
        }

        error_.log("Mismatched types. ", va_type, "[",
                   va.value().get_number_or(0), "] cannot compute with ",
                   vb_type, "[", vb.value().get_number_or(0), "]");

        return nullopt;
    }

    object compiler::compute_unary(token_types oper)
    {
        auto t = context_.stack_pop();
        if (t == nullopt) {
            return nullopt;
        }

        auto type = t.value().get_type();

        if (oper == token_types::MINUS && type == vobject_types::NUMBER) {
            return object_t(t.value().get_number_or(0) * -1);
        }
        else if (oper == token_types::NOT && type == vobject_types::NUMBER) {
            return object_t((t.value().get_number_or(0) == 0) ? true : false);
        }
        else if (oper == token_types::NOT && type == vobject_types::STRING) {
            return object_t((t.value().get_string_or("").size() == 0) ? true : false);
        }

        error_.log("Unary operator '", oper,
                   "' cannot be used with value '",
                   t.value().to_string());

        return nullopt;
    }
}
