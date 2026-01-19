// askb_xiao.h

void init_askb_xiao();
void init_mcp23();
void init_timer();

void table_change(uint8_t key, bool pressed) ;

void matrix_scan();
void clear_data();
