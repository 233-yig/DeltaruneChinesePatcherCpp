#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <functional>
#include <string>
#include <thread>

class DownloadTask {
  std::jthread impl;
  DownloadTask(const DownloadTask &) = delete;
  DownloadTask(DownloadTask &&) = delete;

public:
  enum class DownloadState { RUNNING, FINISHED, FAILED } state;
  DownloadTask(const std::string &url,
               std::function<void(const std::string)> onSuccess,
               std::function<void(const std::string)> onError,
               int maxRetries = 3);
  int progress;
  std::string result;
  void Cancel() { impl.request_stop(); }
};
#endif
