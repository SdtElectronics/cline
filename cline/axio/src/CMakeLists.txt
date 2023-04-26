add_library(axio
    channel.cpp
    emitter.cpp
    socket.cpp
    # stdin.cpp
    timer.cpp
    write.cpp
)

if(MSVC)
    set_target_properties(
        axio

      PROPERTIES
        COMPILE_OPTIONS /W4
    )
else()
    set_target_properties(
        axio

      PROPERTIES
        COMPILE_OPTIONS -Wall -Wextra -Wpedantic
    )
endif()


target_include_directories(axio
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
