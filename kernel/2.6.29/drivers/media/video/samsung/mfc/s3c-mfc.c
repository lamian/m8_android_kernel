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
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/page.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <asm/pgtable.h>

#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <asm/memory.h>

#include <linux/mutex.h>
#include <linux/wait.h>

#include <linux/version.h>
#include <mach/irqs.h>
#include <plat/regs-clock.h>
#include <plat/map-base.h>
#include <linux/pm.h>
#include <plat/pm.h>
#ifdef CONFIG_S3C6400_PDFW
#include <asm/arch/pd.h>
#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
#include <asm/arch/kdpmd.h>
#endif
#endif

#include "Mfc.h"
#include "MfcConfig.h"
#include "MFC_HW_Init.h"
#include "MFC_Instance.h"
#include "MFC_Inst_Pool.h"
#include "LogMsg.h"
#include "MfcMutex.h"
#include "s3c-mfc.h"
#include "FramBufMgr.h"
#include "MfcMemory.h"
#include "DataBuf.h"
#include "MfcSfr.h"
#include "MfcIntrNotification.h"
#include "MfcDrvParams.h"

#ifdef CONFIG_PLAT_S3C64XX
#include <plat/s3c64xx-dvfs.h>
#include <plat/power-clock-domain.h>

#ifdef CONFIG_S3C64XX_DOMAIN_GATING
#define USE_MFC_DOMAIN_GATING
#endif /* CONFIG_S3C64XX_DOMAIN_GATING */

static char banner[] __initdata = KERN_INFO "S3C6400 MFC Driver, (c) 2007 Samsung Electronics\n";

static struct clk	*mfc_hclk;
static struct clk	*mfc_sclk;
static struct clk	*mfc_pclk;

#define DOMAIN_POWER_ON do { \
	s3c_set_normal_cfg(S3C64XX_DOMAIN_V, S3C64XX_ACTIVE_MODE, S3C64XX_MFC); \
	if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_V)) { \
		return -1; \
	} \
} while (0)

#define DOMAIN_POWER_OFF do { \
	s3c_set_normal_cfg(S3C64XX_DOMAIN_V, S3C64XX_LP_MODE, S3C64XX_MFC); \
} while (0)

#define CLOCK_ENABLE do { \
	clk_enable(mfc_hclk); \
	clk_enable(mfc_sclk); \
	clk_enable(mfc_pclk); \
} while (0)

#define CLOCK_DISABLE do { \
	clk_disable(mfc_hclk); \
	clk_disable(mfc_sclk); \
	clk_disable(mfc_pclk); \
} while (0)

#else
#ifdef CONFIG_PLAT_S5P64XX
#include <plat/s5p6442-dvfs.h>
#include <plat/power_clk_gating.h>

static char banner[] __initdata = KERN_INFO "S5P6442 MFC Driver, (c) 2009 Samsung Electronics\n";

static struct clk	*mfc_hd1clk;

#ifdef S5P6442_POWER_GATING_MFC
#define USE_MFC_DOMAIN_GATING
#endif /* S5P6442_POWER_GATING_MFC */

#define DOMAIN_POWER_ON do { \
	s5p6442_pwrgate_config(S5P6442_MFC_ID, S5P6442_ACTIVE_MODE); \
} while (0)

#define DOMAIN_POWER_OFF do { \
	s5p6442_pwrgate_config(S5P6442_MFC_ID, S5P6442_LP_MODE); \
} while (0)

#define CLOCK_ENABLE do { \
	clk_enable(mfc_hd1clk); \
} while (0)

#define CLOCK_DISABLE do { \
	clk_disable(mfc_hd1clk); \
} while (0)

#else
#error Unsupported platform

#endif /* CONFIG_PLAT_S5P64XX */
#endif /* CONFIG_PLAT_S3C64XX */

static int _openhandle_count = 0;


#define SAVE_START_ADDR 0x100
#define SAVE_END_ADDR	0x200
//static struct sleep_save mfc_save[SAVE_END_ADDR - SAVE_START_ADDR];
static unsigned int mfc_save[SAVE_END_ADDR - SAVE_START_ADDR];
#define MFC_READ_REG( ADDR ) (unsigned int)*(volatile unsigned int *)(ADDR)
#define MFC_WRITE_REG( ADDR, DATA ) *(volatile unsigned int *)(ADDR) = DATA

#if CONFIG_PM
extern void s3c2410_pm_do_save(struct sleep_save *ptr, int count);
extern void s3c2410_pm_do_restore(struct sleep_save *ptr, int count);
#endif
extern int MFC_GetConfigParams(MFCInstCtx  *pMfcInst, MFC_ARGS   *args);
extern int MFC_SetConfigParams(MFCInstCtx  *pMfcInst, MFC_ARGS   *args);


typedef struct _MFC_HANDLE
{
    MFCInstCtx  *mfc_inst;

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
    unsigned char  *pMV;
    unsigned char  *pMBType;
#endif
} MFC_HANDLE;


#ifdef CONFIG_S3C6400_PDFW
int mfc_before_pdoff(void)
{
	printk("MFC context saving before pdoff\n");
	return 0;
}

int mfc_after_pdon(void)
{
	printk("MFC initialization after pdon\n");	
	return 0;
}

