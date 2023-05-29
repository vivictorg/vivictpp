// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include "libav/HwAccelUtils.hh"

extern "C" {
#include <libavutil/pixdesc.h>
}

#include <algorithm>

#include "logging/Logging.hh"

static const std::vector<AVPixelFormat> preferredPixelFormats = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P10, AV_PIX_FMT_NV12, AV_PIX_FMT_P010};

AVPixelFormat vivictpp::libav::selectSwPixelFormat(AVBufferRef *hwFramesCtx) {
  std::vector<AVPixelFormat> availableTargetFormats;
  enum AVPixelFormat *pixelFormats;
  av_hwframe_transfer_get_formats(hwFramesCtx, AV_HWFRAME_TRANSFER_DIRECTION_FROM,
                                  &pixelFormats, 0);

  for(enum AVPixelFormat *curr = pixelFormats; *curr != AV_PIX_FMT_NONE; curr++) {
    availableTargetFormats.push_back(*curr);
    spdlog::info("possible target format: {}", av_get_pix_fmt_name(*curr));
  }
  av_free(pixelFormats);
  for(const auto &format : preferredPixelFormats) {
    if(std::find(availableTargetFormats.begin(), availableTargetFormats.end(), format) != availableTargetFormats.end()) {
      return format;
    }
  }
  return availableTargetFormats[0];
}

bool vivictpp::libav::isHwAccelFormat(AVPixelFormat pixelFormat) {
  const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(pixelFormat);
  return descriptor->flags & AV_PIX_FMT_FLAG_HWACCEL;
}

std::vector<std::string> vivictpp::libav::allHwAccelFormats() {
  std::vector<std::string> formats;

  enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
  while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
    formats.push_back(av_hwdevice_get_type_name(type));
  }
  formats.push_back("test-format1");
  formats.push_back("test-format2");
  return formats;
}

/*
std::string
enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;

    printf("Hardware acceleration methods:\n");
    while ((type = av_hwdevice_iterate_types(type)) !=
           AV_HWDEVICE_TYPE_NONE)
        printf("%s\n", av_hwdevice_get_type_name(type));
    printf("\n");
    return 0;
 */
