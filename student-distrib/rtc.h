#include "types.h"
/*initialize the rtc*/
void rtc_init();

/*handle the rtc interrupt*/
void rtc_interrupt_handler();

/* rtc operations */
int32_t rtc_open(const uint8_t* filename, int32_t fd);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* importants variables */
char rate;
//volatile int read_wait;


