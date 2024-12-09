#include <fstream>
#include <list>
#include <ranges>

#include "nvl/data/Maybe.h"
#include "nvl/data/List.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Assert.h"
#include "nvl/macros/Pure.h"
#include "nvl/time/Clock.h"
#include "nvl/time/Duration.h"

struct Block {
    explicit Block(const nvl::Maybe<U64> id, const U64 begin, const U64 end) : id(id), begin(begin), end(end) {}
    pure bool is_file() const { return id.has_value(); }
    pure bool is_free() const { return !id.has_value(); }
    pure U64 size() const { return end - begin; }

    pure U64 checksum() const {
        return_if(!id.has_value(), 0);
        U64 checksum = 0;
        ASSERT(begin <= end, "Invalid block " << *id << ": [" << begin << ", " << end << ")");
        for (U64 i = begin; i < end; ++i) {
            checksum += *id * i;
        }
        return checksum;
    }

    nvl::Maybe<U64> id;
    U64 begin;
    U64 end;
};

U64 checksum(const std::list<Block> &list) {
    U64 checksum = 0;
    for (const auto &block : list) {
        checksum += block.checksum();
    }
    return checksum;
}

std::list<Block> do_part1(std::list<Block> list) {
    nvl::Maybe<U64> fwd_max_id = nvl::None;
    nvl::Maybe<U64> bwd_min_id = nvl::None;
    auto file = list.end();
    --file;
    auto free = list.begin();
    bool done = false;
    while (!done) {
        // Move to next free location
        while ((!free->is_free() || free->size() == 0) && free != list.end()) {
            fwd_max_id = nvl::max(fwd_max_id, free->id);
            ++free;
        }
        while ((file->is_free() || file->size() == 0) && file != list.end()) { --file; }
        bwd_min_id = nvl::min(bwd_min_id, file->id);
        done = fwd_max_id && bwd_min_id && *fwd_max_id >= *bwd_min_id;
        if (!done) {
            const U64 move = std::min(free->size(), file->size());
            list.emplace(free, file->id, free->begin, free->begin + move);
            file->end -= move;
            free->begin += move;
            Block new_free(nvl::None, file->end, file->end + move);
            ++file;
            list.insert(file, new_free);
            --file;
        }
    }
    return list;
}

std::list<Block> do_part2(std::list<Block> list) {
    auto file = list.end();
    --file;
    while (file != list.begin()) {
        if (file->is_file()) {
            auto free = list.begin();
            while (free != file) {
                if (free->is_free() && free->size() >= file->size()) {
                    list.emplace(free, file->id, free->begin, free->begin + file->size());
                    free->begin += file->size();
                    file->id = nvl::None;
                    break;
                }
                ++free;
            }
        }
        --file;
    }
    return list;
}

int main() {
    std::ifstream file ("../data/full/09");
    std::string line;
    std::getline(file, line);
    std::list<Block> list;
    U64 offset = 0;
    U64 id = 0;
    bool is_file = true;
    for (const char c : line) {
        const U64 x = static_cast<U64>(c - '0');
        list.emplace_back(nvl::SomeIf(id, is_file), offset, offset + x);
        offset += x;
        id += is_file;
        is_file = !is_file;
    }
    const auto start = nvl::Clock::now();
    const auto part1 = do_part1(list);
    const auto part2 = do_part2(list);
    const auto end = nvl::Clock::now();
    std::cout << "Part 1: " << checksum(part1) << std::endl;
    std::cout << "Part 2: " << checksum(part2) << std::endl;
    std::cout << "Time: " << nvl::Duration(end - start) << std::endl;
}
