/* -*- tab-width: 2; mode: c; -*-
 * 
 * A program for capturing opendroneid / ASTM F3411 / ASD-STAN 4709-002 
 * WiFi beacon direct remote identification signals.
 *
 * Copyright (c) 2022-2023 Steve Jack
 *
 * MIT licence
 *
 * Options
 *
 * See the print_help() function.
 *
 * Notes
 *
 * Developed on -
 * 1) an NC10 running Debian using its native WiFi hardware and a ZyXEL AC1200 (RTL2812AU),
 * 2) a Raspberry Pi running Raspbian using a ZyXEL AC1200.
 *
 * Don't compile with -std=c99, libpcap doesn't like it.
 *
 * Pi
 * RTL8812AU driver 88XXAU, trying to set monitor mode via libpcap causes an error.
 *
 * Keep an eye on the main loop time. It may require tuning if you are using the 
 * display or exporting.
 *
 *
 * To Do
 *
 * Sort out the hangs that can occur if there is no data being received.
 *
 * Fully setup the WiFi device.
 *
 */

#define DEBUG_FILE 1

#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "rid_capture.h"

#if ENABLE_PCAP
#include <pcap/pcap.h>
#endif

#include "parse.h"
#include "iwlib.h"

#include "RemoteIdInfo.h"


#define BUFFER_SIZE     2048
#define MAX_KEY_LEN       16
#define NRF_BUFFER_SIZE  128

unsigned char             key[MAX_KEY_LEN + 2], iv[MAX_KEY_LEN + 2];
uid_t                     nobody  = 1000;
gid_t                     nogroup = 1000;
const mode_t              file_mode = 0666, dir_mode = 0777;

static int                enable_display = 0, enable_udp = 0, json_socket = -1,
                          max_udp_length = 0;
static char              *log_dir = NULL, *www_dir = NULL;
static double             setup_ms = 0.0, loop_us = 0.0;
static unsigned int       port = 32001;
static volatile int       end_program  = 0;
static FILE              *debug_file = NULL;
static const char         default_key[]     = "0123456789abcdef",
                          default_iv[]      = "nopqrs",
                          default_server[]  = "127.0.0.1",
                          // debug_filename[]  = "debug.txt",
                          debug_filename[]  = "remoteid_uav_data.txt",
                          device_pi[]       = "wlan1",
                          device_i686[]     = "wlp5s0b1",
                          // default_log_dir[] = "/tmp/rid_capture",
                          default_log_dir[] = "/run/media/mmcblk0p1",
                          default_www_dir[] = "www",
                          dummy[]           = "";
const char                pass_s[] = "Pass", fail_s[] = "Fail"; 
static volatile uint32_t  rx_packets = 0, odid_packets = 0;
static struct UAV_RID     RID_data[MAX_UAVS];
static struct sockaddr_in server;
#if ENABLE_PCAP
static int                header_type = 0;
static const char        *filter_text = "ether broadcast or ether dst 51:6f:9a:01:00:00 ";
#endif
static const char         device_bluez[] = "hci0";
#if BLUEZ_SNIFFER
static int                ble_sniffer = -1;
#endif
static const char         device_nrf[] = "/dev/ttyACM0";
#if NRF_SNIFFER
static int                nrf_pipe = -1;
#endif
#if FA_EXPORT
static time_t             last_fa_export = 0;
#endif
#if ASTERIX
static time_t             last_asterix = 0;
#endif
#if WWW_PAGE
static time_t             last_www_page = 0;
#endif


static int   skfd = -1;
static int   rid_capture_inited = 0;
static char  wifi_name_tmp[32] = {0};
static char  *sg_wifi_name = NULL;

#define NO_RUN_RID_CAPTURE_HANDLER 0
static int   sg_flag_run_rid_capture_handler = 1;// if /run/media/mmcblk0p1/flag_debug_run_rid_capture
int   g_flag_run_rid_capture_print = 0; // "/run/media/mmcblk0p1/flag_debug_run_rid_capture_print"
#define FLAG_DEBUG_RUN_RID_CAPTURE "/run/media/mmcblk0p1/flag_debug_run_rid_capture"
#define FLAG_DEBUG_RUN_RID_CAPTURE_PRINT "/run/media/mmcblk0p1/flag_debug_run_rid_capture_print"


#if ENABLE_PCAP
void list_devices(char *);
void packet_handler(u_char *,const struct pcap_pkthdr *,const u_char *);
#endif

static void print_help(void);
static void print_version(void);
static void signal_handler(int);

/*
 *
 */
extern int wifi_set_info(int		skfd,		/* The socket */
	 char *		args[],		/* Command line args */
	 int		count,		/* Args count */
	 char *		ifname);		/* Dev name */


int wifi_if_up(int fd, char *if_name, int is_up)
{
	struct ifreq ifreq;
	int ret;

	if(!if_name || fd < 0)
		return -1;
	
	memset(&ifreq, 0x00, sizeof(struct ifreq));
	strcpy(ifreq.ifr_name, if_name);
	ret = ioctl(fd, SIOCGIFFLAGS, &ifreq);
	if (ret < 0) {
	  perror("SIOCGIFFLAGS: ");
	  return -1;
	}  

	if(is_up){
		if(ifreq.ifr_flags & IFF_UP)
			return 0;
		ifreq.ifr_flags |= IFF_UP;
	}else{
		if(!(ifreq.ifr_flags & IFF_UP))
			return 0;
		ifreq.ifr_flags &= ~IFF_UP;
	}

	ret = ioctl(skfd, SIOCSIFFLAGS, &ifreq);
	if (ret < 0) {
	  perror("SIOCGIFFLAGS: ");
		  return -1;
	}
	return 0;
}


int wifi_set_promisc(int fd, char *if_name, int is_promisc)
{
	struct ifreq ifreq;
	int ret;

	if(!if_name || fd < 0)
		return -1;
	
	memset(&ifreq, 0x00, sizeof(struct ifreq));
	strcpy(ifreq.ifr_name, if_name);
	ret = ioctl(fd, SIOCGIFFLAGS, &ifreq);
	if (ret < 0) {
	  perror("SIOCGIFFLAGS: ");
	  return -1;
	}  

	if(is_promisc){
		if(ifreq.ifr_flags & IFF_PROMISC)
			return 0;
		ifreq.ifr_flags |= IFF_PROMISC;
	}else{
		if(!(ifreq.ifr_flags & IFF_PROMISC))
			return 0;
		ifreq.ifr_flags &= ~IFF_PROMISC;
	}
	ret = ioctl(skfd, SIOCSIFFLAGS, &ifreq);
	if (ret < 0) {
	  perror("SIOCGIFFLAGS: ");
		  return -1;
	}
	return 0;
}

