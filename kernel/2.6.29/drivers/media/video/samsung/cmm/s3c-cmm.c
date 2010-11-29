
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/memory.h>

#include <mach/irqs.h>
#include <mach/hardware.h>
#include <plat/regs-clock.h>
#include <plat/reserved_mem.h>

#include "s3c-cmm.h"
#include "CMMMisc.h"
#include "LogMsg.h"


#define CODEC_MEM_START				(CMM_RESERVED_MEM_START)

static int		instanceNo[MAX_INSTANCE_NUM];
ALLOC_MEM_T		*AllocMemHead;
ALLOC_MEM_T		*AllocMemTail;
FREE_MEM_T		*FreeMemHead;
FREE_MEM_T		*FreeMemTail;
unsigned char	*CachedVirAddr;
unsigned char	*NonCachedVirAddr;

static void InsertNodeToAllocList(ALLOC_MEM_T *node, int inst_no);
static void InsertNodeToFreeList(FREE_MEM_T *node,  int inst_no);
static void DeleteNodeFromAllocList(ALLOC_MEM_T *node, int inst_no);
static void DeleteNodeFromFreeList( FREE_MEM_T *node, int inst_no);
static void ReleaseAllocMem(ALLOC_MEM_T *node, CODEC_MEM_CTX *CodecMem);
static void MergeFragmentation(int inst_no);
static unsigned int GetMemArea(int allocSize, int inst_no, char cache_flag);
static ALLOC_MEM_T * GetCodecVirAddr(int inst_no, CODEC_MEM_ALLOC_ARG *in_param);
static int GetInstanceNo(void);
static void ReturnInstanceNo(int inst_no);
static void PrintList(void);

static int s3c_cmm_open(struct inode *inode, struct file *file)
{
	CODEC_MEM_CTX	*CodecMem;
	int			ret;
	int			inst_no;


	ret = LockCMMMutex();
	if (!ret){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_open", "DD::CMM Mutex Lock Fail\n");
		return -1;
	}

	// check the number of instance 
	if((inst_no = GetInstanceNo()) < 0){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_open", "Instance Number error-too many instance\r\n");
		UnlockCMMMutex();
		return -1;
	}
	
	CodecMem = (CODEC_MEM_CTX *)kmalloc(sizeof(CODEC_MEM_CTX), GFP_KERNEL);
	if(CodecMem == NULL){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_open", "CodecMem application failed\n");
		UnlockCMMMutex();
		return -1;
	}
	
	memset(CodecMem, 0x00, sizeof(CODEC_MEM_CTX));

	CodecMem->inst_no = inst_no;
	printk("\n*****************************\n[CMM_Open] instanceNo : %d\n*****************************\n", CodecMem->inst_no);
	PrintList();
	
	file->private_data = (CODEC_MEM_CTX	*)CodecMem;

	UnlockCMMMutex();


	return 0;
}


static int s3c_cmm_release(struct inode *inode, struct file *file)
{
	DWORD			ret;
	CODEC_MEM_CTX	*CodecMem;
	ALLOC_MEM_T *node, *tmp_node;


	ret = LockCMMMutex();
	if(!ret){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_release", "DD::CMM Mutex Lock Fail\r\n");
		return -1;
	}
	
	CodecMem = (CODEC_MEM_CTX *)file->private_data;
	CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_release", "[%d][CMM Close] \n", CodecMem->inst_no);

	if(!CodecMem){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_close", "CMM Invalid Input Handle\r\n");
		UnlockCMMMutex();
		return -1;
	}

	CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_close", "CodecMem->inst_no : %d\n", CodecMem->inst_no);

	// release u_addr and v_addr accoring to inst_no
	for(node = AllocMemHead; node != AllocMemTail; node = node->next){
		if(node->inst_no == CodecMem->inst_no){
			tmp_node = node;
			node = node->prev;
			ReleaseAllocMem(tmp_node, CodecMem);
		}
	}

	CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_release", "[%d]instance MergeFragmentation\n", CodecMem->inst_no);
	MergeFragmentation(CodecMem->inst_no);

	ReturnInstanceNo(CodecMem->inst_no);

	kfree(CodecMem);
	UnlockCMMMutex();


	return 0;
}


static ssize_t s3c_cmm_write (struct file *file, const char *buf, size_t
		count, loff_t *pos)
{
	return 0;
}

static ssize_t s3c_cmm_read(struct file *file, char *buf, size_t count, loff_t *pos)
{	
	return 0;
}

