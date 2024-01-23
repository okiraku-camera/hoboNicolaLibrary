void rp_pwm_init(uint8_t gpio); 
void rp_start_pwm();
void rp_stop_pwm() ;

void rp_pwm_dimmer();
void rp_pwm_brighter();

void set_rp_pwm_max_value(int16_t val);
int16_t get_rp_pwm_max_value();