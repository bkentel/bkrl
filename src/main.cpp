#include "engine_client.hpp"
#include "config.hpp"

int main(int, char**) {
    auto const config = bkrl::load_config("./data/config.def");

    bkrl::engine_client client {config};

    return 0;
}
