#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#if defined(__APPLE__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#include "radiotap_iter.h"
#include "parse.h"

#define DEBUG_PRINT_RADIOTAP_INFO 0
#define DEBUG_PRINT_CHANNEL_SWITCHED  1


static int fcshdr = 0;

static const struct radiotap_align_size align_size_000000_00[] = {
	[0] = { .align = 1, .size = 4, },
	[52] = { .align = 1, .size = 4, },
};

static const struct ieee80211_radiotap_namespace vns_array[] = {
	{
		.oui = 0x000000,
		.subns = 0,
		.n_bits = sizeof(align_size_000000_00),
		.align_size = align_size_000000_00,
	},
};

static const struct ieee80211_radiotap_vendor_namespaces vns = {
	.ns = vns_array,
	.n_ns = sizeof(vns_array)/sizeof(vns_array[0]),
};

void radiotap_print_radiotap_info(radiotap_info *info)
{
	if(!info)
		return;
	
	printf("\n{");
	printf("TSFT: %llu,", info->TSFT);
	printf("flags: %02x,",info->flags);
	printf("rate: %lf,", info->rate);
	printf("ch:%hd,freq:%hd, type:%c,", info->channel,info->freq,info->channel_mode);			
	printf("dbm_ant_signal:%hhd,", info->dbm_ant_signal);
	printf("dbm_ant_noise:%hhd,", info->dbm_ant_noise);
	printf("rx_flags_FCS:%.8x,",info->rx_flags_FCS);
	printf("rx_flags:%#.4x",info->rx_flags);
	printf("}\n");
}

static void print_radiotap_namespace(struct ieee80211_radiotap_iterator *iter,radiotap_info *info,int print_flag)
{
	switch (iter->this_arg_index) {
	case IEEE80211_RADIOTAP_TSFT:
		
		info->TSFT = le64toh(*(unsigned long long *)iter->this_arg);
		if(print_flag)printf("\tTSFT: %llu\n", info->TSFT);
		break;
	case IEEE80211_RADIOTAP_FLAGS:
		info->flags = (int)*iter->this_arg;
		if(print_flag)printf("\tflags: %02x\n",info->flags);
		break;
	case IEEE80211_RADIOTAP_RATE:
		info->rate = (double)*iter->this_arg/2;
		if(print_flag)printf("\trate: %lf\n", info->rate);
		break;
#define IEEE80211_CHAN_A \
		(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define IEEE80211_CHAN_G \
		(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
	// 通信信息
	case IEEE80211_RADIOTAP_CHANNEL:
		info->freq = le16toh(*(uint16_t*)iter->this_arg); // 信道
		info->channel = convertFrequencyToChannel(info->freq);
		iter->this_arg = iter->this_arg + 2; // 通道信息如2G、5G，等
		int x = le16toh(*(uint16_t*)iter->this_arg);
		if(print_flag)printf("\tch:%hd,freq: %hd type: ", info->channel,info->freq);
		info->channel_mode = '?';
		if ((x & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)
		{
			if(print_flag)printf("A\n");
			info->channel_mode = 'A';
		}
		else if ((x & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)
		{
			if(print_flag)printf("G\n");
			info->channel_mode = 'G';
		}
		else if ((x & IEEE80211_CHAN_2GHZ) == IEEE80211_CHAN_2GHZ)
		{
			if(print_flag)printf("B\n");
			info->channel_mode = 'B';
		}
		break;
			
	case IEEE80211_RADIOTAP_FHSS:
		break;
	case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
		info->dbm_ant_signal = *(signed char*)iter->this_arg;
		if(print_flag)printf("\tdbm_ant_signal: %hhd dBm\n", info->dbm_ant_signal);
		break;
		
	case IEEE80211_RADIOTAP_DBM_ANTNOISE:
		info->dbm_ant_noise = *(signed char*)iter->this_arg;
		if(print_flag)printf("\tdbm_ant_noise: %hhd dBm\n", info->dbm_ant_noise);
		break;
	case IEEE80211_RADIOTAP_LOCK_QUALITY:
	case IEEE80211_RADIOTAP_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DB_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DBM_TX_POWER:
	case IEEE80211_RADIOTAP_ANTENNA:
	case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
	case IEEE80211_RADIOTAP_DB_ANTNOISE:
	case IEEE80211_RADIOTAP_TX_FLAGS:
		break;
	case IEEE80211_RADIOTAP_RX_FLAGS:
		if (fcshdr) {
			info->rx_flags_FCS = le32toh(*(uint32_t *)iter->this_arg);
			if(print_flag)printf("\tFCS in header: %.8x\n",
				info->rx_flags_FCS);
			break;
		}
		info->rx_flags = le16toh(*(uint16_t *)iter->this_arg);
		if(print_flag)printf("\tRX flags: %#.4x\n",
			info->rx_flags);
		break;
	case IEEE80211_RADIOTAP_RTS_RETRIES:
	case IEEE80211_RADIOTAP_DATA_RETRIES:
		break;
	default:
		if(print_flag)printf("\tBOGUS DATA\n");
		break;
	}
}

static void print_test_namespace(struct ieee80211_radiotap_iterator *iter)
{
	switch (iter->this_arg_index) {
	case 0:
	case 52:
		printf("\t00:00:00-00|%d: %.2x/%.2x/%.2x/%.2x\n",
			iter->this_arg_index,
			*iter->this_arg, *(iter->this_arg + 1),
			*(iter->this_arg + 2), *(iter->this_arg + 3));
		break;
	default:
		printf("\tBOGUS DATA - vendor ns %d\n", iter->this_arg_index);
		break;
	}
}

static const struct radiotap_override overrides[] = {
	{ .field = 14, .align = 4, .size = 4, }
};

#if 0
int main(int argc, char *argv[])
{
	struct ieee80211_radiotap_iterator iter;
	struct stat statbuf;
	int fd, err, fnidx = 1, i;
	void *data;

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "usage: parse [--fcshdr] <file>\n");
		fprintf(stderr, "	--fcshdr: read bit 14 as FCS\n");
		return 2;
	}

	if (strcmp(argv[1], "--fcshdr") == 0) {
		fcshdr = 1;
		fnidx++;
	}

	fd = open(argv[fnidx], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "cannot open file %s\n", argv[fnidx]);
		return 2;
	}

	if (fstat(fd, &statbuf)) {
		perror("fstat");
		return 2;
	}

	data = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

	err = ieee80211_radiotap_iterator_init(&iter, data, statbuf.st_size, &vns);
	if (err) {
		printf("malformed radiotap header (init returns %d)\n", err);
		return 3;
	}

	if (fcshdr) {
		iter.overrides = overrides;
		iter.n_overrides = sizeof(overrides)/sizeof(overrides[0]);
	}

	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.this_arg_index == IEEE80211_RADIOTAP_VENDOR_NAMESPACE) {
			printf("\tvendor NS (%.2x-%.2x-%.2x:%d, %d bytes)\n",
				iter.this_arg[0], iter.this_arg[1],
				iter.this_arg[2], iter.this_arg[3],
				iter.this_arg_size - 6);
			for (i = 6; i < iter.this_arg_size; i++) {
				if (i % 8 == 6)
					printf("\t\t");
				else
					printf(" ");
				printf("%.2x", iter.this_arg[i]);
			}
			printf("\n");
		} else if (iter.is_radiotap_ns){
			radiotap_info info = {0};
			print_radiotap_namespace(&iter,&info,1);
			radiotap_print_radiotap_info(info);
		}else if (iter.current_namespace == &vns_array[0])
			print_test_namespace(&iter);
	}

	if (err != -ENOENT) {
		printf("malformed radiotap data\n");
		return 3;
	}

	return 0;
}
#else



