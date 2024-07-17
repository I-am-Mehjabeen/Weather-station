cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(pressure_sensor_driver)

target_sources(app PRIVATE src/main.c
drivers/sps30/sps30.c
drivers/sps30/sensirion_common.c
drivers/sps30/hal.c)
zephyr_include_directories(drivers)
