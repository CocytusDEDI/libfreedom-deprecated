cmake_minimum_required(VERSION 3.10)
project(libfreedom LANGUAGES C)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add library target
add_library(libfreedom SHARED src/libfreedom.c)

target_include_directories(libfreedom PRIVATE /usr/include/libdrm)

# Prevent it from being called liblibfreedom.so
set_target_properties(libfreedom PROPERTIES
    OUTPUT_NAME "freedom"  # This will create libfreedom.so
)

# Install targets and configuration
install(TARGETS libfreedom DESTINATION lib)
install(FILES include/libfreedom.h DESTINATION include)
