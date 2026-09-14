# Notification tests

From the repository root:

```sh
cmake -S tests/notifications -B build/notification-tests
cmake --build build/notification-tests
ctest --test-dir build/notification-tests --output-on-failure
```

These C++17 tests run Dear ImGui without a window or graphics backend. They cover
worker-thread event delivery, duplicate suppression, bounded queues, independent
category filtering, INI settings round trips, error/info severity, literal message
formatting, toast expiration, and rendering with the existing font.

The library's offline storage and file-reading tests are documented in
[ERROR_HANDLING.md](../../3rdparty/TactCpp/ERROR_HANDLING.md).
