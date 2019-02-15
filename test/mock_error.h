#ifndef MOCK_ERROR_H
#define MOCK_ERROR_H

#include "../include/error.h"
#include "../include/vector_ostream.h"

#include <iostream>

class mock_error : public amps::error
{
    std::ostream stream_;
    amps::vector_ostreambuf buff_;

public:
    mock_error() :
        amps::error(stream_),
        stream_(&buff_)
    {
    }

    std::string get_error_msg(size_t index) const
    {
        return buff_.get_str(index);
    }

    std::string get_first_error_msg() const
    {
        return buff_.get_first_str();
    }

    std::string get_last_error_msg() const
    {
        return buff_.get_last_str();
    }

    void clear()
    {
        buff_.clear();
    }
};

#endif // MOCK_ERROR_H
