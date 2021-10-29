// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FORMATHANDLER_HH_
#define FORMATHANDLER_HH_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>

namespace vivictpp {
namespace libav {

class FormatHandler {
public:
  explicit FormatHandler(std::string inputFile);
  ~FormatHandler();
  const std::vector<AVStream *> &getVideoStreams() const {
    return videoStreams;
  }
  const std::vector<AVStream *> &getAudioStreams() const {
    return audioStreams;
  }
  const std::vector<AVStream *> &getStreams() const {
    return streams;
  }
  AVFormatContext *getFormatContext() const { return formatContext; }
  AVPacket *nextPacket();
  void seek(double t);
  void setStreamActive(int streamIndex);
  void setStreamInactive(int streamIndex);
  void setActiveStreams(const std::set<int> &activeStreams);

public:
  AVFormatContext *formatContext;
  std::string inputFile;

private:
  AVPacket *packet;
  std::vector<AVStream *> videoStreams;
  std::vector<AVStream *> audioStreams;
  std::vector<AVStream *> streams;
  std::set<int> activeStreams;
};

}  // namespace libav
}  // namespace vivictpp

#endif // FORMATHANDLER_HH_
