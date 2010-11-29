#ifndef __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__
#define __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__

#include "MfcTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


BOOL MfcBitProcBufMemMapping(void);
volatile unsigned char *GetBitProcBufVirAddr(void);
unsigned char *GetParamBufVirAddr(void);

void MfcFirmwareIntoCodeBuf(void);


#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__ */
