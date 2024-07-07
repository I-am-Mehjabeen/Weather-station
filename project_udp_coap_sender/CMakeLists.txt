cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(khan_3_1_cli_thread)

target_sources(app PRIVATE src/main.c)

# Add the library files
zephyr_include_directories(cJSON)
zephyr_library_sources(cJSON/cJSON.c)
