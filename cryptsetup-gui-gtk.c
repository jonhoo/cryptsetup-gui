#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>

static void destroy( GtkWidget *widget, gpointer data );
static void unlock_gtk( GtkWidget *widget, GtkWidget *passwd );

int main(int argc, char** argv) {
  fprintf(stderr, "GTK: executing as user %d\n", getuid());

  GtkWidget *window, *passwd;

  gtk_init (&argc, &argv);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  g_signal_connect (window, "destroy", G_CALLBACK (destroy), NULL);

  passwd = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
  gtk_entry_set_activates_default(GTK_ENTRY(passwd), TRUE);
  gtk_entry_set_width_chars(GTK_ENTRY(passwd), 32);
  g_signal_connect (passwd, "activate", G_CALLBACK (unlock_gtk), passwd);

  gtk_container_add (GTK_CONTAINER (window), passwd);

  gtk_widget_show (passwd);
  gtk_widget_show (window);

  gtk_widget_grab_focus(passwd);
  gtk_main ();

  return 0;
}

static void unlock_gtk( GtkWidget *widget, GtkWidget *passwd ) {
  printf("%s", (char *)gtk_entry_get_text(GTK_ENTRY(passwd)));
  gtk_main_quit();
}
static void destroy( GtkWidget *widget, gpointer data ) {
  gtk_main_quit();
}
