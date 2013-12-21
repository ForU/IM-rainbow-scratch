#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include "gui.hh"

#define ALIGN_TYPE_LEFT 0
#define ALIGN_TYPE_CENTER 1
#define ALIGN_TYPE_RIGHT 2

// compile in C++ style, that is, support default value for argument
extern const int s_grid_span_column_number;
extern const int s_grid_span_row_number;
extern const int s_container_spacing;

extern GdkPixbuf* newIconFromPath(const char* icon_path);
extern GtkWidget* newLabel(const char* label_name="", bool line_wrap=true, int align_type=ALIGN_TYPE_RIGHT, bool mnemonic=true);
extern GtkWidget* newTextView(GtkWidget **pp_scrolled_window, GtkShadowType type=GTK_SHADOW_NONE, const char *init_text="", int sw_width=-1, int sw_height=-1, bool line_wrap=true);
extern void writeTextView(GtkWidget* text_view, const char *content, msg_showing_type type=MSG_ST_NORMAL);
extern void setTextViewLineWrap(GtkWidget* text_view, bool line_wrap=true);

extern GtkWidget* newGrid(int row_spacing, int col_spacing, bool row_homo = false, bool column_homo = false);
extern GtkWidget* addGridLabel(GtkWidget*grid, const char *label_name, int row, int col, const char *text="");
extern GtkWidget* addGridEntry(GtkWidget*grid, const char *entry_name, int row, int col, bool entry_input_visible=true);
extern GtkWidget* addGridTextView(GtkWidget*grid, const char* textview_name, int row, int col, const char *text);

extern void defaultCallBack(void* args);
extern void defaultWindowDestoryCallBack(void* args);

extern void int2str(std::string& rst_string, int int_val);

extern int max(int op1, int op2);
extern int min(int op1, int op2);

#endif /* INCLUDE_UTILS_HPP */
