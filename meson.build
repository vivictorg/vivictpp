project('vivict++', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])
add_project_arguments('-fPIC', language: ['c', 'cpp']) # Needed for SDL3

cmake = import('cmake')

extra_args = ['-D__STDC_CONSTANT_MACROS']
if meson.get_compiler('cpp').get_id() == 'gcc'
  extra_args += '-Wno-shadow'
endif

opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({'SDL3_BUILD_TESTS': 'OFF'})
opt_var.add_cmake_defines({'SDL3_BUILD_EXAMPLES': 'OFF'})
opt_var.add_cmake_defines({'POSITION_INDEPENDENT_CODE': 'ON'})
opt_var.append_compile_args('c++', '-fPIC')
opt_var.append_compile_args('c', '-fPIC')

fmt_proj = subproject('fmt', default_options: ['warning_level=0', 'default_library=static'])  
spdlog_proj = subproject('spdlog', default_options: 'warning_level=0')
cli11_proj = subproject('cli11', default_options: 'warning_level=0')
catch2_proj = subproject('catch2', default_options: 'warning_level=0')
freetype2_proj = subproject('freetype2', default_options: 'warning_level=0')
platformfolders_proj = subproject('platformfolders', default_options: 'warning_level=0')
#sdl3_proj = subproject('sdl3', default_options: ['warning_level=0', 'default_library=static','b_staticpic=true'])

if get_option('use_zlib_subproject')
  zlib_proj = subproject('zlib', default_options: ['warning_level=0', 'default_library=static'])
  zlib_dep = zlib_proj.get_variable('zlib_dep')
else
  zlib_dep = dependency('zlib')
endif

if build_machine.system() == 'windows'

# https://libsdl.org/release/SDL2-devel-2.26.5-VC.zip
# https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.20.2-VC.zip
# https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-7.0.2-full_build-shared.7z

  cc = meson.get_compiler('cpp')

  sdl3_dir = meson.project_source_root() / 'winlibs' / 'SDL3'
  sdl3_dep = cc.find_library('sdl3', dirs: [sdl3_dir / 'lib' / 'x64'])

  sdl3_ttf_dir = meson.project_source_root() / 'winlibs' / 'SDL3_ttf'
  sdl3_ttf_dep = cc.find_library('sdl3_ttf', dirs: [sdl3_ttf_dir / 'lib' / 'x64'])

  ffmpeg_dir = meson.project_source_root() / 'winlibs' / 'ffmpeg'
  # get_option('ffmpeg_install_dir')

  ffmpeg_inc = [ffmpeg_dir / 'include', sdl3_dir / 'include', sdl3_ttf_dir / 'include']
  libavformat_dep = cc.find_library('avformat', dirs : [ffmpeg_dir / 'lib'])
  libavcodec_dep = cc.find_library('avcodec', dirs : [ffmpeg_dir / 'lib'])
  libavutil_dep = cc.find_library('avutil', dirs : [ffmpeg_dir / 'lib'])
  libavfilter_dep = cc.find_library('avfilter', dirs : [ffmpeg_dir / 'lib'])
  libswscale_dep = cc.find_library('swscale', dirs : [ffmpeg_dir / 'lib'])
else
  opt_var = cmake.subproject_options()
  opt_var.add_cmake_defines({'SDL3_BUILD_TESTS': 'OFF'})
  opt_var.add_cmake_defines({'SDL3_BUILD_EXAMPLES': 'OFF'})
  opt_var.add_cmake_defines({'POSITION_INDEPENDENT_CODE': 'ON'})
  opt_var.append_compile_args('c++', '-fPIC')
  opt_var.append_compile_args('c', '-fPIC')

sdl3_proj = cmake.subproject('sdl3', options: opt_var)

#binary_dir = sdl3_dep.get_variable('SDL3_BINARY_DIR')
#message('binary_dir: ' + binary_dir)
#sdl3_dep.get_variable('', apa: 'bepa')
#  message('CMake targets:\n - ' + '\n - '.join(sdl3_proj.target_list()))
  sdl3_dep = sdl3_proj.dependency('SDL3-shared')

  
#  sdl3_ttf_options = cmake.subproject_options()
#  sdl3_ttf_options.add_cmake_defines({'SDL3_DIR': '/home/gustav/gitrepos/vivictorg/vivictpp/build-sdl3/subprojects/SDL3-3.2.14/__CMake_build'})
  #sdl3_ttf_options.add_cmake_defines({'SDL3_DIR': meson.project_build_root() + '/subprojects/SDL3-3.2.14/__CMake_build'})
#  cmake_args: '-DSDL3_DIR=build-sdl3/subprojects/SDL3-3.2.14/__CMake_build/')
#  sdl3_ttf_project = cmake.subproject('sdl3_ttf', options: sdl3_ttf_options)
  #sdl3_ttf_dep = sdl3_ttf_project.dependency('SDL3_ttf_shared')
  sdl3_ttf_proj = subproject('sdl3_ttf', default_options: 'warning_level=0')
  sdl3_ttf_dep = sdl3_ttf_proj.get_variable('sdl3_ttf_dep')


  libavformat_dep = dependency('libavformat')
  libavcodec_dep = dependency('libavcodec')
  libavutil_dep =  dependency('libavutil')
  libavfilter_dep =  dependency('libavfilter')
  libswscale_dep =  dependency('libswscale')
  ffmpeg_inc = []
