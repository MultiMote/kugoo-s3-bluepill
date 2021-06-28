#ifndef GUI_H
#define GUI_H

#include <stdint.h>

enum view_id_t {
  GUI_VIEW_MAIN,
  GUI_VIEW_MAIN_DETAILED,
  GUI_VIEW_SETTINGS,
  GUI_VIEW_TRIGGER_CALIBRATION,
  GUI_VIEW_LAST_TRIPS,
};

struct view_t {
  enum view_id_t id;
  void (*on_key)(uint8_t);
  void (*on_draw)();
  void (*on_open)();
};

void process_events(void);
void gui_redraw(void);
void gui_redraw_force(void);
void gui_set_view(enum view_id_t id);
uint8_t gui_is_view(enum view_id_t id);

#endif /* GUI_H */
