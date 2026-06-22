#pragma once
#include <string>

#ifdef _WIN32
  #include <winsock2.h>
  using socket_t = SOCKET;
#else
  using socket_t = int;
#endif

class KVCache;  // forward declaration — not used in Phase 3 yet

// Handle a single client connection: read lines, echo them back
// Phase 3: just echoes. Phase 4 will wire to KVCache.
void handle_connection(socket_t client_fd);
