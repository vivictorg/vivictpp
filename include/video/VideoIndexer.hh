// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_VIDEOINDEXER_HH
#define VIVICTPP_VIDEOINDEXER_HH

#include "logging/Logging.hh"
#include "time/Time.hh"
#include "video/Thumbnail.hh"
#include <mutex>
#include <vector>

namespace vivictpp::video {

struct IndexFrameData {
  vivictpp::time::Time pts;
  int size;
  bool keyFrame;
};

class VideoIndex {

private:
  std::vector<vivictpp::time::Time> keyFrames;
  std::vector<vivictpp::time::Time> ptsValues;
  std::vector<int> frameSizes;
  std::vector<bool> keyFrameFlag;
  std::vector<vivictpp::video::Thumbnail> thumbnails;
  mutable std::mutex m;

private:
  void addFrameData(const IndexFrameData &frameData) {
    std::lock_guard<std::mutex> lg(m);
    this->ptsValues.push_back(frameData.pts);
    this->frameSizes.push_back(frameData.size);
    this->keyFrameFlag.push_back(frameData.keyFrame);
    if (frameData.keyFrame) {
      keyFrames.push_back(frameData.pts);
    }
  }

  void addThumbnail(const vivictpp::video::Thumbnail &thumbnail) {
    std::lock_guard<std::mutex> lg(m);
    thumbnails.push_back(thumbnail);
  }
  void clear() {
    std::lock_guard<std::mutex> lg(m);
    thumbnails.clear();
    keyFrames.clear();
  }

  friend class VideoIndexer;

public:
  VideoIndex() {}

  const std::vector<vivictpp::time::Time> &getKeyFrames() const {
    std::lock_guard<std::mutex> lg(m);
    return keyFrames;
  }
  const std::vector<vivictpp::video::Thumbnail> &getThumbnails() const {
    std::lock_guard<std::mutex> lg(m);
    return thumbnails;
  }
  const std::vector<vivictpp::time::Time> &getPtsValues() {
    std::lock_guard<std::mutex> lg(m);
    return ptsValues;
  }
  const std::vector<int> &getFrameSizes() {
    std::lock_guard<std::mutex> lg(m);
    return frameSizes;
  }
};

class VideoIndexer {
public:
  VideoIndexer()
      : logger(vivictpp::logging::getOrCreateLogger(
            "vivictpp::video::VideoIndexer")) {
    index = std::make_shared<VideoIndex>();
  }
  ~VideoIndexer() { stopIndexThread(); }
  void prepareIndex(const std::string &inputFile,
                    const std::string &formatOptions,
                    const bool generatThumbnails = true);

  const std::shared_ptr<VideoIndex> getIndex() const { return index; }

private:
  void prepareIndexInternal(const std::string &inputFile,
                            const std::string &formatOptions,
                            bool generateThumbnails);
  void stopIndexThread() {
    if (indexingThread) {
      stopIndexing = true;
      indexingThread->join();
      indexingThread.reset();
    }
  }

private:
  vivictpp::logging::Logger logger;
  std::unique_ptr<std::thread> indexingThread;
  std::shared_ptr<VideoIndex> index;
  std::atomic_bool stopIndexing{false};
  const int maxThumbnails = 200;
  const int maxThumbnailSize = 256;
  const int minThumbnailInterval = 5;
};

}; // namespace vivictpp::video

#endif // VIVICTPP_VIDEOINDEXER_HH
