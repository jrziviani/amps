#ifndef VECTOR_OSTREAM_H
#define VECTOR_OSTREAM_H

#include <string>
#include <streambuf>
#include <cstring>
#include <vector>
#include <iostream>

namespace amps
{
    /*
     * thanks to http://www.voidcn.com/article/p-vjnlygmc-gy.html
     */
    class vector_ostreambuf : public std::streambuf
    {
        constexpr static size_t MAX_RING_SIZE = 256;

        std::vector<std::string> data_;
        char tempbuff_[MAX_RING_SIZE];

    private:
        int overflow(int c) final
        {
            if (c == traits_type::eof()) {
                return c;
            }

            *pptr() = static_cast<char>(c);
            pbump(1);

            if (sync() == 0) {
                return c;
            }

            return traits_type::eof();
        }

        int sync() final
        {
            char *start = pbase();
            ptrdiff_t len = 0;

            std::string str = std::string(start, pptr() - pbase());
            for (char c : str) {
                if (c != '\n') {
                    len++;
                    continue;
                }

                data_.push_back(std::string(start, len));
                start += len + 1;
                len = 0;
            }

            if (len > 0) {
                data_.push_back(std::string(start, len));
            }

            pbump(-(pptr() - pbase()));
            return 0;
        }

    public:
        vector_ostreambuf()
        {
            memset(tempbuff_, 0, sizeof(char) * MAX_RING_SIZE);
            setp(tempbuff_, tempbuff_ + sizeof(char) * MAX_RING_SIZE);
        }

        ~vector_ostreambuf()
        {
            sync();
        }

        vector_ostreambuf(const vector_ostreambuf&) = delete;
        vector_ostreambuf(vector_ostreambuf&&) = delete;
        vector_ostreambuf &operator=(const vector_ostreambuf&) = delete;
        vector_ostreambuf &operator=(vector_ostreambuf&&) = delete;

        const std::vector<std::string> get_errors() const
        {
            return data_;
        }

        std::string get_str(size_t index) const
        {
            if (index >= data_.size()) {
                return std::string("");
            }

            return data_[index];
        }

        std::string get_first_str() const
        {
            if (data_.size() == 0) {
                return std::string("");
            }

            return data_.front();
        }

        std::string get_last_str() const
        {
            if (data_.size() == 0) {
                return std::string("");
            }

            return data_.back();
        }

        void clear()
        {
            data_.clear();
        }
    };
}

#endif // VECTOR_OSTREAM_H
