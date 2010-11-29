#define DEBUG_S3C_MEM
#undef	DEBUG_S3C_MEM

#ifdef DEBUG_S3C_MEM
#define DEBUG(fmt,args...) 	printk(fmt, ##args)
#else
#define DEBUG(fmt,args...) 	do {} while(0)
#endif

#define MEM_IOCTL_MAGIC			'M'

#define S3C_MEM_ALLOC			_IOWR(MEM_IOCTL_MAGIC, 310, struct s3c_mem_alloc)
#define S3C_MEM_FREE			_IOWR(MEM_IOCTL_MAGIC, 311, struct s3c_mem_alloc)

#define S3C_MEM_SHARE_ALLOC		_IOWR(MEM_IOCTL_MAGIC, 314, struct s3c_mem_alloc)
#define S3C_MEM_SHARE_FREE		_IOWR(MEM_IOCTL_MAGIC, 315, struct s3c_mem_alloc)

#define S3C_MEM_CACHEABLE_ALLOC		_IOWR(MEM_IOCTL_MAGIC, 316, struct s3c_mem_alloc)
#define S3C_MEM_CACHEABLE_SHARE_ALLOC	_IOWR(MEM_IOCTL_MAGIC, 317, struct s3c_mem_alloc)

#define S3C_MEM_DMA_COPY		_IOWR(MEM_IOCTL_MAGIC, 318, struct s3c_mem_dma_param)
#define S3C_MEM_DMA_SET			_IOWR(MEM_IOCTL_MAGIC, 319, struct s3c_mem_dma_param)

#define MEM_ALLOC			1
#define MEM_ALLOC_SHARE			2
#define MEM_ALLOC_CACHEABLE		3
#define MEM_ALLOC_CACHEABLE_SHARE	4

#define S3C_MEM_MINOR  			13

static DEFINE_MUTEX(mem_alloc_lock);
static DEFINE_MUTEX(mem_free_lock);

static DEFINE_MUTEX(mem_share_alloc_lock);
static DEFINE_MUTEX(mem_share_free_lock);

static DEFINE_MUTEX(mem_cacheable_alloc_lock);
static DEFINE_MUTEX(mem_cacheable_share_alloc_lock);

struct s3c_mem_alloc {
	int		size;
	unsigned int 	vir_addr;
	unsigned int 	phy_addr;
};

struct s3c_mem_dma_param {
	int		size;
	unsigned int 	src_addr;
	unsigned int 	dst_addr;
	int		cfg;
};

