if (NOT _NVL_FOUND)
    set(_NVL_FOUND TRUE)

    include(FetchContent)
    FetchContent_Declare(
            nvl
            URL https://github.com/dkoeplin/nvl/archive/refs/heads/main.zip
    )
    FetchContent_MakeAvailable(nvl)
endif ()
