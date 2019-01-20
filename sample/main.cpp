#include "engine.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cerr << "./" << argv[0] << " <template>\n";
        return 1;
    }

    amps::user_map ht {
          {"name", "Jose"},
          {"cities", vector<string>{
                 "Sao Paulo",
                 "Paris",
                 "NYC",
                 "London",
                 "Lisbon"}},
          {"songs", unordered_map<string, string>{
                {"guns and roses", "patience"},
                {"aerosmith", "crazy"},
                {"led zeppelin", "immigrant song"},
                {"pink floyd", "high hopes"}}},
    };

    amps::error err;
    amps::engine engine(err);
    engine.set_template_directory("/tmp");
    engine.prepare_template(argv[1]);
    engine.compile(ht);

    return 0;
}
