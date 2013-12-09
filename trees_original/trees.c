#include <math.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include <gtkgl/gtkglarea.h>

#include "draw.h"

GtkWidget *glarea;
GtkWidget *entry;


int cb_init_glarea (GtkWidget *widget)
{
   if (gtk_gl_area_make_current (GTK_GL_AREA(widget)))
   {
      glViewport(0,0, widget->allocation.width, widget->allocation.height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0,100, 100,0, -1,1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
   }

   return TRUE;
}

int cb_draw_glarea (GtkWidget *widget, GdkEventExpose *event)
{
   char *buf;

   if (event != NULL && event->count > 0)
      return TRUE;

   buf = get_params_str();
   gtk_entry_set_text (GTK_ENTRY(entry), buf);

   if (gtk_gl_area_make_current (GTK_GL_AREA(widget)))
   {
      render_frame();
      gtk_gl_area_swap_buffers (GTK_GL_AREA(widget));
   }

   return TRUE;
}

int cb_reshape_glarea (GtkWidget *widget, GdkEventConfigure *event)
{
   if (gtk_gl_area_make_current (GTK_GL_AREA(widget)))
      glViewport(0,0, widget->allocation.width, widget->allocation.height);

   return TRUE;
}

int cb_click_glarea (GtkWidget *widget, GdkEventButton *event)
{
   cb_draw_glarea(widget, NULL);
   return TRUE;
}

void cb_adjust (GtkAdjustment *adj, float* f)
{
   *f = adj->value;
   cb_draw_glarea(glarea, NULL);
}

void cb_set_seed (GtkAdjustment *adj)
{
   tree_seed = (long) adj->value;
   cb_draw_glarea(glarea, NULL);
}

int main (int argc, char *argv[])
{
  GtkWidget *window;

  gtk_init (&argc, &argv);

  drawing_init();

  //window
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window), "Tree generator by mik01aj <md262484@students.mimuw.edu.pl>");
  gtk_container_border_width (GTK_CONTAINER (window), 10);
  g_signal_connect (GTK_OBJECT(window), "delete_event",
                    GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

  // You should always delete gtk_gl_area widgets before exit or else
  // GLX contexts are left undeleted, this may cause problems (=core dump)
  // in some systems.
  // Destroy method of objects is not automatically called on exit.
  // You need to manually enable this feature. Do gtk_quit_add_destroy()
  // for all your top level windows unless you are certain that they get
  // destroy signal by other means.
  gtk_quit_add_destroy (1, GTK_OBJECT(window));

  {
     GtkWidget *table;
     int i = 0;
     table = gtk_table_new((NUM_PARAMS+1)/2 + 1, 5, FALSE);

     {
        //glarea, declared as global
        int attrlist[] = { GDK_GL_RGBA,
                           GDK_GL_RED_SIZE, 1,
                           GDK_GL_GREEN_SIZE, 1,
                           GDK_GL_BLUE_SIZE, 1,
                           GDK_GL_DOUBLEBUFFER,
                           GDK_GL_NONE };

        glarea = gtk_gl_area_new (attrlist);
        gtk_widget_set_size_request (GTK_WIDGET(glarea), 500, 500);
        gtk_widget_set_events (GTK_WIDGET(glarea),
                               GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
        g_signal_connect (glarea, "expose_event",
                          G_CALLBACK(cb_draw_glarea), NULL);
        g_signal_connect (glarea, "button_press_event",
                          G_CALLBACK(cb_click_glarea), NULL);
        g_signal_connect (glarea, "configure_event",
                          G_CALLBACK(cb_reshape_glarea), NULL);
        g_signal_connect (glarea, "realize",
                          G_CALLBACK(cb_init_glarea), NULL);

        gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(glarea),
                          4, 5, 0, (NUM_PARAMS+1)/2+1,
                          GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 1, 1);
        gtk_widget_show (GTK_WIDGET(glarea));
     }

     {
        entry = gtk_entry_new ();
        gtk_entry_set_text (GTK_ENTRY(entry), "blah");
        gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);
        gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(entry),
                          0, 3, 0, 1,
                          GTK_FILL, GTK_FILL|GTK_EXPAND, 1, 1);
        gtk_widget_show (GTK_WIDGET(entry));
     }

     {
        GtkWidget *spin_button;
        GtkObject *adjustment;

        adjustment = gtk_adjustment_new(0, 0, 2000000,
                                        1, 1, 0);
        spin_button = gtk_spin_button_new (GTK_ADJUSTMENT(adjustment), 1, 0);

        g_signal_connect (adjustment, "value_changed",
                          G_CALLBACK(cb_set_seed), NULL);

        gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(spin_button),
                          3, 4, 0, 1,
                          GTK_FILL, GTK_FILL|GTK_EXPAND, 1, 1);
        gtk_widget_show (GTK_WIDGET(spin_button));
     }

     for (i=0; i<NUM_PARAMS; i++) {
        GtkWidget *scale;
        GtkWidget *label;
        GtkObject *adjustment;
        float *ptr = (float*)tree_params_p;
        int row = i/2+1, col = (i%2)*2;

        adjustment = gtk_adjustment_new(ptr[i], 0, 1,
                                        0.001, 0.1, 0);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment));
        gtk_scale_set_digits(GTK_SCALE(scale), 3);
        gtk_widget_set_size_request (GTK_WIDGET(scale), 200, 20);

        g_signal_connect (adjustment, "value_changed",
                          G_CALLBACK(cb_adjust), ptr+i);

        gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(scale),
                          col+1, col+2, row, row+1,
                          GTK_FILL, GTK_FILL|GTK_EXPAND, 1, 1);
        gtk_widget_show (GTK_WIDGET(scale));

        label = gtk_label_new (tree_param_labels[i]);
        gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(label),
                          col, col+1, row, row+1,
                          GTK_FILL, GTK_FILL|GTK_EXPAND, 1, 1);
        gtk_widget_show (GTK_WIDGET(label));
     }

     gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(table));
     gtk_widget_show (GTK_WIDGET(table));
  }

  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
