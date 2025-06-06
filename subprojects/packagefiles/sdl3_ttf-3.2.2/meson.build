project('SDL3_ttf',
        'c',
        version : '3.2.2',
        license: 'zlib',
        default_options: [
          meson.version().version_compare('>=1.3.0') ? 'c_std=gnu99,c99' : 'c_std=gnu99',
        ],
       )

cc = meson.get_compiler('c')


c_args = [
  '-DSDL_BUILD_MAJOR_VERSION=3',
  '-DSDL_BUILD_MICRO_VERSION=2',
  '-DSDL_BUILD_MINOR_VERSION=2',
  '-DTTF_USE_HARFBUZZ=1'
]
if get_option('default_library') != 'static' and host_machine.system() == 'windows'
  c_args += '-DDLL_EXPORT'
endif

m_dep = cc.find_library('m', required : false)
freetype_dep = dependency('freetype2')
sdl3_dep = dependency('sdl3')
harfbuzz_dep = dependency('harfbuzz')


#sdl3_ttf_dir = include_directories('include', '/usr/include/harfbuzz', '/usr/include/freetype2', '.')
#sdl3_ttf_sys = include_directories('/usr/include/harfbuzz', '/usr/include/freetype2', is_system : true)
sdl3_ttf_inc = include_directories('include')
sdl3_ttf_src = files(
  'src/SDL_hashtable.c',
  'src/SDL_hashtable_ttf.c',
  'src/SDL_gpu_textengine.c',
  'src/SDL_renderer_textengine.c',
  'src/SDL_surface_textengine.c',
  'src/SDL_ttf.c'
)
sdl3_ttf = library(
  'sdl3_ttf',
  sdl3_ttf_src,
  build_by_default : true,
  c_args: c_args,
  dependencies: [freetype_dep, m_dep, sdl3_dep, harfbuzz_dep],
  include_directories : sdl3_ttf_inc,
  install : true,
#  c_args : [
#    '-g',
#    '-fvisibility=hidden',
#    '-DBUILD_SDL',
#    '-DDLL_EXPORT',
#    '-DSDL_BUILD_MAJOR_VERSION=3', '-DSDL_BUILD_MICRO_VERSION=2',
#    '-DSDL_BUILD_MINOR_VERSION=2', '-DTTF_USE_HARFBUZZ=1'
#  ]
)
sdl3_ttf_dep = declare_dependency(
  link_with : sdl3_ttf,
  compile_args : [],
  include_directories : sdl3_ttf_inc,
  link_args : []
)
