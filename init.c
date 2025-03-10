// init: The initial user-level program


#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// MY DEFINE CODE STARTS

// #define USERNAME "myuser"
// #define PASSWORD "mypass"
#define MAX_ATTEMPTS 3

char *argv[] = { "sh", 0 };



// void login() {
//   char user[20], pass[20];
//   int attempts = 0;

//   while (attempts < MAX_ATTEMPTS) {
//       printf(1,"Enter Username: ");
//       gets(user, sizeof(user));

//       if (strcmp(user, USERNAME) != 0) {
//           printf(1,"Incorrect username. Try again.\n");
//           attempts++;
//           continue;  // Skip password prompt if username is incorrect
//       }

//       printf(1,"Enter Password: ");
//       gets(pass, sizeof(pass));

//       if (strcmp(pass, PASSWORD) == 0) {
//           printf(1,"Login successful!\n");
//           return;  // Successful login, proceed to shell
//       } else {
//           printf(1,"Incorrect password. Try again.\n");
//           attempts++;
//       }
//   }

//   printf(1,"Too many failed attempts. Access denied.\n");
//   exit();  // Terminate shell if login fails
// }

// Function to handle user login
void login()
{
    char username[20];
    char password[20];
    int attempts = 0;

    while (attempts < MAX_ATTEMPTS)
    {
        printf(1, "Enter Username: ");
        gets(username, sizeof(username));
        username[strlen(username) - 1] = '\0'; // Remove newline

        if (strcmp(username, USERNAME) != 0)
        {
            printf(1, "Invalid username.\n");
            attempts++;
            continue;
        }

        printf(1, "Enter Password: ");
        gets(password, sizeof(password));
        password[strlen(password) - 1] = '\0'; // Remove newline

        if (strcmp(password, PASSWORD) == 0)
        {
            printf(1, "Login successful\n");
            return;
        }
        else
        {
            printf(1, "Incorrect password.\n");
            attempts++;
        }
      }

      printf(1, "Too many failed attempts. System locked.\n");
      exit();
  }
  

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

    // Call login before starting the shell
    login();

  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
