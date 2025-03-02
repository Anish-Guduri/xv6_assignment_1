// #include "types.h"
// #include "stat.h"
// #include "user.h"

// int main() {
//     if (sys_gethistory() < 0) {
//         printf(1, "Error: Failed to fetch history\n");

//     }
//     exit();
// }


#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_COMMAND_LEN 100

int
main(int argc, char *argv[])
{
  int i;
  char buf[MAX_COMMAND_LEN];
  int n = 10; // Default to showing 10 history entries
  
  // If argument provided, use it as the number of history entries to show
  if(argc > 1) {
    n = atoi(argv[1]);
    if(n <= 0) {
      printf(2, "history: invalid number of entries\n");
      exit();
    }
  }
  
  // Print history entries in reverse order (most recent first)
  for(i = n-1; i >= 0; i--) {
    if(gethistory(i, buf) == 0) {
      printf(1, "%d: %s", i+1, buf);
      // Add newline if not already present
      if(buf[strlen(buf)-1] != '\n')
        printf(1, "\n");
    }
  }
  
  exit();
}