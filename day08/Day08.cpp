#include <fstream>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/Tensor.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"

nvl::Map<char, nvl::List<nvl::Pos<2>>> frequencies(const nvl::Tensor<2,char> &map) {
    nvl::Map<char, nvl::List<nvl::Pos<2>>> freqs;
    for (const auto i : map.indices()) {
        if (map[i] != '.') {
            freqs[map[i]].push_back(i);
        }
    }
    return freqs;
}

nvl::Set<nvl::Pos<2>> antinodes(const nvl::Tensor<2, char> &map,
                                const nvl::Map<char, nvl::List<nvl::Pos<2>>> &frequencies,
                                const bool resonant) {
    nvl::Set<nvl::Pos<2>> set;
    for (const auto &antennae : frequencies.values()) {
        for (U64 i = 0; i < antennae.size(); ++i) {
            const auto &a = antennae[i];
            for (U64 j = i + 1; j < antennae.size(); ++j) {
                const auto &b = antennae[j];
                const nvl::Pos<2> delta = a - b;
                if (resonant) {
                    nvl::Pos<2> d = a;
                    while (map.has(d)) {
                        set.insert(d);
                        d += delta;
                    }
                    d = b;
                    while (map.has(d)) {
                        set.insert(d);
                        d -= delta;
                    }
                } else {
                    const nvl::Pos<2> d0 = a + delta;
                    const nvl::Pos<2> d1 = b - delta;
                    if (map.has(d0))
                        set.insert(d0);
                    if (map.has(d1))
                        set.insert(d1);
                }
            }
        }
    }
    return set;
}

int main() {
    const nvl::Tensor<2, char> map = nvl::matrix_from_file("../data/full/08");
    const nvl::Map<char, nvl::List<nvl::Pos<2>>> f = frequencies(map);
    const nvl::Set<nvl::Pos<2>> part1 = antinodes(map, f, /*resonant*/false);
    const nvl::Set<nvl::Pos<2>> part2 = antinodes(map, f, /*resonant*/true);
    std::cout << "Part 1: " << part1.size() << std::endl;
    std::cout << "Part 2: " << part2.size() << std::endl;
}
