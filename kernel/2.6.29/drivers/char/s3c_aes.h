#ifndef __S3C_AES_H__
#define __S3C_AES_H__

#define S3C_AES_NAME			"s3c-aes"
#define S3C_AES_MAJOR			248
#define S3C_AES_MINOR			0

#define S3C_AES_IOCTL_MAGIC		'A'
#define S3C_AES_ECB_ENCRYPT		_IO(S3C_AES_IOCTL_MAGIC, 10)
#define S3C_AES_ECB_DECRYPT		_IO(S3C_AES_IOCTL_MAGIC, 11)
#define S3C_AES_CBC_ENCRYPT		_IO(S3C_AES_IOCTL_MAGIC, 12)
#define S3C_AES_CBC_DECRYPT		_IO(S3C_AES_IOCTL_MAGIC, 13)
#define S3C_AES_CTR_ENCRYPT		_IO(S3C_AES_IOCTL_MAGIC, 14)
#define S3C_AES_CTR_DECRYPT		_IO(S3C_AES_IOCTL_MAGIC, 15)

#define	MODE_AES_ECB_ENCRYPT		((0x1<<4) | (0x0<<3))
#define	MODE_AES_ECB_DECRYPT		((0x1<<4) | (0x1<<3))

#define	MODE_AES_CBC_ENCRYPT		((0x2<<4) | (0x0<<3))
#define	MODE_AES_CBC_DECRYPT		((0x2<<4) | (0x1<<3))

#define	MODE_AES_CTR_ENCRYPT		((0x3<<4) | (0x0<<3))
#define	MODE_AES_CTR_DECRYPT		MODE_AES_CTR_ENCRYPT

#define MAX_FIFO_SIZE_WORD		32
#define MAX_FIFO_SIZE_BYTE		128

#define S3C_AES_BUF_SIZE		(360 * 1024)

typedef struct {
	int		mode;		// FIFO: 0 or DMA: 1
	u32		*userkey;	// AES user key length(16 or 32)
	int		userkey_len;
	u32		*param;		// AES parameter(IV or CTR)
	int		param_len;	// AES paramter length(16 or 32)
	unsigned int	idata_len;
	unsigned int	odata_len;
} s3c_aes_context_t;

typedef struct {
	int		sbuf_empty;
	char		*sbufp;
	dma_addr_t	sdma_addr;
	int		dbuf_full;
	char		*dbufp;
	dma_addr_t	ddma_addr;
} s3c_aes_dev_t;

extern void s3c_aes_copyfifo_8w(unsigned int uSrc, unsigned int uDst, unsigned int uCnt);
extern void s3c_aes_getfifo_8w(unsigned int uSrc, unsigned int uDst, unsigned int uCnt);







#endif /* __S3C_AES_H__ */