int wifi_set_channel(int fd, char *if_name,int channel)
{
	char *wifi_cmd_args[10] = {0};
	int  wifi_cmd_argc = 0;
    int ret;
	char channel_str[10];
	if(channel <= 0 || !if_name || fd < 0)
		return -1;
	
	sprintf(channel_str,"%d",channel);
	
	wifi_cmd_args[0] = "channel";
	wifi_cmd_args[1] = channel_str;
	wifi_cmd_argc=2;
	//printf("skfd:%d,arg0:%s,arg1:%s,argc:%d,wifi:%s\n",fd,wifi_cmd_args[0],wifi_cmd_args[1],wifi_cmd_argc,if_name);
	ret = wifi_set_info(fd, wifi_cmd_args, wifi_cmd_argc, if_name);
	if(ret < 0){
	  perror("set channel");
	  return -2;
	}

    //struct timeval tv1;
    //gettimeofday(&tv1, NULL);
    //printf("wifi_set_channel = %d, second = %d, microseconds = %d \n", channel, tv1.tv_sec, tv1.tv_usec);

	return 0;
}

int wifi_set_mode(int fd, char *if_name,char *mode)
{
	char *wifi_cmd_args[10] = {0};
	int  wifi_cmd_argc = 0;
    int ret;
	if(!mode  || !if_name || fd < 0)
		return -1;

	wifi_cmd_args[0] = "mode";
	wifi_cmd_args[1] = mode;
	wifi_cmd_argc=2;
	if(g_flag_run_rid_capture_print){
		printf("skfd:%d,arg0:%s,arg1:%s,argc:%d,wifi:%s\n",fd,wifi_cmd_args[0],wifi_cmd_args[1],wifi_cmd_argc,if_name);
	}
	ret = wifi_set_info(fd, wifi_cmd_args, wifi_cmd_argc, if_name);
	if(ret < 0){
	  perror("set monitor");
	  return -2;
	}
	return 0;
}

int wifi_file_exists(char *filename)
{
   return (access(filename, 0) == 0);
}

int remoteid_switch_channel(int channel)
{
	if(!rid_capture_inited || skfd < 0 || !sg_wifi_name)
		return -1;
	return wifi_set_channel(skfd, sg_wifi_name, channel);
}

#ifdef RID_CFG_RUN_BY_OUTSIDE
static RID_PACKAGE_HANDLER sg_outside_handler = NULL;
static RID_CAPTURE_RESULT_CALLBACK  sg_rid_capture_result_callback = NULL;

int remoteid_main_rid_capture(int argc,char *argv[],RID_PACKAGE_HANDLER outside_handler, RID_CAPTURE_RESULT_CALLBACK rid_capture_result_callback)
{
	sg_outside_handler = outside_handler;
	sg_rid_capture_result_callback = rid_capture_result_callback;
	return remoteid_main(argc,argv);
	
}
#endif

void remoteid_stop() {

  end_program = 1;
  return;
}


