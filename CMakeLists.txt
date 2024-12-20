cmake_minimum_required(VERSION 3.29)
project(aoc24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS " \
-O0 \
-Wall \
-Wextra \
-Wpedantic \
-Wshadow \
-Werror \
")
# -Ofast \

find_package(nvl)

function(add_day day)
    set(name "day${day}")
    set(file "Day${day}.cpp")
    add_executable(${name} "${name}/${file}")
    target_link_libraries(${name} PUBLIC nvl)
endfunction()

add_day("01")
add_day("02")
add_day("03")
add_day("04")
add_day("05")
add_day("06")
add_day("07")
add_day("08")
add_day("09")
add_day("10")
add_day("11")
add_day("12")
add_day("13")
add_day("14")
add_day("15")
add_day("16")
add_day("17")
add_day("18")
add_day("19")
