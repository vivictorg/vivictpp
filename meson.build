project('vivict++', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

spdlog_proj = subproject('spdlog')
cli11_proj = subproject('cli11')
catch2_proj = subproject('catch2')
fmt_proj = subproject('fmt')
sdl2_proj = subproject('sdl2')
sdl2_ttf_proj = subproject('sdl2_ttf')

deps = [
  sdl2_proj.get_variable('sdl2_dep'),
  sdl2_ttf_proj.get_variable('sdl2_ttf_dep'),
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

incdir = include_directories('include')

git_hash_cmd = run_command('git', 'rev-parse', '--short', 'HEAD')
git_hash = git_hash_cmd.stdout().strip()

sources = [
  'src/AVSync.cc',
  'src/Controller.cc',
  'src/VideoInputs.cc',
  'src/VideoMetadata.cc',
  'src/VivictPP.cc',
  'src/libav/Decoder.cc',
  'src/libav/Filter.cc',
  'src/libav/FormatHandler.cc',
  'src/libav/Frame.cc',
  'src/libav/Packet.cc',
  'src/logging/Logging.cc',
  'src/sdl/SDLAudioOutput.cc',
  'src/sdl/SDLEventLoop.cc',
  'src/sdl/SDLUtils.cc',
  'src/sdl/SDLUtils.cc',
  'src/time/TimeUtils.cc',
  'src/ui/Container.cc',
  'src/ui/Fonts.cc',
  'src/ui/MetadataDisplay.cc',
  'src/ui/ScreenOutput.cc',
  'src/ui/SeekBar.cc',
  'src/ui/TextBox.cc',
  'src/ui/TextTexture.cc',
  'src/ui/VideoDisplay.cc',
  'src/ui/VmafGraph.cc',
  'src/vmaf/VmafLog.cc',
  'src/workers/DecoderWorker.cc',
  'src/workers/FrameBuffer.cc',
  'src/workers/PacketQueue.cc',
  'src/workers/PacketWorker.cc',
  'src/workers/QueuePointer.cc',
  'src/workers/VideoInputMessage.cc',
]

vivictpplib = static_library('vivictpplib',
                             sources: sources,
                             dependencies: deps,
                             include_directories: incdir)

executable('vivictpp', 'src/main.cc', link_with: vivictpplib, dependencies: deps,
           include_directories: incdir, install: true, cpp_args : '-DVPP_GIT_HASH="' + git_hash + '"', link_args: '-g')

#seekTest= executable('seekTest', 'test/SeekTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir)
# test('FormatHandler.seek', seekTest)
playbackTest= executable('playbackTest', 'test/PlaybackTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir)
test('Playback', playbackTest)
