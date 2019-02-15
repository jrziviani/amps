#ifndef ERROR_H
#define ERROR_H

#include <vector>
#include <string>
#include <functional>
#include <iostream>

namespace amps
{
    class error
    {
        std::ostream &stream_;

    public:
        error(std::ostream &stream) :
            stream_(stream)
        {
        }

        error() :
            stream_(std::cerr)
        {
        }

        template <typename T, typename... Ts>
        void log(const T &msg, const Ts... msgs)
        {
            if constexpr (sizeof...(Ts) > 0) {
                stream_ << msg << " ";
                log(msgs...);
            }
            else {
                stream_ << msg << std::flush;
            }
        }
    };
}

#endif // ERROR_H
