/*
 *	time.c
 *	dedreamer@163.com
 *	COMS time module
 */
#ifndef _TIME_
#define _TIME_

#define CMOS_TIME_CTL_PORT		0x70
#define CMOS_TIME_DATA_PORT		0x71

/*CMOS register index
 */
#define CMOS_REG_SEC			0x00
#define CMOS_REG_MIN			0x02
#define CMOS_REG_HOUR			0x04
#define CMOS_REG_WEEKDAY		0x06
#define CMOS_REG_DAY			0x07
#define CMOS_REG_MONTH			0x08
#define CMOS_REG_YEAR			0x09

/*system time struct.
 */
struct sys_time{
	_u8 t_sec,t_min,t_hour,t_weekday,t_day,t_month;
	_u16 t_year;
};

void getsystime(struct sys_time *);
unsigned short gettime(void);
unsigned short getdate(void);

#endif // _TIME_
