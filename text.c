#include <unistd.h>
int main() {
char *args[8] = {
"ls",
"-a"
};
execvp(args[0], args);
}