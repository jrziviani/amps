#include "../src/compiler.h"
#include "../src/error.h"
#include "../src/types.h"
#include "../src/context.h"

#include <string>
#include <array>
#include <fstream>
#include <functional>

class compiler_test : public ::testing::Test
{
protected:
    volt::error error_;
    volt::scan scan_;
    volt::compiler compiler_;

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
        compiler_.generate(scan_.get_metainfo(), volt::user_map {{"", ""}});
    }

    void compile(const volt::user_map &usermap)
    {
        compiler_.generate(scan_.get_metainfo(), usermap);
    }
};

TEST_F (compiler_test, valid_for_range)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.for.1");

    int64_t step = 0;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("val", step++), true);
        EXPECT_THAT(branches.back().type, volt::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
    });

    compile();
}

TEST_F (compiler_test, valid_for_range_neg)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.for.2");

    int64_t step = -6;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("ident", step), true);
        EXPECT_THAT(branches.back().type, volt::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
        step += 2;
    });

    compile();
}

TEST_F (compiler_test, valid_for_range_reverse)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.for.3");

    int64_t step = 10;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("blah", step--), true);
        EXPECT_THAT(branches.back().type, volt::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
    });

    compile();
}

TEST_F (compiler_test, invalid_for)
{
    set_file("code.for.4");
    compile();

    EXPECT_THAT(error_.get_first_error_msg(), "expect ','");
    EXPECT_THAT(error_.get_error_msg(4), "expect ','");
    EXPECT_THAT(error_.get_error_msg(8), "expect 'in' operator after identifier");
    EXPECT_THAT(error_.get_error_msg(12), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(16), "expect '('");
    EXPECT_THAT(error_.get_error_msg(20), "unexpected token found: RIGHT_PAREN");
    EXPECT_THAT(error_.get_error_msg(24), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(28), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(32), "loop statement requires an identifier");
    EXPECT_THAT(error_.get_error_msg(36), "range expects only numbers");
    EXPECT_THAT(error_.get_error_msg(40), "expected closing ')'");
    EXPECT_THAT(error_.get_error_msg(44), "expected an identifier after ','");
    EXPECT_THAT(error_.get_error_msg(48), "variable val already exists, name must be unique");
    EXPECT_THAT(error_.get_error_msg(52), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(56), "variable test is not defined");
    EXPECT_THAT(error_.get_error_msg(60), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(64), "invalid loop");
    EXPECT_THAT(error_.get_error_msg(68), "variable val already exists, name must be unique");
    EXPECT_THAT(error_.get_error_msg(72), "endfor doesn't match a for");
    EXPECT_THAT(error_.get_error_msg(74), "expected endfor");
}

TEST_F (compiler_test, nested_range)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.for.5");

    int64_t step1 = -1;
    int64_t step2 = 0;
    compiler_.set_callback([&step1, &step2](const context &ctx,
                                            const vector<branch> &branches) {
        if (ctx.environment_is_key_defined("bleh")) {
            EXPECT_THAT(ctx.environment_check_value("bleh", step2++), true);
            EXPECT_THAT(branches.back().type, volt::token_types::FOR);
            EXPECT_THAT(branches.back().taken, true);
        }
        else {
            step2 = 0;
            ++step1;
            EXPECT_THAT(branches.back().type, volt::token_types::FOR);
            EXPECT_THAT(branches.back().taken, true);
        }
        EXPECT_THAT(ctx.environment_check_value("blah", step1), true);
    });

    compile();
}

TEST_F (compiler_test, skip_for_range)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.for.6");

    compiler_.set_callback([](const context&,
                              const vector<branch> &branches) {
        EXPECT_THAT(branches.back().taken, false);
    });

    compile();
}

TEST_F (compiler_test, test_print)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.print.1");

    compile();
}

/*
TEST (compiler_test, valid_for)
{
    using volt::metatype;

    compiler_mock mock;
    mock.set_file("code.for.1");
    mock.generate(volt::user_map {{"", ""}});

    EXPECT_CALL(mock, run_test(_, _)).Times(4);
}
*/
