#ifdef __cplusplus

#include "engine.h"
#include "vector_ostream.h"

#include "httpd.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>

using std::vector;
using std::string;
using std::unordered_map;

unordered_map<string, string> query_to_map(const char *query)
{
    unordered_map<string, string> result;
    if (query == nullptr) {
        return result;
    }

    char *tmp = strdup(query);
    for (char *tok = strtok(tmp, "&"); tok != NULL; tok = strtok(NULL, "&")) {
        char *value = strchr(tok, '=');
        if (value == nullptr) {
            continue;
        }

        result[string(tok, value - tok)] = string(&value[1]);
    }

    free(tmp);
    return result;
}

static void get_custom_template(request_rec *r, char **result)
{
    if (r->args == 0) {
        return;
    }

    amps::vector_ostreambuf buff;
    std::ostream stream(&buff);

    amps::error err(stream);
    amps::engine engine(err);
    engine.set_template_directory("/tmp");

    auto user = query_to_map(r->args);
    amps::user_map ht {{"user_data", user}};

    // html template is the default, xml returned when content=xml
    auto content = user.find("content");
    if (content == user.end() || content->second == "html") {
        engine.prepare_template("template.tpl");
        r->content_type = "text/html";
    }
    else {
        engine.prepare_template("template_xml.tpl");
        r->content_type = "text/xml";
    }

    string rendered = engine.render(ht);

    const auto &xerr = buff.get_errors();
    if (xerr.size() > 0) {
        engine.prepare_template("errors.tpl");
        r->content_type = "text/html";
        amps::user_map errht {{"errors", xerr}};
        string r = engine.render(errht);
        *result = (char*)malloc(sizeof(char) * r.size() + 1);
        strcpy(*result, r.c_str());
        (*result)[r.size()] = '\0';
        return;
    }

    *result = (char*)malloc(sizeof(char) * rendered.size() + 1);
    strcpy(*result, rendered.c_str());
    (*result)[rendered.size()] = '\0';
}
#endif

extern "C" {
    #include "amps_wrapper.h"

    void get_template(request_rec *r, char **result)
    {
        get_custom_template(r, result);
    }
}