static int s3c_cmm_ioctl(struct inode *inode, struct file *file, unsigned
		int cmd, unsigned long arg)
{
	int                     ret;
	CODEC_MEM_CTX *         CodecMem;
	CODEC_MEM_ALLOC_ARG     codec_mem_alloc_arg;
	CODEC_CACHE_FLUSH_ARG   codec_cache_flush_arg;
	CODEC_GET_PHY_ADDR_ARG  codec_get_phy_addr_arg;
	int                     result = 0;
	void *                  start;
	void *                  end;
	ALLOC_MEM_T *           node;
	CODEC_MEM_FREE_ARG      codec_mem_free_arg;

	CodecMem = (CODEC_MEM_CTX *)file->private_data;
	if (!CodecMem) {
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "CMM Invalid Input Handle\n");
		return -1;
	}

	ret = LockCMMMutex();
	if(!ret){
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "DD::CMM Mutex Lock Fail\r\n");
		return -1;
	}

	switch (cmd) 
	{
		case IOCTL_CODEC_MEM_ALLOC:
			
			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_MEM_GET\n");

			copy_from_user(&codec_mem_alloc_arg, (CODEC_MEM_ALLOC_ARG *)arg, sizeof(CODEC_MEM_ALLOC_ARG));

			node = GetCodecVirAddr(CodecMem->inst_no, &codec_mem_alloc_arg);
			if(node == NULL){
				CMM_LOG_MSG(LOG_WARNING, "s3c_cmm_ioctl", "GetCodecVirAddr(%d)\r\n", CodecMem->inst_no);
				result = -1;
				break;
			}
			
			ret = copy_to_user((void *)arg, (void *)&codec_mem_alloc_arg, sizeof(CODEC_MEM_ALLOC_ARG));
			break;

		case IOCTL_CODEC_MEM_FREE:

			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_MEM_FREE\n");

			copy_from_user(&codec_mem_free_arg, (CODEC_MEM_FREE_ARG *)arg, sizeof(CODEC_MEM_FREE_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_mem_free_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_mem_free_arg.u_addr);
				result = -1;
				break;
			}

			ReleaseAllocMem(node, CodecMem);

			break;

		case IOCTL_CODEC_CACHE_FLUSH:
		
			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_CACHE_FLUSH\n");

			copy_from_user(&codec_cache_flush_arg, (CODEC_CACHE_FLUSH_ARG *)arg, sizeof(CODEC_CACHE_FLUSH_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_cache_flush_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_cache_flush_arg.u_addr);
				result = -1;
				break;
			}

			start = node->v_addr;
			end = start + codec_cache_flush_arg.size;
			dmac_clean_range(start, end);
			outer_clean_range(__pa(start), __pa(end));

			break;

		case IOCTL_CODEC_GET_PHY_ADDR:
			copy_from_user(&codec_get_phy_addr_arg, (CODEC_GET_PHY_ADDR_ARG *)arg, sizeof(CODEC_GET_PHY_ADDR_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_get_phy_addr_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_get_phy_addr_arg.u_addr);
				result = -1;
				break;
			}

			if(node->cacheFlag)
				codec_get_phy_addr_arg.p_addr = node->cached_p_addr;
			else
				codec_get_phy_addr_arg.p_addr = node->uncached_p_addr;
			
			copy_to_user((void *)arg, (void *)&codec_get_phy_addr_arg, sizeof(CODEC_GET_PHY_ADDR_ARG));

			break;
		case IOCTL_CODEC_MERGE_FRAGMENTATION:

			MergeFragmentation(CodecMem->inst_no);

			break;

		case IOCTL_CODEC_CACHE_INVALIDATE:
			
			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_CACHE_INVALIDATE\n");

			copy_from_user(&codec_cache_flush_arg, (CODEC_CACHE_FLUSH_ARG *)arg, sizeof(CODEC_CACHE_FLUSH_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_cache_flush_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_cache_flush_arg.u_addr);
				result = -1;
				break;
			}

			start = node->v_addr;
			end = start + codec_cache_flush_arg.size;
			dmac_flush_range(start, end);

			break;

		case IOCTL_CODEC_CACHE_CLEAN:

			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_CACHE_CLEAN\n");

			copy_from_user(&codec_cache_flush_arg, (CODEC_CACHE_FLUSH_ARG *)arg, sizeof(CODEC_CACHE_FLUSH_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_cache_flush_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_cache_flush_arg.u_addr);
				result = -1;
				break;
			}

			start = node->v_addr;
			end = start + codec_cache_flush_arg.size;
			dmac_clean_range(start, end);

			break;

		case IOCTL_CODEC_CACHE_CLEAN_INVALIDATE:

			CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_ioctl", "IOCTL_CODEC_CACHE_INVALIDATE\n");

			copy_from_user(&codec_cache_flush_arg, (CODEC_CACHE_FLUSH_ARG *)arg, sizeof(CODEC_CACHE_FLUSH_ARG));

			for(node = AllocMemHead; node != AllocMemTail; node = node->next) {
				if(node->u_addr == (unsigned char *)codec_cache_flush_arg.u_addr)
					break;
			}

			if(node  == AllocMemTail){
				CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "invalid virtual address(0x%x)\r\n", codec_cache_flush_arg.u_addr);
				result = -1;
				break;
			}

			start = node->v_addr;
			end = start + codec_cache_flush_arg.size;
			dmac_clean_range(start, end);
			dmac_flush_range(start, end);
			
			break;
			
		default : 
			CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_ioctl", "DD::CMM Invalid ioctl : 0x%X\r\n", cmd);
	}

	UnlockCMMMutex();

	if(result == 0)
		return TRUE;
	else
		return FALSE;
}


