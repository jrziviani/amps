#ifndef STACK_H
#define STACK_H

#include "types.h"

namespace volt
{
    class gstack
    {
        objects stack_;

    public:
        gstack()                    = default;
        gstack(const gstack&)       = delete;
        gstack(gstack&&)            = delete;
        ~gstack()                   = default;

        gstack &operator=(gstack&)  = delete;
        gstack &operator=(gstack&&) = delete;

        object pop();
        object look_back_at(int index);
        object look_back();
        void push(object_t value);
        void clear();
        bool empty();
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

    inline object gstack::look_back_at(int index)
    {
        if (empty() || index > 0 || stack_.size() <= -index) {
            return {};
        }

        object_t value = stack_[stack_.size() - 1 + index];
        return value;
    }

    inline object gstack::look_back()
    {
        return look_back_at(0);
    }

    inline bool gstack::empty()
    {
        return stack_.size() == 0;
    }

    inline void gstack::clear()
    {
        stack_.clear();
    }
}

#endif // STACK_H
