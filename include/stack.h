#ifndef STACK_H
#define STACK_H

#include "types.h"

namespace amps
{
    class gstack
    {
        objects stack_;

    public:
        gstack()                        = default;
        gstack(const gstack&)           = delete;
        gstack(gstack&&)                = delete;
        ~gstack()                       = default;

        gstack operator=(const gstack&) = delete;
        gstack operator=(gstack&&)      = delete;

        object pop();
        object look_back() const;
        void push(object_t value);
        void clear();
        bool empty() const;
    };

    inline void gstack::push(object_t value)
    {
        stack_.emplace_back(value);
    }

    inline object gstack::pop()
    {
        if (empty()) {
            return {};
        }

        object_t value = stack_.back();
        stack_.pop_back();
        return value;
    }

    inline object gstack::look_back() const
    {
        return stack_.back();
    }

    inline bool gstack::empty() const
    {
        return stack_.size() == 0;
    }

    inline void gstack::clear()
    {
        stack_.clear();
    }
}

#endif // STACK_H
