// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WORKERS_INPUTWORKER_HH
#define WORKERS_INPUTWORKER_HH

#include "workers/VideoInputMessage.hh"
#include <memory>
#include <thread>
#include <mutex>

#include <unistd.h>
#include "logging/Logging.hh"

namespace vivictpp {
namespace workers {

enum class InputWorkerState { INACTIVE, ACTIVE, SEEKING, STOPPED };

template <class T>
class InputWorker {

public:
  InputWorker(int queueDataLimit, std::string);
  virtual ~InputWorker();

  void sendCommand(vivictpp::workers::Command *cmd);
  bool offerData(const vivictpp::workers::Data<T> &data,
                 const std::chrono::milliseconds& timeout);
  void start();
  void stop();

protected:
  InputWorker();
  void quit();

private:
  void pollMessageQueue();
  void run();
  virtual bool filterData(const vivictpp::workers::Data<T> &data) {
    (void) data;
    return true;
  };
  virtual void doWork(){}
  virtual bool onData(const vivictpp::workers::Data<T> &data) {
    (void) data;
    return true;
  }

protected:
  vivictpp::logging::Logger logger;
  vivictpp::logging::Logger seeklog;
  InputWorkerState state;
  vivictpp::workers::Queue<T> messageQueue;

private:
  std::unique_ptr<std::thread> thread;

};


template<class T>
InputWorker<T>::InputWorker(int queueDataLimit, std::string name):
    logger(vivictpp::logging::getOrCreateLogger(name)),
    seeklog(vivictpp::logging::getOrCreateLogger("seeklog")),
    state(InputWorkerState::INACTIVE),
    messageQueue(queueDataLimit) {
}

template<class T>
InputWorker<T>::~InputWorker() {

}

template<class T>
void InputWorker<T>::sendCommand(vivictpp::workers::Command *cmd) {
  messageQueue.pushCommand(cmd);
}

template<class T>
bool InputWorker<T>::offerData(const vivictpp::workers::Data<T> &data,
                               const std::chrono::milliseconds& timeout) {
  if (!filterData(data)) {
    return true;
  }
  return messageQueue.offerData(data, timeout);
}

template<class T>
void InputWorker<T>::start() {
  logger->trace("InputWorker::start()");
  if (!thread) {
    thread.reset(new std::thread(&InputWorker<T>::run, this));
  }
  InputWorker<T> *inputWorker(this);
  messageQueue.pushCommand(new vivictpp::workers::Command([=](uint64_t serialNo){
                                                            (void) serialNo;
                                                            inputWorker->state = InputWorkerState::ACTIVE;
                                                            return true;
                                                          }, "start"));
}

template<class T>
void InputWorker<T>::stop() {
  InputWorker<T> *inputWorker(this);
  messageQueue.pushCommand(new vivictpp::workers::Command([=](uint64_t serialNo){
                                                            (void) serialNo;
        inputWorker->state = InputWorkerState::INACTIVE;
        return true;
      }, "stop"));
}

template<class T>
void InputWorker<T>::quit() {
  if (state != InputWorkerState::STOPPED) {
    InputWorker<T> *inputWorker(this);
    messageQueue.pushCommand(new vivictpp::workers::Command([=](uint64_t serialNo){
                                                              (void)serialNo;
          inputWorker->state = InputWorkerState::STOPPED;
          return true;
        }, "quit"));
    if (thread) {
      thread->join();
    }
  }
}

template<class T>
void InputWorker<T>::pollMessageQueue() {
  while (!messageQueue.empty()) {
    vivictpp::workers::Message& message = messageQueue.peek();
    if (typeid(message) == typeid(vivictpp::workers::Data<T>)) {
      if (state == InputWorkerState::INACTIVE) {
        return;
      }
      auto data = dynamic_cast<vivictpp::workers::Data<T>&>(message);
      logger->debug("InputWorker::pollMessageQueue Recieved DATA");
      if (onData(data)) {
        messageQueue.pop();
      } else {
          usleep(5 * 1000);
        break;
      }
    } else {
      vivictpp::workers::Command& command = dynamic_cast<vivictpp::workers::Command&>(message);
      logger->debug("InputWorker::pollMessageQueue Recieved Command '{}'", command.name);
      if (command.apply()) {
        messageQueue.pop();
      }
    }
  }
}

template<class T>
void InputWorker<T>::run() {
  while (state == InputWorkerState::INACTIVE) {
    if (messageQueue.waitForCommand(std::chrono::milliseconds(100))) {
      pollMessageQueue();
    }
  }
  while (state != InputWorkerState::STOPPED) {
    doWork();
    pollMessageQueue();
  }
}

}  // namespace workers
}  // namespace vivictpp
#endif // WORKERS_INPUTWORKER_HH
