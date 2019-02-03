#include "engine.h"
#include "scan.h"
#include "fileops.h"
#include "config.h"

#include <fstream>
#include <string>
#include <any>

using namespace std;

namespace amps
{
    engine::engine(error &err) :
        path_("."),
        scanner_(err),
        compiler_(err)
    {
    }

    engine::~engine()
    {
    }

    void engine::set_template_directory(const string &path)
    {
        if (!is_readable_directory(path)) {
            return;
        }

        path_ = path;
    }

    void engine::prepare_template(const string &name)
    {
        std::string fullname = append(path_, name);
        if (!is_readable_file(fullname)) {
            return;
        }

        ifstream file(name);
        if (!file.is_open()) {
            return;
        }

        std::string content;
        file.seekg(0, std::ios::end);
        content.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        content.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

        scanner_.do_scan(content);

    }

    std::string engine::render(const user_map &um)
    {
        return compiler_.generate(scanner_.get_metainfo(), um);
    }
}
