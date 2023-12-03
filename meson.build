project('vivict++', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc'
  extra_args=['-Wno-shadow', '-Wno-declaration-after-statement']
else
  extra_args=[]
endif

fmt_proj = subproject('fmt', default_options: ['warning_level=0', 'default_library=static'])  
spdlog_proj = subproject('spdlog', default_options: 'warning_level=0')
cli11_proj = subproject('cli11', default_options: 'warning_level=0')
catch2_proj = subproject('catch2', default_options: 'warning_level=0')
zlib_proj = subproject('zlib', default_options: 'warning_level=0')
freetype2_proj = subproject('freetype2', default_options: 'warning_level=0')
platformfolders_proj = subproject('platformfolders', default_options: 'warning_level=0')

if build_machine.system() == 'windows'

# https://libsdl.org/release/SDL2-devel-2.26.5-VC.zip
# https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.20.2-VC.zip
# https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-5.1.2-full_build-shared.7z

  cc = meson.get_compiler('cpp')

  sdl2_dir = meson.source_root() / 'winlibs' / 'SDL2-2.26.5'
  sdl2_dep = cc.find_library('sdl2', dirs: [sdl2_dir / 'lib' / 'x64'])

  sdl2_ttf_dir = meson.source_root() / 'winlibs' / 'SDL2_ttf-2.20.2'
  sdl2_ttf_dep = cc.find_library('sdl2_ttf', dirs: [sdl2_ttf_dir / 'lib' / 'x64'])

  ffmpeg_dir = meson.source_root() / 'winlibs' / 'ffmpeg-5.1.2-full_build-shared'
  # get_option('ffmpeg_install_dir')

  ffmpeg_inc = [ffmpeg_dir / 'include', sdl2_dir / 'include', sdl2_ttf_dir / 'include']
  libavformat_dep = cc.find_library('avformat', dirs : [ffmpeg_dir / 'lib'])
  libavcodec_dep = cc.find_library('avcodec', dirs : [ffmpeg_dir / 'lib'])
  libavutil_dep = cc.find_library('avutil', dirs : [ffmpeg_dir / 'lib'])
  libavfilter_dep = cc.find_library('avfilter', dirs : [ffmpeg_dir / 'lib'])
  libswscale_dep = cc.find_library('swscale', dirs : [ffmpeg_dir / 'lib'])
else

  if get_option('use_sdl2_subproject')
    sdl2_proj = subproject('sdl2', default_options: ['warning_level=0', 'test=false', 'default_library=static'])
    sdl2_dep = sdl2_proj.get_variable('sdl2_dep')
    sdl2_ttf_proj = subproject('sdl2_ttf', default_options: ['warning_level=0', 'default_library=static'])
    sdl2_ttf_dep = sdl2_ttf_proj.get_variable('sdl2_ttf_dep')
  else
    sdl2_dep = dependency('sdl2', fallback: ['sdl2', 'sdl2_dep'])
    sdl2_ttf_dep = dependency('sdl2_ttf', fallback: ['sdl2_ttf', 'sdl2_ttf_dep'])
  endif

  libavformat_dep = dependency('libavformat')
  libavcodec_dep = dependency('libavcodec')
  libavutil_dep =  dependency('libavutil')
  libavfilter_dep =  dependency('libavfilter')
  libswscale_dep =  dependency('libswscale')
  ffmpeg_inc = []
endif



deps = [
  zlib_proj.get_variable('zlib_dep'),
  sdl2_dep,
  sdl2_ttf_dep,
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
                             'imgui',
                             'imgui/backends',
                             'libs/ImGuiFileDialog',
                             'libs/tomlplusplus',
                             ffmpeg_inc)

git_hash_cmd = run_command('git', 'rev-parse', '--short', 'HEAD')
git_hash = git_hash_cmd.stdout().strip()


imgui_sources=[
  'imgui/imgui_demo.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/imgui.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/backends/imgui_impl_sdl.cpp',
  'imgui/backends/imgui_impl_sdlrenderer.cpp',
  'src/imgui/VivictPPImGui.cc',
  'src/imgui/Controls.cc',
  'src/imgui/VideoMetadataDisplay.cc',
  'src/imgui/ImGuiSDL.cc',
  'src/imgui/Fonts.cc',
  'src/imgui/MainMenu.cc',
  'src/imgui/FileDialog.cc',
  'src/imgui/Help.cc',
  'src/imgui/About.cc',
  'src/imgui/SettingsDialog.cc',
  'src/imgui/WidgetUtils.cc',
  'src/imgui/Logs.cc',
  'libs/ImGuiFileDialog/ImGuiFileDialog.cpp'
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
  'src/sdl/SDLAudioOutput.cc',
  'src/sdl/SDLUtils.cc',
  'src/time/TimeUtils.cc',
  'src/ui/FontSize.cc',
  'src/ui/VideoTextures.cc',
  'src/vmaf/VmafLog.cc',
  'src/workers/DecoderWorker.cc',
  'src/workers/FrameBuffer.cc',
  'src/workers/PacketQueue.cc',
  'src/workers/PacketWorker.cc',
  'src/workers/QueuePointer.cc',
  'src/workers/VideoInputMessage.cc',
  imgui_sources
]

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