int s3c_cmm_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long offset	= vma->vm_pgoff << PAGE_SHIFT;
	unsigned long size;
	unsigned long maxSize;
	unsigned long pageFrameNo = 0;


	CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_mmap", "vma->vm_end - vma->vm_start = %d\n", offset);

	if(offset == 0) {
		pageFrameNo = __phys_to_pfn(CODEC_MEM_START);
		maxSize = CODEC_CACHED_MEM_SIZE + PAGE_SIZE - (CODEC_CACHED_MEM_SIZE % PAGE_SIZE);		
		vma->vm_flags |= VM_RESERVED | VM_IO;
		size = CODEC_CACHED_MEM_SIZE;
	}
	else if(offset == CODEC_CACHED_MEM_SIZE) {
		pageFrameNo = __phys_to_pfn(CODEC_MEM_START + CODEC_CACHED_MEM_SIZE);
		maxSize = CODEC_NON_CACHED_MEM_SIZE + PAGE_SIZE - (CODEC_NON_CACHED_MEM_SIZE % PAGE_SIZE);		
		vma->vm_flags |= VM_RESERVED | VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		size = CODEC_NON_CACHED_MEM_SIZE;
	}

	if( remap_pfn_range(vma, vma->vm_start, pageFrameNo, size,	\
				vma->vm_page_prot) ) {
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_mmap", "cmm remap error");
		return -EAGAIN;
	}

	return 0;
}


static struct file_operations cmm_fops = {
owner:		THIS_MODULE,
			open:		s3c_cmm_open,
			release:	s3c_cmm_release,
			ioctl:		s3c_cmm_ioctl,
			read:		s3c_cmm_read,
			write:		s3c_cmm_write,
			mmap:		s3c_cmm_mmap,
};



static struct miscdevice s3c_cmm_miscdev = {
minor:		250, 		
			name:		"s3c-cmm",
			fops:		&cmm_fops
};

static char banner[] __initdata = KERN_INFO "S3C CMM Driver, (c) 2008 Samsung Electronics\n";

