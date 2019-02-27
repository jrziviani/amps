#include "compiler.h"
#include "scan.h"
#include "fileops.h"

#include <iostream>
#include <algorithm>

using namespace std;

namespace amps
{
    compiler::compiler(error &err) :
        running_cache_(false),
        update_main_cache_(false),
        error_(err),
        current_cache_(0)
    {
    }

    void compiler::reset()
    {
        running_cache_ = false;
        update_main_cache_ = false;
        result_ = string("");
        current_cache_ = 0;
        branches_.clear();
        cache_.clear();
        context_.reset();
    }

    string compiler::generate(metainfo &metainfo, const user_map &usermap)
    {
        result_ = string("");
        const size_t &counter = context_.get_counter();

        // put user data in the environment table
        context_.environment_setup(usermap);

        cache_[metainfo.hash()] = block_cache{counter - 1,
                                              metainfo.size() - 1,
                                              0, 1};
        update_main_cache_ = true;


        // program main loop
        for (; counter < metainfo.size(); context_.jump_to(counter + 1)) {

            // if we're finished running a cached code, go back to the
            // place where the cache was executed
            if (running_cache_ && cache_[current_cache_].end == counter) {
                context_.jump_to(cache_[current_cache_].resume);
                running_cache_ = false;
                continue;
            }

            // text isn't processed so it only verify if the branch it
            // belongs to has been taken and print it
            if (metainfo[counter].type == metatype::TEXT) {
                if (metainfo[counter].data.size() > 0) {
                    if (branches_.size() == 0 || branches_.back().taken) {
                        if (metainfo[counter].data[0] != 0) {
                            result_ += metainfo[counter].data;
                        }
                    }
                }

                continue;
            }

            // ignore comment metatypes
            else if (metainfo[counter].type == metatype::COMMENT) {
                continue;
            }

            // execute the program in the meta tags
            parser_iterator it(metainfo[counter].tokens, metainfo[counter].range);
            while (!it.is_eot()) {
                if (!run_statement(it, metainfo)) {
                    context_.stack_clear();
                    break;
                }
            }
        }

        // it's not expected to have any branch left after
        // program execution
        if (branches_.size() > 0 && branches_.back().type == token_types::FOR) {
            error_.log("expected closing endfor before EOF");
            branches_.clear();
        }
        else if (branches_.size() > 0 && branches_.back().type == token_types::IF) {
            error_.log("expected closing endif before EOF");
            branches_.clear();
        }

        return result_;
    }