int radiotap_parse_header(void *data, int data_len, radiotap_info * info)
{
	struct ieee80211_radiotap_iterator iter;
	int err, i;
	radiotap_info info_tmp = {0};
	if(!info)
	info = &info_tmp;

	err = ieee80211_radiotap_iterator_init(&iter, data, data_len, &vns);
	if (err) {
		printf("malformed radiotap header (init returns %d)\n", err);
		return 3;
	}

	if (fcshdr) {
		iter.overrides = overrides;
		iter.n_overrides = sizeof(overrides)/sizeof(overrides[0]);
	}

	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.this_arg_index == IEEE80211_RADIOTAP_VENDOR_NAMESPACE) {
			printf("\tvendor NS (%.2x-%.2x-%.2x:%d, %d bytes)\n",
				iter.this_arg[0], iter.this_arg[1],
				iter.this_arg[2], iter.this_arg[3],
				iter.this_arg_size - 6);
			for (i = 6; i < iter.this_arg_size; i++) {
				if (i % 8 == 6)
					printf("\t\t");
				else
					printf(" ");
				printf("%.2x", iter.this_arg[i]);
			}
			printf("\n");
		} else if (iter.is_radiotap_ns){
			print_radiotap_namespace(&iter,info,0);
			if(DEBUG_PRINT_RADIOTAP_INFO && info->freq){
				radiotap_print_radiotap_info(info);
			}	
			
			if(DEBUG_PRINT_CHANNEL_SWITCHED && info->freq){
				static uint16_t pre_channel = 0;
				if(pre_channel  != info->channel){
					extern int g_flag_run_rid_capture_print;
					if(g_flag_run_rid_capture_print){
						printf("\n\n*****channed switch (%hu<-%hu)****\n",info->channel,pre_channel);
						radiotap_print_radiotap_info(info);
					}
					pre_channel = info->channel;
				}
			}	
		}else if (iter.current_namespace == &vns_array[0])
			print_test_namespace(&iter);
	}

	if (err != -ENOENT) {
		printf("malformed radiotap data\n");
		return 3;
	}

	return 0;
}

#endif

int convertFrequencyToChannel(int freq)
{
    if (freq >= 2412 && freq <= 2484) {
        return (freq - 2412) / 5 + 1;
    } else if (freq >= 5170 && freq <= 5825) {
        return (freq - 5170) / 5 + 34;
    } else {
        return -1;
    }
}

