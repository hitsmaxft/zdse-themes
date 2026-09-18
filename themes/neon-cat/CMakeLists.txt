if(CONFIG_ZDSE_NEON_CAT_THEME)
  zephyr_library()
  zephyr_library_include_directories(include)
  zephyr_library_compile_options(-O2 -ffp-contract=off)
  zephyr_library_sources(
    src/common/pixel_ui.c
    src/common/neon_font.c
    src/common/spleen_font.c
    src/themes/neon_atlas_v2.c
    src/themes/neon_animation.c
    src/themes/neon_scene.c
    src/themes/neon_widgets.c
    src/themes/neon_cat.c
  )
endif()
