#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <gtk/gtk.h>

bool decrypt(char* name, char* device, char* options, char* password);
bool mount(char* mountpoint);
char *strstrip(char *s);
void show_password_prompt(int argc, char** argv);
void usage();

bool do_mount = false;
char* arg0 = NULL;
char *name = NULL, *device = NULL, *options = NULL;
char* mountpoint = "/dev/mapper/";
int main(int argc, char** argv) {
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
  }

  if (argc != 1) {
    usage();
    exit(EXIT_SUCCESS);
  }

  char* cryptpoint = *argv;
  char* ct = cryptpoint;

  while (*ct != 0) {
    if (*ct < 'a' || *ct > 'z') {
      fprintf(stderr, "Non a-z cryptpoint given\n");
      exit(EXIT_FAILURE);
    }
  }

  size_t mps = strlen(mountpoint);
  size_t cps = strlen(cryptpoint);
  char* tmp = malloc(sizeof(char) * (mps + cps));
  strncpy(tmp, mountpoint, mps);
  strncpy(tmp + mps, cryptpoint, cps);
  mountpoint = tmp;

  if (access(mountpoint, F_OK) == 0) {
    // Mountpoint already exists...
    if (do_mount && !mount(mountpoint)) {
      fprintf(stderr, "Failed to mount device %s\n", cryptpoint);
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }

  FILE *f = fopen("/etc/crypttab", "re");
  if (!f) {
    perror("Failed to open crypttab for reading");
    exit(EXIT_FAILURE);
  }

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

  if (name == NULL || device == NULL) {
    fprintf(stderr, "Entry for %s not found in crypttab\n", cryptpoint);
  }

  show_password_prompt(argc, argv);

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
  char* command = NULL;
  asprintf(&command, "/usr/sbin/cryptsetup -q luksOpen %s %s", device, name);
  FILE *crypt = popen(command, "w");
  free(command);
  if (!crypt) {
    perror("call to cryptsetup failed");
    return false;
  }
  fprintf(crypt, "%s\n%d", password, EOF);
  int ret = pclose(crypt);
  return WEXITSTATUS(ret) == 0;
}

bool mount(char* mountpoint) {
  char* command = NULL;
  asprintf(&command, "/bin/mount %s", mountpoint);
  FILE *mnt = popen(command, "r");
  int ret = pclose(mnt);
  return WEXITSTATUS(ret) == 0;
}

// Thank you kernel
bool isspace(char s) {
  return s == ' ' || s == '\t' || s == '\n';
}
char *strstrip(char *s) {
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
        s++;

    return s;
}

static void unlock_gtk( GtkWidget *widget, GtkWidget *passwd ) {
  if (unlock((char *)gtk_entry_get_text(GTK_ENTRY(passwd)))) {
    gtk_main_quit();
    return;
  }
  // TODO: Add alert here
}
static void destroy( GtkWidget *widget, gpointer data ) {
  gtk_main_quit();
}
void show_password_prompt(int argc, char** argv) {
    GtkWidget *window, *passwd;

    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    g_signal_connect (window, "destroy", G_CALLBACK (destroy), NULL);

    /* Creates a new button with the label "Hello World". */
    passwd = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
    gtk_entry_set_activates_default(GTK_ENTRY(passwd), TRUE);
    g_signal_connect (passwd, "activate", G_CALLBACK (unlock_gtk), passwd);

    gtk_container_add (GTK_CONTAINER (window), passwd);

    gtk_widget_show (passwd);
    gtk_widget_show (window);

    gtk_main ();
}

void usage() {
  printf("Usage: %s [-m] cryptpoint\n", arg0);
}
