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

    set_file("code.1");

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

    set_file("code.2");

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

    set_file("code.3");

    int64_t step = 10;
    compiler_.set_callback([&step](const context &ctx,
                                   const vector<branch> &branches) {
        EXPECT_THAT(ctx.environment_check_value("blah", step--), true);
        EXPECT_THAT(branches.back().type, volt::token_types::FOR);
        EXPECT_THAT(branches.back().taken, true);
    });

    compile();
}

TEST_F (compiler_test, invalid_for_range_1_arg)
{
    set_file("code.4");

    compile();

    EXPECT_THAT(error_.get_first_error_msg(), "expect ','");
}

TEST_F (compiler_test, invalid_for_range_2_arg)
{
    set_file("code.5");

    compile();

    EXPECT_THAT(error_.get_first_error_msg(), "expect ','");
}

TEST_F (compiler_test, skip_for_range)
{
    using volt::context;
    using volt::branch;
    using std::vector;

    set_file("code.6");

    compiler_.set_callback([](const context&,
                              const vector<branch> &branches) {
        EXPECT_THAT(branches.back().taken, false);
    });

    compile();
}

/*
TEST (compiler_test, valid_for)
{
    using volt::metatype;

    compiler_mock mock;
    mock.set_file("code.1");
    mock.generate(volt::user_map {{"", ""}});

    EXPECT_CALL(mock, run_test(_, _)).Times(4);
}
*/
