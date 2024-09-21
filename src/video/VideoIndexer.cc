// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "video/VideoIndexer.hh"
#include "libav/FormatHandler.hh"
#include "libav/Decoder.hh"
#include "libav/Filter.hh"
#include "time/Time.hh"
#include "time/TimeUtils.hh"

void vivictpp::video::VideoIndexer::prepareIndex(const std::string &inputFile,
                      const std::string &formatOptions) {
    stopIndexThread();
    stopIndexing = false;
    indexingThread = std::make_unique<std::thread>(&VideoIndexer::prepareIndexInternal, this, inputFile, formatOptions);
}

void vivictpp::video::VideoIndexer::prepareIndexInternal(const std::string &inputFile, const std::string &formatOptions) {
    int64_t t0 = vivictpp::time::relativeTimeMicros();
    vivictpp::libav::FormatHandler formatHandler(inputFile, formatOptions);
    if (formatHandler.getVideoStreams().empty()) {
        //throw std::runtime_error("No video streams found in input file");
        logger->warn("Indexing failed, No video streams found in input file");
    }
    index->clear();
    formatHandler.setStreamActive(formatHandler.getVideoStreams()[0]->index);

    vivictpp::libav::DecoderOptions decoderOptions;
    AVStream* stream = formatHandler.getVideoStreams()[0];
    vivictpp::libav::Decoder decoder(stream->codecpar, decoderOptions);
    std::string filterStr = fmt::format("scale=w={}:h={}:force_original_aspect_ratio=decrease:flags=neighbor,"
                                        "format=yuv420p", maxThumbnailSize, maxThumbnailSize);
    vivictpp::libav::VideoFilter filter(stream, decoder.getCodecContext(),
                                        filterStr);

    vivictpp::time::Time lastPts = vivictpp::time::NO_TIME;
    vivictpp::time::Time duration = formatHandler.formatContext->duration;
    // Try to get around 100 thumbnails, with at least 5s interval
    vivictpp::time::Time thumbnailInterval = std::max(duration / maxThumbnails, vivictpp::time::seconds(minThumbnailInterval));

    while (!formatHandler.eof() && !stopIndexing) {
        AVPacket *packet = formatHandler.nextPacket();
        if (packet != nullptr) {
            bool keyFrame = packet->flags & AV_PKT_FLAG_KEY;
            if (keyFrame) {
                vivictpp::time::Time pts = av_rescale_q(packet->pts, stream->time_base, vivictpp::time::TIME_BASE_Q);
                index->addKeyFrame(pts);
                if (lastPts == vivictpp::time::NO_TIME || pts - lastPts >= thumbnailInterval) {
                    lastPts = pts;
                    std::vector<vivictpp::libav::Frame> frames;
                    for (auto frame: decoder.handlePacket(packet)) {
                        frames.push_back(filter.filterFrame(frame));
                    }
                    for (auto frame: decoder.handlePacket(nullptr)) {
                        frames.push_back(filter.filterFrame(frame));
                    }
                    for (auto frame: frames) {
                        if (!frame.empty()) {
                            index->addThumbnail(Thumbnail(pts, frame));
                            break;
                        }
                    }
                    decoder.flush();
                }
            }
            av_packet_unref(packet);
        }
    }

    int64_t t1 = vivictpp::time::relativeTimeMicros();
    logger->debug("Found {} keyframes, generated {} thumbnails", index->getKeyFrames().size(), index->getThumbnails()
    .size
    ());
    logger->debug("Indexing took {} ms", (t1 - t0) / 1000);
}
