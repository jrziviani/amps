#ifndef ENGINE_H
#define ENGINE_H

#include "scan.h"
#include "compiler.h"
#include "error.h"

#include <string>

namespace amps
{
    class engine
    {
        std::string path_;
        std::string result_;
        error error_;

        scan scanner_;
        compiler compiler_;

    public:
        engine(error &err);
        ~engine();

        void set_template_directory(const std::string &path);
        void prepare_template(const std::string &name);
        bool compile(const user_map &um);

        /*
        const error &get_error() const
        {
            return error_;
        }

        const std::string &get_result() const
        {
            return result_;
        }
        */
    };
}

#endif // ENGINE_H
