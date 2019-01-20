#include "scan.h"
#include "config.h"

using namespace std;

namespace amps
{
    scan::scan(error &err)
        : error_(err)
    {
        // KEYWORDS are defined in token.h
        // this X-Macro adds each defined keyword into the map
        keywords_ = {
        #define X(kw, name) {kw, token_types::name},
            KEYWORDS
        #undef X
        };
    }

    void scan::do_scan(const string &content)
    {
        metainfo_.clear();
        parse_block(content);
    }

    void scan::parse_block(const string &content)
    {
        for (size_t i = 0; i < content.size(); ++i) {
            metadata mtdt;

            // any string starting with '{' is a possible code
            // to be scanned, code_block will ensure it
            if (content[i] == '{') {
                mtdt = code_block(content, i);
            }
            else {
                mtdt = text_block(content, i, false);
            }

            metainfo_.add_metadata(mtdt);

            // text blocks don't require special treatment
            if (mtdt.type == metatype::TEXT) {
                continue;
            }

            // on the other hand, code blocks is scanned and
            // tokenized into metainfo
            // NOTE: code block will be reverted to text block
            //       if it finds any issue during this phase
            scan_iterator it(mtdt.data);
            scan_code(it, metainfo_.back());
        }
    }

    metadata scan::code_block(const string &content,
                              size_t &position)
    {
        // a valid code block must respect the following layout
        // {% statement expression %}
        // or
        // {= expression =}
        // IMPORTANT: empty space between opening and closing tags

        metadata metadata = {
            0,
            metatype::CODE,
            {position, position},
            "",
            {},
        };

        // it's not a valid template pattern if != {% or {=
        // so handle it like a regular text
        auto confirm_code = position + 1;
        if (confirm_code < content.size() &&
            content[confirm_code] != TAG_CODE &&
            content[confirm_code] != TAG_ECHO) {
            return text_block(content, position, true);
        }

        // {= is an echo template pattern
        if (confirm_code < content.size() &&
            content[confirm_code] == TAG_ECHO) {
            metadata.type = metatype::ECHO;
        }

        // it's not a valid template pattern if it doesn't have
        // an empty space after the pattern
        ++confirm_code;
        if (confirm_code < content.size() &&
            content[confirm_code] != ' ') {
            return text_block(content, position, true);
        }

        position = ++confirm_code;
        while (position < content.size()) {
            if (content[position] == '\n') {
                line_++;
            }

            // don't stop searching until we find the closing pattern
            if (content[position] == TAG_CLSE) {
                if (metadata.type == metatype::ECHO &&
                    content[position - 1] == TAG_ECHO &&
                    content[position - 2] == ' ') {
                        break;
                }
                else if (metadata.type == metatype::CODE &&
                    content[position - 1] == TAG_CODE &&
                    content[position - 2] == ' ') {
                        break;
                }

                // any closing match problem
                //   i.e. {%  =} or {=  %}
                // is invalid, the whole content will be handled
                // as common text
                else if (content[position - 1] == TAG_ECHO) {
                    error_.log("expects %",
                               line_,
                               metadata.range.start,
                               position);
                    position = confirm_code - 3;
                    return text_block(content, position, true);
                }
                else if (content[position - 1] == TAG_CODE) {
                    error_.log("expects =",
                               line_, 
                               metadata.range.start,
                               position);
                    position = confirm_code - 3;
                    return text_block(content, position, true);
                }
            }
            ++position;
        }

        // line ended without a closing =} or %}
        if (content[position] != '}') {
            position = confirm_code - 2;
            return text_block(content, position, true);
        }

        // {= expression =} is an alias to {% print expression %}
        metadata.range.end = position;
        if (metadata.type == metatype::ECHO) {
            metadata.data = "print ";
        }

        metadata.data += content.substr(confirm_code,
                                        position - confirm_code - 2);

        if (metadata.type == metatype::CODE) {
            if (position + 1 < content.size() &&
                content[position + 1] == '\n') {
                position++;
                line_++;
            }
        }

        return metadata;
    }

