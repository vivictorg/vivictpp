// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WORKERS_VIDEOINPUTMESSAGE_HH
#define WORKERS_VIDEOINPUTMESSAGE_HH

#include <memory>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace vivictpp {
namespace workers {

class Message {
public:
  virtual ~Message() = default;
public:
  const uint64_t serialNo;
protected:
  Message():
    serialNo(serialCounter++) {}
private:
  static std::atomic<uint64_t> serialCounter;
};

class Command: public Message {
public:
  Command(std::function<bool(uint64_t)> lambda, std::string name = "UNKNOWN"):
    name(name),
    lambda(lambda) {
  }
  virtual ~Command() = default;
  bool apply() {
    return lambda(serialNo);
  }
public:
  const std::string name;
private:
  std::function<bool(uint64_t)> lambda;
};

template <class T>
class Data : public Message {
public:
  const std::shared_ptr<T> data;

public:
  Data(T* data):
    data(data) {}
  virtual ~Data() = default;
  T* operator->() const { return data.get(); }
};

template <class T>
class Queue {
private:
  std::queue<std::shared_ptr<Command>> queue_;
  std::queue<std::shared_ptr<Data<T>>> dataQueue;
  std::mutex mutex;
  size_t maxDataQueueSize;
  std::condition_variable conditionVariable;
  bool popData;

public:
  Queue(size_t maxDataQueueSize):
    maxDataQueueSize(maxDataQueueSize) {}
  bool empty();
  bool offerData(const Data<T> &data, const std::chrono::milliseconds& timeout);
  bool waitForCommand(const std::chrono::milliseconds& timeout);
  void clearDataOlderThan(uint64_t serialNo);
  void pushCommand(Command* command);
  Message & peek();
  void pop();
};

template <class T>
bool Queue<T>::empty() {
  const std::lock_guard<std::mutex> lock(mutex);
  return queue_.empty() && dataQueue.empty();
}

template <class T>
bool Queue<T>::offerData(const Data<T> &data,
                                            const std::chrono::milliseconds& timeout) {
  std::unique_lock<std::mutex> lock(mutex);
    if (conditionVariable.wait_for(lock, timeout,
                                 [&]{ return dataQueue.size() < maxDataQueueSize; })) {
    dataQueue.push(std::shared_ptr<Data<T>>(new Data<T>(data)));
    return true;
  }
  return false;
}

template <class T>
bool Queue<T>::waitForCommand(const std::chrono::milliseconds& timeout) {
  std::unique_lock<std::mutex> lock(mutex);
  return conditionVariable.wait_for(lock, timeout,
                                    [&]{ return queue_.size() > 0; });
}


template <class T>
void Queue<T>::clearDataOlderThan(uint64_t serialNo) {
  const std::lock_guard<std::mutex> lock(mutex);
  while(!dataQueue.empty() && dataQueue.front()->serialNo < serialNo) {
    dataQueue.pop();
  }
}

template <class T>
void Queue<T>::pushCommand(Command *command) {
  bool wasEmpty;
  {
    const std::lock_guard<std::mutex> lock(mutex);
    wasEmpty = queue_.empty();
    queue_.push(std::shared_ptr<Command>(command));
  }
  if (wasEmpty) {
    conditionVariable.notify_all();
  }
}

template <class T>
Message& Queue<T>::peek() {
  const std::lock_guard<std::mutex> lock(mutex);
  if (!queue_.empty()) {
    popData = false;
    return *queue_.front();
  }
  if (!dataQueue.empty()) {
    popData = true;
    return *dataQueue.front();
  }
  throw std::runtime_error("Queue is empty");
}

template <class T>
void Queue<T>::pop() {
  bool dataWasFull{false};
  {
    const std::lock_guard<std::mutex> lock(mutex);
    if (popData) {
      dataWasFull = dataQueue.size() == maxDataQueueSize;
      dataQueue.pop();
    } else {
      queue_.pop();
    }
  }
  if (dataWasFull) {
    conditionVariable.notify_all();
  }
}

}  // namespace workers
}  // namespace vivictpp

#endif // WORKERS_VIDEOINPUTMESSAGE_HH
