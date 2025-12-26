#include "DownloadTask.h"
#include "LogManager.h"
#include <cpr/cpr.h>

DownloadTask::DownloadTask(const std::string &url,
                           std::function<void(const std::string)> onSuccess,
                           std::function<void(const std::string)> onError,
                           int maxRetries) {
  progress = 0;
  state = DownloadState::RUNNING;

  impl = std::jthread([this, url, onSuccess, onError,
                       maxRetries](std::stop_token stoken) {
    for (int retry = 0; retry < maxRetries && !stoken.stop_requested();
         ++retry) {

      cpr::Response r = cpr::Get(
          cpr::Url{url},
          cpr::ProgressCallback([this, &stoken](size_t total, size_t now,
                                                size_t, size_t, intptr_t) {
            if (stoken.stop_requested())
              return false; // 只有用户取消才中断

            if (total > 0) {
              progress = static_cast<int>(now * 100 / total);
            }
            return true;
          }));

      if (stoken.stop_requested()) {
        onError("Download task cancelled.");
        state = DownloadState::FAILED;
        return;
      }

      if (r.error.code == cpr::ErrorCode::OK && r.status_code == 200) {
        onSuccess(r.text);
        state = DownloadState::FINISHED;
        return;
      }

      LogManager::Warning("[DownloadTask] retry " + std::to_string(retry + 1) +
                          ", status=" + std::to_string(r.status_code) +
                          ", error=" + r.error.message);
    }

    onError("Download failed after retries.");
    state = DownloadState::FAILED;
  });
}
