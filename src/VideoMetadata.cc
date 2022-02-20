// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoMetadata.hh"
#include "time/TimeUtils.hh"

int getBitrate(AVStream *videoStream) {
  int bitrate = videoStream->codecpar->bit_rate;
  if (bitrate == 0) {
    if (auto entry =
            av_dict_get(videoStream->metadata, "variant_bitrate", nullptr, 0)) {
      auto bitrateStr = std::string(entry->value);
      try {
        bitrate = std::stoi(bitrateStr);
      } catch (std::invalid_argument &e) {
        // ignore
      }
    }
  }
  return bitrate;
}

VideoMetadata::VideoMetadata(
    std::string source, AVFormatContext *formatContext,
    //                             AVCodecContext *codecContext,
    AVStream *videoStream)
    : source(source),
      //   pixelFormat(std::string(av_get_pix_fmt_name(codecContext->pix_fmt))),
      streamIndex(videoStream->index),
      width(videoStream->codecpar->width),
      height(videoStream->codecpar->height),
      resolution(width, height),
      bitrate(getBitrate(videoStream)),
      frameRate(av_q2d(videoStream->r_frame_rate)),
      frameDuration(av_rescale_q(1, videoStream->r_frame_rate, vivictpp::time::TIME_BASE_Q)),
      startTime(av_rescale_q(videoStream->start_time, videoStream->time_base, vivictpp::time::TIME_BASE_Q)),
      duration(formatContext->duration), // allready in av_time_base
      endTime(startTime - frameDuration + duration),
      codec(avcodec_get_name(videoStream->codecpar->codec_id)) {}

std::string VideoMetadata::toString() const {
  std::string separator = "\t";
  std::ostringstream oss;
  oss << std::setprecision(3)
      << std::fixed
      //      << std::setw(20) << std::left << "source" <<
      //      this->source.substr(this->source.find_last_of("/") + 1) <<
      //      std::endl
      << std::setw(20) << std::left << "codec" << this->codec << std::endl
      << std::setw(20) << std::left << "resolution" << this->width << "x"
      << this->height << std::endl
      << std::setw(20) << std::left << "bitrate" << this->bitrate / 1000.0
      << "kb/s" << std::endl
      << std::setw(20) << std::left << "frameRate" << this->frameRate << "fps"
      << std::endl
      << std::setw(20) << std::left << "duration"
      << vivictpp::time::formatTime(vivictpp::time::ptsToDouble(this->duration))
      << std::endl
      << std::setw(20) << std::left << "startTime" << vivictpp::time::ptsToDouble(this->startTime)
      << std::endl
      << std::setw(20) << std::left << "pixelFormat" << this->pixelFormat
      << std::endl;
  return oss.str();
}
