// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/FrameBuffer.hh"

#include <iostream>
#include <libavutil/avutil.h>
#include <sstream>

#include "logging/Logging.hh"

std::string
ptsBufferToString(const std::vector<vivictpp::time::Time> &ptsBuffer) {
  std::ostringstream os;
  int c = 0;
  os << "[";
  for (vivictpp::time::Time i : ptsBuffer) {
    if (c++ > 0) {
      os << ",";
    }
    os << i;
  }
  os << "]";
  std::string str(os.str());
  return str;
}

vivictpp::workers::FrameBuffer::FrameBuffer(int _maxSize)
    : logger(vivictpp::logging::getOrCreateLogger(
          "vivictpp::workers::FrameBuffer")),
      queue(_maxSize), ptsBuffer(_maxSize), _writePos(0, _maxSize),
      _cursor(0, _maxSize), _size(0), _maxSize(_maxSize), mutex() {
  for (int i = 0; i < _maxSize; i++) {
    queue[i] = vivictpp::libav::Frame::emptyFrame();
  }
}

bool vivictpp::workers::FrameBuffer::isFull() {
  const std::lock_guard<std::mutex> lock(mutex);
  return _size == _maxSize;
}

bool vivictpp::workers::FrameBuffer::waitForNotFull(
    const std::chrono::milliseconds &relTime) {
  std::unique_lock<std::mutex> lock(mutex);
  bool result = false;
  if (_size < _maxSize) {
    result = true;
  } else {
    result = conditionVariable.wait_for(lock, relTime,
                                        [&] { return _size < _maxSize; });
  }
  logger->trace("waitForNotNull _size={} _maxSize={} returning {}", _size,
                _maxSize, result);
  return result;
}

int vivictpp::workers::FrameBuffer::size() {
  const std::lock_guard<std::mutex> lock(mutex);
  return _size;
}

void vivictpp::workers::FrameBuffer::write(vivictpp::libav::Frame frame,
                                           vivictpp::time::Time pts) {
  bool wasEmpty = false;
  {
    const std::lock_guard<std::mutex> lock(mutex);
    if (_size == _maxSize) {
      throw std::runtime_error("Buffer is full");
    }
    wasEmpty = _size == 0;
    queue[_writePos] = frame;
    ptsBuffer[_writePos.getValue()] = pts;
    _writePos = _writePos + 1;
    _size++;
  }
  if (wasEmpty) {
    conditionVariable.notify_all();
  }
  logger->debug("Wrote frame with pts {}, size is now {}", pts, _size);
  logger->trace("_size={}, ptsBuffer: {}", _size, ptsBufferToString(ptsBuffer));
}

bool vivictpp::workers::FrameBuffer::isEmpty() {
  const std::lock_guard<std::mutex> lock(mutex);
  return _size == 0;
}

vivictpp::libav::Frame vivictpp::workers::FrameBuffer::first() {
  logger->trace("vivictpp::workers::FrameBuffer::first enter");
  std::unique_lock<std::mutex> lock(mutex);
  conditionVariable.wait(lock, [&] { return _size > 0; });
  logger->trace("vivictpp::workers::FrameBuffer::first exit");
  return queue[_cursor.getValue()];
}

vivictpp::time::Time vivictpp::workers::FrameBuffer::currentPts() {
  const std::lock_guard<std::mutex> lock(mutex);
  if (_size == 0) {
    return 0;
  }
  return ptsBuffer[_cursor.getValue()];
}

bool vivictpp::workers::FrameBuffer::ptsInRange(vivictpp::time::Time pts) {
  logger->trace(
      "vivictpp::workers::FrameBuffer::ptsInRange pts={} minPts={} maxPts={}",
      pts, minPts(), maxPts());
  bool result;
  const std::lock_guard<std::mutex> lock(mutex);
  if (_size == 0) {
    result = false;
  } else if (_size == 1) {
    result = pts == ptsBuffer[_cursor.getValue()];
  } else {
    result = this->minPts() <= pts && pts <= this->maxPts();
  }
  //  if (!result) {
  logger->trace("_size={}, ptsBuffer: {}", _size, ptsBufferToString(ptsBuffer));
  //  }
  return result;
}

vivictpp::time::Time vivictpp::workers::FrameBuffer::minPts() {
  return ptsBuffer[this->tail().getValue()];
}

vivictpp::time::Time vivictpp::workers::FrameBuffer::maxPts() {
  QueuePointer index = _writePos - 1;
  logger->debug("maxPts() maxPts={}", ptsBuffer[index.getValue()]);
  return ptsBuffer[index.getValue()];
}