    bool compiler::run_statement(parser_iterator &it, metainfo &metainfo)
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
                return run_insert(it, metainfo);

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
            if (inspect_) {
                inspect_(context_, branches_);
            }
            result_ += string("<null>");
            return false;
        }

        if (inspect_) {
            inspect_(context_, branches_);
        }

        auto result = context_.stack_pop();
        if (result == nullopt) {
            error_.log("print cannot be evaluated. Line: ",
                       it.range().line);
            result_ += string("<null>");
            return false;
        }

        auto type = result.value().get_type();
        if (type == vobject_types::STRING) {
            result_ += result.value().get_string_or("<null>");
        }
        else if (type == vobject_types::NUMBER) {
            int64_t num = static_cast<int64_t>(result.value().get_number_or(0));
            result_ += to_string(num);
        }
        else {
            result_ = (!result.value().get_bool_or(false)) ? "false" : "true";
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
            error_.critical("loop statement requires an identifier",
                            ". Line: ", it.range().line);
            return false;
        }

        string id_or_key = it.look_back().value().value_or("");
        if (context_.environment_is_key_defined(id_or_key)) {
            error_.critical("variable ", id_or_key,
                            " already exists, name must be unique",
                            ". Line: ", it.range().line);
            return false;
        }

        string value = "";
        if (it.match(token_types::COMMA)) {
            if (it.match(token_types::IDENTIFIER)) {
                value = it.look_back().value().value_or("");
            }
            else {
                error_.critical("expected identifier after ','. Line: ",
                                it.range().line);
                return false;
            }

            if (context_.environment_is_key_defined(value) ||
                id_or_key == value) {
                error_.critical("variable ", value,
                                " already exists, name must be unique",
                                ". Line: ", it.range().line);
                return false;
            }
        }

        if (!it.match(token_types::IN)) {
            error_.critical("expect 'in' operator after identifier. Line: ",
                            it.range().line);
            return false;
        }

        // range expects three arguments, values can be negative
        // for x in range(b, e, s)
        //                |  |  +> step
        //                |  +---> end
        //                +------> begin
        if (it.match(token_types::RANGE) && value.size() == 0) {

            if (!it.match(token_types::LEFT_PAREN)) {
                error_.critical("expect '('. Line: ", it.range().line);
                return false;
            }

            for (unsigned int i = 0; i < 3; i++) {
                if (!parse_unary(it)) {
                    return false;
                }

                if (context_.stack_top_type() != vobject_types::NUMBER) {
                    error_.critical("range expects only numbers. Line: ",
                                    it.range().line);
                    return false;
                }

                if (i < 2 && !it.match(token_types::COMMA)) {
                    error_.critical("expect ','. Line: ", it.range().line);
                    return false;
                }
            }

            if (!it.match(token_types::RIGHT_PAREN)) {
                error_.critical("expected closing ')'. Line: ",
                                it.range().line);
                return false;
            }

            int64_t step  = static_cast<int64_t>(context_.stack_pop_number_or(0));
            int64_t end   = static_cast<int64_t>(context_.stack_pop_number_or(0));
            int64_t start = static_cast<int64_t>(context_.stack_pop_number_or(0));

            // condition is made or impossible to complete,
            // set the branch to "not taken" and go to the next
            // block
            if (step == 0 || start == end ||
                (step > 0 && start > end) ||
                (step < 0 && start < end)) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            vector<number_t> range;
            for (; ((step < 0 && start > end) || start < end); start += step) {
                range.emplace_back(start);
            }

            // cannot pass the max number of configured iterations
            if (range.size() / static_cast<uint64_t>(step) > MAX_ITERATION) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            context_.environment_add_or_update(string("range" + id_or_key), range);
            context_.environment_add_or_update(id_or_key, range.at(0));
            context_.stack_push(object_t(string("range" + id_or_key)));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(number_t(0)));
            context_.stack_push(object_t(static_cast<number_t>(context_.get_counter())));
            branches_.push_back(branch{token_types::FOR, true});
        }

        // for item in vector
        // expects only an identifier that represents a vector<number_t>
        // or vector<string>
        else if (value.size() == 0 && it.match(token_types::IDENTIFIER)) {
            string vect = it.look_back().value().value_or("");
            it.next();

            if (!context_.environment_is_key_defined(vect)) {
                error_.critical("variable ", vect, " is not defined. Line: ",
                                it.range().line);
                return false;
            }

            if (context_.environment_get_size(vect) == 0 ||
                context_.environment_get_size(vect) > MAX_ITERATION) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            context_.environment_add_or_update(string(id_or_key + "_idx"), 0);
            context_.environment_add_or_update(vect, id_or_key, 0);
            context_.stack_push(object_t(vect));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(number_t(0)));
            context_.stack_push(object_t(static_cast<number_t>(context_.get_counter())));
            branches_.push_back(branch{token_types::FOR, true});
        }

        // for key, value in table
        // expects only an identifier that represents an
        // unordered_map<number_t> or unordered_map<string>
        else if (value.size() > 0 && it.match(token_types::IDENTIFIER)) {
            string tbl = it.look_back().value().value_or("");
            it.next();

            if (!context_.environment_is_key_defined(tbl)) {
                error_.critical("variable ", tbl, " is not defined. Line: ",
                                it.range().line);
                return false;
            }

            if (context_.environment_get_size(tbl) == 0 ||
                context_.environment_get_size(tbl) > MAX_ITERATION) {
                branches_.push_back(branch{token_types::FOR, false});
                return true;
            }

            number_t index = 0;
            index = context_.environment_add_or_update(tbl, id_or_key, value, index);
            context_.environment_add_or_update(string(id_or_key + "_idx"), 0);
            context_.stack_push(object_t(tbl));
            context_.stack_push(object_t(id_or_key));
            context_.stack_push(object_t(value));
            context_.stack_push(object_t(index));
            context_.stack_push(object_t(static_cast<number_t>(context_.get_counter())));
            branches_.push_back(branch{token_types::FOR, true});
        }
        else {
            error_.critical("invalid loop. Line: ", it.range().line);
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
            error_.critical("endfor doesn't match a for. Line: ",
                            it.range().line);
            return false;
        }

        // get loop parameters
        number_t counter = context_.stack_pop_number_or(0);
        number_t index = context_.stack_pop_number_or(0);
        string value = context_.stack_pop_string_or("");
        string id_or_key = context_.stack_pop_string_or("");
        string identifier = context_.stack_pop_string_or("");

        // endfor is currently looping a unordered_map (key, value)
        if (value.size() > 0) {
            index = context_.environment_add_or_update(identifier,
                                                       id_or_key,
                                                       value,
                                                       index);

            // clean the context after reaching the last item
            if (index >= context_.environment_get_size(identifier)) {
                context_.environment_erase(id_or_key);
                context_.environment_erase(value);
                context_.environment_erase(string(id_or_key + "_idx"));
                branches_.pop_back();
                return true;
            }
        }

        // endfor is currently looping a vector (or range)
        else {
            // clean the context after reaching the last item
            if (++index >= context_.environment_get_size(identifier)) {
                context_.environment_erase(id_or_key);
                context_.environment_erase(string("range" + id_or_key));
                context_.environment_erase(string(id_or_key + "_idx"));
                branches_.pop_back();
                return true;
            }

            // not the last item yet, update the environment
            context_.environment_add_or_update(identifier, id_or_key, index);
        }

        // add 1 to the hidden counter if exists
        context_.environment_increment_value(string(id_or_key + "_idx"));

        // update the data and push them onto the stack
        context_.stack_push(object_t(identifier));
        context_.stack_push(object_t(id_or_key));
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
            error_.critical("if cannot be parsed. Line: ", it.range().line);
            return false;
        }

        bool ret = context_.stack_pop_bool_or(false);
        branches_.push_back(branch{token_types::IF, ret});

        if (inspect_) {
            inspect_(context_, branches_);
        }

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
            error_.critical("expected ENDIF, ELSE, or ELIF. Line: ",
                            it.range().line);
            return false;
        }

        branches_.back().taken = true;
        if (inspect_) {
            inspect_(context_, branches_);
        }

        return true;
    }

    bool compiler::run_elif(parser_iterator &it)
    {
        if (branches_.size() > 0 && branches_.back().taken) {
            it.skip_all();
            return true;
        }

        if (branches_.back().type != token_types::IF) {
            error_.critical("expected ENDIF, ELSE, or ELIF. Line: ",
                            it.range().line);
            return false;
        }

        branches_.pop_back();
        return run_if(it);
    }

    bool compiler::run_endif(parser_iterator &it)
    {
        it.next();

        if (branches_.size() == 0 || branches_.back().type != token_types::IF) {
            error_.critical("expected ENDIF, ELSE, or ELIF. Line: ",
                            it.range().line);
            return false;
        }

        branches_.pop_back();
        return true;
    }

    bool compiler::run_insert(parser_iterator &it, metainfo &info)
    {
        it.next();

        if (branches_.size() > 0 && !branches_.back().taken) {
            it.skip_all();
            return true;
        }

        if (!it.match(token_types::STRING)) {
            error_.critical("expected file name string. Line: ",
                            it.range().line);
            return false;
        }

        string filename = it.look_back().value().value_or("");
        if (!is_readable_file(filename)) {
            error_.critical("template ", filename, " cannot be accessed",
                            "Line: ", it.range().line);
            return false;
        }

        size_t counter = context_.get_counter();
        scan insert_scan(error_);
        insert_scan.do_scan(read_full(filename));
        metainfo &new_info = insert_scan.get_metainfo();

        if (update_main_cache_) {
            cache_[info.hash()].end = counter;
            update_main_cache_ = false;
        }

        auto cache_it = cache_.find(new_info.hash());

        // the block inserted isn't cached: cache it, put the content in
        // the current program and execute it
        if (cache_it == cache_.end()) {

            for (auto &kv : cache_) {
                if (counter >= kv.second.start &&
                        counter <= kv.second.end) {
                    kv.second.end = new_info.size() + counter;
                }
            }

            auto diff = static_cast<metainfo::difference_type>(counter);
            cache_[new_info.hash()] = block_cache{counter - 1,
                                                  new_info.size() + counter,
                                                  0, 1};

            size_t new_size = new_info.size();
            copy(info.begin() + diff + 1, info.end(), back_inserter(new_info));
            info.resize(info.size() + new_size);
            copy(new_info.begin(), new_info.end(), info.begin() + diff);

            // restart the block execution
            context_.jump_to(counter - 1);
        }

        // the block is cached: execute it
        else {

            for (auto &kv : cache_) {
                if (counter >= kv.second.start &&
                        counter <= kv.second.end) {
                    kv.second.end = counter;
                }
            }

            // remove the "insert" code...
            info.remove(counter);

            if (++cache_it->second.iterations > MAX_ITERATION) {
                error_.critical(filename, " has run for more than ",
                                MAX_ITERATION, ", cannot execute it",
                                ". Line:", it.range().line);
                return true;
            }

            // ...and execute the cached code
            cache_it->second.resume = counter - 1;
            context_.jump_to(cache_it->second.start);

            running_cache_ = true;
            current_cache_ = cache_it->first;
        }

        return true;
    }

    bool compiler::parse_expression(parser_iterator &it)
    {
        return parse_equality(it);
    }

    bool compiler::parse_equality(parser_iterator &it)
    {
        if (!parse_logical(it)) {
            return false;
        }

        while (it.match(token_types::EQ)  ||
               it.match(token_types::NE)) {
            token_t oper = it.look_back();
            if (!parse_logical(it)) {
                return false;
            }

            auto result = compute(oper.type(), it.range().line);
            if (result == nullopt) {
                return false;
            }
            context_.stack_push(result.value());
        }

        return true;
    }

    bool compiler::parse_logical(parser_iterator &it)
    {
        if (!parse_comparison(it)) {
            return false;
        }

        while (it.match(token_types::AND) ||
               it.match(token_types::OR)) {
            token_t oper = it.look_back();
            if (!parse_comparison(it)) {
                return false;
            }

            auto result = compute(oper.type(), it.range().line);
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

            auto result = compute(oper.type(), it.range().line);
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

            auto result = compute(oper.type(), it.range().line);
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

            auto result = compute(oper.type(), it.range().line);
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

            auto result = compute_unary(oper.type(), it.range().line);
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
        else if (it.match(token_types::TRUE)) {
            context_.stack_push(object_t(true));
            return true;
        }
        else if (it.match(token_types::FALSE)) {
            context_.stack_push(object_t(false));
            return true;
        }
        else if (it.match(token_types::IDENTIFIER)) {
            string id = it.look_back().value().value_or("");
            if (!context_.environment_is_key_defined(id)) {
                return false;
            }

            // evaluate variable[index] or variable["key"]
            if (it.match(token_types::LEFT_BRACKET)) {
                // push "variable"
                context_.stack_push(object_t(id));

                // push index or "key"
                if (!parse_primary(it)) {
                    return false;
                }

                // pop variable and (index or "key"), look for that
                // variable[x] in the environment and push it onto
                // the stack
                vobject_types tp = context_.stack_top_type();
                if (tp == vobject_types::STRING) {
                    std::string index = context_.stack_pop_string_or("");
                    std::string id = context_.stack_pop_string_or("");
                    if(!context_.stack_push_from_environment(id, index)) {
                        error_.critical(id, "[", index, "] not found",
                                        ". Line: ", it.range().line);
                    }
                }
                else if (tp == vobject_types::NUMBER) {
                    number_t index = context_.stack_pop_number_or(0);
                    std::string id = context_.stack_pop_string_or("");
                    if (!context_.stack_push_from_environment(id, index)) {
                        error_.critical(id, "[", index, "] not found",
                                        ". Line: ", it.range().line);
                    }
                }
                else {
                    context_.stack_pop();
                    context_.stack_push(object_t(std::string("<null>")));
                }

                if (!it.match(token_types::RIGHT_BRACKET)) {
                    error_.critical("expect closing ']'. Line: ",
                                    it.range().line);
                    return false;
                }

                return true;
            }

            // evaluate a simple variable
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
                error_.critical("expected closing ')'. Line: ",
                                it.range().line);
                return false;
            }

            return true;
        }
        else {
            if (it.is_eot()) {
                error_.critical("no token found. Line: ",
                                it.range().line);
            }
            else {
                error_.critical("unexpected token found: ", it.look().type(),
                                ". Line: ", it.range().line);
            }
            return false;
        }
    }

    object compiler::compute_numbers(number_t a,
                                     number_t b,
                                     token_types oper, size_t line)
    {
        switch(oper) {
            case token_types::MINUS:
                return object_t(a - b);

            case token_types::PLUS:
                return object_t(a + b);

            case token_types::SLASH:
                if (b == 0) {
                    error_.critical("cannot divide by 0. Line: ", line);
                    return nullopt;
                }
                return object_t(a / b);

            case token_types::PERCENT:
                if (b == 0) {
                    error_.critical("cannot divide by 0. Line: ", line);
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

            case token_types::AND:
                return object_t(a && b);

            case token_types::OR:
                return object_t(a || b);

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

    object compiler::compute(token_types oper, size_t line)
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
                                   oper, line);
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

        error_.critical("Mismatched types. ", va_type, " [",
                        va.value().get_number_or(0),
                        "] cannot compute with ",
                        vb_type, " [", vb.value().get_number_or(0),
                        "]. Line: ", line);

        return nullopt;
    }

    object compiler::compute_unary(token_types oper, size_t line)
    {
        auto t = context_.stack_pop();
        if (t == nullopt) {
            return nullopt;
        }

        auto type = t.value().get_type();

        if (oper == token_types::MINUS && type == vobject_types::NUMBER) {
            return object_t(t.value().get_number_or(0) * static_cast<uint64_t>(-1));
        }
        else if (oper == token_types::NOT && type == vobject_types::NUMBER) {
            return object_t((t.value().get_number_or(0) == 0) ? true : false);
        }
        else if (oper == token_types::NOT && type == vobject_types::STRING) {
            return object_t((t.value().get_string_or("").size() == 0) ? true : false);
        }

        error_.critical("Unary operator '", oper,
                        "' cannot be used with value '",
                        t.value().to_string(), ". Line: ", line);

        return nullopt;
    }
}
