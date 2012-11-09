/*
 * 8042.h
 *	bedreamer@163com
 *	Thursday, May 31, 2012 08:09:54 CST 
 *	Intel 8042
 */
#ifndef _INTEL_8042_
#define _INTEL_8042_

#define _8042OUTPUTBUF	0x60
#define _8042INPUTBYF	0x60
#define _8042STATUSREG	0x64
#define _8042CTLREG		0x64
#define _8042CMDMOURSE	0xD4

/*Intel 8042命令*/
#define _8042CMD_SETMOURSE_1X1_				0xE6/*设置鼠标比例为1：1*/
#define _8042CMD_SETMOURSE_2X1_				0xE7/*设置鼠标比例为2：1*/
#define _8042CMD_SETMOURSE_RESOLUTION		0xE8/*设置鼠标分辨率*/
#define _8042CMD_GETMOURSE_INFO				0xE9/*获取鼠标信息*/
#define _8042CMD_WRITE_LED					0xDE/*写键盘LED*/
#define _8042CMD_DETECT_REFLECT				0xEE/*诊断回声*/
#define _8042CMD_SETSWITCHSCANCODE			0xF0/*设置／获取交替扫描码*/
#define _8042CMD_GETKEYBORADID				0xF2/*读键盘ID*/
#define _8042CMD_GETMOURSEID				0xF2/*读鼠标ID*/
#define _8042CMD_SETREPEATINPUT				0xF3/*设置重复键入信息*/
#define _8042CMD_SETMOUSESIMPLE				0xF3/*设置鼠标采样率*/
#define _8042CMD_KEYBOARDOPEN				0xF4/*键盘开放*/
#define _8042CMD_SETDEFAULTDISABLEKEY		0xF5/*设置默认和禁止键盘*/
#define _8042CMD_SETDEFAULTDISABLEMOUESE	0xF6/*设置默认和禁止鼠标*/
#define _8042CMD_SETALLREPEAT				0xF7/*将所有的键设为重复按键*/
#define _8042CMD_SETALLPRESSORRELEASE		0xF8/*设置所有按键为按键/释放*/
#define _8042CMD_SETALLPRESS				0xF9/*设置所有键为按键*/
#define _8042CMD_SETALLREPEATPRESS			0xFA/*将所有的键设为重复按键/按键*/
#define _8042CMD_SETONEREPEAT				0xFB/*将某个按键设为重复按键*/
#define _8042CMD_SETKEYTRASLATE				0xFC/*将某个键设为按键/翻译*/
#define _8042CMD_SETONLYPRESS				0xFD/*将某个键设为仅按键*/
#define _8042CMD_RESEND						0xFE/*重新发送*/
#define _8042CMD_RESTART					0xFF/*键盘重启*/
#define _8042CMD_MOURSERESTART				0xFF/*鼠标重启*/

void init8042(void);
_u8 _8042readscancode(void);
void _8042cmdmourse(void);
void _8042setresolution1x1(void);
void _8042setresolution2x1(void);
#define RESOLUTION_25	0
#define RESOLUtION_50	1
#define RESOLUTION_100	2
#define RESOLUTION_200	3
void _8042setresolution(_u8);
#define MOURSE_ENABLE		0x00100000
#define MOURSE_LBTNDOWN		0x00040000
#define MOURSE_RBTNDOWN		0x00010000
#define MOURSE_DPI			0x00000300
#define MOURSE_RESOLUTION	0x000000FF
#define mourseenable(_u32_) (MOURSE_ENABLE&_u32_)>>20
#define mourselbtndown(_u32_) (MOURSE_LBTNDOWN&_u32_)>>18
#define mourserbtndown(_u32_) (MOURSE_RBTNDOWN&_u32_)>>16
#define moursedpi(_u32_) (MOURSE_DPI&_u32_)>>8
#define mourseresolution(_u32_) (MOURSE_RESOLUTION&_u32_)
_u32 _8042getmourseinfo(void);
#define LEDCAPSLOCKON	0x04
#define LEDNUMLOCKON	0x02
#define LEDSCROLLOCKON	0x01
void _8042setled(_u8 led);
_u8 _8042detect(void);
void _8042keyboardenable(void);
void _8042mourseenable(void);

#endif /*_INTEL_8042_*/

















