// misc.c

#include "stdio.h"
#include "defs.h"
#include "unistd.h"

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#include "sys/select.h"
#include "unistd.h"
#include "string.h"
#endif

int GetTimeMs() {
#ifdef WIN32
  return GetTickCount(); // Windows specific time function
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec * 1000) + (t.tv_usec / 1000); // Convert seconds and microseconds to milliseconds
#endif
}

/**
 * @brief Checks if there is input waiting to be read
 * @return Non-zero if input is waiting, 0 otherwise
 * @note Original source: http://home.arcor.de/dreamlike/chess/
 */
int InputWaiting()
{
#ifndef WIN32
  // Unix implementation using select
  fd_set readfds;
  struct timeval tv = {0, 0}; // Zero timeout for non-blocking check
  
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  
  select(16, &readfds, NULL, NULL, &tv);
  
  return FD_ISSET(fileno(stdin), &readfds);
#else
  static int init = 0;
  static int pipe;
  static HANDLE inh;
  DWORD dw;

  // One-time initialization
  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    
    if (!pipe) {
      // Disable mouse and window input events
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }
  
  // Check for input based on input type (pipe or console)
  if (pipe) {
    // Handle pipe input
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) {
      return 1; // Error or broken pipe, assume data available
    }
    return dw; // Return number of bytes available
  } else {
    // Handle console input
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw; // Return 0 if no events or just 1 event
  }
#endif
}

void ReadInput(S_SEARCHINFO *info) {
  // Only process if there's input waiting
  if (!InputWaiting()) {
    return;
  }
  
  int bytes;
  char input[256] = "";
  char *endc;
  
  // Mark search as stopped
  info->stopped = TRUE;
  
  // Read input, retry if read fails
  do {
    bytes = read(fileno(stdin), input, sizeof(input) - 1);
  } while (bytes < 0);
  
  // Ensure null-termination
  input[bytes] = '\0';
  
  // Remove newline character if present
  endc = strchr(input, '\n');
  if (endc) {
    *endc = '\0';
  }
  
  // Process commands if input is not empty
  if (strlen(input) > 0) {
    if (!strncmp(input, "quit", 4)) {
      info->quit = TRUE;
    }
  }
}