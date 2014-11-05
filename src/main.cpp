#include "engine_client.hpp"
#include "definitions.hpp"

int main(int, char**) try {
    std::set_terminate([] {
        std::abort();
    });

    std::set_unexpected([] {
        std::terminate();
    });

    bkrl::data_definitions defs;
    bkrl::engine_client client {defs};

    return 0;
} catch (std::exception& e) {
    std::abort();
} catch (...) {
    std::abort();
}