static int __init s3c_cmm_init(void)
{
	HANDLE 		h_Mutex;
	int			ret;
	FREE_MEM_T	*node;
	ALLOC_MEM_T	*alloc_node;
	
	
	printk(banner);

	// Mutex initialization
	h_Mutex = CreateCMMmutex();
	if (h_Mutex == NULL) 
	{
		CMM_LOG_MSG(LOG_ERROR, "s3c_cmm_init", "DD::CMM Mutex Initialize error\r\n");
		return FALSE;
	}

	ret = LockCMMMutex();

	ret = misc_register(&s3c_cmm_miscdev);


	// First 4MB will use cacheable memory
	CachedVirAddr = (unsigned char *)ioremap_cached( (unsigned long)CODEC_MEM_START, 		\
					(int)CODEC_CACHED_MEM_SIZE );

	// Second 4MB will use non-cacheable memory
	NonCachedVirAddr = (unsigned char *)ioremap_nocache( (unsigned long)(CODEC_MEM_START + 	\
					CODEC_CACHED_MEM_SIZE), (int)CODEC_NON_CACHED_MEM_SIZE );

	// init alloc list, if(AllocMemHead == AllocMemTail) then, the list is NULL
	alloc_node = (ALLOC_MEM_T *)kmalloc(sizeof(ALLOC_MEM_T), GFP_KERNEL);
	memset(alloc_node, 0x00, sizeof(ALLOC_MEM_T));
	alloc_node->next = alloc_node;
	alloc_node->prev = alloc_node;
	AllocMemHead = alloc_node;
	AllocMemTail = AllocMemHead;

	// init free list, if(FreeMemHead == FreeMemTail) then, the list is NULL
	node = (FREE_MEM_T *)kmalloc(sizeof(FREE_MEM_T), GFP_KERNEL);
	memset(node, 0x00, sizeof(FREE_MEM_T));
	node->next = node;
	node->prev = node;
	FreeMemHead = node;
	FreeMemTail = FreeMemHead;

	node = (FREE_MEM_T *)kmalloc(sizeof(FREE_MEM_T), GFP_KERNEL);
	memset(node, 0x00, sizeof(FREE_MEM_T));
	node->startAddr = CODEC_MEM_START;
	node->cacheFlag = 1;
	node->size = CODEC_CACHED_MEM_SIZE;
	InsertNodeToFreeList(node, -1);

	node = (FREE_MEM_T *)kmalloc(sizeof(FREE_MEM_T), GFP_KERNEL);
	memset(node, 0x00, sizeof(FREE_MEM_T));
	node->startAddr = CODEC_MEM_START + CODEC_CACHED_MEM_SIZE;
	node->cacheFlag = 0;
	node->size = CODEC_NON_CACHED_MEM_SIZE;
	InsertNodeToFreeList(node, -1);

	UnlockCMMMutex();

	return 0;
}

static void __exit s3c_cmm_exit(void)
{
	
	CMM_LOG_MSG(LOG_TRACE, "s3c_cmm_exit", "CMM_Deinit\n");

	iounmap(CachedVirAddr);
	iounmap(NonCachedVirAddr);

	DeleteCMMMutex();

	misc_deregister(&s3c_cmm_miscdev);
	
	printk("S3C CMM driver module exit\n");
}


// insert node ahead of AllocMemHead
static void InsertNodeToAllocList(ALLOC_MEM_T *node, int inst_no)
{
	CMM_LOG_MSG(LOG_TRACE, "InsertNodeToAllocList", "[%d]instance (cached_p_addr : 0x%08x uncached_p_addr : 0x%08x size:%ld cacheflag : %d)\n",	\
		inst_no, node->cached_p_addr, node->uncached_p_addr, node->size, node->cacheFlag);
	node->next = AllocMemHead;
	node->prev = AllocMemHead->prev;
	AllocMemHead->prev->next = node;
	AllocMemHead->prev = node;
	AllocMemHead = node;
	CMM_LOG_MSG(LOG_TRACE, "InsertNodeToAllocList", "Finished InsertNodeToAllocList\n");
}

// insert node ahead of FreeMemHead
static void InsertNodeToFreeList(FREE_MEM_T *node,  int inst_no)
{
	CMM_LOG_MSG(LOG_TRACE, "InsertNodeToFreeList", "[%d]instance(startAddr : 0x%08x size:%ld  cached flag : %d)\n",	\
		inst_no, node->startAddr, node->size, node->cacheFlag);
	node->next = FreeMemHead;
	node->prev = FreeMemHead->prev;
	FreeMemHead->prev->next = node;
	FreeMemHead->prev = node;
	FreeMemHead = node;

	PrintList();
}

static void DeleteNodeFromAllocList(ALLOC_MEM_T *node, int inst_no)
{
	CMM_LOG_MSG(LOG_TRACE, "DeleteNodeFromAllocList", "[%d]instance (uncached_p_addr : 0x%08x cached_p_addr : 0x%08x size:%ld cacheflag : %d)\n",	\
		inst_no, node->uncached_p_addr, node->cached_p_addr, node->size, node->cacheFlag);
	
	if(node == AllocMemTail){
		CMM_LOG_MSG(LOG_TRACE, "DeleteNodeFromAllocList", "InValid node\n");
		return;
	}

	if(node == AllocMemHead)
		AllocMemHead = node->next;

	node->prev->next = node->next;
	node->next->prev = node->prev;

	kfree(node);

	PrintList();
}