struct pm_devtype mfc_pmdev = {
	name: "mfc",
	state: DEV_IDLE,
	before_pdoff: mfc_before_pdoff,
	after_pdon: mfc_after_pdon,
};
#endif

/*
static unsigned long measureTime(struct timeval *start, struct timeval *stop)
{

        unsigned long sec, usec, time;

        sec = stop->tv_sec - start->tv_sec;
        if(stop->tv_usec >= start->tv_usec)
        {
                usec = stop->tv_usec - start->tv_usec;
        }
        else
        {
                usec = stop->tv_usec + 1000000 - start->tv_usec;
                sec--;
        }
        time = sec*1000000 + (usec);
        return time/1000;
}

static struct timeval start, stop;
static unsigned int		tmp_time;
*/

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_MFC);

static struct resource	*mfc_mem;
static void __iomem	*mfc_base;


static irqreturn_t s3c_mfc_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int	intReason;
	MFCInstCtx		*pMfcInst;

	pMfcInst = (MFCInstCtx *)dev_id;
	
	intReason	= MfcIntrReason();

	if (intReason & 0xC00E) {	// if PIC_RUN, buffer full and buffer empty interrupt
		SendInterruptNotification(intReason);
	}
	
	MfcClearIntr();

	return IRQ_HANDLED;
}

static int s3c_mfc_open(struct inode *inode, struct file *file)
{
	MFC_HANDLE		*handle;
	int			ret;

	//////////////////
	//  Mutex Lock	//
	//////////////////
	MFC_Mutex_Lock();

#ifdef USE_MFC_DOMAIN_GATING
	if(_openhandle_count == 0) {
		DOMAIN_POWER_ON;
	}
#endif /* USE_MFC_DOMAIN_GATING */

	_openhandle_count++;

#ifdef USE_MFC_DOMAIN_GATING
	CLOCK_ENABLE;
#endif /* USE_MFC_DOMAIN_GATING */

	if(_openhandle_count == 1) {
#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		kdpmd_set_event(mfc_pmdev.devid, KDPMD_DRVOPEN);
		kdpmd_wakeup();
		kdpmd_wait(mfc_pmdev.devid);
		mfc_pmdev.state = DEV_RUNNING;
		printk("mfc_open woke up\n");
#endif

		//////////////////////////////////////
		//	3. MFC Hardware Initialization	//
		//////////////////////////////////////
		if (MFC_HW_Init() == FALSE) 
		{
			ret = -ENODEV;	
			goto err_MFC_HW_Init;
		}
	}


	handle = (MFC_HANDLE *)kzalloc(sizeof(MFC_HANDLE), GFP_KERNEL);
	if(!handle) {
		LOG_MSG(LOG_ERROR, "s3c_mfc_open", "MFC open error\n");
		ret = -1;
		goto err_kzmalloc;
	}
	
	
	//////////////////////////////
	//	MFC Instance creation	//
	//////////////////////////////
	handle->mfc_inst = MFCInst_Create();
	if (handle->mfc_inst == NULL) {
		LOG_MSG(LOG_ERROR, "s3c_mfc_open", "MFC Instance allocation was failed!\r\n");
		ret = -1;
		goto err_MFCInst_Create;
	}

	/*
	 * MFC supports multi-instance. so each instance have own data structure
	 * It saves file->private_data
	 */
	file->private_data = (MFC_HANDLE *)handle;

#ifdef CONFIG_CPU_FREQ
	set_dvfs_level(0);
#endif /* CONFIG_CPU_FREQ */

	LOG_MSG(LOG_TRACE, "mfc_open", "MFC open success! \r\n");
	ret = 0;
	goto no_err;

err_MFCInst_Create:
	kfree (handle);
err_kzmalloc:
err_MFC_HW_Init:
#ifdef USE_MFC_DOMAIN_GATING
	CLOCK_DISABLE;
#endif /* USE_MFC_DOMAIN_GATING */
	_openhandle_count --;

#ifdef USE_MFC_DOMAIN_GATING
	if(_openhandle_count == 0) 
		DOMAIN_POWER_OFF;
#endif /* USE_MFC_DOMAIN_GATING */
no_err:
	MFC_Mutex_Release();
	return ret;
}


static int s3c_mfc_release(struct inode *inode, struct file *file)
{
	MFC_HANDLE		*handle = NULL;
	MFCINST_DEC_INBUF_TYPE	inbuf_type;
	int			ret;

	MFC_Mutex_Lock();

	handle = (MFC_HANDLE *)file->private_data;
	if (handle->mfc_inst == NULL) {
		ret = -1;
		goto err_handle;
	};
	
	//printk("Exit MFC Linux Driver\n");

	inbuf_type = handle->mfc_inst->inbuf_type;
	
	LOG_MSG(LOG_TRACE, "mfc_release", "delete inst no : %d\n", handle->mfc_inst->inst_no);

#if (MFC_LINE_RING_SHARE == 1)
	// In case of (MFC_LINE_RING_SHARE == 1), all the instances were reserved.
	// Therefore the instances need to be released.
	if (inbuf_type == DEC_INBUF_RING_BUF) {
		MfcInstPool_ReleaseAll();
	}
#endif

	MFCInst_Delete(handle->mfc_inst);

	kfree(handle);

#ifdef USE_MFC_DOMAIN_GATING
	CLOCK_DISABLE;
#endif /* USE_MFC_DOMAIN_GATING */

	_openhandle_count--;

	if(_openhandle_count == 0) {

#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		mfc_pmdev.state = DEV_IDLE;
		kdpmd_set_event(mfc_pmdev.devid, KDPMD_DRVCLOSE);
		kdpmd_wakeup();
		kdpmd_wait(mfc_pmdev.devid);
#endif
#ifdef CONFIG_CPU_FREQ
		set_dvfs_level(1);
#endif /* CONFIG_CPU_FREQ */

#ifdef USE_MFC_DOMAIN_GATING
		DOMAIN_POWER_OFF;
#endif /* USE_MFC_DOMAIN_GATING */
	}

	ret = 0;

err_handle:
	MFC_Mutex_Release();
	return ret;
}


