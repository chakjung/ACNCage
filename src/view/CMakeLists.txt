add_library(view STATIC view.c listener.c)

target_compile_options(view PRIVATE -DWLR_USE_UNSTABLE)
