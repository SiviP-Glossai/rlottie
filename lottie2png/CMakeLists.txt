add_executable(lottie2png "lottie2png.cpp" "png/fpng.h" "png/fpng.cpp")

if(MSVC)
    target_compile_options(lottie2png
                           PRIVATE
                           /std:c++14)
else()
    target_compile_options(lottie2png
                           PRIVATE
                           -std=c++14)
endif()

target_link_libraries(lottie2png rlottie)

target_include_directories(lottie2png
                           PRIVATE
                           "${CMAKE_CURRENT_LIST_DIR}/../inc/")
