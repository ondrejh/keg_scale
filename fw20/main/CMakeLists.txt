idf_component_register(SRCS "main.c" "task_disp.c" "task_scale.c" "task_net.c"
                            "flash_data.c" "config.c" "display.c" "ui.c" "ui_gauge.c" "ui_input.c" "touch.c" "form.c" "http.c" "scale.c"
                       INCLUDE_DIRS "."
                       EMBED_FILES "img/foam.rgb565" "img/beer.rgb565" "img/edge.rgb565a8")

# Web UI: stage web20 assets (no dev server, no mocks served live by fw) and pack them
# into the "storage" LittleFS image, flashed together with `idf.py flash`.
set(WEB_SRC ${CMAKE_CURRENT_LIST_DIR}/../../web20)
set(WEB_DIR ${CMAKE_BINARY_DIR}/www)
file(GLOB WEB_FILES ${WEB_SRC}/*.html ${WEB_SRC}/*.css ${WEB_SRC}/*.ico
                    ${WEB_SRC}/keglist.txt)
add_custom_target(www_stage
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${WEB_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WEB_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${WEB_FILES} ${WEB_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${WEB_SRC}/img ${WEB_DIR}/img
    VERBATIM)
littlefs_create_partition_image(storage ${WEB_DIR} FLASH_IN_PROJECT DEPENDS www_stage)
