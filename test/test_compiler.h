#include "../include/compiler.h"
#include "../include/error.h"
#include "../include/types.h"
#include "../include/context.h"

#include <string>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

#ifndef DEBUG
    #define disable_stdout(fn)                       \
        std::stringstream ss;                        \
        std::streambuf *std_out = std::cout.rdbuf(); \
        std::cout.rdbuf(ss.rdbuf());                 \
        fn;                                          \
        std::cout.rdbuf(std_out);
#else
    #define disable_stdout(fn)                       \
        fn;
#endif

class compiler_test : public ::testing::Test
{
protected:
    amps::error error_;
    amps::scan scan_;
    amps::compiler compiler_;

    compiler_test() :
        scan_(error_),
        compiler_(error_)
    {
    }

    void set_file(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string content(1024 + 1, '\0');

        while (true) {
            file.read(&content[0], static_cast<std::streamsize>(1024));
            scan_.do_scan(content);

            if (file.eof()) {
                break;
            }
        }
    }

    void compile()
    {
        compiler_.generate(scan_.get_metainfo(), amps::user_map {{"", ""}});
    }

    void compile(const amps::user_map &usermap)
    {
        compiler_.generate(scan_.get_metainfo(), usermap);
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F (compiler_test, block_test)
{
    set_file("block.1");
    disable_stdout(compile());
}

TEST_F (compiler_test, valid_for_range)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.for.1");

    int64_t step = 0;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("val", step++), true);
        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, valid_for_range_neg)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.for.2");

    int64_t step = -6;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("ident", step), true);
        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
        step += 2;
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, valid_for_range_reverse)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.for.3");

    int64_t step = 10;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("blah", step--), true);
        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, invalid_for)
{
    set_file("code.for.4");

    disable_stdout(compile());

    EXPECT_THAT(error_.get_first_error_msg(), "expect ','");
    EXPECT_THAT(error_.get_error_msg(2), "expect ','");
    EXPECT_THAT(error_.get_error_msg(4), "expect 'in' operator after identifier");
    EXPECT_THAT(error_.get_error_msg(6), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(8), "expect '('");
    EXPECT_THAT(error_.get_error_msg(10), "unexpected token found: RIGHT_PAREN");
    EXPECT_THAT(error_.get_error_msg(12), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(14), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(16), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(18), "range expects only numbers");
    EXPECT_THAT(error_.get_error_msg(20), "expected closing ')'");
    EXPECT_THAT(error_.get_error_msg(22), "expected an identifier after ','");
    EXPECT_THAT(error_.get_error_msg(24), "variable val already exists, name must be unique");
    EXPECT_THAT(error_.get_error_msg(26), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(28), "variable test is not defined");
    EXPECT_THAT(error_.get_error_msg(30), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(32), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(34), "variable val already exists, name must be unique");
    EXPECT_THAT(error_.get_error_msg(36), "endfor doesn't match a for");
    EXPECT_THAT(error_.get_error_msg(37), "expected endfor");
}

