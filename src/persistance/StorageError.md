# Storage notifications

`StorageError.h` defines the application-owned notification contract. Storage
adapters report events through an optional `StorageErrorCallback`; notification
queues and the UI consume these events without knowing the storage library.

`CascRequestProcessor.cpp` owns all TactCpp integration, including exception
translation. `CascRequestProcessor.h` exposes no TactCpp types or headers.

Callbacks run synchronously on the calling thread and must not throw. Capture
shared ownership of a thread-safe queue when requests run on workers. A failed
constructor reports `StorageOpen` and leaves `isOpen()` false when a callback is
provided; without a callback the exception propagates. Failed file requests
report once and are rejected. Successful reads produce no error event, including
partially encrypted files whose unavailable later blocks are zero-filled.

`StorageNotifications` and `StorageNotificationUI` implement the queue, category
preferences, and ImGui toasts. Existing `notify*` INI keys are retained.
