#pragma once

#include "../persistance/StorageError.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <mutex>
#include <vector>

// Producers may run on request-processing threads. Only the UI drains the queue
// and reads/writes preferences. Capturing shared ownership avoids UI lifetime races.
class StorageNotifications {
public:
    std::array<bool, 4> enabled{{true, true, true, true}};
    static constexpr std::array<const char*, 4> labels{{
        "Storage opening errors", "Missing files or archives",
        "Missing decryption keys", "Other file reading errors"
    }};
    static constexpr std::array<const char*, 4> keys{{
        "notifyStorageErrors", "notifyMissingFiles", "notifyMissingKeys", "notifyFileErrors"
    }};

    void push(const StorageError& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
        while (!recent_.empty() && now - recent_.front().time >= std::chrono::seconds(5))
            recent_.pop_front();
        if (std::any_of(recent_.begin(), recent_.end(), [&](const Recent& item) {
            return item.error.category == error.category && item.error.message == error.message;
        })) return;
        if (recent_.size() == 128) recent_.pop_front();
        recent_.push_back({error, now});
        if (pending_.size() == 64) pending_.pop_front();
        pending_.push_back(error);
    }

    std::vector<StorageError> drain() {
        std::deque<StorageError> pending;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending.swap(pending_);
        }
        std::vector<StorageError> result;
        for (auto& error : pending)
            if (enabled.at(static_cast<size_t>(error.category)))
                result.push_back(std::move(error));
        return result;
    }
private:
    struct Recent { StorageError error; std::chrono::steady_clock::time_point time; };
    std::mutex mutex_;
    std::deque<StorageError> pending_;
    std::deque<Recent> recent_;
};
