#ifndef __CMM_DRIVER_H__
#define __CMM_DRIVER_H__


#define MAX_INSTANCE_NUM			10

#define CODEC_CACHED_MEM_SIZE		(4*1024*1024)
#define CODEC_NON_CACHED_MEM_SIZE	(4*1024*1024) 
#define CODEC_MEM_SIZE				(8*1024*1024)

#define IOCTL_CODEC_MEM_ALLOC				0x0000001A	
#define IOCTL_CODEC_MEM_FREE				0x0000001B
#define IOCTL_CODEC_CACHE_FLUSH				0x0000001C
#define IOCTL_CODEC_GET_PHY_ADDR			0x0000001D
#define IOCTL_CODEC_MERGE_FRAGMENTATION		0x0000001E
#define IOCTL_CODEC_CACHE_INVALIDATE		0x0000001F
#define IOCTL_CODEC_CACHE_CLEAN				0x00000020
#define IOCTL_CODEC_CACHE_CLEAN_INVALIDATE	0x00000021


typedef struct tagCMM_ALLOC_PRAM_T{
	char					cacheFlag;
	int			size;       // memory size
}CMM_ALLOC_PRAM_T;

typedef struct tagCODEC_MEM_CTX
{
	int			inst_no;
	int			callerProcess;
}CODEC_MEM_CTX;

typedef struct tagALLOC_MEM_T{
	struct tagALLOC_MEM_T	*prev;
	struct tagALLOC_MEM_T	*next;
	union{
		unsigned int	cached_p_addr;  // physical address of cacheable area
		unsigned int	uncached_p_addr;  // physical address of non-cacheable area
	};
	unsigned char			*v_addr;  // virtual address in cached area
	unsigned char			*u_addr;  // copyed virtual address for user mode process
	int						size;       // memory size	
	int						inst_no;
	char					cacheFlag;
} ALLOC_MEM_T;
	
typedef struct tagFREE_MEM_T
{
	struct tagFREE_MEM_T	*prev;
	struct tagFREE_MEM_T	*next;
	unsigned int			startAddr;
	unsigned int			size;
	char					cacheFlag;
}FREE_MEM_T;

// ioctl arguments
typedef struct tagCODEC_MEM_ALLOC_ARG
{
	char			cacheFlag;
	int				buffSize;
	unsigned int	cached_mapped_addr;
	unsigned int	non_cached_mapped_addr;
	unsigned int	out_addr;
}CODEC_MEM_ALLOC_ARG;

typedef struct tagCODEC_MEM_FREE_ARG
{
	unsigned int	u_addr;
}CODEC_MEM_FREE_ARG;

typedef struct tagCODEC_CACHE_FLUSH_ARG
{
	unsigned int	u_addr;
	int				size;
}CODEC_CACHE_FLUSH_ARG;

typedef struct tagCODEC_GET_PHY_ADDR_ARG
{
	unsigned int 	u_addr;
	unsigned int	p_addr;
}CODEC_GET_PHY_ADDR_ARG;


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#endif /*__CMM_DRIVER_H__*/
