// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoMetadata.hh"
#include "time/TimeUtils.hh"
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>

int getBitrate(const AVStream *videoStream) {
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

vivictpp::time::Time getStartTime(const AVStream *stream) {
  return stream->start_time == AV_NOPTS_VALUE ? 0 :
    av_rescale_q(stream->start_time, stream->time_base, vivictpp::time::TIME_BASE_Q);
}

const AVRational zeroSafeRational(const AVRational r) {
    return r.num == 0 || r.den == 0 ? AVRational{1,1} : r;
}

VideoMetadata::VideoMetadata():
  resolution(0,0),
  filteredResolution(0,0),
  _empty(true) {}

VideoMetadata::VideoMetadata(
    const std::string &source,
    const AVFormatContext *formatContext,
    const AVStream *videoStream,
    const FilteredVideoMetadata &filteredVideoMetadata)
    : source(source),
      pixelFormat(std::string(av_get_pix_fmt_name((AVPixelFormat) videoStream->codecpar->format))),
      streamIndex(videoStream->index),
      resolution(videoStream->codecpar->width, videoStream->codecpar->height),
      sampleAspectRatio(zeroSafeRational(videoStream->codecpar->sample_aspect_ratio)),
      filteredResolution(filteredVideoMetadata.empty() ? resolution : filteredVideoMetadata.resolution),
      filteredSampleAspectRatio(
              filteredVideoMetadata.empty() ? sampleAspectRatio : filteredVideoMetadata.sampleAspectRatio),
      filteredVideoMetadata(filteredVideoMetadata),
      bitrate(getBitrate(videoStream)),
      frameRate(av_q2d(videoStream->r_frame_rate)),
      frameDuration(av_rescale(vivictpp::time::TIME_BASE, videoStream->r_frame_rate.den,
                               videoStream->r_frame_rate.num)),
      startTime(getStartTime(videoStream)),
      duration(formatContext->duration), // allready in av_time_base
      // endTime is aproximate pts of last frame
      endTime(startTime - frameDuration + duration),
      codec(avcodec_get_name(videoStream->codecpar->codec_id)),
      _empty(false) { }

std::string VideoMetadata::resolutionAsString() const {
  std::ostringstream oss;
  oss << std::left << this->filteredResolution.w << "x"
      << this->filteredResolution.h;
  if (filteredResolution.w != resolution.w || filteredResolution.h != resolution.h) {
    oss << " (" << resolution.w << "x" << resolution.h << ")";
  }
  return oss.str();
}

std::string VideoMetadata::toString() const {
  std::string separator = "\t";
  std::ostringstream oss;
  oss << std::setprecision(3)
      << std::fixed
      //      << std::setw(20) << std::left << "source" <<
      //      this->source.substr(this->source.find_last_of("/") + 1) <<
      //      std::endl
      << std::setw(20) << std::left << "codec" << this->codec << std::endl
//      << std::setw(20) << std::left << "resolution" << this->resolution.w << "x"
//      << this->resolution.h << std::endl
      << std::setw(20) << std::left
      << "resolution" << this->filteredResolution.w << "x"  << this->filteredResolution.h << std::endl;
    if (filteredResolution.w != resolution.w || filteredResolution.h != resolution.h) {
       oss << std::setw(20) << std::left
           << "orig resolution" << this->resolution.w << "x"  << this->resolution.h << std::endl;
    }
    oss  << std::setw(20) << std::left << "bitrate" << this->bitrate / 1000.0
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

FilteredVideoMetadata::FilteredVideoMetadata(std::string filterDefinition ,
                                             Resolution resolution,
                                             AVRational sampleAspectRatio,
                                             double frameRate):
  filteredDefinition(filterDefinition),
  resolution(resolution),
  sampleAspectRatio(zeroSafeRational(sampleAspectRatio)),
  frameRate(frameRate) {}
