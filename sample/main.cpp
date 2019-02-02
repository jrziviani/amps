#include "engine.h"
#include <unordered_map>
#include <vector>

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

    /*
    unordered_map<string, amps::number_t> ages = {
        {"John", 53},
        {"Mary", 21},
        {"Nobody", -15},
    };

    vector<amps::number_t> random_nrs = {3, 12, 8, 1, 55,
                                         static_cast<amps::number_t>(-12),
                                         static_cast<amps::number_t>(-1), 0};

    amps::user_map um {{"ages", ages}, {"nums", random_nrs}};
    */

    amps::error err;
    amps::engine engine(err);
    engine.set_template_directory("/tmp");
    engine.prepare_template(argv[1]);
    string result = engine.render(ht);

    cout << result << endl;

    return 0;
}
