#ifndef MDEC_H_
#define MDEC_H_


int mdec_init(void);
void mdec_shutdown(void);
int32_t media_dec_setup_data_playback(uint32_t addr, uint32_t filled_size, uint32_t blocking);
void media_dec_tear_down(uint32_t blocking);
void media_dec_set_state(const uint32_t play, uint32_t blocking);
#endif
