#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "gpioapp.h"

#include "ev.h"
#include "hal.h"

typedef struct {
	ev_io read_watcher;
	void  *cb;
	burst_data_irq_info_t *info;
}data_path_ev_io;

#define DATA_PATH_CHANNEL_MAX  4
typedef struct data_path_cb
{
    struct ev_loop * loop;	
	
	burst_data_task_init_param_t _init_param;
	data_path_ev_io data_path_io[DATA_PATH_CHANNEL_MAX];	
}data_path_cb;


static data_path_cb  _sg_data_path_cb_tmp;;
static data_path_cb  *_sg_data_path_cb = NULL;

static int ddr_burst_init(burst_data_irq_info_t *info)
{
	int fd;
	
	//a adv point is 8bytes. VALID_ADC_LEN is adc point nums.
	uint32_t len_align_to_page = (info->phy_data_len + (sysconf(_SC_PAGE_SIZE) - 1)) &  ~(sysconf(_SC_PAGE_SIZE) - 1);

	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd < 0)
	{
		printf("open /dev/mem failed!\n");
		return -1;
	}else
	{
		info->vir_addr = mmap(NULL, len_align_to_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, info->phy_addr);
		if(NULL==info->vir_addr)
            printf("mmap DDR virtual address 0x%x failed!\n", info->phy_addr);
		else
		{
			//uncache the mem
			//madvise(READ_DDR_BURST_ADDR,len_align_to_page, MADV_DONTNEED);
			info->vir_data_len = len_align_to_page;
			printf("mmap DDR success: 0x%x <==> 0x%x length:%d\n", info->phy_addr, info->vir_addr, info->vir_data_len);
		}
		close(fd);
	}
	return 0;
}


static void *_data_path_burst_task(void* arg)
{
	if(_sg_data_path_cb){
		EAP_LOG_ERROR("already inited\n");
		return NULL;
	}
	_sg_data_path_cb = &_sg_data_path_cb_tmp;
	_sg_data_path_cb->_init_param = *(burst_data_task_init_param_t*)arg;
	_sg_data_path_cb->loop = ev_loop_new(EVFLAG_AUTO); 

	_sg_data_path_cb->_init_param.init();

	ev_run(_sg_data_path_cb->loop, 0);
	return arg;
}

/**
 * @brief data path task init func
 *   
 * @param [in]    arg    init param, burst_data_task_init_param_t
 *
 * @return 
 * @retval   if ok ,return arg.
 *
 * @see 
 * @note 
 * @warning 
 */
void *data_path_burst_task(void* arg)
{
	static  burst_data_task_init_param_t init_param;
	init_param = *(burst_data_task_init_param_t*)arg;
	if(!arg){
		EAP_LOG_ERROR("arg invalid\n");
		return NULL;
	}		
	pthread_t thread;
	pthread_create(&thread, NULL, _data_path_burst_task, &init_param);
	pthread_setname_np(thread, "data_path_burst_task");

	return arg;
}

/*
	ev_loop read callback.
*/
static void _data_path_irq_read_cb(EV_P_ ev_io *w, int revents)
{
	data_path_ev_io * g = (data_path_ev_io *)w;
	burst_data_irq_info_t *info = g->info;
	int ret;
    char buf[128];
	ssize_t n;
	
    if (EV_ERROR & revents) {
        perror("got invalid event");
        return;
    }
	
	(*info->rcv_handler)(info);
#if 0
    n = read(w->fd, buf, sizeof(buf));
    if (n < 0) {
        perror("read error");
    }else if(n == 0){
		//TODO
        perror("read n== 0");
    }
#endif
	
}


int data_path_irq_init()
{
	int i;
	burst_data_irq_info_t *info = NULL;
	for(i = 0; i < _sg_data_path_cb->_init_param.info_counts; i++){
		info = &_sg_data_path_cb->_init_param.info[i];
		if(info->is_enable && info->irq_dev_file){
			info->fd = open( info->irq_dev_file, O_RDONLY );
			if(info->fd){
				data_path_ev_io *io = &_sg_data_path_cb->data_path_io[i];
				ddr_burst_init(info);
				io->cb = _sg_data_path_cb;
				io->info = info;
				ev_io_init(&_sg_data_path_cb->data_path_io[i].read_watcher,_data_path_irq_read_cb, info->fd, EV_READ);
				ev_io_start(_sg_data_path_cb->loop, &_sg_data_path_cb->data_path_io[i].read_watcher);
			}else{
				EAP_LOG_ERROR("open %s fail", info->irq_dev_file);
			}

		}
	}
	return 0;
}