#ifdef RID_CFG_RUN_BY_OUTSIDE
int remoteid_main(int argc,char *argv[]) {
#else
  int main(int argc,char *argv[]) {
#endif
  int                 i, j, set_monitor = 1, man_dev = 0, key_len, iv_len, status;
  char               *arg, *wifi_name, *nrf_name, *bluez_name, *udp_server,
                      text[128];
  uint32_t            packets_1s, last_odid_packets = 0;
  u_char              message[16];
  uid_t               uid;
  time_t              secs, last_debug = 0, last_diag = 0, last_summary = 0;
  struct timespec     start, setup_done, loop_entry, last_loop;
#if ENABLE_PCAP
  char                errbuf[PCAP_ERRBUF_SIZE];
  pcap_t             *session = NULL;
  bpf_u_int32         network = 0;
  struct bpf_program  filter;
#endif
  struct passwd      *user = NULL;
  struct utsname      sys_uname;
#if NRF_SNIFFER
  int                 bytes, nrf_bytes = 0, nrf_reads = 0;
  u_char              nrf_buffer[NRF_BUFFER_SIZE];
  pid_t               nrf_child;
#endif
  int ret;
  char *wifi_cmd_args[10] = {0};
  int  wifi_cmd_argc = 0;
  struct ifreq ifreq;
  printf("****%s:%d\n",__FUNCTION__,__LINE__);
  
  clock_gettime(CLOCK_REALTIME,&start);// 获取系统时间
  
  if (user = getpwnam("nobody")) {// 用来逐一搜索参数name 指定的账号名称, 找到时便将该用户的数据以passwd 结构返回
    nobody  = user->pw_uid; 
    nogroup = user->pw_gid;
  }

  uid = geteuid(); // 返回调用进程的有效用户标识

  memset(&RID_data,0,sizeof(RID_data));
  memset(message,  0,sizeof(message));

  memset(key,0,sizeof(key));
  memset(iv, 0,sizeof(iv));

  memcpy(key,default_key,sizeof(default_key));
  memcpy(iv, default_iv, sizeof(default_iv));

  udp_server = (char *) default_server;
  wifi_name  = (char *) dummy;
  bluez_name = (char *) device_bluez;
  nrf_name   = (char *) device_nrf;
  log_dir    = (char *) default_log_dir;
  www_dir    = (char *) default_www_dir;

  uname(&sys_uname);
  
  if (!strncmp("i686",sys_uname.machine,4)) {
  
    wifi_name = (char *) device_i686;

  } else {

    wifi_name = (char *) device_pi;
  }

  /* Parse the command line. */
  
  for (i = 1; i < argc; ++i) {

    arg = argv[i];

    if (strcmp(arg,"--help") == 0) {
      print_help();
      exit(0);
    }
  
    if (strcmp(arg,"--version") == 0) {
      print_version();
      exit(0);
    }
  
    if (*arg == '-') {
      switch (arg[1]) {

      case 'h':
        print_help();
        exit(0);
        break;

      case 'b': /* Bluez device */
        if (++i < argc) {
          bluez_name = argv[i];
        }
        break;

      case 'd': /* Curses display */
        enable_display = 1;
        enable_udp     = 1;
        break;

      case 'f': /* nRF sniffer device */
        if (++i < argc) {
          nrf_name = argv[i];
        }
        break;

      case 'k': /* key */
        if (++i < argc) {
          strncpy((char *) key,argv[i],MAX_KEY_LEN);
        }
        break;

      case 'n': /* nonce/iv */
        if (++i < argc) {
          strncpy((char *) iv,argv[i],MAX_KEY_LEN);
        }
        break;

      case 's': /* UDP server */
        if (++i < argc) {
          udp_server = argv[i];
        }
        break;

      case 'p':
        if (++i < argc) {
          if ((j = atoi(argv[i])) > 1023) {
            port = j;
          }
        }
      // No break.

      case 'u': /* UDP output. */
        enable_udp = 1;
        break;

      case 'v':
        print_version();
        exit(0);
        break;

      case 'w': /* WiFi device */
        if (++i < argc) {
          wifi_name = argv[i];
        }
        break;

      case 'x': /* WiFi device already in monitor mode. */

        set_monitor = 0;
        break;
        
      default:
        break;
      }

    } else {

      man_dev     = 1;
    }
  }

  fprintf(stderr,"%s -w %s",argv[0],wifi_name);

#if VERIFY
  fprintf(stderr," -k \'%s\' -n \'%s\'",key,iv);
#endif

  if (!set_monitor) {
    fputs(" -x",stderr);
  }

  if (enable_udp) {
    fprintf(stderr," -p %u -s %s",port,udp_server);
  }

  fprintf(stderr,"\n%s %s\n",sys_uname.sysname,sys_uname.machine);

#ifndef RID_CFG_RUN_BY_OUTSIDE
  signal(SIGINT,signal_handler);
#endif
  /* Somewhere to put the debug and summary files. */

  mkdir(log_dir,dir_mode);
  chmod(log_dir,dir_mode);
  chown(log_dir,nobody,nogroup);

#if WWW_PAGE
  mkdir(www_dir,dir_mode);
  chmod(www_dir,dir_mode);
  chown(www_dir,nobody,nogroup);
#endif

#if DEBUG_FILE
  sprintf(text,"%s/%s",log_dir,debug_filename);
  // debug_file = fopen(text,"w");
  debug_file = fopen(text,"a+");
  chmod(text,file_mode);
#endif

  if (enable_udp) {

    status = 0;

    if ((json_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) >= 0) {

      memset(&server,0,sizeof(server));
      server.sin_family = AF_INET;
      server.sin_port   = htons(port);
      inet_aton(udp_server,&server.sin_addr);
    }
    
#if 0
    fprintf(stderr,"opening socket, %08x/%04x, %d\n",
            (uint32_t) server.sin_addr.s_addr,server.sin_port,json_socket);
#endif
  }
  
#if ASTERIX
  asterix_init();
#endif

  key_len = strlen((char *) key);
  iv_len  = strlen((char *) iv);

#if VERIFY
  if (i = init_crypto(key,key_len,iv,iv_len,debug_file)) {

    exit(i);
  } 
#endif
  
/* 
*/
/* Create a channel to the NET kernel. */
if((skfd = iw_sockets_open()) < 0)
{
  perror("socket");
  exit(-1);
}


#if ENABLE_PCAP
#if 0

  if (!(session = pcap_create(wifi_name,errbuf))) {
    fprintf(stderr,"pcap_open_live(): %s\n",errbuf);
    list_devices(errbuf);
    exit(1);
  }

#if 0

  /* This doesn't seem to work. 
   */
  
  int status;

  if ((status = pcap_can_set_rfmon(session)) != 0) {
    fprintf(stderr,"pcap_can_set_rfmon(): cannot set rfmon (%d), aborting\n",status);
    pcap_close(session);
    exit(1);
  }

#endif

  if (set_monitor) {  
    if (pcap_set_rfmon(session,1) != 0) {
      fprintf(stderr,"pcap_set_rfmon():  %s\n",pcap_geterr(session));
    }
  }

#if 0

  bpf_u_int32 netmask = 0;

  if (pcap_lookupnet(wifi_name,&network,&netmask,errbuf) < 0) {
    fprintf(stderr,"pcap_lookupnet(%s): %s\n",wifi_name,errbuf);
  }

#endif

  if (i = pcap_activate(session)) {
    fprintf(stderr,"\npcap_activate():  %s, %s\n",
            pcap_geterr(session),pcap_strerror(i));
    fputs("This error may mean that you don\'t have permission to access your wifi hardware or that it is not capable of being put into monitor mode.\n",stderr);

    if (set_monitor) {
      fputs("Try setting monitor mode using iw and using the -x option to this program.\n",stderr);
    }

    if (uid) {
      fputs("You made need to run this program as root.\n",stderr);
    }

    fputs("\n",stderr);
    list_devices(errbuf);
    exit(1);
  }
#else
  wifi_if_up(skfd, wifi_name, 1);
  sleep(2);
  wifi_set_promisc(skfd, wifi_name, 1);
  sleep(1);
  wifi_set_mode(skfd, wifi_name, "monitor");  
  sleep(1);
  wifi_set_channel(skfd,wifi_name,6);
  
  //buffer 35MB 足够容纳 2.4G wifi 速率300Mbps
  session = pcap_open_live(wifi_name, 35000000, 1, 1, errbuf); //设置超时时间为1ms，超时后获取数据包的函数就会立即返回

  if(!session)
  {
	  printf("error: pcap_open_live(): %s\n", errbuf);
	  exit(1);
  }

  

#endif


  if ((header_type = pcap_datalink(session)) != DLT_IEEE802_11_RADIO) {  // 后接802.11头的链路层信息
    fprintf(stderr,"pcap_datalink(): Not tested on this header type. ^C now to abort. : %d\n",
            header_type);
  }
#if 0 
  if (pcap_compile(session,&filter,filter_text,0,network)) {
    fprintf(stderr,"pcap_compile(): \"%s\" : %s\n",filter_text,pcap_geterr(session));
  }

  if (pcap_setfilter(session,&filter)) {
    fprintf(stderr,"pcap_setfilter():  %s\n",pcap_geterr(session));
  }
#endif

#endif
  
#if BLUEZ_SNIFFER
  ble_sniffer = start_bluez_sniffer(bluez_name);
#endif

#if NRF_SNIFFER
  nrf_child = start_nrf_sniffer(nrf_name,&nrf_pipe);
#endif
  
  /* 
   */

  time(&secs);

  last_summary   =
  last_debug     = secs;
#if FA_EXPORT
  last_fa_export = secs + 2 + FA_EXPORT;
#endif
#if ASTERIX
  last_asterix   = secs + 3 + ASTERIX;
#endif
#if WWW_PAGE
  last_www_page  = secs + 4 + WWW_PAGE;
#endif

  clock_gettime(CLOCK_REALTIME,&setup_done);

  setup_ms = ((double) setup_done.tv_sec * 1e3 + (double) setup_done.tv_nsec * 1e-6) -
             ((double) start.tv_sec      * 1e3 + (double) start.tv_nsec      * 1e-6);
  
#if ENABLE_PCAP
  if (header_type != DLT_IEEE802_11_RADIO) {

      do {

        time(&secs);

      } while (secs < (last_debug + 2));
  }
#endif

  if (enable_display) {
    display_init();
  }

  sprintf(text,"{ \"setup_time_ms\" : %.0f }\n",setup_ms);
  write_json(text);

  clock_gettime(CLOCK_REALTIME,&last_loop);

  /* Main loop. */
  rid_capture_inited = 1;
  strcpy(&wifi_name_tmp,wifi_name);
  sg_wifi_name = wifi_name_tmp;
  if(wifi_file_exists(FLAG_DEBUG_RUN_RID_CAPTURE)){
	printf("***set sg_flag_run_rid_capture_handler  = 1\n");
	sg_flag_run_rid_capture_handler  = 1;
  }

  
  if(wifi_file_exists(FLAG_DEBUG_RUN_RID_CAPTURE_PRINT)){
	printf("***set g_flag_run_rid_capture_print  = 1\n");
	g_flag_run_rid_capture_print  = 1;
  }

  
  while (!end_program) {

    clock_gettime(CLOCK_REALTIME,&loop_entry);
    loop_us           = ((double) loop_entry.tv_sec * 1e6 + (double) loop_entry.tv_nsec * 1e-3) -
                        ((double) last_loop.tv_sec  * 1e6 + (double) last_loop.tv_nsec  * 1e-3);
    last_loop.tv_sec  = loop_entry.tv_sec;
    last_loop.tv_nsec = loop_entry.tv_nsec;

#if ENABLE_PCAP
    pcap_loop(session,1,packet_handler,message);
#endif

#if BLUEZ_SNIFFER
    if (ble_sniffer > -1) {
      for (j = 0; (j < 4)&&(!end_program); ++j) {

        if (parse_bluez_sniffer() < 1) {
          break;
        }
      }
    }
#endif

    // The nRF sniffer can put out a lot of data hence the separate process.

#if NRF_SNIFFER
    if (nrf_child > 0) {
      for (nrf_reads = 0, nrf_bytes = 0; (nrf_reads < 4)&&(!end_program);) {

        if ((bytes = read(nrf_pipe,nrf_buffer,sizeof(nrf_buffer))) > 0) {

          nrf_bytes += bytes;
          parse_nrf_sniffer(nrf_buffer,bytes);
        }

        ++nrf_reads;

        if (bytes < sizeof(nrf_buffer)) {
          break;
        }
      }
    }
#endif

    time(&secs);

    if ((secs - last_debug) > 9) {
	  static int scan_idx = 0;

#ifndef RID_CFG_RUN_BY_OUTSIDE
	  //scan channel
	  int scan_channels[] = {1,6,11};
	  int scan_channel_nums = sizeof(scan_channels)/sizeof(int);
	  int chn = scan_idx%scan_channel_nums;
	  scan_idx++;
	  wifi_set_channel(skfd,wifi_name,scan_channels[chn]);
#endif

//if(sg_flag_run_rid_capture_handler)
{

      sprintf(text,"{ \"debug\" : \"rx packets %u (%u)\", \"loop_time_us\" : %.0f, \"max udp len\" : %d }\n",
              rx_packets,odid_packets,loop_us,max_udp_length);
      write_json(text);
#if NRF_SNIFFER
      sprintf(text,"{ \"nrf_reads\" : %d, \"nrf_bytes\" : %d }\n",
              nrf_reads,nrf_bytes);
      write_json(text);
#endif  
}
      last_debug = secs;
    }

#if 0
    if ((odid_packets % 20) == 19) {

      write(2,".",1);
    }
#endif

    if ((secs - last_summary) > 899) {

      print_summary(log_dir,NULL,RID_data,MAX_UAVS);
      last_summary = secs;
    }

    if (secs != last_diag) {

      packets_1s        = odid_packets - last_odid_packets;
      last_odid_packets = odid_packets;

      display_loop_diag(loop_us,packets_1s);

      last_diag = secs;
    }

#if ASTERIX > 0
    if ((secs - last_asterix) >= ASTERIX) {
      asterix_transmit(RID_data);
      last_asterix = secs;
    }
#endif

#if FA_EXPORT > 0
    if ((secs - last_fa_export) >= FA_EXPORT) {
      fa_export(secs,RID_data);
      last_fa_export = secs;
    }
#endif

#if WWW_PAGE > 0
    if ((secs - last_www_page) >= WWW_PAGE) {
      www_export(www_dir,secs,RID_data);
      last_www_page = secs;
    }
#endif
  }

  /* 
   */

  display_end();

#if BLUEZ_SNIFFER
  stop_bluez_sniffer();
#endif

#if NRF_SNIFFER
  time_t term_sent;
  
  if (nrf_child > 0) {
    kill(nrf_child,SIGTERM);
  }

  time(&term_sent);
#endif

#if VERIFY
  close_crypto();
#endif

#if ENABLE_PCAP
  pcap_close(session);

  wifi_set_mode(skfd, sg_wifi_name, "managed");  
  wifi_set_promisc(skfd, sg_wifi_name, 0);
#endif

  if (enable_udp) {
    close(json_socket);
  }

  print_summary(log_dir,stderr,RID_data,MAX_UAVS);

  if (debug_file) {

    fclose(debug_file);
  }

#if NRF_SNIFFER
  if (nrf_child > 0) {

    fputs("\nWaiting for child process ",stderr);
 
    int wstatus;

    for (i = 0; i < 10; ++i) {
      
      if (waitpid(nrf_child,&wstatus,WNOHANG) > 0) {
        break;
      }

      write(2,"*",1);
      time(&secs);

      if ((secs - term_sent) > 4) {
        kill(nrf_child,SIGKILL);
      }
      
      sleep(1);
    }
  }
#endif

  fprintf(stderr,"\n");

  return(0);
}

/*
 *
 */

#if ENABLE_PCAP

void list_devices(char *errbuf) {

  int        i;
  pcap_if_t *devices, *dev;

  fprintf(stderr,"Available devices\n");
  
  if (pcap_findalldevs(&devices,errbuf) < 0) {

    fprintf(stderr,"pcap_findalldevs(): %s\n",errbuf);
    return;
  }

  for(i = 0, dev = devices; dev; dev = dev->next, ++i) {

    fprintf(stderr,"%d: %s\n",i,dev->name);
  }

  pcap_freealldevs(devices);  
  
  return;
}

/*
 *
 */

struct timeval last_tv;
void packet_handler(u_char *args,const struct pcap_pkthdr *header,const u_char *packet) {

  int            i, offset = 36, length, typ, len;
  char           ssid_tmp[32], text[128];
  u_char        *payload, *val, mac[6], uchardata[4];
  u_int16_t     *radiotap_len; 
  static u_char  nan_cluster[6]  = {0x50, 0x6f, 0x9a, 0x01, 0x00, 0xff},
                 nan_service[6]  = {0x88, 0x69, 0x19, 0x9d, 0x92, 0x09},
                 oui_alliance[3] = {0x50, 0x6f, 0x9a};

  ssid_tmp[0]  =
  ssid_tmp[32] = 0;
  radiotap_info info = {0};

  ++rx_packets;

  length       = header->len;
  radiotap_len = (u_int16_t *) &packet[2];

  if (*radiotap_len < length) {
    
    payload = (u_char *)  &packet[*radiotap_len];
	radiotap_parse_header(packet, *radiotap_len,&info);
  } else {

#if 1
    sprintf(text,"{ \"debug\" : \"%d, ",*radiotap_len);
    write_json(text);

    for (i = 0; i < 40; ++i) {

      sprintf(text,"%02x ",packet[i]);
      write_json(text);
    }
        
    write_json("\" }\n");
#endif
    return;
  }

  if (payload[0] == 0x80) { // beacon

#ifdef  RID_CFG_RUN_BY_OUTSIDE
		if(sg_outside_handler){
			(*sg_outside_handler)(args,header,packet);
		}
	
	#if  NO_RUN_RID_CAPTURE_HANDLER
			return;
	#else
			if(!sg_flag_run_rid_capture_handler)
				return;
	#endif
#endif

    offset = 36;

    for (i = 0; i < 6; ++i) {

      mac[i] = payload[i + 10];
    }
    
    while (offset < length) {

      typ =  payload[offset];
      len =  payload[offset + 1];
      val = &payload[offset + 2];

      if ((typ    == 0xdd)&&
          (val[0] == 0xfa)&& // ODID
          (val[1] == 0x0b)&&
          (val[2] == 0xbc)) {

#if 0
        printf("**%s:%d ************ rx_packets :  %d ************length:  %d ************** \r\n",__FUNCTION__,__LINE__,rx_packets,length - (*radiotap_len));
        for (int k = 0; k < (length - (*radiotap_len)); ++k)
        {
          if (k % 20 == 0)
            printf("\n");
          printf("%02x ", payload[k]);
        }
        printf("\r\n");
#endif 

        //struct timeval tv1;
        //gettimeofday(&tv1, NULL);
        //printf("received 0xdd header, second = %d, microseconds = %d \n", tv1.tv_sec, tv1.tv_usec);

        if (wifi_file_exists(FLAG_DEBUG_REMOTEID_GET_UAV_DATA))
        {
          rid_storeuavdatatofile(debug_file, "\r\n ********************************************************************");

          struct timeval tv;
          gettimeofday(&tv, NULL);

          time_t currentTime = time(NULL);
          char* timeString = ctime(&currentTime);

          uint64_t diffInMs = 0;
          if (tv.tv_sec > last_tv.tv_sec)
          {
              if (tv.tv_usec >= last_tv.tv_usec)
              {
                  diffInMs = (tv.tv_sec - last_tv.tv_sec) * 1000 + (tv.tv_usec - last_tv.tv_usec) / 1000;
              }
              else
              {
                  diffInMs = (tv.tv_sec - last_tv.tv_sec - 1) * 1000 + (1000000 + tv.tv_usec - last_tv.tv_usec) / 1000;            
              }
          }
          else if (tv.tv_sec == last_tv.tv_sec)
          {
              if (tv.tv_usec >= last_tv.tv_usec)
              {
                  diffInMs = (tv.tv_usec - last_tv.tv_usec) / 1000;
              }
          }
          last_tv = tv;

          u_char ubuffer[96];
          memset(ubuffer, 0, 96);

          if ((0 == diffInMs) || (diffInMs > 4000)) // 3 channels switch, each 700ms
          {
            sprintf(ubuffer, "long diffInMs = %d, second = %d, microseconds = %d", diffInMs, tv.tv_sec, tv.tv_usec);
          }
          else
          {
            sprintf(ubuffer, "normal diffInMs = %d, second = %d, microseconds = %d", diffInMs, tv.tv_sec, tv.tv_usec);
          }

          rid_storeuavdatatofile(debug_file, timeString);
          rid_storeuavdatatofile(debug_file, ubuffer);

          for (int k = 0; k < (length - (*radiotap_len)); ++k)
          {
            if (k % 20 == 0)
            {
              rid_storeuavdatatofile(debug_file,"\n");
            } 
            sprintf(uchardata, "%02x ", payload[k]);
            rid_storeuavdatatofile(debug_file, uchardata);
          }
          rid_storeuavdatatofile(debug_file, "\n ******************************************************************** \r\n");
        }
        parse_odid(mac,&payload[offset + 6],length - offset - 6,0,"pcap beacon",NULL,&info);
 
      } else if ((typ    == 0xdd)&&
                 (val[0] == oui_alliance[0])&& // WiFi Alliance
                 (val[1] == oui_alliance[1])&&
                 (val[2] == oui_alliance[2])) {
#if 0
        write_json("{ \"debug\" : \"Beacon with Alliance OUI\" }\n");
#else
        ;
#endif
      } else if ((typ    == 0xdd)&&
                 (val[0] == 0x6a)&& // French ID
                 (val[1] == 0x5c)&&
                 (val[2] == 0x35)) {
#if ID_FRANCE
        parse_id_france(mac,&payload[offset],RID_data);
#else
        write_json("{ \"debug\" : \"French ID\" }\n");
#endif
      } else if ((typ == 0)&&(!ssid_tmp[0])) {

        for (i = 0; (i < 32)&&(i < len); ++i) {

          ssid_tmp[i] = val[i];
        }
      }

      offset += len + 2;
    }

  } else {

#if 0
    sprintf(text,"{ \"debug\" : \"%d | ",length);
    write_json(text);

    for (i = 0; i < 24; ++i) {

      sprintf(text,"%02x ",payload[i]);
      write_json(text);
    }
     
    write_json("| ");

    for (i = 44; i < 60; ++i) {

      sprintf(text,"%02x ",payload[i]);
      write_json(text);
    }
     
    write_json("\" }\n");
#endif
    
    if (memcmp(nan_cluster,&payload[16],6) == 0) { // NAN

      offset = 24;

      if ((length > 44)&&
          (payload[offset]     == 0x04)&&
          (payload[offset + 1] == 0x09)&&
          (payload[offset + 2] == oui_alliance[0])&&
          (payload[offset + 3] == oui_alliance[1])&&
          (payload[offset + 4] == oui_alliance[2])&&
          (memcmp(nan_service,&payload[offset + 9],6) == 0)) {

        /* write_json("{ \"debug\" : \"NAN action frame\" }\n"); */

        offset += 19;
      
        parse_odid(mac,&payload[offset],length - offset,0,"pcap NAN",NULL,&info);
      }
    }
  }

  return;
}

#endif // PCAP

/*
 *
 */

void parse_odid(u_char *mac,u_char *payload,int length,int rssi,const char *note,const float *volts,radiotap_info *info) {

  int                       i, j, RID_index, page, authenticated;
  char                      json[128];
  uint8_t                   counter, index;
  double                    latitude, longitude, altitude;
  ODID_UAS_Data             UAS_data;
  ODID_MessagePack_encoded *encoded_data = (ODID_MessagePack_encoded *) &payload[1];
  RemoteId_Info uav_info = {0};

  i             = 0;
  authenticated = 0;

  /* */

  RID_index = mac_index(mac,RID_data);

  /* Decode */
  counter = payload[0];
  index   = payload[1] >> 4;

#if 0
  if (RID_data[RID_index].counter[index] == counter) {
    return;
  }
#endif

  RID_data[RID_index].counter[index] = counter;

  ++odid_packets;
  ++RID_data[RID_index].packets;
  RID_data[RID_index].rssi = rssi;

  if (note) {
    display_note(RID_index + 1,note);
  }
  
  if (volts) {
    display_voltage(RID_index + 1,*volts);
  }

  memset(&UAS_data,0,sizeof(UAS_data));

  switch (payload[1] & 0xf0) {

  case 0x00:
    decodeBasicIDMessage(&UAS_data.BasicID[0],(ODID_BasicID_encoded *) &payload[1]);
    UAS_data.BasicIDValid[0] = 1;
    break;
 
  case 0x10:
    decodeLocationMessage(&UAS_data.Location,(ODID_Location_encoded *) &payload[1]);
    UAS_data.LocationValid = 1;
    break;

  case 0x20:
    page = payload[2] & 0x0f;
    decodeAuthMessage(&UAS_data.Auth[page],(ODID_Auth_encoded *) &payload[1]);
    UAS_data.AuthValid[page] = 1;
    break;

  case 0x30:
    decodeSelfIDMessage(&UAS_data.SelfID,(ODID_SelfID_encoded *) &payload[1]);
    UAS_data.SelfIDValid = 1;
    break;

  case 0x40:
    decodeSystemMessage(&UAS_data.System,(ODID_System_encoded *) &payload[1]);
    UAS_data.SystemValid = 1;
    break;

  case 0x50:
    decodeOperatorIDMessage(&UAS_data.OperatorID,(ODID_OperatorID_encoded *) &payload[1]);
    UAS_data.OperatorIDValid = 1;
    break;

  case 0xf0:
    decodeMessagePack(&UAS_data,encoded_data);
    break;
  }
  
  /* JSON */
  sprintf(json,"{ \"mac\" : \"%02x:%02x:%02x:%02x:%02x:%02x\"",
          mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  write_json(json);
  rid_storeuavdatatofile(debug_file, json);
  memcpy(uav_info.mac,mac,6);
  
#if 1
	sprintf(json,", \"chn\" : %hu, \"freq\" : %hu, \"signal_dbm\" : %hhd,\"noise_dbm\" : %hhd,",
		info->channel,info->freq,info->dbm_ant_signal,info->dbm_ant_noise);
	write_json(json);
  rid_storeuavdatatofile(debug_file, json);
	uav_info.uChn = info->channel;
	uav_info.uFreq = info->freq;
	uav_info.dbm_ant_signal = info->dbm_ant_signal;
	uav_info.dbm_ant_noise = info->dbm_ant_noise;
	
#endif

#if 1
  sprintf(json,", \"rssi\" : %d",rssi);
  write_json(json);
  rid_storeuavdatatofile(debug_file, json);
  uav_info.rssi = rssi;
#endif

#if 0
  sprintf(json,", \"debug\" : \"%d | ",length);
  write_json(json);

  for (i = 0; i < 16; ++i) {

    sprintf(json,"%02x ",payload[i]);
    write_json(json);
  }

  write_json("\"");
#endif

  if (UAS_data.OperatorIDValid) {

    sprintf(json,", \"operator\" : \"%s\"",UAS_data.OperatorID.OperatorId);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);

    memcpy(&RID_data[RID_index].odid_data.OperatorID,&UAS_data.OperatorID,sizeof(ODID_OperatorID_data));
    display_identifier(RID_index + 1,UAS_data.OperatorID.OperatorId);

	uav_info.operatorIdType = (uint32_t)UAS_data.OperatorID.OperatorIdType;
	snprintf(uav_info.operatorId,sizeof(uav_info.operatorId)-1,"%s",UAS_data.OperatorID.OperatorId);
  }

  for (j = 0; j < ODID_BASIC_ID_MAX_MESSAGES; ++j) {

    if (UAS_data.BasicIDValid[j]) {

      memcpy(&RID_data[RID_index].odid_data.BasicID[j],&UAS_data.BasicID[j],sizeof(ODID_BasicID_data));

      switch (UAS_data.BasicID[j].IDType) {

      case ODID_IDTYPE_SERIAL_NUMBER:
        memcpy(&RID_data[RID_index].basic_serial,&UAS_data.BasicID[j],sizeof(ODID_BasicID_data));
        sprintf(json,", \"uav id\" : \"%s\"",UAS_data.BasicID[j].UASID);
        write_json(json);
        rid_storeuavdatatofile(debug_file, json);
		
		snprintf(uav_info.uavId,sizeof(uav_info.uavId)-1,"%s",UAS_data.BasicID[j].UASID);
#if 0
        display_identifier(RID_index + 1,UAS_data.BasicID[j].UASID);
#endif
        break;
        
      case ODID_IDTYPE_CAA_REGISTRATION_ID:
        memcpy(&RID_data[RID_index].basic_caa_reg,&UAS_data.BasicID[j],sizeof(ODID_BasicID_data));
        sprintf(json,", \"caa registration\" : \"%s\"",UAS_data.BasicID[j].UASID);
        write_json(json);
        rid_storeuavdatatofile(debug_file, json);
        display_identifier(RID_index + 1,UAS_data.BasicID[j].UASID);
		
		snprintf(uav_info.uasid,sizeof(uav_info.uasid)-1,"%s",UAS_data.BasicID[j].UASID);
        break;
        
      case ODID_IDTYPE_NONE:
      default:
        break;
      }
    }
  }

  if (UAS_data.LocationValid) {

    latitude  = UAS_data.Location.Latitude;
    longitude = UAS_data.Location.Longitude;
    altitude  = UAS_data.Location.Height;

    sprintf(json,", \"uav latitude\" : %11.6f, \"uav longitude\" : %11.6f",
            latitude,longitude);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);
	uav_info.latitude = latitude;
	uav_info.longitude = longitude;
    sprintf(json,", \"uav altitude\" : %d, \"uav heading\" : %d",
           (int) UAS_data.Location.AltitudeGeo,(int) UAS_data.Location.Direction);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);
	uav_info.direction = UAS_data.Location.Direction;
  uav_info.atmosaltitude = UAS_data.Location.AltitudeBaro;
  uav_info.earthaltitude = UAS_data.Location.AltitudeGeo;
  uav_info.altitude = UAS_data.Location.Height;
  uav_info.HeightType = UAS_data.Location.HeightType;
#if 0
    sprintf(json,", \"uav speed horizontal\" : %d, \"uav speed vertical\" : %d",
           (int) UAS_data.Location.SpeedHorizontal,(int) UAS_data.Location.SpeedVertical);
    write_json(json);
#endif
    sprintf(json,", \"uav speed\" : %d, \"seconds\" : %d",
           (int) UAS_data.Location.SpeedHorizontal,(int) UAS_data.Location.TimeStamp);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);
	
    uav_info.speedHorizontal = UAS_data.Location.SpeedHorizontal;
    uav_info.speedVertical = UAS_data.Location.SpeedVertical;

    memcpy(&RID_data[RID_index].odid_data.Location,&UAS_data.Location,sizeof(ODID_Location_data));

    display_uav_loc(RID_index + 1,latitude,longitude,
                    (int) altitude,(int) UAS_data.Location.TimeStamp);

    if ((latitude < RID_data[RID_index].min_lat)||(RID_data[RID_index].min_lat == 0.0)) {
      RID_data[RID_index].min_lat = latitude;
    }

    if ((latitude > RID_data[RID_index].max_lat)||(RID_data[RID_index].max_lat == 0.0)) {
      RID_data[RID_index].max_lat = latitude;
    }

    if ((longitude < RID_data[RID_index].min_long)||(RID_data[RID_index].min_long == 0.0)) {
      RID_data[RID_index].min_long = longitude;
    }

    if ((longitude > RID_data[RID_index].max_long)||(RID_data[RID_index].max_long == 0.0)) {
      RID_data[RID_index].max_long = longitude;
    }

    if (altitude > INV_ALT) {
    
      if ((altitude < RID_data[RID_index].min_alt)||(RID_data[RID_index].min_alt == 0.0)) {
        RID_data[RID_index].min_alt = altitude;
      }

      if ((altitude > RID_data[RID_index].max_alt)||(RID_data[RID_index].max_alt == 0.0)) {
        RID_data[RID_index].max_alt = altitude;
      }
    }
  }
  
  if (UAS_data.SystemValid) {

    sprintf(json,", \"base latitude\" : %11.6f, \"base longitude\" : %11.6f",
           UAS_data.System.OperatorLatitude,UAS_data.System.OperatorLongitude);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);
    sprintf(json,", \"unix time\" : %lu",
           ((unsigned long int) UAS_data.System.Timestamp) + ID_OD_AUTH_DATUM);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);

	uav_info.pilot_latitude =  UAS_data.System.OperatorLatitude;
	uav_info.pilot_longitude = UAS_data.System.OperatorLongitude;


    memcpy(&RID_data[RID_index].odid_data.System,&UAS_data.System,sizeof(ODID_System_data));

    display_timestamp(RID_index + 1,(time_t) UAS_data.System.Timestamp + ID_OD_AUTH_DATUM);
  }

  if (UAS_data.SelfIDValid) {

    sprintf(json,", \"self id\" : \"%s\"",UAS_data.SelfID.Desc);
    write_json(json);
    rid_storeuavdatatofile(debug_file, json);

	snprintf(uav_info.selfid,sizeof(uav_info.selfid)-1,"%s",UAS_data.SelfID.Desc);
    memcpy(&RID_data[RID_index].odid_data.SelfID,&UAS_data.SelfID,sizeof(ODID_SelfID_data));
  }

  for (page = 0; page < ODID_AUTH_MAX_PAGES; ++page) {
  
    if (UAS_data.AuthValid[page]) {

      if (page == 0) {

        sprintf(json,", \"unix time (alt)\" : %lu",
               ((unsigned long int) UAS_data.Auth[page].Timestamp) + ID_OD_AUTH_DATUM);
        write_json(json);
        rid_storeuavdatatofile(debug_file, json);

        display_timestamp(RID_index + 1,(time_t) UAS_data.Auth[page].Timestamp + ID_OD_AUTH_DATUM);
      }

      sprintf(json,", \"auth page %d\" : { \"text\" : \"%s\"",page,
             printable_text(UAS_data.Auth[page].AuthData,
                            (page) ? ODID_AUTH_PAGE_NONZERO_DATA_SIZE: ODID_AUTH_PAGE_ZERO_DATA_SIZE));
      write_json(json);
      rid_storeuavdatatofile(debug_file, json);

#if 1
      write_json(", \"values\" : [");
      rid_storeuavdatatofile(debug_file, ", \"values\" : [");
    
      for (i = 0; i < ((page) ? ODID_AUTH_PAGE_NONZERO_DATA_SIZE: ODID_AUTH_PAGE_ZERO_DATA_SIZE); ++i) {

        sprintf(json,"%s %d",(i) ? ",":"",UAS_data.Auth[page].AuthData[i]);
        write_json(json);
        rid_storeuavdatatofile(debug_file, json);
      }

      write_json(" ]");
      rid_storeuavdatatofile(debug_file, " ]");
#endif
      write_json(" }");
      rid_storeuavdatatofile(debug_file, " }");
    
      memcpy(&RID_data[RID_index].odid_data.Auth[page],&UAS_data.Auth[page],sizeof(ODID_Auth_data));
    }
  }

