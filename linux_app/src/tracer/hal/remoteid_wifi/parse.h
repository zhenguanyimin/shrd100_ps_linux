#ifndef __parse_h__
#define __parse_h__

//huaguoqing a23286 add,2023.08.04
typedef struct {
	unsigned long long TSFT;
	int flags;
	double rate;
	uint16_t freq;
	uint16_t channel;
	char     channel_mode;
	signed char dbm_ant_signal;
	signed char dbm_ant_noise;
	char        rsv;
	uint32_t rx_flags_FCS;
	uint16_t rx_flags;
}radiotap_info;

void radiotap_print_radiotap_info(radiotap_info *info);
int radiotap_parse_header(void *data, int data_len, radiotap_info * info);
int convertFrequencyToChannel(int freq);

#endif
