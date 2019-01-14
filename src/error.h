#ifndef ERROR_H
#define ERROR_H

#include <vector>
#include <string>
#include <sstream>
#include <functional>

namespace volt
{
    class error
    {
        std::vector<std::string> errors_;
        std::stringstream stream_;

    private:
        template <typename T>
        void log_(const T &msg)
        {
            stream_ << msg;
            errors_.emplace_back(stream_.str());
            stream_.str(std::string());
        }

        template <typename T, typename... Ts>
        void log_(const T &msg, const Ts... msgs)
        {
            stream_ << msg << " ";
            log(msgs...);
        }

    public:
        template <typename... Ts>
        void log(const Ts... msgs)
        {
            log_(msgs...);
        }

        std::string get_last_error_msg()
        {
            if (errors_.size() == 0) {
                return "";
            }

            return errors_.back();
        }

        std::string get_first_error_msg()
        {
            if (errors_.size() == 0) {
                return "";
            }

            return errors_[0];
        }

        std::string get_error_msg(size_t index)
        {
            if (errors_.size() <= index) {
                return "";
            }

            return errors_[index];
        }

        void for_each(std::function<void(const std::string&)> fn)
        {
            for (const std::string &err : errors_) {
                fn(err);
            }
        }

        void clear()
        {
            errors_.clear();
        }
    };
}

#endif // ERROR_H
