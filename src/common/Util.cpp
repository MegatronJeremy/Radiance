#include "../../inc/common/Util.hpp"

int16_t Util::getDisplacement(Elf32_Addr from, Elf32_Addr to) {
    auto disp = static_cast<Elf32_Sword>(from - to);

    if (!valueFitsInDisp(disp)) {
        throw runtime_error("Assembler error: text segment is too large, displacement cannot fit into 12 bits!");
    }

    return static_cast<int16_t>(disp);
}
