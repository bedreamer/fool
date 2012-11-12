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
struct sys_time
{
	_u8 t_sec,t_min,t_hour,t_weekday,t_day,t_month;
	_u16 t_year;
};

void getsystime(struct sys_time *);

/* 
 * time_t
  +--------+--------+--------+--------+
  + unused +  hour  +   min  +  sec   +
  +--------+--------+--------+--------+
  31       23       15       7        0
 */
#define MAKETIME(hour,min,sec) (((hour&0x000000FF)<<16)|((min&0x000000FF)<<8)|((sec&0x000000FF)))
/* 
 * date_t
  +--------+--------+--------+--------+
  +  year  + month  +   day  +weekday +
  +--------+--------+--------+--------+
  31       23       15       7        0
 */
#define MAKEDATE(year,month,day,weekday) (((year&0x000000FF)<<24)|((month&0x000000FF)<<16)|((day&0x000000FF)<<8)|((weekday&0x000000FF)))

time_t gettime(void);
date_t getdate(void);

#endif // _TIME_