#if VERIFY
  authenticated = parse_auth(&UAS_data,encoded_data,&RID_data[RID_index]);
  display_pass(RID_index + 1,(authenticated)? pass_s: "    "); 
#endif

  write_json(" }\n");
  rid_storeuavdatatofile(debug_file, " }\n");

  /* */
  
#if 0
  for (i = 0; (i < length)&&(i < 16); ++i) {

    fprintf(stderr,"%02x ",payload[i]);
  }
  
  fprintf(stderr,"%s\n",printable_text(payload,16));
#endif
  //printf("**%s:%d\n",__FUNCTION__,__LINE__);

  // if(sg_rid_capture_result_callback)
  if((sg_rid_capture_result_callback) && (UAS_data.BasicIDValid[0] | UAS_data.BasicIDValid[1]) && (UAS_data.LocationValid) && ((UAS_data.BasicID[0].IDType == 1) || (UAS_data.BasicID[1].IDType == 1)))
  {
    printf("********************  %s  :   %d    ****************************\n",__FUNCTION__,__LINE__);
    (*sg_rid_capture_result_callback)(&uav_info,sizeof(uav_info));
  }
  return;
}

/*
 *
 */

static void signal_handler(int sig) {

  end_program = 1;
  
  return;
}

/*
 * Function to find a record to store the decoded data in.
 *
 */

