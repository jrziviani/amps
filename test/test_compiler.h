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
        compiler_.set_callback(std::bind(&compiler_test::check_state,
                                         this,
                                         std::placeholders::_1));
    }

    void check_state(const volt::context &ctx)
    {
        //std::cout << ctx.get_counter() << std::endl;
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

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F (compiler_test, valid_for)
{
    using volt::metatype;

    set_file("code.1");
    compile();
}

