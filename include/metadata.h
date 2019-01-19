#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>

#include "token.h"

namespace volt
{
    enum class metatype : uint8_t
    {
        TEXT,
        CODE,
        ECHO,
        COMMENT,
    };

    struct metarange
    {
        size_t start;
        size_t end;
    };

    struct metadata
    {
        size_t hash_tokens;
        metatype type;
        metarange range;
        std::string data;
        std::vector<token_t> tokens;

        void add_token(const token_t &tk);
    };

    inline void metadata::add_token(const token_t &tk)
    {
        tokens.emplace_back(tk);
        hash_tokens += tk.hash();
    }

    class metainfo
    {
        size_t hash_metadata;
        std::vector<metadata> metadata_;

    public:
        typedef metadata value_type;

        metainfo() :
            hash_metadata(0)
        {
        }

        void add_metadata(const metadata &data);
        size_t hash() const;
        void rehash();

        void push_back(const metadata &data);
        metadata &back();
        void remove(size_t idx);
        void clear();
        size_t size() const;
        void resize(size_t new_size);

        const metadata &operator[](size_t idx) const;
        std::vector<metadata> &operator=(std::vector<metadata> &&other) noexcept;
        std::vector<metadata>::iterator begin() noexcept;
        std::vector<metadata>::iterator end() noexcept;
        std::vector<metadata>::const_iterator begin() const noexcept;
        std::vector<metadata>::const_iterator end() const noexcept;
    };

    inline void metainfo::rehash()
    {
        for (const auto &tk : metadata_) {
            hash_metadata += tk.hash_tokens;
        }
    }

    inline void metainfo::remove(size_t idx)
    {
        if (idx > metadata_.size()) {
            return;
        }

        metadata_.erase(metadata_.begin() + idx);
    }

    inline size_t metainfo::hash() const
    {
        return hash_metadata;
    }

    inline void metainfo::push_back(const metadata &data)
    {
        add_metadata(data);
    }

    inline std::vector<metadata> &metainfo::operator=(std::vector<metadata> &&other) noexcept
    {
        return metadata_.operator=(other);
    }

    inline void metainfo::resize(size_t new_size)
    {
        metadata_.resize(new_size);
    }

    inline std::vector<metadata>::iterator metainfo::begin() noexcept
    {
        return metadata_.begin();
    }

    inline std::vector<metadata>::iterator metainfo::end() noexcept
    {
        return metadata_.end();
    }

    inline std::vector<metadata>::const_iterator metainfo::begin() const noexcept
    {
        return metadata_.begin();
    }

    inline std::vector<metadata>::const_iterator metainfo::end() const noexcept
    {
        return metadata_.end();
    }

    inline const metadata &metainfo::operator[](size_t idx) const
    {
        return metadata_[idx];
    }

    inline size_t metainfo::size() const
    {
        return metadata_.size();
    }

    inline metadata &metainfo::back()
    {
        return metadata_.back();
    }

    inline void metainfo::clear()
    {
        metadata_.clear();
    }

    inline void metainfo::add_metadata(const metadata &data)
    {
        metadata_.emplace_back(data);
        hash_metadata += data.hash_tokens;
    }
}

#endif // METADATA_H
