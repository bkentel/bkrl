#include "engine_client.hpp"
#include "definitions.hpp"

int main(int, char**) {
    bkrl::data_definitions defs;
    bkrl::engine_client client {defs};

    return 0;
}