TEST_F (compiler_test, nested_range)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.for.5");

    int64_t step1 = -1;
    int64_t step2 = 0;
    compiler_.set_callback([&step1, &step2](const context &ctx,
                                            const vector<branch> &branches) {
        if (ctx.environment_is_key_defined("bleh")) {
            EXPECT_THAT(ctx.environment_check_value("bleh", step2++), true);
            EXPECT_THAT(branches.back().type, amps::token_types::FOR);
            EXPECT_THAT(branches.back().taken, true);
        }
        else {
            step2 = 0;
            ++step1;
            EXPECT_THAT(branches.back().type, amps::token_types::FOR);
            EXPECT_THAT(branches.back().taken, true);
        }
        EXPECT_THAT(ctx.environment_check_value("blah", step1), true);
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, skip_for_range)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.for.6");

    compiler_.set_callback([](const context&,
                              const vector<branch> &branches) {
        EXPECT_THAT(branches.back().taken, false);
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, test_for_vector)
{
    using amps::context;
    using amps::branch;
    using amps::user_map;
    using std::vector;
    using std::string;

    set_file("code.for.7");

    vector<string> cities = {"São Paulo", "NYC", "Paris", "London", "Lisbon"};

    int64_t step = 0;
    compiler_.set_callback([&step, &cities](const context &ctx,
                                   const vector<branch> &branches) {
        if (!ctx.environment_is_key_defined("var")) {
            return;
        }

        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
        EXPECT_THAT(ctx.environment_check_value("var", cities[step++]), true);
    });

    user_map um {{"vect", cities}};
    disable_stdout(compile(um));

    EXPECT_THAT(error_.get_error_msg(0), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(0), "loop statement requires an identifier");
}

TEST_F (compiler_test, test_for_empty_vector)
{
    using amps::context;
    using amps::branch;
    using amps::user_map;
    using std::vector;
    using std::string;

    set_file("code.for.8");

    vector<string> cities = {};

    int64_t step = 0;
    compiler_.set_callback([&step, &cities](const context &,
                                   const vector<branch> &branches) {
        if (step > 0) {
            return;
        }

        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, false);
        ++step;
    });

    user_map um {{"vect", cities}};
    disable_stdout(compile(um));

    EXPECT_THAT(error_.get_error_msg(0), "variable voct is not defined");
}

TEST_F (compiler_test, test_for_map)
{
    using amps::context;
    using amps::branch;
    using amps::user_map;
    using std::unordered_map;
    using std::vector;
    using std::string;

    set_file("code.for.9");

    unordered_map<string, string> capitals = {
        {"Brasil", "Brasilia"},
        {"USA", "Washington"},
        {"France", "Paris"},
        {"England", "London"},
        {"Portugal", "Lisbon"}};

    vector keys = {"Brasil", "USA", "France", "England", "Portugal"};
    vector values = {"Brasilia", "Washington", "Paris", "London", "Lisbon"};

    compiler_.set_callback([&capitals, &keys, &values](
                const context &ctx,
                const vector<branch> &branches) {
        if (!ctx.environment_is_key_defined("key") ||
            !ctx.environment_is_key_defined("value")) {
            return;
        }

        EXPECT_THAT(branches.back().type, amps::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);

        for (auto it = keys.begin(); it != keys.end(); ++it) {
            if (ctx.environment_check_value("key", *it)) {
                EXPECT_THAT(ctx.environment_check_value("value", capitals[*it]), true);
                keys.erase(it);
                break;
            }
        }
    });

    user_map um {{"dict", capitals}};
    disable_stdout(compile(um));

    EXPECT_THAT(keys.size(), 0);
    EXPECT_THAT(error_.get_error_msg(0), "expect 'in' operator after identifier");
    EXPECT_THAT(error_.get_error_msg(2), "variable doct is not defined");
}

TEST_F (compiler_test, test_if)
{
    using amps::context;
    using amps::branch;
    using std::vector;

    set_file("code.if.1");

    int64_t step = 0;
    compiler_.set_callback([&step](const context &,
                                   const vector<branch> &branches) {
        switch (step) {
            case 0:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, true);
                break;

            case 1:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 2:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, true);
                break;

            case 3:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 4:
                EXPECT_THAT(branches.back().type, amps::token_types::IF); // ELSE
                EXPECT_THAT(branches.back().taken, true);
                break;

            case 5:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 6:
                EXPECT_THAT(branches.back().type, amps::token_types::IF); // ELIF
                EXPECT_THAT(branches.back().taken, true);
                break;

            case 7:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 8:
                EXPECT_THAT(branches.back().type, amps::token_types::IF); // ELIF
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 9:
                EXPECT_THAT(branches.back().type, amps::token_types::IF); // ELSE
                EXPECT_THAT(branches.back().taken, true);
                break;
            }
            step++;
    });

    disable_stdout(compile());
}

