#include <fstream>
#include <regex>

#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/Tensor.h"

using namespace nvl;

List<Pos<2>> parse_pairs(const std::string &filename) {
    static const std::regex pattern("([0-9]+),([0-9]+)");
    std::fstream file (filename);
    std::string line;
    std::smatch match;
    List<Pos<2>> pairs;
    while (std::getline(file, line)) {
        if (std::regex_search(line.cbegin(), line.cend(), match, pattern)) {
            pairs.emplace_back(std::stoll(match[1].str()), std::stoll(match[2].str()));
        }
    }
    return pairs;
}

static constexpr Pos<2> kMoves [4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

struct Pair {
    Pos<2> pos;
    U64 cost = 0;
};

template <>
struct std::less<Pair> {
    bool operator()(const Pair &a, const Pair &b) const noexcept { return a.cost > b.cost; }
};

struct Dijkstra {
    static constexpr char kObstacle = '#';
    explicit Dijkstra(const Tensor<2, char> &map, const Pos<2> &start, const Pos<2> &end) : start(start), end(end) {
        // There isn't a way to update the priority of an entry of a std::priority_queue in place, so instead
        // keep a visited set and just ignore repeat entries within the queue.
        Set<Pos<2>> visited;
        std::priority_queue<Pair> queue;
        dist[start] = 0;
        queue.emplace(start, 0);

        while (!queue.empty()) {
            const auto [u, _] = queue.top();
            queue.pop();
            if (!visited.has(u)) {
                visited.insert(u);
                for (auto m : kMoves) {
                    const Pos<2> next = u + m;
                    if (map.get_or(next, kObstacle) != kObstacle) {
                        const U64 alt = dist[u] + 1;
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
    }

    pure Maybe<U64> min_cost() const {
        if (auto iter = dist.find(end); iter != dist.end())
            return iter->second;
        return None;
    }

    pure List<Pos<2>> path() const {
        List<Pos<2>> path {end};
        while (path.back() != start) {
            path.push_back(prev.get(path.back())->front());
        }
        return path;
    }

    pure U64 tiles() const {
        static const List<Pos<2>> kNone = {};
        Set<Pos<2>> tiles;
        List<Pos<2>> frontier { end };
        while (!frontier.empty()) {
            const Pos<2> curr = frontier.back();
            frontier.pop_back();
            tiles.insert(curr);
            const auto &previous = prev.get_or(curr, kNone);
            frontier.append(previous);
        }
        return tiles.size();
    }

    Map<Pos<2>, U64> dist;
    Map<Pos<2>, List<Pos<2>>> prev;
    Pos<2> start;
    Pos<2> end;
};

int main() {
    const List<Pos<2>> pairs = parse_pairs("../data/full/18");
    Tensor<2,char> map({71,71}, '.');
    U64 i = 0;
    for (i = 0; i < std::min<U64>(pairs.size(), 1024); ++i) {
        map[pairs[i]] = '#';
    }

    Dijkstra solution (map, {0, 0}, map.shape() - 1);
    const auto part1 = solution.min_cost();
    std::cout << "Part 1: " << (part1 ? std::to_string(*part1) : "?") << std::endl;

    Maybe<U64> part2 = part1;
    while (part2.has_value()) {
        map[pairs[i]] = '#';
        Dijkstra solve (map, {0, 0}, map.shape() - 1);
        part2 = solve.min_cost();
        if (part2.has_value()) {
            i += 1;
        }
    }
    std::cout << "Part2: " << pairs[i] << std::endl;
}
