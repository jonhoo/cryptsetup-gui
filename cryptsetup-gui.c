#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DEBUG true

bool decrypt(char* name, char* device, char* options, char* password);
bool mount(char* mountpoint);
char *strstrip(char *s);
void show_password_prompt(char* arg0);
void usage();

bool do_mount = false;
char* arg0 = NULL;
char *name = NULL, *device = NULL, *options = NULL;
char* mountpoint = "/dev/mapper/";
int main(int argc, char** argv) {
  if (DEBUG)
    printf("starting cryptsetup-gui\n");

  arg0 = *argv;

  argv++;
  argc--;

  if (argc == 0) {
    usage();
    exit(EXIT_SUCCESS);
  }

  if (strcmp(*argv, "-m") == 0) {
    do_mount = true;
    argv++;
    argc--;
    if (DEBUG)
      printf("mount after unlocking\n");
  }

  if (argc != 1) {
    usage();
    exit(EXIT_SUCCESS);
  }

  char* cryptpoint = *argv;
  char* ct = cryptpoint;

  if (DEBUG)
    printf("verifying cryptpoint\n");

  while (*ct != 0) {
    if (*ct < 'a' || *ct > 'z') {
      fprintf(stderr, "Non a-z cryptpoint given\n");
      exit(EXIT_FAILURE);
    }
    ct++;
  }

  if (DEBUG)
    printf("cryptpoint verified\n");

  size_t mps = strlen(mountpoint);
  size_t cps = strlen(cryptpoint);
  char* tmp = malloc(sizeof(char) * (mps + cps));
  strncpy(tmp, mountpoint, mps);
  strncpy(tmp + mps, cryptpoint, cps);
  mountpoint = tmp;

  if (DEBUG)
    printf("mountpoint resolved to '%s'\n", mountpoint);

  if (access(mountpoint, F_OK) == 0) {
    if (DEBUG)
      printf("mountpoint already exists\n");

    // Mountpoint already exists...
    if (do_mount && !mount(mountpoint)) {
      fprintf(stderr, "Failed to mount device %s\n", cryptpoint);
      exit(EXIT_FAILURE);
    }

    if (DEBUG)
      printf("mountpoint successfully mounted\n");

    exit(EXIT_SUCCESS);
  }

  if (DEBUG)
    printf("parsing crypttab\n");

  FILE *f = fopen("/etc/crypttab", "re");
  if (!f) {
    perror("Failed to open crypttab for reading");
    exit(EXIT_FAILURE);
  }

  if (DEBUG)
    printf("crypttab opened\n");

  char *l, *p = NULL;
  char line[1024];
  int n = 0;
  for (;;) {
    int k;

    if (!fgets(line, sizeof(line), f))
      break;

    n++;

    l = strstrip(line);
    if (*l == '#' || *l == 0)
      continue;

    k = sscanf(l, "%ms %ms %ms %ms", &name, &device, &p, &options);
    free(p);
    p = NULL;
    if (k < 2 || k > 4) {
      fprintf(stderr, "Failed to parse /etc/crypttab:%u, ignoring\n", n);
      goto next;
    }

    if (strcmp(name, cryptpoint) == 0) {
      // We found our cryptpoint
      if (DEBUG)
        printf("crypttab entry found for cryptpoint '%s'\n", device);
      break;
    }

    next:
      free(name);
      free(device);
      free(options);
      name = NULL;
      device = NULL;
      options = NULL;
  }
  fclose(f);

  if (DEBUG)
    printf("crypttab parsing ended\n");

  if (name == NULL || device == NULL) {
    fprintf(stderr, "Entry for %s not found in crypttab\n", cryptpoint);
  }

  show_password_prompt(arg0);

  return 0;
}

bool unlock(char* password) {
  // Try decrypting (note that password is not needed)
  if (!decrypt(name, device, options, password)) {
    fprintf(stderr, "Failed to decrypt device %s\n", name);
    return false;
  }
  if (do_mount) {
    if (!mount(mountpoint)) {
      fprintf(stderr, "Failed to mount device %s\n", name);
      return false;
    }
  }
  return true;
}

bool decrypt(char* name, char* device, char* options, char* password) {
  // TODO: Respect options list
  // We need to be weary of a bug in cryptsetup
  // https://groups.google.com/forum/#!msg/linux.debian.bugs.dist/7yRXc5NGMJM/q80hakUzDVMJ
  // cryptsetup drops privileges if EUID != UID
  // so, we store the old UID so we can restore it later
  uid_t ruid = getuid();

  char* command = NULL;
  asprintf(&command, "/usr/sbin/cryptsetup -q luksOpen %s %s", device, name);

  setreuid(0, 0);

  fflush(stdout);
  FILE *crypt = popen(command, "w");
  free(command);
  fprintf(crypt, "%s\n%d", password, EOF);
  int ret = pclose(crypt);

  // restore UID
  setreuid(ruid, 0);

  return WEXITSTATUS(ret) == 0;
}

bool mount(char* mountpoint) {
  char* command = NULL;
  fflush(stdout);
  asprintf(&command, "/bin/mount %s", mountpoint);
  FILE *mnt = popen(command, "r");
  int ret = pclose(mnt);
  return WEXITSTATUS(ret) == 0;
}

// Thank you kernel
bool is_space(char s) {
  return s == ' ' || s == '\t' || s == '\n';
}
char *strstrip(char *s) {
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && is_space(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && is_space(*s))
        s++;

    return s;
}

void show_password_prompt(char* arg0) {
  if (DEBUG)
    printf("showing gui\n");

  if (DEBUG)
    printf("dropping permissions\n");

  // We don't want to give the GTK any root access
  uid_t ruid = getuid();
  seteuid(ruid);

  if (DEBUG)
    printf("permissions dropped\n");

  fflush(stdout);

  char* command = NULL;
  asprintf(&command, "%s-gtk", arg0);
  FILE *pw = popen(command, "r");
  free(command);

  if (DEBUG)
    printf("gui started\n");

  // Now get the password
  char password[1024];
  fgets(password, 1024, pw);
  strtok(password, "\n");

  if (DEBUG)
    printf("password received\n");

  pclose(pw);
  fflush(stdout);

  if (DEBUG)
    printf("gui closed, resuming root\n");

  // Need root to do the unlocking
  seteuid(0);

  if (DEBUG)
    printf("EUID now %d, unlocking...\n", geteuid());

  if (unlock(password)) {
    if (DEBUG)
      printf("unlocked successfully\n");
  } else {
    fflush(stdout);
    fprintf(stderr, "invalid password given, unlock not possible\n");
    exit(EXIT_FAILURE);
  }
}

void usage() {
  printf("Usage: %s [-m] cryptpoint\n", arg0);
}