bool vivictpp::workers::FrameBuffer::previous() {
  const std::lock_guard<std::mutex> lock(mutex);
  if (_cursor != tail()) {
    _cursor = _cursor - 1;
    return true;
  }
  return false;
}

bool vivictpp::workers::FrameBuffer::next() {
  int dropN = 0;
  bool result = false;
  {
    const std::lock_guard<std::mutex> lock(mutex);

    if (_size > 1 && _cursor + 1 != _writePos) {
      _cursor = _cursor + 1;
      int distance = _cursor.distance(_writePos);
      if (_size == _maxSize && distance < 5) {
        dropN = 5 - distance;
      }
      result = true;
    }
  }
  if (dropN > 0) {
    drop(dropN);
  }
  return result;
}

void vivictpp::workers::FrameBuffer::step(vivictpp::time::Time pts) {
  if (pts > currentPts()) {
    stepForward(pts);
  } else {
    stepBackward(pts);
  }
}

int vivictpp::workers::FrameBuffer::stepForward(vivictpp::time::Time pts) {
  int c = 0;
  vivictpp::time::Time nextPts = this->nextPts();
  while (!vivictpp::time::isNoPts(nextPts) && nextPts <= pts && next()) {
    c++;
    nextPts = this->nextPts();
  }
  return c;
}

void vivictpp::workers::FrameBuffer::stepBackward(vivictpp::time::Time pts) {
  logger->debug(
      "vivictpp::workers::Framebuffer::stepBackward entry _cursor={}, pts={}",
      _cursor.getValue(), pts);
  while (ptsBuffer[(_cursor - 1).getValue()] >= pts && previous()) {
    logger->trace(
        "vivictpp::workers::Framebuffer::stepBackward _cursor={}, pts={}",
        _cursor.getValue(), pts);
  }
}

vivictpp::time::Time vivictpp::workers::FrameBuffer::nextPts() {
  const std::lock_guard<std::mutex> lock(mutex);
  vivictpp::time::Time nextPts;
  if (_size == 0 || _cursor + 1 == _writePos) {
    nextPts = vivictpp::time::NO_TIME;
  } else {
    nextPts = ptsBuffer[(_cursor + 1).getValue()];
  }
  logger->debug(
      "vivictpp::workers::FrameBuffer::nextPts _size={} _cursor={} nextPts={}",
      _size, _cursor.getValue(), nextPts);
  if (nextPts < 0.01) {
    logger->debug("vivictpp::workers::FrameBuffer::nextPts ptsBuffer={}",
                  ptsBufferToString(ptsBuffer));
  }
  return nextPts;
}

vivictpp::time::Time vivictpp::workers::FrameBuffer::previousPts() {
  const std::lock_guard<std::mutex> lock(mutex);
  vivictpp::time::Time previousPts;
  if (_size == 0 || _cursor == this->tail()) {
    previousPts = vivictpp::time::NO_TIME;
  } else {
    previousPts = ptsBuffer[(_cursor - 1).getValue()];
  }
  logger->debug("vivictpp::workers::FrameBuffer::previousPts _size={} "
                "_cursor={} previousPts={}",
                _size, _cursor.getValue(), previousPts);
  if (previousPts < 0.01) {
    logger->debug("vivictpp::workers::FrameBuffer::previousPts ptsBuffer={}",
                  ptsBufferToString(ptsBuffer));
  }
  return previousPts;
}

void vivictpp::workers::FrameBuffer::drop(int n) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    _drop(n);
  }
  conditionVariable.notify_all();
}

void vivictpp::workers::FrameBuffer::_drop(int n) {
  logger->trace("vivictpp::workers::FrameBuffer::_drop n={}", n);
  for (int i = 0; i < n && _size > 0; i++) {
    if (_cursor == this->tail()) {
      _cursor = _cursor + 1;
    }
    auto clearPos = this->tail().getValue();
    queue[clearPos] = vivictpp::libav::Frame::emptyFrame();
    ptsBuffer[clearPos] = vivictpp::time::NO_TIME;
    _size--;
  }
}

void vivictpp::workers::FrameBuffer::dropIfFull(int n) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    if (_size < _maxSize) {
      return;
    }
  }
  drop(n);
}

void vivictpp::workers::FrameBuffer::clear() {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    _drop(_size);
    _cursor = 0;
    _writePos = 0;
  }
  conditionVariable.notify_all();
}
