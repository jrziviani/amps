#include "context.h"

namespace volt
{
    using v_number = std::vector<number_t>;
    using v_string = std::vector<std::string>;
    using m_number = std::unordered_map<std::string, number_t>;
    using m_string = std::unordered_map<std::string, std::string>;

    void context::stack_push_from_environment(const std::string &key)
    {
        // simply push the value of environment_[key] onto the stack
        // this method only handles string and number value types
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, number_t> ||
                          std::is_same_v<T, std::string>) {
                stack_push(object_t(var));
            }
        }, environment_[key]);
    }

    void context::stack_push_from_environment(const std::string &key,
                                              size_t index)
    {
        // simply push the value of environment_[key] onto the stack
        // this method handles vector<string> and vector<number_t>
        // values, so the index of that vector is also required
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, v_number> ||
                          std::is_same_v<T, v_string>) {

                // make sure we're not accessing out of bounds item
                if (index >= var.size()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(var.at(index)));
                }
            }
        }, environment_[key]);
    }

    void context::stack_push_from_environment(const std::string &key,
                                              const std::string &user_key)
    {
        // simply push the value of environment_[key] onto the stack
        // this method handles map<string, string> and map<string, number_t>
        // values, so the key of that map is also required
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, m_number> ||
                          std::is_same_v<T, m_string>) {

                // make sure that the key exists
                if (var.find(user_key) == var.end()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(var[user_key]));
                }
            }
        }, environment_[key]);
    }

    void context::environment_add_or_update(const std::string &key,
                                            const user_var &data)
    {
        if (environment_is_key_defined(key)) {
            environment_[key] = data;
            return;
        }

        environment_.insert(std::pair(key, data));
    }

    void context::environment_add_or_update(const std::string &key,
                                            const std::string &dest_key,
                                            size_t index)
    {
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, v_number> ||
                          std::is_same_v<T, v_string>) {
                environment_add_or_update(dest_key, var.at(index));
            }
        }, environment_[key]);
    }

    size_t context::environment_add_or_update(const std::string &key,
                                              const std::string &dest_key,
                                              const std::string &value,
                                              size_t index)
    {
        std::string current_key = "";
        auto it = environment_.find(dest_key);
        if (it != environment_.end()) {
            auto try_string = std::get_if<std::string>(&(it->second));
            if (try_string != nullptr) {
                current_key = *try_string;
            }
        }

        return std::visit([&](auto &&var) -> size_t {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, m_number> ||
                          std::is_same_v<T, m_string>) {

                if (index >= var.bucket_count()) {
                    return 0;
                }

                // set the iterator to the next position based on
                // the last key...
                auto iter = var.begin(index);
                if (current_key.size() > 0) {
                    while (iter != var.end(index) && iter->first != current_key) {
                        ++iter;
                    }

                    if (iter->first == current_key) {
                        ++iter;
                    }
                }

                // ...and look for the next item in hash after that
                size_t idx = index;
                for (; idx < var.bucket_count(); ++idx) {
                    if (idx > index) {
                        iter = var.begin(idx);
                    }

                    for (; iter != var.end(idx); ++iter) {
                        if (iter->first.size() == 0) {
                            continue;
                        }

                        environment_add_or_update(dest_key, iter->first);
                        environment_add_or_update(value, iter->second);
                        return idx;
                    }
                }

                return idx;
            }

            return 0;
        }, environment_[key]);
    }

    bool context::environment_check_value(const std::string &key,
                                          const user_var &value) const
    {
        if (!environment_is_key_defined(key)) {
            return false;
        }

        if (environment_.find(key)->second != value) {
            return false;
        }

        return true;
    }

    size_t context::environment_get_size(const std::string &key) const
    {
        if (!environment_is_key_defined(key)) {
            return 0;
        }

        return std::visit([](const auto var) -> size_t {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, v_number> ||
                          std::is_same_v<T, v_string>) {
                return var.size();
            }
            else if constexpr (std::is_same_v<T, m_number> ||
                               std::is_same_v<T, m_string>) {
                return var.bucket_count();
            }

            return 0;
        }, environment_.find(key)->second);
    }

    void context::environment_increment_value(const std::string &key)
    {
        if (!environment_is_key_defined(key)) {
            return;
        }

        std::visit([](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, number_t>) {
                var++;
            }
        }, environment_[key]);
    }
}