static void DeleteNodeFromFreeList( FREE_MEM_T *node, int inst_no)
{
	CMM_LOG_MSG(LOG_TRACE, "DeleteNodeFromFreeList", "[%d]DeleteNodeFromFreeList(startAddr : 0x%08x size:%ld)\n", inst_no, node->startAddr, node->size);
	if(node == FreeMemTail){
		CMM_LOG_MSG(LOG_ERROR, "DeleteNodeFromFreeList", "InValid node\n");
		return;
	}

	if(node == FreeMemHead)
		FreeMemHead = node->next;

	node->prev->next = node->next;
	node->next->prev = node->prev;

	kfree(node);
}

// Releae cacheable memory
static void ReleaseAllocMem(ALLOC_MEM_T *node, CODEC_MEM_CTX *CodecMem)
{
	FREE_MEM_T *free_node;
	

	free_node = (FREE_MEM_T	*)kmalloc(sizeof(FREE_MEM_T), GFP_KERNEL);

	if(node->cacheFlag) {
		free_node->startAddr = node->cached_p_addr;
		free_node->cacheFlag = 1;
	}
	else {
		free_node->startAddr = node->uncached_p_addr;
		free_node->cacheFlag = 0;
	}
	
	free_node->size = node->size;
	InsertNodeToFreeList(free_node, CodecMem->inst_no);
	
	// Delete from AllocMem list
	DeleteNodeFromAllocList(node, CodecMem->inst_no);
}



// Remove Fragmentation in FreeMemList
static void MergeFragmentation(int inst_no)
{
	FREE_MEM_T *node1, *node2;

	node1 = FreeMemHead;

	while(node1 != FreeMemTail){
		node2 = FreeMemHead;
		while(node2 != FreeMemTail){
			if( (node1->startAddr + node1->size == node2->startAddr) && (node1->cacheFlag == node2->cacheFlag) ){
				node1->size += node2->size;
				CMM_LOG_MSG(LOG_TRACE, "MergeFragmentation", "find merge area !! ( node1->startAddr + node1->size == node2->startAddr)\n");
				DeleteNodeFromFreeList(node2, inst_no);
				break;
			}
			else if( (node1->startAddr == node2->startAddr + node2->size) && (node1->cacheFlag == node2->cacheFlag) ){
				CMM_LOG_MSG(LOG_TRACE, "MergeFragmentation", "find merge area !! ( node1->startAddr == node2->startAddr + node2->size)\n");
				node1->startAddr = node2->startAddr;
				node1->size += node2->size;
				DeleteNodeFromFreeList(node2, inst_no);
				break;
			}
			node2 = node2->next;
		}
		node1 = node1->next;
	}
}

static unsigned int GetMemArea(int allocSize, int inst_no, char cache_flag)
{
	FREE_MEM_T		*node, *match_node = NULL;
	unsigned int	allocAddr = 0;


	CMM_LOG_MSG(LOG_TRACE, "GetMemArea", "request Size : %ld\n", allocSize);
	
	if(FreeMemHead == FreeMemTail){
		CMM_LOG_MSG(LOG_ERROR, "GetMemArea", "all memory is gone\n");
		return(allocAddr);
	}

	// find best chunk of memory
	for(node = FreeMemHead; node != FreeMemTail; node = node->next)
	{
		if(match_node != NULL)
		{
			if(cache_flag)
			{
				if( (node->size >= allocSize) && (node->size < match_node->size) && (node->cacheFlag) )
					match_node = node;
			}
			else
			{
				if( (node->size >= allocSize) && (node->size < match_node->size) && (!node->cacheFlag) )
					match_node = node;
			}
		}
		else
		{
			if(cache_flag)
			{
				if( (node->size >= allocSize) && (node->cacheFlag) )
					match_node = node;
			}
			else
			{
				if( (node->size >= allocSize) && (!node->cacheFlag) )
					match_node = node;
			}
		}
	}

	if(match_node != NULL) {
		CMM_LOG_MSG(LOG_TRACE, "GetMemArea", "match : startAddr(0x%08x) size(%ld) cache flag(%d)\n", 	\
			match_node->startAddr, match_node->size, match_node->cacheFlag);
	}
	
	// rearange FreeMemArea
	if(match_node != NULL){
		allocAddr = match_node->startAddr;
		match_node->startAddr += allocSize;
		match_node->size -= allocSize;
		
		if(match_node->size == 0)          // delete match_node.
		 	DeleteNodeFromFreeList(match_node, inst_no);

		return(allocAddr);
	}
	else
	{
		CMM_LOG_MSG(LOG_ERROR, "GetMemArea", "there is no suitable chunk\n");
		return 0;
	}

	return(allocAddr);
}


