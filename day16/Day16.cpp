#include <fstream>

#include <queue>

#include "nvl/data/List.h"
#include "nvl/data/Set.h"
#include "nvl/data/SipHash.h"
#include "nvl/data/Tensor.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Face.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/data/Map.h"

using namespace nvl;

static constexpr Pos<2> kEast (0, 1);
static constexpr Pos<2> kNorth (-1, 0);
static constexpr Pos<2> kWest (0, -1);
static constexpr Pos<2> kSouth (1, 0);

static constexpr Pos<2> kFacings [4] = {kEast, kNorth,  kWest, kSouth};

enum Move {kLeft, kRight, kMove };
static constexpr Move kMoves [3] = {kLeft, kRight, kMove};

static constexpr char kWall = '#';

struct Entry {
    static U64 cost(const Move m) {
        static const Map<Move,U64> kCosts {{kLeft, 1000}, {kRight, 1000}, {kMove,1}};
        return kCosts.get_or(m, 1);
    }
    pure Entry next(const Move m) const {
        switch (m) {
        case kLeft: return {pos, ((facing + 1) % 4 + 4) % 4};
        case kRight: return {pos, ((facing - 1) % 4 + 4) % 4};
        case kMove: return {pos + kFacings[facing], facing};
        }
        UNREACHABLE;
    }

    pure bool operator==(const Entry &rhs) const { return pos == rhs.pos && facing == rhs.facing; }
    pure bool operator!=(const Entry &rhs) const { return !(*this == rhs); }

    Pos<2> pos;
    I64 facing;
};

template <>
struct std::hash<Entry> {
    pure U64 operator()(const Entry &entry) const noexcept { return sip_hash(entry); }
};

struct Pair {
    Entry entry;
    U64 cost;
};

template <>
struct std::less<Pair> {
    bool operator()(const Pair &a, const Pair &b) const noexcept { return a.cost > b.cost; }
};

struct Dijkstra {
    explicit Dijkstra(const Tensor<2, char> &map, const Pos<2> &start, const Pos<2> &end) :
        starting(start, 0), ending(end, 0)
    {
        // There isn't a way to update the priority of an entry of a std::priority_queue in place, so instead
        // keep a visited set and just ignore repeat entries within the queue.
        Set<Entry> visited;
        std::priority_queue<Pair> queue;
        dist[starting] = 0;
        queue.emplace(starting, 0);

        while (!queue.empty()) {
            const auto [u, _] = queue.top();
            queue.pop();
            if (!visited.has(u)) {
                visited.insert(u);
                for (auto m : kMoves) {
                    const Entry next = u.next(m);
                    if (map.get_or(next.pos, kWall) != kWall) {
                        const U64 alt = dist[u] + Entry::cost(m);
                        const U64 prev_dist = dist.get_or(next, UINT64_MAX);
                        if (alt < prev_dist) {
                            dist[next] = alt;
                            prev[next] = {u};
                            queue.emplace(next, alt);
                        } else if (alt == prev_dist) {
                            prev[next].push_back(u);
                        }
                    }
                }
            }
        }
        for (I64 f = 0; f < 4; ++f) {
            const U64 cost = dist.get_or({end, f}, UINT64_MAX);
            if (cost < best_cost) {
                best_cost = cost;
                ending = {end, f};
            }
        }
    }

    pure List<Entry> path() const {
        List<Entry> path {ending};
        while (path.back() != starting) {
            path.push_back(prev.get(path.back())->front());
        }
        return path;
    }

    pure U64 tiles() const {
        static const List<Entry> kNone = {};
        Set<Pos<2>> tiles;
        List<Entry> frontier { ending };
        while (!frontier.empty()) {
            const Entry curr = frontier.back();
            frontier.pop_back();
            tiles.insert(curr.pos);
            const auto &previous = prev.get_or(curr, kNone);
            frontier.append(previous);
        }
        return tiles.size();
    }

    Map<Entry, U64> dist;
    Map<Entry, List<Entry>> prev;
    U64 best_cost = UINT64_MAX;
    Entry starting;
    Entry ending;
};

int main() {
    const auto map = matrix_from_file("../data/full/16");
    const auto start = map.index_where([](char c){ return c == 'S'; }).value();
    const auto end = map.index_where([](char c){ return c == 'E'; }).value();
    const Dijkstra solution(map, start, end);
    std::cout << "Part 1: " << solution.best_cost << std::endl;
    std::cout << "Part 2: " << solution.tiles() << std::endl;
}
