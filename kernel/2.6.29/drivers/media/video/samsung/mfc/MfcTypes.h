#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_TYPE_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32_WCE
#include <windows.h>

#else

#include <linux/types.h>

typedef enum {FALSE, TRUE} BOOL;

#endif


#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_TYPE_H__ */