static ALLOC_MEM_T * GetCodecVirAddr(int inst_no, CODEC_MEM_ALLOC_ARG *in_param)
{

	unsigned int			p_startAddr;
	ALLOC_MEM_T 			*p_allocMem;
	

	// if user request cachable area, allocate from reserved area
	// if user request uncachable area, allocate dynamically	
	p_startAddr = GetMemArea((int)in_param->buffSize, inst_no, in_param->cacheFlag);

	if(!p_startAddr){
		CMM_LOG_MSG(LOG_TRACE, "GetCodecVirAddr", "There is no more memory\n\r");
		in_param->out_addr = -1;
		return NULL;
	}

	p_allocMem = (ALLOC_MEM_T *)kmalloc(sizeof(ALLOC_MEM_T), GFP_KERNEL);
	memset(p_allocMem, 0x00, sizeof(ALLOC_MEM_T));
		

	if(in_param->cacheFlag) {
		p_allocMem->cached_p_addr = p_startAddr;
		p_allocMem->v_addr = CachedVirAddr + (p_allocMem->cached_p_addr - CODEC_MEM_START);
		p_allocMem->u_addr = (unsigned char *)(in_param->cached_mapped_addr + (p_allocMem->cached_p_addr - CODEC_MEM_START));
		
		if (p_allocMem->v_addr == NULL) {
			CMM_LOG_MSG(LOG_ERROR, "GetCodecVirAddr", "Mapping Failed [PA:0x%08x]\n\r", p_allocMem->cached_p_addr);
			return NULL;
		}
	}
	else {
		p_allocMem->uncached_p_addr = p_startAddr;
		p_allocMem->v_addr = NonCachedVirAddr + (p_allocMem->uncached_p_addr - CODEC_MEM_START - CODEC_CACHED_MEM_SIZE);
		p_allocMem->u_addr = (unsigned char *)(in_param->non_cached_mapped_addr + (p_allocMem->uncached_p_addr - CODEC_MEM_START - CODEC_CACHED_MEM_SIZE));
		
		if (p_allocMem->v_addr == NULL)
		{
			CMM_LOG_MSG(LOG_ERROR, "GetCodecVirAddr", "Mapping Failed [PA:0x%08x]\n\r", p_allocMem->uncached_p_addr);
			return NULL;
		}
	}

	in_param->out_addr = (unsigned int)p_allocMem->u_addr;
	CMM_LOG_MSG(LOG_TRACE, "GetCodecVirAddr", "u_addr : 0x%x v_addr : 0x%x cached_p_addr : 0x%x, uncached_p_addr : 0x%x\n", 	\
			p_allocMem->u_addr, p_allocMem->v_addr, p_allocMem->cached_p_addr, p_allocMem->uncached_p_addr);
		

	p_allocMem->size = (int)in_param->buffSize;
	p_allocMem->inst_no = inst_no;
	p_allocMem->cacheFlag = in_param->cacheFlag;
	
	InsertNodeToAllocList(p_allocMem, inst_no);

	return(p_allocMem);
}

static int GetInstanceNo(void)
{
	int	i;

	for(i = 0; i < MAX_INSTANCE_NUM; i++)
	{
		if(instanceNo[i] == FALSE){
			instanceNo[i] = TRUE;
			return i;
		}
	}
	
	if(i == MAX_INSTANCE_NUM)
		return -1;

	return i;
}


static void ReturnInstanceNo(int inst_no)
{
	instanceNo[inst_no] = FALSE;
}


static void PrintList()
{
	ALLOC_MEM_T		*node1;
	FREE_MEM_T		*node2;
	int 			count = 0;
	unsigned int	p_addr;

	for(node1 = AllocMemHead; node1 != AllocMemTail; node1 = node1->next){
		if(node1->cacheFlag)
			p_addr = node1->cached_p_addr;
		else
			p_addr = (unsigned int)node1->uncached_p_addr;
		
		CMM_LOG_MSG(LOG_TRACE, "PrintList", "[AllocList][%d] inst_no : %d p_addr : 0x%08x v_addr:0x%08x size:%ld cacheflag : %d\n", 
			count++, node1->inst_no,  p_addr, node1->v_addr, node1->size, node1->cacheFlag);

	}
				
	count = 0;
	for(node2 = FreeMemHead; node2 != FreeMemTail; node2 = node2->next){
			CMM_LOG_MSG(LOG_TRACE, "PrintList", "[FreeList][%d] startAddr : 0x%08x size:%ld\n", count++, node2->startAddr , node2->size);

	}
	
	
}



module_init(s3c_cmm_init);
module_exit(s3c_cmm_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");
