#include <fstream>
#include <regex>

#include "nvl/data/Maybe.h"
#include "nvl/data/Set.h"
#include "nvl/geo/RTree.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

using namespace nvl;

struct World {
    explicit World(Pos<2> size) : size(size) {
        const Pos<2> half = size / 2;
        quadrants[0] = Box<2>(Pos<2>::zero, half);
        quadrants[1] = Box<2>({half[0] + 1, 0}, {size[0], half[1]});
        quadrants[2] = Box<2>({0, half[1] + 1}, {half[0], size[1]});
        quadrants[3] = Box<2>(half + 1, size);
        for (U64 i = 0; i < 4; ++i) {
            std::cout << "Q" << i << ": " << quadrants[i] << std::endl;
        }
    }
    pure Maybe<U64> quadrant(const Pos<2> &pos) const {
        for (U64 i = 0; i < 4; ++i) {
            return_if(quadrants[i].contains(pos), i);
        }
        return None;
    }
    Pos<2> size;
    Box<2> quadrants[4];
};

struct Robot {
    static Maybe<Robot> parse(std::fstream &file) {
        static const std::regex regex ("p=(-?[0-9]+),(-?[0-9]+) v=(-?[0-9]+),(-?[0-9]+)");
        std::string line;
        std::smatch match;
        std::getline(file, line);
        if (std::regex_search(line.cbegin(), line.cend(), match, regex)) {
            Robot robot;
            robot.p[0] = std::stoll(match[1].str());
            robot.p[1] = std::stoll(match[2].str());
            robot.v[0] = std::stoll(match[3].str());
            robot.v[1] = std::stoll(match[4].str());
            return robot;
        }
        return None;
    }
    pure Pos<2> after(const World &world, const I64 steps) const {
        return ((p + v*steps) % world.size + world.size) % world.size;
    }
    Pos<2> p;
    Pos<2> v;
};

List<Pos<2>> after(const World &world, const List<Robot> &robots, const I64 steps) {
    List<Pos<2>> pos;
    for (auto &robot : robots) {
        pos.push_back(robot.after(world, steps));
    }
    return pos;
}

void draw(const World &world, const List<Robot> &robots, const I64 steps) {
    Set<Pos<2>> set;
    for (const auto &pos : after(world, robots, steps)) {
        set.insert(pos);
    }
    for (I64 i = 0; i < world.size[1]; ++i) {
        for (I64 j = 0; j < world.size[0]; ++j) {
            std::cout << (set.has(Pos<2>(j, i)) ? '#' : '.');
        }
        std::cout << std::endl;
    }
}

I64 part1(const World &world, const List<Robot> &robots) {
    const List<Pos<2>> positions = after(world, robots, 100);
    Pos<4> quadrants = Pos<4>::zero;
    for (auto &pos : positions) {
        if (auto q = world.quadrant(pos)) {
            quadrants[*q] += 1;
        }
    }
    return quadrants.product();
}

// This takes a little time to run, RTree is a bit slow right now
I64 part2(const World &world, const List<Robot> &robots) {
    I64 best_i = 0;
    U64 best_size = 0;
    // There's no good way of knowing the max here, so just guessing...
    for (I64 i = 0; i < 10000; ++i) {
        RTree<2, Box<2>> grid;
        for (auto &pos : after(world, robots, i)) {
            grid.emplace(pos, pos + 1);
        }
        U64 largest = 0;
        for (const auto &component : grid.components()) {
            largest = std::max(component.size(), largest);
        }
        if (largest > best_size) {
            best_i = i;
            best_size = largest;
        }
    }
    return best_i;
}

int main() {
    std::fstream file ("../data/full/14");
    const World world({101, 103});
    List<Robot> robots;
    while (auto robot = Robot::parse(file)) {
        robots.push_back(*robot);
    }
    std::cout << "Part 1: " << part1(world, robots) << std::endl;
    const I64 frame = part2(world, robots);
    draw(world, robots, frame);
    std::cout << "Part 2: " << frame << std::endl;
}