// askb_xiao.h

void init_askb_xiao();
void init_mcp23();
void init_timer();

void table_change(uint8_t key, bool pressed) ;

void matrix_scan();
void clear_data();

//親指キーを専用化するときに使うコード



#define DEDICATED_OYA_LEFT  (HID_F24 + 1)
#define DEDICATED_OYA_RIGHT  (HID_F24 + 2)
