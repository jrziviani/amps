#ifndef ERROR_H
#define ERROR_H

#include <iostream>

namespace volt
{
    class error
    {

    private:
        template <typename T>
        void log_(const T &msg)
        {
            std::cerr << msg << "\n";
        }

        template <typename T, typename... Ts>
        void log_(const T &msg, const Ts... msgs)
        {
            std::cerr << msg << " ";
            log(msgs...);
        }

    public:
        template <typename... Ts>
        void log(const Ts... msgs)
        {
            log_(msgs...);
        }
    };
}

#endif // ERROR_H
