project('platformfolders', 'cpp', default_options : ['cpp_std=c++11', 'warning_level=3'])

platformfolders_inc = include_directories('sago')


platformfolders_lib = static_library('platform_folders', 'sago/platform_folders.cpp', include_directories: platformfolders_inc)


platformfolders_dep = declare_dependency(
    include_directories : platformfolders_inc,
    link_with : platformfolders_lib
)
