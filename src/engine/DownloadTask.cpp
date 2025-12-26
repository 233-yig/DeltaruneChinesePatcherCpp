#include "DownloadTask.h"
#include <cpr/cpr.h>
DownloadTask::DownloadTask(const std::string &url,
                           std::function<void(const std::string)> onSuccess,
                           std::function<void(const std::string)> onError,
                           int maxRetries) {
  progress = 0;
  state = DownloadState::RUNNING;
  impl = std::jthread([=](std::stop_token stoken) {
    for (int retry = 0;; retry++) {
      cpr::Response r = cpr::Get(
          cpr::Url(url),
          cpr::ProgressCallback([this, &stoken](size_t total, size_t now,
                                                size_t, size_t, intptr_t) {
            if (total == 0 || stoken.stop_requested()) {
              return false;
            }
            progress = now * 100 / total;
            return true;
          }));
      if (stoken.stop_requested()) {
        onError("Download task cancelled.");
        state = DownloadState::FAILED;
        return;
      } else if (r.error.code == cpr::ErrorCode::OK && r.status_code == 200) {
        onSuccess(result);
        state = DownloadState::FINISHED;
        return;
      } else if (++retry == maxRetries) {
        onError("HTTP error: " + std::to_string(r.status_code) +
                ", please check your network, or download the offline patch.");
        state = DownloadState::FAILED;
        return;
      }
    }
  });
}