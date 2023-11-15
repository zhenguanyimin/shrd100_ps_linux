/**
 * @file libaxidma.c
 * @date Saturday, December 05, 2015 at 10:10:39 AM EST
 * @author Brandon Perez (bmperez)
 * @author Jared Choi (jaewonch)
 *
 * This is a simple library that wraps around the AXI DMA module,
 * allowing for the user to abstract away from the finer grained details.
 *
 * @bug No known bugs.
 **/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>             // Memset and memcpy functions
#include <fcntl.h>              // Flags for open()
#include <sys/stat.h>           // Open() system call
#include <sys/types.h>          // Types for open()
#ifndef __UNITTEST__
#include <sys/mman.h>           // Mmap system call
#include <sys/ioctl.h>          // IOCTL system call
#endif
#include <unistd.h>             // Close() system call
#include <errno.h>              // Error codes
#include <signal.h>             // Signal handling functions

#include "libaxidma.h"          // Local definitions
#include "axidma_ioctl.h"       // The IOCTL interface to AXI DMA

/*----------------------------------------------------------------------------
 * Internal definitions
 *----------------------------------------------------------------------------*/

// A structure that holds metadata about each channel
typedef struct dma_channel {
    enum axidma_dir dir;        ///< Direction of the channel
    enum axidma_type type;      ///< Type of the channel
    int channel_id;             ///< Integer id of the channel.
    axidma_cb_t callback;       ///< Callback function for channel completion
    void *user_data;            ///< User data to pass to the callback
} dma_channel_t;

// The structure that represents the AXI DMA device
struct axidma_dev {
    bool initialized;           ///< Indicates initialization for this struct.
    int fd;                     ///< File descriptor for the device
    array_t dma_tx_chans;       ///< Channel id's for the DMA transmit channels
    array_t dma_rx_chans;       ///< Channel id's for the DMA receive channels
    array_t vdma_tx_chans;      ///< Channel id's for the VDMA transmit channels
    array_t vdma_rx_chans;      ///< Channel id's for the VDMA receive channels
    int num_channels;           ///< The total number of DMA channels
    dma_channel_t *channels;    ///< All of the VDMA/DMA channels in the system
};

// The DMA device structure, and a boolean checking if it's already open
struct axidma_dev axidma_dev = {0};

/*----------------------------------------------------------------------------
 * Private Helper Functions
 *----------------------------------------------------------------------------*/

/* Categorizes the DMA channels by their type and direction, getting their ID's
 * and placing them into separate arrays. */
static int categorize_channels(axidma_dev_t dev,
        struct axidma_chan *channels, struct axidma_num_channels *num_chan)
{
    return 0;
}

/* Probes the AXI DMA driver for all of the available channels. It places
 * returns an array of axidma_channel structures. */
static int probe_channels(axidma_dev_t dev)
{
    return 0;
}

/* Sets up a signal handler for the lowest real-time signal to be delivered
 * whenever any asynchronous DMA transaction compeletes. */
// TODO: Should really check if real time signal is being used
static int setup_dma_callback(axidma_dev_t dev)
{
    return 0;
}

// Finds the DMA channel with the given id
static dma_channel_t *find_channel(axidma_dev_t dev, int channel_id)
{
    return NULL;
}

// Converts the AXI DMA direction to the corresponding ioctl for the transfer
static unsigned long dir_to_ioctl(enum axidma_dir dir)
{
    return 0;
}

/*----------------------------------------------------------------------------
 * Public Interface
 *----------------------------------------------------------------------------*/

/* Initializes the AXI DMA device, returning a new handle to the
 * axidma_device. */
struct axidma_dev *axidma_init()
{
    return NULL;
}

// Tears down the given AXI DMA device structure
void axidma_destroy(axidma_dev_t dev)
{
    return;
}

// Returns an array of all the available AXI DMA transmit channels
const array_t *axidma_get_dma_tx(axidma_dev_t dev)
{
    return NULL;
}

// Returns an array of all the available AXI DMA receive channels
const array_t *axidma_get_dma_rx(axidma_dev_t dev)
{
    return NULL;
}

// Returns an array of all the available AXI VDMA transmit channels
const array_t *axidma_get_vdma_tx(axidma_dev_t dev)
{
    return NULL;
}

// Returns an array of all the available AXI VDMA receive channels
const array_t *axidma_get_vdma_rx(axidma_dev_t dev)
{
    return NULL;
}

/* Allocates a region of memory suitable for use with the AXI DMA driver. Note
 * that this is a quite expensive operation, and should be done at initalization
 * time. */
void *axidma_malloc(axidma_dev_t dev, size_t size)
{
    return NULL;
}

/* This frees a region of memory that was allocated with a call to
 * axidma_malloc. The size passed in here must match the one used for that
 * call, or this function will throw an exception. */
void axidma_free(axidma_dev_t dev, void *addr, size_t size)
{
    return;
}

/* Sets up a callback function to be called whenever the transaction completes
 * on the given channel for asynchronous transfers. */
void axidma_set_callback(axidma_dev_t dev, int channel, axidma_cb_t callback,
                        void *data)
{
    return;
}

/* Registers a DMA buffer allocated by another driver with the AXI DMA driver.
 * This allows it to be used in DMA transfers later on. The user must make sure
 * that the driver that allocated the buffer has exported it. The file
 * descriptor is the one that is returned by the other driver's export. */
int axidma_register_buffer(axidma_dev_t dev, int dmabuf_fd, void *user_addr,
                           size_t size)
{
    return 0;
}

/* Unregisters a DMA buffer preivously registered with the driver. This is
 * required to clean up the kernel data structures. */
void axidma_unregister_buffer(axidma_dev_t dev, void *user_addr)
{
    return;
}

/* This performs a one-way transfer over AXI DMA, the direction being specified
 * by the user. The user determines if this is blocking or not with `wait. */
int axidma_oneway_transfer(axidma_dev_t dev, int channel, void *buf,
        size_t len, bool wait)
{
    return 0;
}

/* This performs a two-way transfer over AXI DMA, both sending data out and
 * receiving it back over DMA. The user determines if this call is blocking. */
int axidma_twoway_transfer(axidma_dev_t dev, int tx_channel, void *tx_buf,
        size_t tx_len, struct axidma_video_frame *tx_frame, int rx_channel,
        void *rx_buf, size_t rx_len, struct axidma_video_frame *rx_frame,
        bool wait)
{
    return 0;
}

/* This function performs a video transfer over AXI DMA, setting up a VDMA
 * channel to either read from or write to given frame buffers on-demand
 * continuously. This call is always non-blocking. The transfer can only be
 * stopped with a call to axidma_stop_transfer. */
int axidma_video_transfer(axidma_dev_t dev, int display_channel, size_t width,
        size_t height, size_t depth, void **frame_buffers, int num_buffers)
{
    return 0;
}

/* This function stops all transfers on the given channel with the given
 * direction. This function is required to stop any video transfers, or any
 * non-blocking transfers. */
void axidma_stop_transfer(axidma_dev_t dev, int channel)
{
    return;
}

#ifdef __cplusplus
}
#endif
