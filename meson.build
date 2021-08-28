project('vivict++', 'cpp')

spdlog_proj = subproject('spdlog')
cli11_proj = subproject('cli11')
catch2_proj = subproject('catch2')

deps = [
  dependency('sdl2', version: '>=2.0.8'),
  dependency('SDL2_ttf', version: '>=2.0.14'),
  dependency('freetype2', version: '>=2.8.1'),
  dependency('threads'),
  dependency('libavformat'),
  dependency('libavcodec'),
  dependency('libavutil'),
  dependency('libavfilter'),
  dependency('libswscale'),
  spdlog_proj.get_variable('spdlog_dep'),
  cli11_proj.get_variable('CLI11_dep')
]

test_deps = [
  catch2_proj.get_variable('catch2_dep')
]

incdir = include_directories('include')

vivictpplib = static_library('vivictpplib', 'src/libav/Decoder.cc', 'src/libav/FormatHandler.cc',
                             'src/ui/TextBox.cc', 'src/VideoInputs.cc', 'src/sdl/AudioOutput.cc',
                             'src/workers/FrameBuffer.cc', 'src/workers/QueuePointer.cc', 'src/VideoMetadata.cc',
                             'src/workers/VideoInputMessage.cc', 'src/AVSync.cc',
                             'src/ui/ScreenOutput.cc', 'src/VivictPP.cc', 'src/sdl/SDLUtils.cc',
                             'src/sdl/SDLEventLoop.cc', 'src/TimeUtils.cc', 'src/sdl/SDLUtils.cc',
                             'src/libav/Filter.cc', 'src/workers/PacketQueue.cc',
                             'src/workers/DecoderWorker.cc', 'src/workers/PacketWorker.cc',
                             'src/libav/Packet.cc', 'src/vmaf/VmafLog.cc',
                             'src/libav/Frame.cc', 'src/logging/Logging.cc',
                             'src/ui/VmafGraph.cc', 'src/ui/Fonts.cc', 'src/ui/TextTexture.cc',
                             dependencies: deps, include_directories: incdir)

executable('vivictpp', 'src/main.cc', link_with: vivictpplib, dependencies: deps, include_directories: incdir, install: true)

seekTest = executable('seekTest', 'test/SeekTest.cc', link_with: vivictpplib,  dependencies: deps + test_deps, include_directories: incdir)
test('FormatHandler.seek', seekTest)