static ssize_t s3c_mfc_write (struct file *file, const char *buf, size_t
		count, loff_t *pos)
{
	return 0;
}

static ssize_t s3c_mfc_read(struct file *file, char *buf, size_t count, loff_t *pos)
{	
	return 0;
}

static int s3c_mfc_ioctl(struct inode *inode, struct file *file, unsigned
		int cmd, unsigned long arg)
{
	int		ret	= 0;
	MFCInstCtx	*pMfcInst;
	MFC_HANDLE	*handle;
	unsigned char	*OutBuf	= NULL;
	MFC_CODECMODE	codec_mode	= 0;
	int		buf_size;
	unsigned int	tmp;
	MFC_ARGS	args;
	enc_info_t	enc_info;

// modified by RainAde for header type of mpeg4 (+ VOS/VO)
#if 1
	// Hdr0 : SPS or VOL
	// Hdr1 : PPS or VOS
	// Hdr2 : VO (VIS)
	int 	        nStrmLen, nHdrLen, nHdr0Len, nHdr1Len, nHdr2Len;
#else 
// modified by RainAde for composer interface
	int 	        nStrmLen, nHdrLen, nSps, nPps;
#endif

	void		*temp;
	unsigned int		vir_mv_addr;
	unsigned int		vir_mb_type_addr;

	
	//////////////////////
	//	Parameter Check	//
	//////////////////////
	handle = (MFC_HANDLE *)file->private_data;
	if (handle->mfc_inst == NULL) {
		return -EFAULT;
	}

	pMfcInst = handle->mfc_inst;


	switch(cmd)
	{
		case IOCTL_MFC_MPEG4_ENC_INIT:
		case IOCTL_MFC_H264_ENC_INIT:
		case IOCTL_MFC_H263_ENC_INIT:
			MFC_Mutex_Lock();
			LOG_MSG(LOG_TRACE, "mfc_ioctl", "cmd = %d\r\n", cmd);

			Copy_From_User(&args.enc_init, (MFC_ENC_INIT_ARG *)arg, sizeof(MFC_ENC_INIT_ARG));
			
			if ( cmd == IOCTL_MFC_MPEG4_ENC_INIT )
				codec_mode = MP4_ENC;
			else if ( cmd == IOCTL_MFC_H264_ENC_INIT )
				codec_mode = AVC_ENC;
			else if ( cmd == IOCTL_MFC_H263_ENC_INIT )
				codec_mode = H263_ENC;
			
			//////////////////////////////
			//	Initialize MFC Instance	//
			//////////////////////////////
			enc_info.width			= args.enc_init.in_width;
			enc_info.height			= args.enc_init.in_height;
			enc_info.bitrate		= args.enc_init.in_bitrate;
			enc_info.gopNum			= args.enc_init.in_gopNum;
			enc_info.frameRateRes	= args.enc_init.in_frameRateRes;
			enc_info.frameRateDiv	= args.enc_init.in_frameRateDiv;
			
			ret = MFCInst_Enc_Init(pMfcInst, codec_mode, &enc_info);
			
			args.enc_init.ret_code = ret;
			Copy_To_User((MFC_ENC_INIT_ARG *)arg, &args.enc_init, sizeof(MFC_ENC_INIT_ARG));
			
			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_MPEG4_ENC_EXE:
		case IOCTL_MFC_H264_ENC_EXE:
		case IOCTL_MFC_H263_ENC_EXE:
			MFC_Mutex_Lock();

			Copy_From_User(&args.enc_exe, (MFC_ENC_EXE_ARG *)arg, sizeof(MFC_ENC_EXE_ARG));

			tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
        
                        // from 2.8.5
			//dmac_clean_range(pMfcInst->pFramBuf, pMfcInst->pFramBuf + tmp);
			//outer_clean_range(__pa(pMfcInst->pFramBuf), __pa(pMfcInst->pFramBuf + tmp));
			// from 2.8.5 : cache flush
			cpu_cache.flush_kern_all();
			
			//////////////////////////
			//	Decode MFC Instance	//
			//////////////////////////

			// modified by RainAde for composer interface
			//ret = MFCInst_Encode(pMfcInst, &nStrmLen, &nHdrLen, &nHdr0Len, &nHdr1Len);
			// modified by RainAde for header type of mpeg4 (+ VOS/VO)
			ret = MFCInst_Encode(pMfcInst, &nStrmLen, &nHdrLen, &nHdr0Len, &nHdr1Len, &nHdr2Len);
			
                        // from 2.8.5
                        //dmac_clean_range(pMfcInst->pStrmBuf, pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);
			//outer_clean_range(__pa(pMfcInst->pStrmBuf), __pa(pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE));
			
			args.enc_exe.ret_code	= ret;
			if (ret == MFCINST_RET_OK) {
				args.enc_exe.out_encoded_size = nStrmLen;
				args.enc_exe.out_header_size  = nHdrLen;
// modified by RainAde for composer interface
				args.enc_exe.out_header0_size  = nHdr0Len;
				args.enc_exe.out_header1_size  = nHdr1Len;	
				// modified by RainAde for header type of mpeg4 (+ VOS/VO)
				args.enc_exe.out_header2_size  = nHdr2Len;					
			}
			Copy_To_User((MFC_ENC_EXE_ARG *)arg, &args.enc_exe, sizeof(MFC_ENC_EXE_ARG));
			
			// added by RainAde for cache coherency
			cpu_cache.dma_inv_range(pMfcInst->pStrmBuf, pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);
			
			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_MPEG4_DEC_INIT:
		case IOCTL_MFC_H264_DEC_INIT:
		case IOCTL_MFC_H263_DEC_INIT:
		case IOCTL_MFC_VC1_DEC_INIT:
			MFC_Mutex_Lock();

			Copy_From_User(&args.dec_init, (MFC_DEC_INIT_ARG *)arg, sizeof(MFC_DEC_INIT_ARG));
			
			// yj: fix the PiP problem
			cpu_cache.flush_kern_all();

			if ( cmd == IOCTL_MFC_MPEG4_DEC_INIT )
				codec_mode = MP4_DEC;
			else if ( cmd == IOCTL_MFC_H264_DEC_INIT )
				codec_mode = AVC_DEC;
			else if ( cmd == IOCTL_MFC_H263_DEC_INIT)
				codec_mode = H263_DEC;
			else {
				codec_mode = VC1_DEC;
			}
			
			//////////////////////////////
			//	Initialize MFC Instance	//
			//////////////////////////////
			ret = MFCInst_Init(pMfcInst, codec_mode, args.dec_init.in_strmSize);

			args.dec_init.ret_code	= ret;
			if (ret == MFCINST_RET_OK) {
				args.dec_init.out_width	     = pMfcInst->width;
				args.dec_init.out_height     = pMfcInst->height;
				args.dec_init.out_buf_width  = pMfcInst->buf_width;
				args.dec_init.out_buf_height = pMfcInst->buf_height;
			}
			Copy_To_User((MFC_DEC_INIT_ARG *)arg, &args.dec_init, sizeof(MFC_DEC_INIT_ARG));

			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_MPEG4_DEC_EXE:
		case IOCTL_MFC_H264_DEC_EXE:
		case IOCTL_MFC_H263_DEC_EXE:
		case IOCTL_MFC_VC1_DEC_EXE:
			MFC_Mutex_Lock();

			Copy_From_User(&args.dec_exe, (MFC_DEC_EXE_ARG *)arg, sizeof(MFC_DEC_EXE_ARG));

			// from 2.8.5
			//dmac_clean_range(pMfcInst->pStrmBuf, pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);
			//outer_clean_range(__pa(pMfcInst->pStrmBuf), __pa(pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE));
			// from 2.8.5 : cache flush
			cpu_cache.flush_kern_all();
			
			if (pMfcInst->inbuf_type == DEC_INBUF_LINE_BUF) {
				ret = MFCInst_Decode(pMfcInst, args.dec_exe.in_strmSize); 
			}
			else if (pMfcInst->inbuf_type == DEC_INBUF_RING_BUF) {
				ret = MFCInst_Decode_Stream(pMfcInst, args.dec_exe.in_strmSize);
			}
			else {
				LOG_MSG(LOG_ERROR, "s3c_mfc_ioctl", "Buffer type is not defined.\n");
				MFC_Mutex_Release();
				args.dec_exe.ret_code = -1;
				return -EINVAL;
			}

			args.dec_exe.ret_code = ret;
			Copy_To_User((MFC_DEC_EXE_ARG *)arg, &args.dec_exe, sizeof(MFC_DEC_EXE_ARG));

			// added by RainAde for cache coherency
			tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
			cpu_cache.dma_inv_range(pMfcInst->pFramBuf, pMfcInst->pFramBuf + tmp);

			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_GET_RING_BUF_ADDR:
			MFC_Mutex_Lock();

			Copy_From_User(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			ret = MFCInst_GetRingBuf(pMfcInst, &OutBuf, &buf_size); 
			
			args.get_buf_addr.out_buf_size	= buf_size;
			args.get_buf_addr.out_buf_addr	= args.get_buf_addr.in_usr_data + ((int)(OutBuf - GetDataBufVirAddr()));
			args.get_buf_addr.ret_code		= ret;
			Copy_To_User((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));
			
			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_GET_LINE_BUF_ADDR:
			MFC_Mutex_Lock();

			Copy_From_User(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			ret = MFCInst_GetLineBuf(pMfcInst, &OutBuf, &buf_size);
			
			args.get_buf_addr.out_buf_size	= buf_size;
			args.get_buf_addr.out_buf_addr	= args.get_buf_addr.in_usr_data + (OutBuf - GetDataBufVirAddr());
			args.get_buf_addr.ret_code		= ret;

			Copy_To_User((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_GET_DBK_BUF_ADDR:	// newly added by yj: returns to DBK_BUF' virtual address and its size.
			MFC_Mutex_Lock();

			Copy_From_User(&args.get_dbkbuf_addr, (MFC_GET_DBK_BUF_ARG *)arg, sizeof(MFC_GET_DBK_BUF_ARG));

			if (pMfcInst->isMp4DbkOn != 1) {
				LOG_MSG(LOG_ERROR, "s3c_mfc_ioctl", "MFC DBK_BUF is not internally allocated yet.\n");
				MFC_Mutex_Release();
				return -EFAULT;
			}

			args.get_dbkbuf_addr.out_buf_size	= (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
			args.get_dbkbuf_addr.out_buf_addr = args.get_dbkbuf_addr.in_usr_mapped_addr + 
											((unsigned int)GetDbkBufVirAddr() - (unsigned int)GetDataBufVirAddr());
			
			args.get_dbkbuf_addr.ret_code	= MFCINST_RET_OK;
			
			Copy_To_User((MFC_GET_DBK_BUF_ARG *)arg, &args.get_dbkbuf_addr, sizeof(MFC_GET_DBK_BUF_ARG));
			
			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_GET_FRAM_BUF_ADDR:
			MFC_Mutex_Lock();

			Copy_From_User(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			if (pMfcInst->pFramBuf == NULL) {
				LOG_MSG(LOG_ERROR, "s3c_mfc_ioctl", "MFC Frame buffer is not internally allocated yet.\n");
				MFC_Mutex_Release();
				return -EFAULT;
			}

			// FRAM_BUF address is calculated differently for Encoder and Decoder.
			switch (pMfcInst->codec_mode) {
			case MP4_DEC:
			case AVC_DEC:
			case VC1_DEC:
			case H263_DEC:
				// Decoder case
				args.get_buf_addr.out_buf_size	= (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;

				tmp	= (unsigned int)args.get_buf_addr.in_usr_data + ( ((unsigned int) pMfcInst->pFramBuf)	\
					+ (pMfcInst->idx) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );
#if (MFC_ROTATE_ENABLE == 1)
				if ( (pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->PostRotMode & 0x0010) ) {
					tmp	= (unsigned int)args.get_buf_addr.in_usr_data + ( ((unsigned int) pMfcInst->pFramBuf)	\
					+ (pMfcInst->frambufCnt) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );	
				}
#endif
				args.get_buf_addr.out_buf_addr = tmp;
				break;

			case MP4_ENC:
			case AVC_ENC:
			case H263_ENC:
				// Encoder case
				tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
				args.get_buf_addr.out_buf_addr = args.get_buf_addr.in_usr_data + (pMfcInst->idx * tmp) + (int)(pMfcInst->pFramBuf - GetDataBufVirAddr());
				break;
			}

			args.get_buf_addr.ret_code	= MFCINST_RET_OK;
			Copy_To_User((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			MFC_Mutex_Release();
			break;
			
		case IOCTL_MFC_GET_PHY_FRAM_BUF_ADDR:
			MFC_Mutex_Lock();

			Copy_From_User(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			args.get_buf_addr.out_buf_size	= (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
//			args.get_buf_addr.out_buf_size	= ((pMfcInst->width+2*DIVX_PADDING) * (pMfcInst->height+2*DIVX_PADDING) * 3) >> 1;
			tmp	= (unsigned int)S3C6400_BASEADDR_MFC_DATA_BUF + ( ((unsigned int) pMfcInst->pFramBuf)	\
				+ (pMfcInst->idx) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );

//.[ i: sichoi 081103 (ROTATE)
#if (MFC_ROTATE_ENABLE == 1)
            if ( (pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->PostRotMode & 0x0010) ) {
                tmp = (unsigned int)S3C6400_BASEADDR_MFC_DATA_BUF + ( ((unsigned int) pMfcInst->pFramBuf)   \
                + (pMfcInst->frambufCnt) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );  
            }
#endif
//.] sichoi 081103

			args.get_buf_addr.out_buf_addr = tmp;
			args.get_buf_addr.ret_code = MFCINST_RET_OK;

			Copy_To_User((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			MFC_Mutex_Release();
			break;

		case IOCTL_MFC_GET_MPEG4_ASP_PARAM:
		#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
			Copy_From_User(&args.mpeg4_asp_param, (MFC_GET_MPEG4ASP_ARG *)arg, sizeof(MFC_GET_MPEG4ASP_ARG));
		
			ret = MFCINST_RET_OK;
			args.mpeg4_asp_param.ret_code              = MFCINST_RET_OK;
			args.mpeg4_asp_param.mp4asp_vop_time_res   = pMfcInst->RET_DEC_SEQ_INIT_BAK_MP4ASP_VOP_TIME_RES;
			args.mpeg4_asp_param.byte_consumed         = pMfcInst->RET_DEC_PIC_RUN_BAK_BYTE_CONSUMED;
			args.mpeg4_asp_param.mp4asp_fcode          = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_FCODE;
			args.mpeg4_asp_param.mp4asp_time_base_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_TIME_BASE_LAST;
			args.mpeg4_asp_param.mp4asp_nonb_time_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_NONB_TIME_LAST;
			args.mpeg4_asp_param.mp4asp_trd            = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_MP4ASP_TRD;
			
   			args.mpeg4_asp_param.mv_addr      = (args.mpeg4_asp_param.in_usr_mapped_addr + MFC_STRM_BUF_SIZE) + (pMfcInst->mv_mbyte_addr - pMfcInst->phyadrFramBuf);
			args.mpeg4_asp_param.mb_type_addr = args.mpeg4_asp_param.mv_addr + 25920;	
			args.mpeg4_asp_param.mv_size      = 25920;
			args.mpeg4_asp_param.mb_type_size = 1620;
		
			vir_mv_addr = (unsigned int)((pMfcInst->pStrmBuf + MFC_STRM_BUF_SIZE) + (pMfcInst->mv_mbyte_addr - pMfcInst->phyadrFramBuf));
			vir_mb_type_addr = vir_mv_addr + 25920;

			Copy_To_User((MFC_GET_MPEG4ASP_ARG *)arg, &args.mpeg4_asp_param, sizeof(MFC_GET_MPEG4ASP_ARG));

                        // from 2.8.5
			//dmac_clean_range(vir_mv_addr, vir_mv_addr + args.mpeg4_asp_param.mv_size);
			//outer_clean_range(__pa(vir_mv_addr), __pa(vir_mv_addr + args.mpeg4_asp_param.mv_size));

			//dmac_clean_range(vir_mb_type_addr, vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size);
			//outer_clean_range(__pa(vir_mb_type_addr), __pa(vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size));
		#endif	
			break;
		case IOCTL_MFC_GET_CONFIG:
			MFC_Mutex_Lock();

			Copy_From_User(&args, (MFC_ARGS *)arg, sizeof(MFC_ARGS));

			ret = MFC_GetConfigParams(pMfcInst, &args);

			Copy_To_User((MFC_ARGS *)arg, &args, sizeof(MFC_ARGS));

			MFC_Mutex_Release();
			break;

		case IOCTL_MFC_SET_CONFIG:
			MFC_Mutex_Lock();

			Copy_From_User(&args, (MFC_ARGS *)arg, sizeof(MFC_ARGS));

			ret = MFC_SetConfigParams(pMfcInst, &args);

			Copy_To_User((MFC_ARGS *)arg, &args, sizeof(MFC_ARGS));
			
			MFC_Mutex_Release();
			break;

		case IOCTL_MFC_SET_H263_MULTIPLE_SLICE:
			MFC_Mutex_Lock();

			pMfcInst->multiple_slice = 1;

			MFC_Mutex_Release();
			break;

		case IOCTL_VIRT_TO_PHYS:
			//MFC_Mutex_Lock();

			temp = __virt_to_phys((void *)arg);
			return (int)temp;
			//MFC_Mutex_Release();
			break;
			
		default:
			MFC_Mutex_Lock();
			LOG_MSG(LOG_TRACE, "s3c_mfc_ioctl", "Requested ioctl command is not defined. (ioctl cmd=0x%X)\n", cmd);
			MFC_Mutex_Release();
			return -EINVAL;
	}

	switch(ret) {
	case MFCINST_RET_OK:
		return TRUE;
	default:
		return -1;
	}
	return -1;
}

int s3c_mfc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size	= vma->vm_end - vma->vm_start;
	unsigned long maxSize;
	unsigned long pageFrameNo;

	pageFrameNo = __phys_to_pfn(S3C6400_BASEADDR_MFC_DATA_BUF);

	maxSize = MFC_DATA_BUF_SIZE + PAGE_SIZE - (MFC_DATA_BUF_SIZE % PAGE_SIZE);

	if(size > maxSize) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED | VM_IO;

	//vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if( remap_pfn_range(vma, vma->vm_start, pageFrameNo, size,	\
				vma->vm_page_prot) ) {
		LOG_MSG(LOG_TRACE, "mfc_mmap", "remap error");
		return -EAGAIN;
	}

	return 0;
}


static struct file_operations s3c_mfc_fops = {
			owner:		THIS_MODULE,
			open:		s3c_mfc_open,
			release:	s3c_mfc_release,
			ioctl:		s3c_mfc_ioctl,
			read:		s3c_mfc_read,
			write:		s3c_mfc_write,
			mmap:		s3c_mfc_mmap,
};


static struct miscdevice s3c_mfc_miscdev = {
			minor:		252, 		
			name:		"s3c-mfc",
			fops:		&s3c_mfc_fops
};


static int s3c_mfc_probe(struct platform_device *pdev)
{
	struct resource *res;
	int 			size;
	int				ret;

#ifdef USE_MFC_DOMAIN_GATING
	DOMAIN_POWER_ON;
#endif /* USE_MFC_DOMAIN_GATING */

	// mfc clock enable
#ifdef CONFIG_PLAT_S3C64XX
	mfc_hclk = clk_get(&pdev->dev, "hclk_mfc");
	if (!mfc_hclk || IS_ERR(mfc_hclk)) {
#else
	mfc_hd1clk = clk_get(&pdev->dev, "mfc");
	if (!mfc_hd1clk || IS_ERR(mfc_hd1clk)) {
#endif /* CONFIG_PLAT_S3C64XX */
		printk(KERN_ERR "failed to get mfc clk source\n");
		return -ENOENT;
	}
#ifndef CONFIG_PLAT_S5P64XX
	mfc_sclk = clk_get(&pdev->dev, "sclk_mfc");
	if (!mfc_sclk || IS_ERR(mfc_sclk)) {
		printk(KERN_ERR "failed to get mfc clk source\n");
		return -ENOENT;
	}

	mfc_pclk = clk_get(&pdev->dev, "pclk_mfc");
	if (!mfc_pclk || IS_ERR(mfc_pclk)) {
		printk(KERN_ERR "failed to get mfc clk source\n");
		return -ENOENT;
	}
#endif /* CONFIG_PLAT_S5P64XX */

#ifdef USE_MFC_DOMAIN_GATING
	CLOCK_ENABLE;
#endif /* USE_MFC_DOMAIN_GATING */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk(KERN_INFO "failed to get memory region resouce\n");
		ret = -ENOENT;
		goto err_mem;
	}

	size = (res->end-res->start)+1;
	mfc_mem = request_mem_region(res->start, size, pdev->name);
	if (mfc_mem == NULL) {
		printk(KERN_INFO "failed to get memory region\n");
		ret = -ENOENT;
		goto err_mem;
	}

	mfc_base = ioremap(res->start, size);
	if (mfc_base == 0) {
		printk(KERN_INFO "failed to ioremap() region\n");
		ret = -EINVAL;
		goto err_ioremap;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk(KERN_INFO "failed to get irq resource\n");
		ret = -ENOENT;
		goto err_irq;
	}

	ret = request_irq(res->start, (irq_handler_t) s3c_mfc_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		printk(KERN_INFO "failed to install irq (%d)\n", ret);
		goto err_irq;
	}
	
	if (MFC_Mutex_Create() == FALSE)
	{
		ret = -ENOMEM;
		goto err_mutex;
	}

#ifndef CONFIG_PLAT_S5P64XX
	{
	unsigned 	mfc_clk_val;
	
	// mfc clock set 133 Mhz
	mfc_clk_val = readl(S3C_CLK_DIV0);
	mfc_clk_val |= (1<<28);
	__raw_writel(mfc_clk_val, S3C_CLK_DIV0);
	}
#endif /* CONFIG_PLAT_S5P64XX */

	//////////////////////////////////
	//	2. MFC Memory Setup			//
	//////////////////////////////////
	if (MFC_MemorySetup() == FALSE)
	{
		ret = -ENOMEM;
		goto err_MFC_memory_setup;
	}
	
	//////////////////////////////////////
	//	3. MFC Hardware Initialization	//
	//////////////////////////////////////
	if (MFC_HW_Init() == FALSE)
	{
		ret = -ENODEV;
		goto err_MFC_HW_Init;
	}

	ret = misc_register(&s3c_mfc_miscdev);
	if (ret < 0)
		goto err_misc_register;

	ret = 0;
	goto no_err;

err_misc_register:
err_MFC_HW_Init:
err_MFC_memory_setup:
	MFC_Mutex_Delete ();
err_mutex:
	free_irq (res->start, pdev);
err_irq:
	iounmap (mfc_base);
err_ioremap:
	release_resource(mfc_mem);
	kfree(mfc_mem);
	mfc_mem = NULL;
err_mem:
no_err:
#ifdef USE_MFC_DOMAIN_GATING
	CLOCK_DISABLE;
	DOMAIN_POWER_OFF;
#endif /* USE_MFC_DOMAIN_GATING */

	return ret;
}

static int s3c_mfc_remove(struct platform_device *pdev)
{
	misc_deregister(&s3c_mfc_miscdev);
	MFC_Mutex_Delete ();
	free_irq (IRQ_MFC, pdev);
	iounmap (mfc_base);
	if (mfc_mem != NULL) {
		release_resource(mfc_mem);
		kfree(mfc_mem);
		mfc_mem = NULL;
	}

#ifdef USE_MFC_DOMAIN_GATING
	while (_openhandle_count-- > 0)
	{
		CLOCK_DISABLE;
	}
	DOMAIN_POWER_OFF;
#endif /* USE_MFC_DOMAIN_GATING */

	return 0;
}

static int s3c_mfc_suspend(struct platform_device *dev, pm_message_t state)
{
	MFCInstCtx *mfcinst_ctx;
	int         inst_no;
	int			is_mfc_on = 0;
	int			i, index = 0;
	unsigned int	dwMfcBase;


	MFC_Mutex_Lock();

	is_mfc_on = 0;

	// 1. Power Off state
	// Invalidate all the MFC Instances
	for (inst_no = 0; inst_no < MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = MFCInst_GetCtx(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			// On Power Down, the MFC instance is invalidated.
			// Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will not be performed
			// until it is validated by entering Power up state transition
			MFCInst_PowerOffState(mfcinst_ctx);
			printk(KERN_INFO "MFC_Suspend %d-th instance is invalidated\n", inst_no);
		}
	}

	/* 2. Command MFC sleep and save MFC SFR */
	if (is_mfc_on) {
		dwMfcBase = (unsigned int)GetMfcSfrVirAddr();

		for(i=SAVE_START_ADDR; i <= SAVE_END_ADDR; i+=4)
		{
			mfc_save[index] = (unsigned int)MFC_READ_REG(dwMfcBase + i);
			index++;	
		}

		MFC_Sleep();
	}

#ifdef USE_MFC_DOMAIN_GATING
	{
		/* 3. Disable MFC clock */
		int tmp_openhandle_count = _openhandle_count;
		while (tmp_openhandle_count-- > 0)
		{
			CLOCK_DISABLE;
		}
	}
#endif /* USE_MFC_DOMAIN_GATING */
/*
	// 4. MFC Power Off(Domain V)
	mfc_pwr = readl(S3C_NORMAL_CFG);
	mfc_pwr &= ~(1<<9);
	printk("mfc_pwr : 0x%X\n", mfc_pwr);
	__raw_writel(mfc_pwr, S3C_NORMAL_CFG);
*/

	MFC_Mutex_Release();

	return 0;
}

static int s3c_mfc_resume(struct platform_device *pdev)
{
	unsigned int	dwMfcBase;
	int 			i, index = 0;
	MFCInstCtx 		*mfcinst_ctx;
	int         	inst_no;
	int				is_mfc_on = 0;
#ifndef CONFIG_PLAT_S5P64XX
	unsigned int	mfc_pwr, domain_v_ready;
	unsigned 	mfc_clk_val;
	unsigned int	mfc_clk_temp;
#endif /* CONFIG_PLAT_S5P64XX */
	int		tmp_openhandle_count;

	
	MFC_Mutex_Lock();

	tmp_openhandle_count = _openhandle_count;
#ifdef USE_MFC_DOMAIN_GATING
	{
		int tmp_openhandle_count = _openhandle_count;
		while (tmp_openhandle_count-- > 0)
		{
			CLOCK_ENABLE;
		}
	}
#endif /* USE_MFC_DOMAIN_GATING */

#ifndef CONFIG_PLAT_S5P64XX
	// mfc clock set 133 Mhz
	mfc_clk_val = readl(S3C_CLK_DIV0);
	mfc_clk_temp = (mfc_clk_val & (0xF << 28))>>28;

	if(mfc_clk_temp == 0)
	{ /* MFC clock shouldn't exceed 133MHZ */
		mfc_clk_val |= (1<<28);
	}
	else if(mfc_clk_temp >= 2)
	{ /*If MFC clock rate is less than 66MHZ, It is recommended to reset it to 133MHz */
		mfc_clk_val =  mfc_clk_val & (~(0xF << 28));
		mfc_clk_val |= (1<<28);	
	}
	__raw_writel(mfc_clk_val, S3C_CLK_DIV0);

	// 1. MFC Power On(Domain V)
	mfc_pwr = readl(S3C_NORMAL_CFG);
	mfc_pwr |= (1<<9);
	__raw_writel(mfc_pwr, S3C_NORMAL_CFG);


	// 2. Check MFC power on
	do {
		domain_v_ready = readl(S3C_BLK_PWR_STAT);
		printk("domain v ready : 0x%X\n", domain_v_ready);
		msleep(1);
	}while(!(domain_v_ready & (1<<1)));
#endif /* CONFIG_PLAT_S5P64XX */

	// 3. Firmware download
	MfcFirmwareIntoCodeDownReg();

	// 4. Power On state
	// Validate all the MFC Instances
	for (inst_no = 0; inst_no < MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = MFCInst_GetCtx(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			// When MFC Power On, the MFC instance is validated.
			// Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will be performed again
			MFCInst_PowerOnState(mfcinst_ctx);
			printk(KERN_INFO "MFC_Resume %d-th instance is validated\n", inst_no);
		}
	}


	if (is_mfc_on) {
		// 5. Restore MFC SFR
		dwMfcBase = (unsigned int)GetMfcSfrVirAddr();
		for( i=SAVE_START_ADDR; i<= SAVE_END_ADDR; i+=4 )
		{
			MFC_WRITE_REG( ( dwMfcBase + i ), mfc_save[index] );
			index++;
		}

		// 6. Command MFC wakeup
		MFC_Wakeup();
	}

#ifdef USE_MFC_DOMAIN_GATING
	if(_openhandle_count == 0) {
		DOMAIN_POWER_OFF;
	}
#endif /* USE_MFC_DOMAIN_GATING */

	MFC_Mutex_Release();
	return 0;
}

static struct platform_driver s3c_mfc_driver = {
	.probe		= s3c_mfc_probe,
	.remove		= s3c_mfc_remove,
	.shutdown	= NULL,
	.suspend	= s3c_mfc_suspend,
	.resume		= s3c_mfc_resume,
	.driver		= {
	.owner		= THIS_MODULE,
	.name		= "s3c-mfc",
	},
};

static int __init s3c_mfc_init(void)
{
	printk(banner);

#ifdef CONFIG_S3C6400_PDFW
	pd_register_dev(&mfc_pmdev, "domain_v");
	printk(KERN_INFO "pdfw: mfc devid = %d\n", mfc_pmdev.devid);
#endif

	if(platform_driver_register(&s3c_mfc_driver)!=0)
  	{
   		printk("platform device register Failed \n");
   		return -1;
  	}

	printk("S3C6400 MFC driver module init OK.\n");
	return 0;
}

static void __exit s3c_mfc_exit(void)
{
	MFC_Mutex_Delete();
	
#ifdef CONFIG_S3C6400_PDFW
	pd_unregister_dev(&mfc_pmdev);
#endif

	platform_driver_unregister(&s3c_mfc_driver);
	printk("S3C6400 MFC driver module exit\n");	
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");
