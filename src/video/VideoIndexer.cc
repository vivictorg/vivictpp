// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "video/VideoIndexer.hh"
#include "libav/Decoder.hh"
#include "libav/Filter.hh"
#include "libav/FormatHandler.hh"
#include "time/Time.hh"
#include "time/TimeUtils.hh"

std::string thumbnailFilterStr(int maxThumbnailSize) {
  std::string filterStr = fmt::format(
      "scale=w={}:h={}:force_original_aspect_ratio=decrease:flags=neighbor,"
      "format=yuv420p",
      maxThumbnailSize, maxThumbnailSize);
  return filterStr;
}

class ThumbnailDecoder {
private:
  vivictpp::libav::Decoder decoder;
  vivictpp::libav::VideoFilter filter;

public:
  ThumbnailDecoder(AVStream *stream, int maxThumbnailSize)
      : decoder(stream->codecpar, vivictpp::libav::DecoderOptions()),
        filter(stream, decoder.getCodecContext(),
               thumbnailFilterStr(maxThumbnailSize)) {}

  std::vector<vivictpp::libav::Frame> decode(AVPacket *packet) {
    std::vector<vivictpp::libav::Frame> frames;
    for (auto frame : decoder.handlePacket(packet)) {
      frames.push_back(filter.filterFrame(frame));
    }
    for (auto frame : decoder.handlePacket(nullptr)) {
      frames.push_back(filter.filterFrame(frame));
    }
    decoder.flush();
    return frames;
  }
};

void pairsort(std::vector<float> &a, std::vector<float> &b, int n) {
  std::vector<std::pair<float, float>> pairt(n);

  // Storing the respective array
  // elements in pairs.
  for (int i = 0; i < n; i++) {
    pairt[i].first = a[i];
    pairt[i].second = b[i];
  }

  // Sorting the pair array.
  std::sort(pairt.begin(), pairt.end());

  // Modifying original arrays
  for (int i = 0; i < n; i++) {
    a[i] = pairt[i].first;
    b[i] = pairt[i].second;
  }
}

void vivictpp::video::VideoIndex::addFrameData(
    const IndexFrameData &frameData) {
  std::lock_guard<std::mutex> lg(m);
  this->ptsValues.push_back(frameData.pts);
  this->frameSizes.push_back(frameData.size);
  this->keyFrameFlag.push_back(frameData.keyFrame);
  plotDatas.frameSize.pts.push_back(vivictpp::time::ptsToDouble(frameData.pts));
  plotDatas.frameSize.values.push_back(frameData.size);
  if (frameData.keyFrame) {
    keyFrames.push_back(frameData.pts);
    addGop(frameData.pts);
  }
  currentGopSize += frameData.size;
}

void vivictpp::video::VideoIndex::addGop(const vivictpp::time::Time gopEndPts) {
  float newGopPts = vivictpp::time::ptsToDouble(gopEndPts);
  if (keyFrames.size() > 1) {
    plotDatas.gopBitrate.pts.push_back(currentGopPts);
    plotDatas.gopBitrate.values.push_back(currentGopSize * 8 /
                                          (newGopPts - currentGopPts));
  }
  currentGopSize = 0;
  currentGopPts = newGopPts;
}

void vivictpp::video::VideoIndex::addThumbnail(
    const vivictpp::video::Thumbnail &thumbnail) {
  std::lock_guard<std::mutex> lg(m);
  thumbnails.push_back(thumbnail);
}

void vivictpp::video::VideoIndex::finalizeIndex() {
  std::lock_guard<std::mutex> lg(m);
  if (ptsValues.size() < 2) {
    return;
  }
  vivictpp::time::Time end =
      2 * ptsValues.back() - ptsValues[ptsValues.size() - 2];
  addGop(end);
  pairsort(plotDatas.frameSize.pts, plotDatas.frameSize.values,
           plotDatas.frameSize.pts.size());
  indexingDone = true;
}

void vivictpp::video::VideoIndex::clear() {
  std::lock_guard<std::mutex> lg(m);
  thumbnails.clear();
  keyFrames.clear();
  ptsValues.clear();
  plotDatas.clear();
  frameSizes.clear();
  keyFrameFlag.clear();
  indexingDone = false;
}

void vivictpp::video::VideoIndexer::prepareIndex(
    const std::string &inputFile, const std::string &formatOptions,
    const bool generatThumbnails) {
  stopIndexThread();
  stopIndexing = false;
  indexingThread = std::make_unique<std::thread>(
      &VideoIndexer::prepareIndexInternal, this, inputFile, formatOptions,
      generatThumbnails);
}

void vivictpp::video::VideoIndexer::prepareIndexInternal(
    const std::string &inputFile, const std::string &formatOptions,
    const bool generateThumbnails) {
  int64_t t0 = vivictpp::time::relativeTimeMicros();
  vivictpp::libav::FormatHandler formatHandler(inputFile, formatOptions);
  if (formatHandler.getVideoStreams().empty()) {
    // throw std::runtime_error("No video streams found in input file");
    logger->warn("Indexing failed, No video streams found in input file");
  }
  index->clear();
  std::set<int> activeStreams({formatHandler.getVideoStreams()[0]->index});
  formatHandler.setActiveStreams(activeStreams);

  std::unique_ptr<ThumbnailDecoder> thumbnailDecoder;
  if (generateThumbnails) {
    thumbnailDecoder = std::make_unique<ThumbnailDecoder>(
        formatHandler.getVideoStreams()[0], maxThumbnailSize);
  }

  vivictpp::time::Time lastPts = vivictpp::time::NO_TIME;
  vivictpp::time::Time duration = formatHandler.formatContext->duration;
  AVRational streamTimeBase = formatHandler.getVideoStreams()[0]->time_base;
  // Try to get around 100 thumbnails, with at least 5s interval
  vivictpp::time::Time thumbnailInterval = std::max(
      duration / maxThumbnails, vivictpp::time::seconds(minThumbnailInterval));

  while (!formatHandler.eof() && !stopIndexing) {
    AVPacket *packet = formatHandler.nextPacket();
    if (packet != nullptr) {
      vivictpp::time::Time pts = av_rescale_q(packet->pts, streamTimeBase,
                                              vivictpp::time::TIME_BASE_Q);
      bool keyFrame = packet->flags & AV_PKT_FLAG_KEY;
      index->addFrameData({pts, packet->size, keyFrame});
      if (keyFrame && generateThumbnails &&
          (lastPts == vivictpp::time::NO_TIME ||
           pts - lastPts >= thumbnailInterval)) {
        lastPts = pts;
        for (auto frame : thumbnailDecoder->decode(packet)) {
          if (!frame.empty()) {
            index->addThumbnail(Thumbnail(pts, frame));
            break;
          }
        }
      }
      av_packet_unref(packet);
    }
  }
  index->finalizeIndex();
  int64_t t1 = vivictpp::time::relativeTimeMicros();
  logger->debug("Found {} keyframes, generated {} thumbnails",
                index->getKeyFrames().size(), index->getThumbnails().size());
  logger->debug("Indexing took {} ms", (t1 - t0) / 1000);
}
