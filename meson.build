project('vivict++', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

#if meson.get_compiler('cpp').get_id() == 'gcc'
  extra_args=['-Wno-shadow', '-Wno-declaration-after-statement']
#else
#  extra_args=[]
#endif

fmt_proj = subproject('fmt', default_options: ['warning_level=0', 'default_library=static'])  
spdlog_proj = subproject('spdlog', default_options: 'warning_level=0')
cli11_proj = subproject('cli11', default_options: 'warning_level=0')
catch2_proj = subproject('catch2', default_options: 'warning_level=0')

if get_option('use_sdl2_subproject')
  sdl2_proj = subproject('sdl2', default_options: ['warning_level=0', 'test=false', 'default_library=static'])
  sdl2_dep = sdl2_proj.get_variable('sdl2_dep')
  sdl2_ttf_proj = subproject('sdl2_ttf', default_options: ['warning_level=0', 'default_library=static'])
  sdl2_ttf_dep = sdl2_ttf_proj.get_variable('sdl2_ttf_dep')
else
  sdl2_dep = dependency('sdl2', fallback: ['sdl2', 'sdl2_dep'])
  sdl2_ttf_dep = dependency('sdl2_ttf', fallback: ['sdl2_ttf', 'sdl2_ttf_dep'])
endif

deps = [
  sdl2_dep,
  sdl2_ttf_dep,
  dependency('freetype2', version: '>=2.8.1'),
  dependency('threads'),
  dependency('libavformat'),
  dependency('libavcodec'),
  dependency('libavutil'),
  dependency('libavfilter'),
  dependency('libswscale'),
  spdlog_proj.get_variable('spdlog_dep'),
  cli11_proj.get_variable('CLI11_dep'),
  fmt_proj.get_variable('fmt_dep')
]

test_deps = [
  catch2_proj.get_variable('catch2_dep')
]

incdir = include_directories('include', 'imgui', 'imgui/backends')

git_hash_cmd = run_command('git', 'rev-parse', '--short', 'HEAD')
git_hash = git_hash_cmd.stdout().strip()


imgui_sources=[
  'imgui/imgui_demo.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/imgui.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/backends/imgui_impl_sdl.cpp',
  #  'imgui/backends/imgui_impl_opengl3.cpp',
  'imgui/backends/imgui_impl_sdlrenderer.cpp',
  'src/main_imgui.cc',
  'src/imgui/VivictPPImGui.cc',
  'src/imgui/ImGuiSDL.cc',
]

sources = [
  'src/AVSync.cc',
  'src/Controller.cc',
  'src/OptParser.cc',
  'src/VideoInputs.cc',
  'src/VideoMetadata.cc',
  'src/VideoPlayback.cc',
  'src/VivictPP.cc',
  'src/libav/Decoder.cc',
  'src/libav/Filter.cc',
  'src/libav/FormatHandler.cc',
  'src/libav/Frame.cc',
  'src/libav/HwAccelUtils.cc',
  'src/libav/Packet.cc',
  'src/libav/Utils.cc',
  'src/logging/Logging.cc',
  'src/sdl/SDLAudioOutput.cc',
  'src/sdl/SDLEventLoop.cc',
  'src/sdl/SDLUtils.cc',
  'src/sdl/SDLUtils.cc',
  'src/time/TimeUtils.cc',
  'src/ui/Container.cc',
  'src/ui/FontSize.cc',
  'src/ui/Fonts.cc',
  'src/ui/MetadataDisplay.cc',
  'src/ui/ScreenOutput.cc',
  'src/ui/SeekBar.cc',
  'src/ui/TextBox.cc',
  'src/ui/TextTexture.cc',
  'src/ui/VideoDisplay.cc',
  'src/ui/VideoScaler.cc',
  'src/ui/VideoTextures.cc',
  'src/ui/VmafGraph.cc',
  'src/vmaf/VmafLog.cc',
  'src/workers/DecoderWorker.cc',
  'src/workers/FrameBuffer.cc',
  'src/workers/PacketQueue.cc',
  'src/workers/PacketWorker.cc',
  'src/workers/QueuePointer.cc',
  'src/workers/VideoInputMessage.cc',
  imgui_sources
]

vivictpplib = static_library('vivictpplib',
                             sources: sources,
                             dependencies: deps,
                             include_directories: incdir,
                             cpp_args: extra_args)

vpp_extra_args = extra_args
vpp_extra_args += '-DVPP_GIT_HASH="' + git_hash + '"'

executable('vivictpp', 'src/main.cc', link_with: vivictpplib, dependencies: deps,
           include_directories: incdir, install: true, cpp_args : vpp_extra_args, link_args: '-g')

#seekTest= executable('seekTest', 'test/SeekTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir)
# test('FormatHandler.seek', seekTest)
playbackTest= executable('playbackTest', 'test/PlaybackTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir, cpp_args: extra_args)
test('Playback', playbackTest)


imgui_deps = deps
#=[
#  sdl2_dep,
#  sdl2_ttf_dep
#  dependency('GL'),
#  dependency('glew')
#]

imgui_incdir = include_directories('include', 'imgui', 'imgui/backends')

executable('imguivpp',
           sources: imgui_sources,
           include_directories: imgui_incdir,
           link_with: vivictpplib,
           dependencies: imgui_deps)
#           link_args: ['-lGL'])
