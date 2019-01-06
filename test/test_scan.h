#include "../src/scan.h"
#include "../src/error.h"

#include <string>
#include <array>
#include <fstream>

const unsigned int MAX_TEST_FILES = 10;

class mock_error : public volt::error
{
public:
    mock_error()
    {
    }
};

class scan_test : public ::testing::Test
{
protected:
    mock_error error_;
    volt::scan scan_;
    std::array<std::ifstream, MAX_TEST_FILES> files_;

    scan_test() :
        scan_(error_)
    {
    }

    void SetUp() override
    {
        files_[0].open("block.1");
        files_[1].open("block.2");
    }

    void TearDown() override
    {
        for (std::ifstream &file : files_) {
            if (file.is_open()) {
                file.close();
            }
        }
    }
};

TEST_F (scan_test, invalid_to_text_blocks)
{
    using volt::metatype;

    size_t i = 0;
    for (std::string content; std::getline(files_[0], content); ) {
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();

        EXPECT_EQ(data[0].type, metatype::TEXT);
    }
}

TEST_F (scan_test, valid_and_invalid_blocks)
{
    using volt::metatype;
    std::array<metatype, 25> expected = {
        metatype::CODE, metatype::CODE, metatype::TEXT, metatype::TEXT, metatype::CODE,
        metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::CODE, metatype::CODE,
        metatype::CODE, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::ECHO, metatype::ECHO,
    };

    size_t i = 0;
    for (std::string content; std::getline(files_[1], content); ) {
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();

        EXPECT_EQ(data[0].type, expected[i++]);
    }
}
