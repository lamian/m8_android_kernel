#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__


#ifdef __cplusplus
extern "C" {
#endif


int  SendInterruptNotification(int intr_type);
int  WaitInterruptNotification(void);


#ifdef __cplusplus
}
#endif


#define WAIT_INT_NOTI_TIMEOUT		(-99)


#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__ */
