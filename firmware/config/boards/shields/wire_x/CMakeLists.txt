zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
zephyr_library_sources(wire_x_remap_rpc.c)

if(WIRE_X_ST7789_TEST)
  zephyr_library_sources(wire_x_st7789_test.c)
endif()
