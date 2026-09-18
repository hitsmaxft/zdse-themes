if(CONFIG_ZDSE_RADAR_THEME)
  zephyr_library()
  zephyr_library_include_directories(include)
  zephyr_library_compile_options(-O2 -ffp-contract=off)
  zephyr_library_sources(
    src/radar/background.c
    src/radar/sweep.c
    src/themes/radar_scan.c
  )
endif()
