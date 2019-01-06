#include "engine.h"
#include "scan.h"
#include "fileops.h"
#include "config.h"

#include <fstream>
#include <string>
#include <any>

using namespace std;

namespace volt
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

        string content(MAX_READ_SZ + 1, '\0');

        while (true) {
            file.read(&content[0], static_cast<streamsize>(MAX_READ_SZ));
            scanner_.do_scan(content);

            if (file.eof()) {
                break;
            }
        }
    }

    bool engine::compile()
    {
        user_map ht {
            {"name", "Jose"},
            {"age", number_t(39 * 44)},
            {"cities", vector<string>{"Sao Paulo", "Paris", "NYC"}},
            {"likes", unordered_map<string, string>{{"movie", "matrix"}, {"song", "gnr"}}}
        };

        compiler_.generate(scanner_.get_metainfo(), ht);
        return false;
    }
}

int main()
{
    volt::error err;
    volt::engine eng(err);
    eng.set_template_directory("/tmp");
    eng.prepare_template("test.tpl");
    eng.compile();

    return 0;
}
