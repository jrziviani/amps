#include "../src/scan.h"
#include "../src/error.h"

#include <string>
#include <array>
#include <fstream>

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

    scan_test() :
        scan_(error_)
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F (scan_test, invalid_to_text_blocks)
{
    using volt::metatype;

    std::ifstream file("block.1");
    for (std::string content; std::getline(file, content); ) {
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();

        EXPECT_EQ(data[0].type, metatype::TEXT);
    }
}

TEST_F (scan_test, valid_and_invalid_blocks_all)
{
    using volt::metatype;
    using testing::StartsWith;

    std::array<metatype, 28> expected = {
        metatype::CODE, metatype::TEXT, metatype::CODE, metatype::TEXT, metatype::CODE,
        metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::ECHO, metatype::CODE, metatype::CODE, metatype::CODE,
        metatype::CODE, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::CODE, metatype::ECHO, metatype::ECHO, metatype::ECHO,
        metatype::TEXT, metatype::TEXT,
    };

    std::ifstream file("block.2");

    std::string content(512 + 1, '\0');
    file.read(&content[0], static_cast<std::streamsize>(512));
    scan_.do_scan(content);
    auto &data = scan_.get_metainfo();

    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(data[i].type, expected[i]);
    }
}

TEST_F (scan_test, valid_and_invalid_blocks)
{
    using volt::metatype;
    using testing::StartsWith;

    std::array<metatype, 28> expected = {
        metatype::CODE, metatype::TEXT, metatype::CODE, metatype::TEXT, metatype::CODE,
        metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::ECHO, metatype::CODE, metatype::CODE, metatype::CODE,
        metatype::CODE, metatype::TEXT, metatype::TEXT, metatype::TEXT, metatype::TEXT,
        metatype::TEXT, metatype::CODE, metatype::ECHO, metatype::ECHO, metatype::ECHO,
        metatype::TEXT, metatype::TEXT,
    };

    std::array<std::string, 28> errors_expected = {
        "", "max string length allowed 256", "", "only 32-bit numbers allowed", "",
        "unexpected character", "", "", "", "",
        "", "", "", "", "",
        "", "", "expects =", "expects %", "expects =",
        "only 32-bit numbers allowed", "", "", "", "",
        "max id length allowed:", "",
    };

    std::ifstream file("block.2");

    size_t i = 0;
    for (std::string content; std::getline(file, content); ++i) {
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();

        EXPECT_EQ(data[0].type, expected[i]);
        if (errors_expected[i].size() > 0 || error_.get_last_error_msg().size() > 0) {
            EXPECT_GT(error_.get_last_error_msg().size(), 0);
            EXPECT_GT(errors_expected[i].size(), 0);
            EXPECT_THAT(error_.get_last_error_msg(), StartsWith(errors_expected[i]));
        }
        error_.clear();
    }
}

TEST_F (scan_test, find_code_between_trash)
{
    using volt::metatype;

    std::ifstream file("block.3");
    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 4);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::ECHO);
        EXPECT_EQ(data[2].type, metatype::TEXT);
        EXPECT_EQ(data[3].type, metatype::CODE);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].type, metatype::CODE);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].type, metatype::CODE);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::CODE);
        EXPECT_EQ(data[2].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::ECHO);
        EXPECT_EQ(data[2].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::ECHO);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::CODE);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::ECHO);
        EXPECT_EQ(data[2].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::CODE);
        EXPECT_EQ(data[2].type, metatype::TEXT);
    }

    { //10
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 3);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::CODE);
        EXPECT_EQ(data[2].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].type, metatype::TEXT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 2);
        EXPECT_EQ(data[0].type, metatype::TEXT);
        EXPECT_EQ(data[1].type, metatype::TEXT);
    }
}

TEST_F (scan_test, test_scan)
{
    using volt::token_types;

    std::ifstream file("block.4");
    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 2);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 8);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::STAR);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[4].type(), token_types::SLASH);
        EXPECT_EQ(data[0].tokens[5].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[6].type(), token_types::PLUS);
        EXPECT_EQ(data[0].tokens[7].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::PERCENT);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::STAR);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::PLUS);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::SLASH);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::STRING);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::SLASH);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::STAR);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 1);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::FOR);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::IN);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::IF);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::NE);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 4);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::ELIF);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::EQ);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 2);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::INSERT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 2);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::INSERT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::NUMBER);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 1);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::ENDIF);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 1);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::ELSE);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 1);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::ENDFOR);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 5);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::LEFT_BRACKET);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[4].type(), token_types::RIGHT_BRACKET);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 5);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::LEFT_BRACKET);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[4].type(), token_types::RIGHT_BRACKET);
    }

    {
        std::string content;
        std::getline(file, content);
        scan_.do_scan(content);
        auto &data = scan_.get_metainfo();
        EXPECT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].tokens.size(), 11);
        EXPECT_EQ(data[0].tokens[0].type(), token_types::PRINT);
        EXPECT_EQ(data[0].tokens[1].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[2].type(), token_types::LEFT_BRACKET);
        EXPECT_EQ(data[0].tokens[3].type(), token_types::STRING);
        EXPECT_EQ(data[0].tokens[4].type(), token_types::RIGHT_BRACKET);
        EXPECT_EQ(data[0].tokens[5].type(), token_types::LEFT_BRACKET);
        EXPECT_EQ(data[0].tokens[6].type(), token_types::NUMBER);
        EXPECT_EQ(data[0].tokens[7].type(), token_types::RIGHT_BRACKET);
        EXPECT_EQ(data[0].tokens[8].type(), token_types::LEFT_BRACKET);
        EXPECT_EQ(data[0].tokens[9].type(), token_types::IDENTIFIER);
        EXPECT_EQ(data[0].tokens[10].type(), token_types::RIGHT_BRACKET);
    }
}
