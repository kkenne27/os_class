/* Homework IOC command codes
 *
 */
 
 #ifndef _S_I_HOMEWORK_H
 #define _S_I_HOMEWORK_H
 
 #include <minix/ioctl.h>
 
 #define HIOCSLOT			_IOW('h', 1, u32_t)
 #define HIOCCLEARSLOT		_IOW('h', 2, u32_t)
 #define HIOCGETSLOT		_IOR('h', 3, u32_t)
 
 #endif /* _S_I_HOMEWORK_H */