int mac_index(uint8_t *mac,struct UAV_RID *RID_data) {

  int    i, RID_index = 0, oldest = 0;
  char   text[64];
  time_t secs, oldest_secs;

  time(&secs);

  RID_index   =
  oldest      = 0;
  oldest_secs = secs;
  
  for (i = 0; i < MAX_UAVS; ++i) {

    if (memcmp(mac,RID_data[i].mac,6) == 0) {

      RID_index           = i;
      RID_data[i].last_rx = secs;

      break;
    }
    
    if (RID_data[i].last_rx < oldest_secs) {

      oldest      = i;
      oldest_secs = RID_data[i].last_rx;
    }
  }
  if (i == MAX_UAVS) {

    struct UAV_RID *uav;

    uav = &RID_data[oldest];

    sprintf(text,"%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    display_mac(oldest + 1,mac);
    if (!enable_display) {

      fputs(text,stderr);

      if (uav->mac[0]) {

        fprintf(stderr," - reusing RID record %d (%02x:%02x:%02x:%02x:%02x:%02x)\n",oldest,
                uav->mac[0],uav->mac[1],uav->mac[2],
                uav->mac[3],uav->mac[4],uav->mac[5]);
      } else {

        fprintf(stderr," - using RID record %d\n",oldest);
      }
    }
    RID_index        = oldest;
    uav->last_rx     = secs;
    uav->last_retx   = 0;
    uav->packets     = 0;

    memcpy(uav->mac,mac,6);
    memset(&uav->odid_data, 0,sizeof(ODID_UAS_Data));

#if VERIFY
    uav->auth_length = 0;
    memset(uav->auth_buffer,0,sizeof(uav->auth_buffer));
#endif
  }
  return RID_index;
}

/*
 *
 */

void dump(char *name,uint8_t *data,int len) {

  int i;
  
  if (debug_file) {

    fprintf(debug_file,"%s[] = {",name);

    for (i = 0; i < len; ++i) {

      fprintf(debug_file,"%s 0x%02x",(i) ? ",": "",data[i]);
    }
    
    fprintf(debug_file," };\n%s_s = \"%s\";\n",name,printable_text(data,len));

    fflush(debug_file);
  }
  
  return;
}

/*
 *
 *  \ is 0x5c
 */

char *printable_text(uint8_t *data,int len) {

  int    i;
  static char text[32];

  for (i = 0; (i < 31)&&(i < len); ++i) {

    text[i] = (isprint(data[i])&&(data[i] != '"')&&(data[i] != '\\')) ? (char) data[i]: '.';
  }

  text[i] = 0;

  return text;
}


/*
 *
 */

void print_version() {

  fprintf(stderr,"rid_capture version %s\r\n",VERSION);

  return;
}

/*
 *
 */

void print_help() {

  int          i;
  static const char *help_text[] =
    {"Options",
     "",
#if BLUEZ_SNIFFER
     "-b <bluez device>",
#endif
#if USE_CURSES
     "-d, basic text display",
#endif
#if NRF_SNIFFER
     "-f <tty>, the tty that an nRF52 sniffer is in",
#endif
     "-k <crypto key>",
     "-n <crypto initial vector>",
     "-p <UDP port>",
     "-s <UDP server>",
     "-u, enables UDP output",
#if ENABLE_PCAP
     "-w <WiFi device>",
#endif
     "-x, do not try to put the WiFi device into monitor",
     "",
     NULL },
                     crlf[] = "\r\n";

  for (i = 0; (i < 24)&&(help_text[i]); ++i) {

    fputs(help_text[i],stderr);
    fputs(crlf,stderr);
  }

  return;
}

/*
 *
 */

#define UDP_BUFFER_SIZE 800 // Should really be 500, but we can get close to 500 with packed messages. 

int write_json(char *json) {

  int            status;
#if UDP_BUFFER_SIZE
  static int     index = 0;
  static uint8_t udp_buffer[UDP_BUFFER_SIZE + 2], c;
#else
  int            l;
#endif
  
  if (json_socket > -1) {

#if UDP_BUFFER_SIZE
    while (*json) {

      udp_buffer[index++] = c = (uint8_t) *json++;

      if ((c     == 0x0a)||
          (c     == 0x0d)||
          (index >= UDP_BUFFER_SIZE)){

        udp_buffer[index] = 0;
        
        if ((status = sendto(json_socket,udp_buffer,index,0,
                             (struct sockaddr *) &server,sizeof(server))) < 0) {

          fprintf(stderr,"%s(): %d, %d, %d\n",
                  __func__,index,status,errno);
        }

        if (index > max_udp_length) {
          max_udp_length = index;
        }

        index = 0;
        break;
      }
    }
#else
    if ((status = sendto(json_socket,json,l = strlen(json),0,
                         (struct sockaddr *) &server,sizeof(server))) < 0) {

      fprintf(stderr,"%s(): %d, %d, %d\n",
              __func__,l,status,errno);
    }
#endif
  } else {

	if(g_flag_run_rid_capture_print){
    	fputs(json,stdout);
	}
  }

  return 0;
}



void rid_storeuavdatatofile(FILE *debug_file, const char *json_stream)
{
  if (wifi_file_exists(FLAG_DEBUG_REMOTEID_GET_UAV_DATA))
  {
    fprintf(debug_file,"%s  ", json_stream);
  }
}

/*
 *
 */

 
/*
 *
 */


