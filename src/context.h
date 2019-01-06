#ifndef CONTEXT_H
#define CONTEXT_H

#include "stack.h"
#include "types.h"

#include <cstdarg>

namespace volt
{
    class context
    {
        user_map environment_;
        gstack stack_;
        size_t counter_;

    public:
        context() :
            counter_(0)
        {
        }

        ~context()                    = default;
        context(const context&)       = delete;
        context(context&&)            = delete;

        context &operator=(context&)  = delete;
        context &operator=(context&&) = delete;

        // methods to handle the context (program) counter
        void jump_to(size_t n);
        const size_t &get_counter() const;

        // methods to handle the context stack
        vobject_types stack_top_type();
        std::string stack_pop_string_or(const std::string &opt);
        number_t stack_pop_number_or(number_t opt);
        object stack_pop();
        bool stack_pop_bool_or(bool opt);
        void stack_push(const object_t &obj);
        void stack_clear();
        void stack_push_from_environment(const std::string &key);
        void stack_push_from_environment(const std::string &key,
                                         size_t index);
        void stack_push_from_environment(const std::string &key,
                                         const std::string &user_key);

        // methods to handle the context environment lookup table
        bool environment_is_key_defined(const std::string &key);
        void environment_setup(const user_map &data);
        void environment_add_or_update(const std::string &key,
                                       const user_var &data);
        void environment_add_or_update(const std::string &key,
                                       const std::string &dest_key,
                                       size_t index);
        void environment_erase(const std::string &key);
        size_t environment_get_size(const std::string &key);
        size_t environment_add_or_update(const std::string &key,
                                         const std::string &dest_key,
                                         const std::string &value,
                                         size_t index);
    };

    inline void context::jump_to(size_t n)
    {
        counter_ = n;
    }

    inline const size_t &context::get_counter() const
    {
        return counter_;
    }

    inline vobject_types context::stack_top_type()
    {
        return stack_.look_back().value().get_type();
    }

    inline object context::stack_pop()
    {
        return stack_.pop();
    }

    inline void context::stack_push(const object_t &obj)
    {
        stack_.push(obj);
    }

    inline void context::stack_clear()
    {
        stack_.clear();
    }

    inline std::string context::stack_pop_string_or(const std::string &opt)
    {
        object result = stack_.pop();
        if (result == std::nullopt) {
            return opt;
        }

        return result.value().get_string_or(opt);
    }

    inline number_t context::stack_pop_number_or(number_t opt)
    {
        object result = stack_.pop();
        if (result == std::nullopt) {
            return opt;
        }

        return result.value().get_number_or(opt);
    }

    inline bool context::stack_pop_bool_or(bool opt)
    {
        object result = stack_.pop();
        if (result == std::nullopt) {
            return opt;
        }

        return result.value().get_bool_or(opt);
    }

    inline void context::environment_setup(const user_map &data)
    {
        environment_ = data;
    }

    inline bool context::environment_is_key_defined(const std::string &key)
    {
        if (environment_.find(key) != environment_.end()) {
            return true;
        }

        return false;
    }

    inline void context::environment_erase(const std::string &key)
    {
        if (environment_is_key_defined(key)) {
            environment_.erase(environment_.find(key));
        }
    }
}

#endif // CONTEXT_H
