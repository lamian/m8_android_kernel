#ifndef _S3C_PP_H_
#define _S3C_PP_H_

typedef enum {
	DMA_ONESHOT,
	FIFO_FREERUN
} s3c_pp_out_path_t;

typedef enum {
	PAL1, PAL2, PAL4, PAL8,
	RGB8, ARGB8, RGB16, ARGB16, RGB18, RGB24, RGB30, ARGB24,
	YC420, YC422, // Non-interleave
	CRYCBY, CBYCRY, YCRYCB, YCBYCR, YUV444 // Interleave
} s3c_color_space_t;

typedef enum {
	INTERLACE_MODE,
	PROGRESSIVE_MODE
} s3c_pp_scan_mode_t;

// Structure type for IOCTL commands S3C_PP_SET_PARAMS, S3C_PP_SET_INPUT_BUF_START_ADDR_PHY, 
//      S3C_PP_SET_INPUT_BUF_NEXT_START_ADDR_PHY, S3C_PP_SET_OUTPUT_BUF_START_ADDR_PHY.
typedef struct {
	unsigned int src_full_width;              // Source Image Full Width  (Virtual screen size)
	unsigned int src_full_height;             // Source Image Full Height (Virtual screen size)
	unsigned int src_start_x;                 // Source Image Start width  offset
	unsigned int src_start_y;                 // Source Image Start height offset
	unsigned int src_width;                   // Source Image Width
	unsigned int src_height;                  // Source Image Height
	unsigned int src_buf_addr_phy_rgb_y;      // Base Address of the Source Image (RGB or Y): Physical Address
	unsigned int src_buf_addr_phy_cb;         // Base Address of the Source Image (CB Component) : Physical Address
	unsigned int src_buf_addr_phy_cr;         // Base Address of the Source Image (CR Component) : Physical Address
	unsigned int src_next_buf_addr_phy_rgb_y; // Base Address of Source Image (RGB or Y) to be displayed next time in FIFO_FREERUN Mode
	unsigned int src_next_buf_addr_phy_cb;    // Base Address of Source Image (CB Component) to be displayed next time in FIFO_FREERUN Mode
	unsigned int src_next_buf_addr_phy_cr;    // Base Address of Source Image (CR Component) to be displayed next time in FIFO_FREERUN Mode
	s3c_color_space_t src_color_space;        // Color Space of the Source Image

	unsigned int dst_full_width;              // Destination Image Full Width (Virtual screen size)
	unsigned int dst_full_height;             // Destination Image Full Height (Virtual screen size)
	unsigned int dst_start_x;                 // Destination Image Start width offset
	unsigned int dst_start_y;                 // Destination Image Start height offset
	unsigned int dst_width;                   // Destination Image Width
	unsigned int dst_height;                  // Destination Image Height
	unsigned int dst_buf_addr_phy_rgb_y;      // Base Address of the Destination Image (RGB or Y) : Physical Address
	unsigned int dst_buf_addr_phy_cb;         // Base Address of the Destination Image (CB Component) : Physical Address
	unsigned int dst_buf_addr_phy_cr;         // Base Address of the Destination Image (CR Component) : Physical Address
	s3c_color_space_t dst_color_space;        // Color Space of the Destination Image

	s3c_pp_out_path_t out_path;               // output and run mode (DMA_ONESHOT or FIFO_FREERUN)
	s3c_pp_scan_mode_t scan_mode;             // INTERLACE_MODE, PROGRESSIVE_MODE
} s3c_pp_params_t;

// Structure type for IOCTL commands S3C_PP_ALLOC_KMEM, S3C_PP_FREE_KMEM.
typedef struct {
	int             size;
	unsigned int    vir_addr;
	unsigned int    phy_addr;
} s3c_pp_mem_alloc_t;

#define PP_IOCTL_MAGIC 'P'

#define S3C_PP_SET_PARAMS                 _IO(PP_IOCTL_MAGIC, 0)
#define S3C_PP_START                      _IO(PP_IOCTL_MAGIC, 1)
#define S3C_PP_GET_SRC_BUF_SIZE           _IO(PP_IOCTL_MAGIC, 2)
#define S3C_PP_SET_SRC_BUF_ADDR_PHY       _IO(PP_IOCTL_MAGIC, 3)
#define S3C_PP_SET_SRC_BUF_NEXT_ADDR_PHY  _IO(PP_IOCTL_MAGIC, 4)
#define S3C_PP_GET_DST_BUF_SIZE           _IO(PP_IOCTL_MAGIC, 5)
#define S3C_PP_SET_DST_BUF_ADDR_PHY       _IO(PP_IOCTL_MAGIC, 6)
#define S3C_PP_ALLOC_KMEM                 _IO(PP_IOCTL_MAGIC, 7)
#define S3C_PP_FREE_KMEM                  _IO(PP_IOCTL_MAGIC, 8)
#define S3C_PP_GET_RESERVED_MEM_SIZE      _IO(PP_IOCTL_MAGIC, 9)
#define S3C_PP_GET_RESERVED_MEM_ADDR_PHY  _IO(PP_IOCTL_MAGIC, 10)

#endif // _S3C_PP_H_