endif



deps = [
  zlib_dep,
  sdl3_dep,
  sdl3_ttf_dep,
  #sdl3_proj.get_variable('sdl3_dep'),
  #  dependency('freetype2', version: '>=2.8.1'),
  #  freetype2_proj.get_variable('freetype2_dep'),
  dependency('threads'),
  libavformat_dep,
  libavcodec_dep,
  libavutil_dep,
  libavfilter_dep,
  libswscale_dep,

  spdlog_proj.get_variable('spdlog_dep'),
  cli11_proj.get_variable('CLI11_dep'),
  fmt_proj.get_variable('fmt_dep'),
  platformfolders_proj.get_variable('platformfolders_dep')
#  sdl2_image_proj.get_variable('sdl2_image_dep')        
]

test_deps = [
  catch2_proj.get_variable('catch2_dep')
]

incdir = include_directories('include',
                             'libs/imgui',
                             'libs/imgui/backends',
                             'libs/ImGuiFileDialog',
                             'libs/tomlplusplus',
                             'libs/json',
                             ffmpeg_inc)

git_hash_cmd = run_command('git', 'rev-parse', '--short', 'HEAD')
git_hash = git_hash_cmd.stdout().strip()


imgui_sources=[
  'libs/imgui/imgui_demo.cpp',
  'libs/imgui/imgui_widgets.cpp',
  'libs/imgui/imgui.cpp',
  'libs/imgui/imgui_draw.cpp',
  'libs/imgui/imgui_tables.cpp',
  'libs/imgui/backends/imgui_impl_sdl3.cpp',
  'libs/imgui/backends/imgui_impl_sdlrenderer3.cpp',
  'src/imgui/VivictPPImGui.cc',
  'src/imgui/Controls.cc',
  'src/imgui/VideoMetadataDisplay.cc',
  'src/imgui/ImGuiSDL.cc',
  'src/imgui/Fonts.cc',
  'src/imgui/MainMenu.cc',
  'src/imgui/FileDialog.cc',
  'src/imgui/Help.cc',
  'src/imgui/About.cc',
  'src/imgui/PlotWindow.cc',
  'src/imgui/QualityFileDialog.cc',
  'src/imgui/SettingsDialog.cc',
  'src/imgui/VideoWindow.cc',
  'src/imgui/WidgetUtils.cc',
  'src/imgui/Logs.cc',
  'libs/ImGuiFileDialog/ImGuiFileDialog.cpp',
  'libs/implot/implot.cpp',
  'libs/implot/implot_items.cpp'
]

sources = [
  'src/AVSync.cc',
  'src/OptParser.cc',
  'src/Resolution.cc',
  'src/Settings.cc',
  'src/VideoInputs.cc',
  'src/VideoMetadata.cc',
  'src/VideoPlayback.cc',
  'src/libav/Decoder.cc',
  'src/libav/Filter.cc',
  'src/libav/FormatHandler.cc',
  'src/libav/Frame.cc',
  'src/libav/HwAccelUtils.cc',
  'src/libav/Packet.cc',
  'src/libav/Utils.cc',
  'src/logging/Logging.cc',
  #'src/sdl/SDLAudioOutput.cc',
  'src/sdl/SDLUtils.cc',
  'src/time/TimeUtils.cc',
  'src/qualitymetrics/QualityMetrics.cc',
  'src/ui/FontSize.cc',
  'src/ui/VideoTextures.cc',
  'src/ui/ThumbnailTexture.cc',
  'src/video/VideoIndexer.cc',
  'src/vmaf/VmafLog.cc',
  'src/workers/DecoderWorker.cc',
  'src/workers/FrameBuffer.cc',
  'src/workers/PacketQueue.cc',
  'src/workers/PacketWorker.cc',
  'src/workers/QueuePointer.cc',
  'src/workers/VideoInputMessage.cc',
  imgui_sources
]


if build_machine.system() == 'windows'
  sources += 'libs/utf8/utf8.cpp'
endif


extra_args += '-DVPP_GIT_HASH="' + git_hash + '"'

vivictpplib = static_library('vivictpplib',
                             sources: sources,
                             dependencies: deps,
                             include_directories: incdir,
                             cpp_args: extra_args)

vpp_extra_args = extra_args
#vpp_extra_args += '-DVPP_GIT_HASH="' + git_hash + '"'

executable('vivictpp', 'src/main.cc', link_with: vivictpplib, dependencies: deps,
           include_directories: incdir, install: true, cpp_args : vpp_extra_args, link_args: '-g',
           win_subsystem: 'windows')

#seekTest= executable('seekTest', 'test/SeekTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir)
# test('FormatHandler.seek', seekTest)
#playbackTest= executable('playbackTest', 'test/PlaybackTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir, cpp_args: extra_args)
#test('Playback', playbackTest)
settingsTest = executable('settingsTest', 'test/SettingsTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir, cpp_args: extra_args)
test('Settings', settingsTest)
qualitymetricsTest = executable('qualitymetricsTest', 'test/qualitymetrics/QualityMetricsTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir, cpp_args: extra_args)
test('QualityMetrics', qualitymetricsTest)
