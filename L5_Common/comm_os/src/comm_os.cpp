// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_os.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
namespace drv {

constexpr uint16_t MAX_PORT_NUMBER = 65535;

/**
 * Attempts to find an available port starting from the given port number.
 *
 * @param port_number The initial port number to try.
 * @return The available port number or 0 if no available port is found up to the maximum port number.
 */
// cppcheck-suppress unusedFunction
uint16_t FindAvailablePort(uint16_t port_number) {
  struct sockaddr_in addr;  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  // Start trying from the given port number
  for (uint16_t port = port_number; port <= MAX_PORT_NUMBER; ++port) {
    addr.sin_port = htons(port);  // Host to network short
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
      continue;  // If socket fails, try the next port
    }

    // Try to bind the socket to the port
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
      close(sockfd);  // Close the socket to free the port
      return port;    // Port is available, return it
    }

    close(sockfd);  // Always close the socket descriptor
  }

  return 0;  // Return 0 if no ports are available
}

double CalculateJulianDateTime() {
  auto duration_since_epoch = std::chrono::system_clock::now().time_since_epoch();
  auto microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch).count();
  double days_since_epoch = microseconds_since_epoch / 86400000000.0;  // number of days since Unix epoch

  return JULIAN_DATE_TIME_FOR_UNIX + days_since_epoch;  
}

}  // namespace drv