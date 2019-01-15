#include "context.h"

namespace volt
{
    void context::stack_push_from_environment(const std::string &key)
    {
        // get the variable 'environment_[key]' and push it
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, uint64_t> ||
                          std::is_same_v<T, std::string>) {
                stack_push(object_t(var));
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                stack_push(object_t(static_cast<uint64_t>(var)));
            }
        }, environment_[key]);
    }

    void context::stack_push_from_environment(const std::string &key, size_t index)
    {
        // get the variable 'environment_[key][index]' and push it
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, std::vector<uint64_t>> ||
                          std::is_same_v<T, std::vector<std::string>>) {
                if (index >= var.size()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(var.at(index)));
                }
            }
            else if constexpr (std::is_same_v<T, std::vector<int64_t>>) {
                if (index >= var.size()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(static_cast<uint64_t>(var.at(index))));
                }
            }
        }, environment_[key]);
    }

    void context::stack_push_from_environment(const std::string &key, const std::string &user_key)
    {
        // get the variable 'environment_[key][user_key]' and push it
        std::visit([&](auto &&var) {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, std::unordered_map<std::string, uint64_t>> ||
                          std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
                if (var.find(user_key) == var.end()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(var[user_key]));
                }
            }
            else if constexpr (std::is_same_v<T, std::unordered_map<std::string, int64_t>>) {
                if (var.find(user_key) == var.end()) {
                    stack_push(object_t(std::string("<null>")));
                }
                else {
                    stack_push(object_t(static_cast<uint64_t>(var[user_key])));
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

            if constexpr (std::is_same_v<T, std::vector<uint64_t>> ||
                          std::is_same_v<T, std::vector<int64_t>>  ||
                          std::is_same_v<T, std::vector<std::string>>) {
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
        auto current_it = environment_.find(dest_key);
        if (current_it != environment_.end()) {
            auto tmp = std::get_if<std::string>(&(current_it->second));
            if (tmp != nullptr) {
                current_key = *tmp;
            }
        }

        return std::visit([&](auto &&var) -> size_t {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, std::unordered_map<std::string, uint64_t>> ||
                          std::is_same_v<T, std::unordered_map<std::string, int64_t>>  ||
                          std::is_same_v<T, std::unordered_map<std::string, std::string>>) {

                size_t old_idx = index;
                while (index < var.bucket_count()) {
                    auto iter = var.begin(index);

                    for (; iter != var.end(index) && index == old_idx; ++iter) {
                        if (current_key.size() > 0 &&
                            iter->first.size() > 0 &&
                            iter->first != current_key) {
                            continue;
                        }
                        else if (current_key.size() > 0 &&
                                 iter->first.size() > 0 &&
                                 iter->first == current_key) {
                            ++iter;
                            break;
                        }
                        break;
                    }

                    for (; iter != var.end(index); ++iter) {
                        if (iter->first.size() > 0) {
                            environment_add_or_update(dest_key, iter->first);
                            environment_add_or_update(value, iter->second);
                            return index;
                        }
                    }
                    ++index;
                }

                return index;
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

    size_t context::environment_get_size(const std::string &key)
    {
        return std::visit([&](auto &&var) -> size_t {
            using T = std::decay_t<decltype(var)>;

            if constexpr (std::is_same_v<T, std::vector<uint64_t>> ||
                          std::is_same_v<T, std::vector<int64_t>>  ||
                          std::is_same_v<T, std::vector<std::string>>) {
                return var.size();
            }
            else if constexpr (std::is_same_v<T, std::unordered_map<std::string, uint64_t>> ||
                               std::is_same_v<T, std::unordered_map<std::string, int64_t>>  ||
                               std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
                return var.bucket_count();
            }

            return 0;
        }, environment_[key]);
    }
}
