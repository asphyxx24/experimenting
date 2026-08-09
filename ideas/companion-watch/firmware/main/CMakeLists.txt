idf_component_register(
    SRCS "main.c" "display.c" "touch.c" "ptt.c" "sprite_engine.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    PRIV_REQUIRES esp_timer
)
