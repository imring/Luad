set(DISLUA_INSTALL OFF)
set(DISLUA_TESTS   OFF)

FetchContent_Declare(
    disluapp
    GIT_REPOSITORY https://github.com/imring/disluapp.git
    GIT_TAG ec0c37303c1c72ee71adf93a51d13753ba053d1b # 1.1.3
)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG a33701196adfad74917046096bf5a2aa0ab0bb50 # 9.1.0
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