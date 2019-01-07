#ifndef SCAN_H
#define SCAN_H

#include "error.h"
#include "types.h"

#include <unordered_map>
#include <string>

namespace volt
{
    class scan_iterator;

    class scan
    {
        std::unordered_map<std::string, token_types> keywords_;
        metainfo metainfo_;
        std::string file_;
        uint16_t line_;
        error &error_;

    private:
        metadata code_block(const std::string &content,
                            size_t &position);
        metadata text_block(const std::string &content,
                            size_t &position,
                            bool force);
        void parse_block(const std::string &content,
                         std::streamsize size);

        void scan_code(const scan_iterator &it, metadata &data);
        void parse_string(const scan_iterator &it, metadata &data);
        void parse_number(const scan_iterator &it, metadata &data);
        void parse_id(const scan_iterator &it, metadata &data);

    public:
        scan(error &err);
        ~scan() = default;

        scan(const scan&)           = delete;
        scan(scan&&)                = delete;
        scan &operator=(vobject &)  = delete;
        scan &operator=(vobject &&) = delete;

        void do_scan(const std::string &content);
        const metainfo &get_metainfo() const;
    };

    inline const metainfo &scan::get_metainfo() const
    {
        return metainfo_;
    }

    class scan_iterator
    {
        friend class scan;

        const std::string &data_;
        mutable size_t cursor_;

        scan_iterator(const std::string &data) :
            data_(data),
            cursor_(0)
        {
        }

        bool is_eol() const
        {
            return (cursor_ >= data_.size());
        }

        size_t cursor() const
        {
            return cursor_;
        }

        char look() const
        {
            return data_[cursor_];
        }

        void skip_all() const
        {
            while (!is_eol()) {
                next();
            }
        }

        bool next() const
        {
            if (cursor_ > data_.size() - 1) {
                return false;
            }

            ++cursor_;
            return true;
        }

        bool check(char c) const
        {
            return (!is_eol() && c == look());
        }

        bool match(char c) const
        {
            if (!check(c)) {
                return false;
            }

            if (!next()) {
                return false;
            }

            return true;
        }

        std::string substr(size_t start, size_t len) const
        {
            if (len == 0) {
                len = data_.size();
            }

            return std::string(data_, start, len);
        }
    };
}

#endif // SCAN_H
