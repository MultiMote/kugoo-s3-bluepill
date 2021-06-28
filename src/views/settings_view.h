#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include <stdint.h>


enum settings_item_type_t {
  SETTING_TYPE_NONE,
  SETTING_TYPE_VIEW_CHANGE,
  SETTING_TYPE_TOGGLE,
  SETTING_TYPE_EDIT_FLOAT,
  SETTING_TYPE_EDIT_U16,
};

struct settings_item_t {
  const char *title;
  enum settings_item_type_t type;
  void *data;
};

void settings_view_keyhandler(uint8_t e);
void settings_view_redraw();

#endif /* SETTINGS_VIEW_H */
