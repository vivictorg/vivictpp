// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/FormatHandler.hh"
#include "libav/AVErrorUtils.hh"

#include "spdlog/spdlog.h"

#include <iostream>
#include <libavutil/dict.h>
#include <string>

void parseFormatOptions(std::string formatOptions, std::string &format, AVDictionary** options) {
    if (formatOptions.empty()) {
        return;
    }
    size_t start;
    size_t end = 0;
    std::string delim(":");
    while ((start = formatOptions.find_first_not_of(delim, end)) != std::string::npos)
    {
      end = formatOptions.find(delim, start);
      std::string keyValue = formatOptions.substr(start, end - start);
      size_t index = keyValue.find("=");
      if (index != std::string::npos){
        std::string key = keyValue.substr(0, index);
        std::string value = keyValue.substr(index+1);
        if (key == std::string("format")) {
          format = value;
        } else {
          av_dict_set(options, key.c_str(), value.c_str(), 0);
        }
      } else {
        av_dict_set(options, keyValue.c_str(), nullptr, 0);
      }
    }
}

vivictpp::libav::FormatHandler::FormatHandler(std::string inputFile, std::string formatOptions)
    : formatContext(nullptr), inputFile(inputFile), packet(nullptr),
      logger(vivictpp::logging::getOrCreateLogger("FormatHandler")) {
#if LIBAVFORMAT_VERSION_MAJOR >= 59
  const AVInputFormat *inputFormat = nullptr;
#else
  AVInputFormat *inputFormat = nullptr;
#endif
  AVDictionary *options = NULL;
  std::string format;
  parseFormatOptions(formatOptions, format, &options);
  if (!format.empty() && !(inputFormat = av_find_input_format(format.c_str()))) {
    av_dict_free(&options);
    throw std::runtime_error(std::string("Unknown format: ") + format);
  }

  vivictpp::libav::AVResult result = avformat_open_input(&this->formatContext, this->inputFile.c_str(),
                                                         inputFormat, &options);
  av_dict_free(&options);
  if (result.error()) {
    throw std::runtime_error(std::string("Failed to open input: ") + result.getMessage());
  }

  // Retrieve stream information
  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    throw std::runtime_error("Failed to find stream info");
  }

  // Dump information about file onto standard error
  //  av_dump_format(formatContext, 0, this->inputFile.c_str(), 0);

  for (unsigned int i = 0; i < this->formatContext->nb_streams; i++) {
    this->streams.push_back(this->formatContext->streams[i]);
    switch (this->formatContext->streams[i]->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      this->videoStreams.push_back(this->formatContext->streams[i]);
      break;
    case AVMEDIA_TYPE_AUDIO:
      this->audioStreams.push_back(this->formatContext->streams[i]);
      break;
    default:
      ;
    }
    this->formatContext->streams[i]->discard = AVDISCARD_ALL;
  }

  this->packet = av_packet_alloc();
}

vivictpp::libav::FormatHandler::~FormatHandler() {
  if (formatContext) {
    avformat_close_input(&this->formatContext);
  }
  if (packet) {
    av_packet_unref(packet);
  }
}

void vivictpp::libav::FormatHandler::setActiveStreams(const std::set<int> &activeStreams) {
  for (unsigned int i = 0; i < this->formatContext->nb_streams; i++) {
    if (activeStreams.find(i) != activeStreams.end()) {
      setStreamActive(i);
    } else {
      setStreamInactive(i);
    }
  }
}

void vivictpp::libav::FormatHandler::setStreamActive(int streamIndex) {
  spdlog::debug("FormatHandler::setStreamActive streamIndex={}", streamIndex);
  this->formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;
  activeStreams.insert(streamIndex);
}

void vivictpp::libav::FormatHandler::setStreamInactive(int streamIndex) {
  this->formatContext->streams[streamIndex]->discard = AVDISCARD_ALL;
  activeStreams.erase(streamIndex);
}

AVStream* firstActive(const std::vector<AVStream*> &streams) {
   auto it =std::find_if(streams.begin(), streams.end(), [](const AVStream* entry){
      return entry->discard == AVDISCARD_DEFAULT;
    });
   return it == streams.end() ? nullptr : *it;
}

void vivictpp::libav::FormatHandler::seek(vivictpp::time::Time t) {
  spdlog::debug("FormatHandler::seek t={}", t);
  AVStream *stream = firstActive(videoStreams);
  if (stream == nullptr) {
    stream = firstActive(audioStreams);
  }
  if (stream == nullptr) {
    return;
  }
  vivictpp::time::Time seek_t = t;
  int flags = 0;
  if (std::strcmp("hls", this->formatContext->iformat->name) == 0) {
    // Seeking in a hls stream seeks to the first keyframe after the given
    // timestamp To ensure we seek to a iframe before the point we want to reach
    // we seek to a point 5s before
    spdlog::debug("Using hls seek");
    seek_t -= 10 * vivictpp::time::TIME_BASE;
    spdlog::debug("FormatHandler::seek Adjusted seek: {}", seek_t);
    flags = AVSEEK_FLAG_BACKWARD;
  }

  int64_t ts = av_rescale_q(seek_t, vivictpp::time::TIME_BASE_Q, stream->time_base);

  if (stream->start_time != AV_NOPTS_VALUE && stream->start_time > ts) {
      ts = stream->start_time;
  } else {
      flags = AVSEEK_FLAG_BACKWARD;
  }
  spdlog::debug("vivictpp::libav::FormatHandler::seek ts={}", ts);
  vivictpp::libav::AVResult result = av_seek_frame(this->formatContext, stream->index,
                                                   ts, flags);
  if (result.error()) {
      logger->error("Seek failed: {}", result.getMessage());
      throw std::runtime_error("Seek failed: " + result.getMessage());
  }
}

AVPacket *vivictpp::libav::FormatHandler::nextPacket() {
  vivictpp::libav::AVResult ret;
  while ((ret = av_read_frame(this->formatContext, this->packet)).success()) {
      spdlog::debug("FormatHandler::nextPacket  Got packet: dts={} stream_index={}",
                   this->packet->dts, this->packet->stream_index);
    if (activeStreams.find(packet->stream_index) != activeStreams.end()) {
      return this->packet;
    }
  }
  if (ret.eof()) {
    return nullptr;
  } else {
    throw std::runtime_error("av_read_frame returned error: " + ret.getMessage());
  }
}
