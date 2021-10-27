#pragma once

#include <salt/core/engine.hpp>

namespace salt {

extern Engine create_engine();

} // namespace salt

int main(int argc, char* argv[]) {
    auto engine = create_engine();
    engine.run();
}