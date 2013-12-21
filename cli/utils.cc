#include <gtk/gtk.h>
#include <string.h>

#include "global.hh"
#include "utils.hh"
#include "log.hh"

const int s_grid_span_column_number = 1;
const int s_grid_span_row_number = 1;
const int s_container_spacing = 12;

void writeTextView(GtkWidget* text_view, const char *content, msg_showing_type type)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_end_iter(buffer, &iter);

    if ( isMsgShowingTypeValid(type) ) {
        const char *tag_name = getTagName(type);
        PR_TRACE("apply using tag=%s", tag_name);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                                                 content, -1, tag_name,
                                                 NULL);
    }

    /* Scrolls text_view the minimum distance such that
     * mark is contained within the visible area of the widget. */

    GtkTextMark *mark;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, false);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(text_view), mark);

    // int within_margin = 0.25;
    // gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view),
    //                              gtk_text_buffer_get_insert(buffer),
    //                              within_margin, FALSE, 0, 0);

}

GtkWidget* newTextView(GtkWidget **pp_scrolled_window, GtkShadowType type,
                       const char *init_text, int sw_width,
                       int sw_height, bool line_wrap)
{
    *pp_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(*pp_scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_widget_set_size_request(GTK_WIDGET(*pp_scrolled_window), sw_width, sw_height);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(*pp_scrolled_window), type);

    // GtkTextBuffer* text_buffer = gtk_text_buffer_new(NULL);
    // createTextViewTags(text_buffer);
    // GtkWidget* text_view = gtk_text_view_new_with_buffer(text_buffer);

    GtkWidget* text_view = gtk_text_view_new();
    GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    createTextViewTags(text_buffer);

    // initialized text
    writeTextView(text_view, init_text, MSG_ST_NORMAL);
    // text view settings
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (text_view), 5);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (text_view), 5);
    gtk_text_view_set_pixels_inside_wrap (GTK_TEXT_VIEW (text_view), 5);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 10);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 10);
    // compose
    gtk_container_add(GTK_CONTAINER(*pp_scrolled_window), text_view);
    // gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), editable);
    setTextViewLineWrap(text_view, line_wrap);
    return text_view;
}

void setTextViewLineWrap(GtkWidget* text_view, bool line_wrap)
{
    if ( line_wrap ) {
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    }
}

void defaultCallBack(void* args) {
    PR_DEBUG("this callback using default call back, so nothing!");
}

void defaultWindowDestoryCallBack(void* args)
{
    PR_TRACE("window [CLOSE] button clicked, " global_app_name " terminates");
    gtk_main_quit();
}

GtkWidget* newGrid(int row_spacing, int col_spacing, bool row_homo, bool column_homo)
{
    GtkWidget* self_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(self_grid), col_spacing);
    gtk_grid_set_row_spacing(GTK_GRID(self_grid), row_spacing);
    gtk_grid_set_row_homogeneous(GTK_GRID(self_grid), false);
    gtk_grid_set_column_homogeneous(GTK_GRID(self_grid), false);
    return self_grid;
}

GtkWidget* newLabel(const char* label_name, bool line_wrap, int align_type, bool mnemonic)
{
    GtkWidget *self_label = NULL;
    self_label = gtk_label_new("");
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(self_label), label_name);
    gtk_label_set_line_wrap(GTK_LABEL(self_label), line_wrap);
    gtk_widget_set_size_request(self_label, 15, -1);
    int xalign = 0.99;
    if ( ALIGN_TYPE_LEFT == align_type ) {
        xalign = 0;
    } else if ( ALIGN_TYPE_CENTER == align_type ) {
        xalign = 0.5;
    } else if ( ALIGN_TYPE_RIGHT == align_type ) {
        xalign = 0.99;
    }
    g_object_set (G_OBJECT (self_label), "xalign", xalign, NULL);
    return self_label;
}

GtkWidget* addGridLabel(GtkWidget* grid, const char *label_name, int row, int col, const char *text)
{
    // TODO: [2013-11-23] to check validation of argument
    PR_TRACE("adding grid label=%s at row=%d", label_name, row);
    GtkWidget* label = newLabel(label_name, ALIGN_TYPE_RIGHT, true);
    GtkWidget *label_value = newLabel(text, ALIGN_TYPE_RIGHT, true);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), label_value);
    gtk_grid_attach(GTK_GRID(grid), label, col, row, s_grid_span_column_number, s_grid_span_row_number);
    gtk_grid_attach(GTK_GRID(grid), label_value, col+1, row, s_grid_span_column_number, s_grid_span_row_number);
    return label_value;
}

GtkWidget* addGridEntry(GtkWidget* grid, const char *entry_name, int row, int col, bool entry_input_visible)
{
    PR_TRACE("adding grid entry=%s at row=%d", entry_name, row);
    GtkWidget* label = newLabel(entry_name, ALIGN_TYPE_RIGHT, true);
    GtkWidget *entry = gtk_entry_new();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    gtk_grid_attach(GTK_GRID(grid), label, col, row, s_grid_span_column_number, s_grid_span_row_number);
    gtk_grid_attach(GTK_GRID(grid), entry, col+1, row, s_grid_span_column_number, s_grid_span_row_number);
    gtk_entry_set_visibility((GtkEntry*)entry, entry_input_visible);

    return entry;
}

GtkWidget* addGridTextView(GtkWidget* grid, const char* textview_name, int row, int col, const char *text)
{
    GtkWidget* label = newLabel(textview_name, ALIGN_TYPE_RIGHT, true);
    GtkWidget *sw = NULL;
    const char* tmp_text = text;
    if ( ! tmp_text ) {
        tmp_text = "";
    }
    GtkWidget* textview = newTextView(&sw, GTK_SHADOW_ETCHED_IN, tmp_text, -1, -1);

    // associate label with textview
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), textview);
    gtk_grid_attach(GTK_GRID(grid), label, col, row, s_grid_span_column_number, s_grid_span_row_number);
    gtk_grid_attach(GTK_GRID(grid), sw, col+1, row, s_grid_span_column_number, s_grid_span_row_number);

    return textview;
}

GdkPixbuf* newIconFromPath(const char* icon_path)
{
    if ( ! icon_path ) {
        PR_ERROR("invalid argument, icon path is NULL");
        return NULL;
    }
    if ( ! *icon_path ) {
        icon_path = getIconDefaultPath();
    }

    GError *error = NULL;
    GdkPixbuf *self = gdk_pixbuf_new_from_file(icon_path, &error);
    if ( error ) {
        PR_ERROR("failed to new icon pixbuf from path=[%s]", icon_path);
        return NULL;
    }
    return self;
}

void int2str(std::string& rst_string, int int_val)
{
#define SIZE 12
    char buf[SIZE];
    memset(buf, sizeof buf, 0);
    snprintf(buf, sizeof buf, "%d", int_val);
    rst_string = buf;
}

int max(int op1, int op2)
{
    return op1 > op2 ? op1 : op2;
}

int min(int op1, int op2)
{
    return op1 < op2 ? op1 : op2;
}