    metadata scan::text_block(const string &content,
                              size_t &position,
                              bool force)
    {
        metadata metadata = {
            0,
            metatype::TEXT,
            {position, position},
            "",
            {},
        };

        bool is_blank = true;
        bool is_echo = false;
        size_t initial = position;

        if (force) {
            position++;
            if (initial > 0) {
                --metadata.range.start;
                --initial;
            }
        }

        while (position < content.size() && (content[position] != '{')) {
            if (content[position] == '\n') {
                line_++;
                break;
            }

            if (content[position] != ' ' && content[position] != '\t') {
                is_blank = false;
            }
            ++position;
        }

        if (content[position] == '{') {
            if (position + 1 < content.size() &&
                content[position + 1] == '=') {
                is_echo = true;
            }
            --position;
        }

        if (is_blank && !is_echo) {
            size_t end = (content[initial] == '\n') ? 1 : 0;
            metadata.range.end = initial + end;
            metadata.data = content.substr(initial, end);
        }
        else {
            metadata.range.end = position;
            metadata.data = content.substr(initial, position - initial + 1);
        }

        return metadata;
    }

    void scan::scan_code(const scan_iterator &it, metadata &data)
    {
        while (!it.is_eol()) {
            char token = it.look();
            switch(token) {

            #define X(tk, name) case tk:                          \
                data.add_token(token_t(token_types::name)); \
                it.next();                                            \
                break;
                SINGLE_TOKEN
            #undef X

                case ' ':
                case '\t':
                case '\r':
                    it.next();
                    break;

                case '"':
                    parse_string(it, data);
                    break;

                default:
                    if (isdigit(token)) {
                        parse_number(it, data);
                    }
                    else if (isalpha(token)) {
                        parse_id(it, data);
                    }
                    else {
                        error_.log("unexpected character", token);
                        data.type = metatype::COMMENT;
                        it.next();
                    }
                    break;
            }
        }
    }

    void scan::parse_string(const scan_iterator &it, metadata &data)
    {
        it.match('"');

        size_t start = it.cursor();
        size_t len = 0;

        while (!it.is_eol() && !it.check('"')) {
            ++len;
            it.next();
        }

        if (!it.match('"')) {
            error_.log("expects closing \"");
            data.type = metatype::COMMENT;
            it.skip_all();
            return;
        }

        if (len > MAX_STRING_LEN) {
            error_.log("max string length allowed " +
                       to_string(MAX_STRING_LEN));
            data.type = metatype::COMMENT;
            it.skip_all();
            return;
        }

        string str = (len == 0) ? "" : it.substr(start, len);
        data.add_token(token_t(token_types::STRING, str));
    }

    void scan::parse_number(const scan_iterator &it, metadata &data)
    {
        unsigned long int number = 0;

        while (!it.is_eol() && isdigit(it.look())) {
            int digit = it.look() - '0';
            it.next();

            if (number > (numeric_limits<int>::max() - digit) / 10UL) {
                error_.log("only 32-bit numbers allowed");
                data.type = metatype::COMMENT;
                it.skip_all();
                return;
            }
            number = number * 10 + digit;
        }

        data.add_token(token_t(token_types::NUMBER,
                                         to_string(number)));
    }

    void scan::parse_id(const scan_iterator &it, metadata &data)
    {
        size_t start = it.cursor();
        size_t len = 0;

        while (!it.is_eol() && (isalnum(it.look()) || it.check('_'))) {
            if (len > MAX_VAR_LEN) {
                error_.log("max id length allowed: " +
                           to_string(MAX_VAR_LEN));
                data.type = metatype::COMMENT;
                it.skip_all();
                return;
            }
            ++len;
            it.next();
        }

        const string &text = it.substr(start, len);
        if (keywords_.find(text) != keywords_.end()) {
            data.add_token(token_t(keywords_[text]));
        }
        else {
            data.add_token(token_t(token_types::IDENTIFIER, text));
        }
    }
}
