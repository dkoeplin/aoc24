#include <fstream>

#include "nvl/data/Map.h"
#include "nvl/geo/RTree.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

using namespace nvl;

static const Map<char, Pos<2>> kChar2Direction = {
    {'<', Pos<2>(0, -1)},
    {'^', Pos<2>(-1, 0)},
    {'>', Pos<2>(0, 1)},
    {'v', Pos<2>(1, 0)}
};

struct Object {
    static constexpr char kRobot = '@';
    static constexpr char kWall = '#';
    static constexpr char kBox = 'O';
    static constexpr char kEmpty = '.';

    pure Pos<2> loc() const { return box.min; }
    pure Box<2> bbox() const { return box; }

    explicit Object(const char type, const Box<2> box) : type(type), box(box) {}

    char type;
    Box<2> box;
};

struct Warehouse {
    void parse(const std::string &filename, const I64 width = 1) {
        I64 y = 0;
        std::fstream file (filename);
        std::string line;
        while (std::getline(file, line) && !line.empty()) {
            for (I64 x = 0; x < static_cast<I64>(line.size()); ++x) {
                const char type = line[x];
                const Pos<2> start {y, width*x};
                if (type == Object::kRobot) {
                    const Pos<2> end = start + 1;
                    robot = map.emplace(type, Box<2>(start, end));
                } else if (type != Object::kEmpty) {
                    const Pos<2> end = start + Pos<2>(1, width);
                    map.emplace(type, Box<2>(start, end));
                }
            }
            y += 1;
        }

        while (std::getline(file, line)) {
            for (char c : line) {
                dirs.push_back(c);
            }
        }
    }

    pure Set<Ref<Object>> until_empty(const Pos<2> &dir) const {
        Set<Ref<Object>> set { robot };
        List<Ref<Object>> frontier { robot };
        while (!frontier.empty()) {
            Ref<Object> curr_obj = frontier.back();
            const Box<2> &next = curr_obj->box + dir;
            frontier.pop_back();
            for (const Ref<Object> &obj : map[next]) {
                return_if(obj->type == Object::kWall, {});
                if (obj.ptr() != curr_obj.ptr()) {
                    frontier.push_back(obj);
                    set.insert(obj);
                }
            }
        }
        return set;
    }

    void attempt_move(const char c) {
        const Pos<2> dir = *kChar2Direction.get(c);
        Set<Ref<Object>> moved = until_empty(dir);
        for (Ref<Object> &obj : moved) {
            const Box<2> prev = obj->box;
            obj->box += dir;
            map.move(obj, prev);
        }
    }

    void move_all() {
        for (const char c : dirs) {
            attempt_move(c);
        }
    }

    pure U64 coord_sum() const {
        U64 sum = 0;
        for (const Ref<Object> &obj : map.items()) {
            if (obj->type == Object::kBox) {
                sum += obj->loc()[0] * 100 + obj->loc()[1];
            }
        }
        return sum;
    }

    RTree<2, Object> map;
    List<char> dirs;
    Ref<Object> robot;
};

int main() {
    const std::string filename = "../data/full/15";
    Warehouse warehouse1;
    warehouse1.parse(filename);
    warehouse1.move_all();
    std::cout << "Part 1: " << warehouse1.coord_sum() << std::endl;

    Warehouse warehouse2;
    warehouse2.parse(filename, /*width*/2);
    warehouse2.move_all();
    std::cout << "Part 2: " << warehouse2.coord_sum() << std::endl;
}
