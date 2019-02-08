#ifdef __cplusplus

#include "engine.h"

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
    auto user = query_to_map(r->args);

    amps::user_map ht {{"user_data", user}};

    amps::error err;
    amps::engine engine(err);
    engine.set_template_directory("/tmp");

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
