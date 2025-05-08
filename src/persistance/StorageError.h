#pragma once

#include <functional>
#include <string>

// Application contract shared by storage adapters and notification consumers.
enum class StorageErrorCategory { StorageOpen, FileNotFound, MissingKey, FileRead };
struct StorageError {
    StorageErrorCategory category;
    std::string message;
};
// Called on the requesting thread. Consumers must be thread-safe and not throw.
using StorageErrorCallback = std::function<void(const StorageError&)>;
