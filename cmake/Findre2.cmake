if (NOT _RE2_FOUND)
    set(_RE2_FOUND TRUE)

    include(FetchContent)
    FetchContent_Declare(
            re2
            URL https://github.com/google/re2/archive/refs/heads/main.zip
    )
    FetchContent_MakeAvailable(re2)
endif ()


