if (NOT _ABSL_FOUND)
    set(_ABSL_FOUND TRUE)

    include(FetchContent)
    FetchContent_Declare(
            absl
            URL https://github.com/abseil/abseil-cpp/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(absl)
endif ()