TEST_F (compiler_test, test_if_variables)
{
    using amps::context;
    using amps::branch;
    using amps::user_map;
    using std::unordered_map;
    using std::vector;
    using std::string;

    set_file("code.if.2");

    unordered_map<string, string> capitals = {
        {"Brasil", "Brasilia"},
        {"USA", "Washington"},
        {"France", "Paris"},
        {"England", "London"},
        {"Portugal", "Lisbon"}};

    vector<string> values = {"Brasilia", "Washington", "Paris", "London", "Lisbon"};

    int64_t step = 0;
    compiler_.set_callback([&capitals, &values, &step](
                const context &ctx,
                const vector<branch> &branches) {
        if (!ctx.environment_is_key_defined("map") ||
            !ctx.environment_is_key_defined("vec")) {
            return;
        }

        switch (step) {
            case 0:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, true);
                break;

            case 1:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 2:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;

            case 3:
                EXPECT_THAT(branches.back().type, amps::token_types::IF);
                EXPECT_THAT(branches.back().taken, false);
                break;
        }

        step++;
    });

    user_map um {{"map", capitals}, {"vec", values}};
    disable_stdout(compile(um));

    EXPECT_THAT(error_.get_error_msg(0), "map [ error ] not found");
    EXPECT_THAT(error_.get_error_msg(1), "vec [ 80 ] not found");
    EXPECT_THAT(error_.get_error_msg(2), "unexpected token found: MINUS");
}

TEST_F (compiler_test, test_insert)
{
    using amps::context;
    using amps::branch;
    using amps::user_map;
    using std::unordered_map;
    using std::vector;
    using std::string;

    set_file("code.insert.1");

    unordered_map<string, string> capitals = {
        {"Brasil", "Brasilia"},
        {"USA", "Washington"},
        {"France", "Paris"},
        {"England", "London"},
        {"Portugal", "Lisbon"}};

    vector<string> values = {"Brasilia", "Washington", "Paris", "London", "Lisbon"};

    int64_t step = 0;
    compiler_.set_callback([&capitals, &values, &step](
                const context &ctx,
                const vector<branch> &) {
        if (!ctx.environment_is_key_defined("map") ||
            !ctx.environment_is_key_defined("vec")) {
            return;
        }

        switch (step) {

        }
        step++;
    });

    user_map um {{"map", capitals}, {"vec", values}};
    disable_stdout(compile(um));
}

TEST_F (compiler_test, test_print)
{
    using amps::context;
    using amps::branch;
    using amps::number_t;
    using amps::object;
    using amps::vobject_types;
    using std::vector;

    set_file("code.print.1");

    int64_t step = 0;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &) {
        switch (step) {
            case 0:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::NUMBER);
                EXPECT_THAT(ctx.stack_top().value().get_number_or(0), number_t(1));
                break;

            case 1:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::NUMBER);
                EXPECT_THAT(ctx.stack_top().value().get_number_or(0), number_t(10));
                break;

            case 2:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::NUMBER);
                EXPECT_THAT(ctx.stack_top().value().get_number_or(0), number_t(6));
                break;

            case 3:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::NUMBER);
                EXPECT_THAT(ctx.stack_top().value().get_number_or(0), number_t(-48));
                break;

            case 4: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 5: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 6:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 7:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 8:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 9:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 10:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 11:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 12:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 13:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 14:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::STRING);
                EXPECT_THAT(ctx.stack_top().value().get_string_or(""), std::string("valval"));
                break;

            case 15: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 16: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 17:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 18:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 19:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 20:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 21: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 22: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 23:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 24:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 25: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 26: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 27: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;

            case 28:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::NUMBER);
                EXPECT_THAT(ctx.stack_top().value().get_number_or(0), 7);
                break;

            case 29:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 30:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), false);
                break;

            case 31:
                EXPECT_THAT(ctx.stack_top_type(), vobject_types::BOOL);
                EXPECT_THAT(ctx.stack_top().value().get_bool_or(false), true);
                break;

            case 32: //null
                EXPECT_THAT(ctx.stack_empty(), true);
                break;
        }
        ++step;
    });

    disable_stdout(compile());
}
