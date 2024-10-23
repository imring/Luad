set(DISLUA_INSTALL OFF)
set(DISLUA_TESTS   OFF)

FetchContent_Declare(
    disluapp
    GIT_REPOSITORY https://github.com/imring/disluapp.git
    GIT_TAG v1.1.4
)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(disluapp fmt)

add_library(bclist
    "bclist.cpp"
    "bclist/lj.cpp"
)

target_include_directories(bclist PUBLIC .)
target_link_libraries(bclist PUBLIC fmt::fmt dislua)
target_compile_features(bclist PUBLIC cxx_std_20)

if (MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(bclist PRIVATE
        /W4
        /WX)
else()
    # lots of warnings and all warnings as errors
    target_compile_options(bclist PRIVATE
        -Werror
        -pedantic-errors

        -Wall
        -Wextra
        -Wpedantic

        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        # -Wduplicated-branches
        # -Wduplicated-cond
        -Wextra-semi
        -Wfloat-equal
        # -Wlogical-op
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo)
endif()