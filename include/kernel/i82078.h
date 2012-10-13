/*
 *	i82078.h
 *	bedreamer@163.com
 *	Monday, June 11, 2012 07:11:29 CST 
 *	Intel 82078 64 Pin CHMOS Single-Chip Floppy Disk Controller
 *	CHMOS:(Complementary High-density MOS) A chip with a large 
 *		amount of CMOS circuits(含有大量CMOS电路的芯片)
 */
#ifndef I82078_H_
#define I82078_H_

/*	寄存器端口范围
 *	primary: 0x3F0~0x3F7
 *	secondary: 0x370~0x37F
 *  端口的低三位被驱动到82078的引脚A[2:0]
 */
#define P_SRA	0x3F0	/*主设备状态寄存器A*/
#define S_SRA	0x370	/*从....*/
#define P_SRB	0x3F1	/*主设备状态寄存器B*/
#define S_SRB	0x371	/*从....*/
#define P_SOR	0x3F2	/*主设备数字输出寄存器*/
#define S_SOR	0x372	/*从....*/
#define P_TDR	0x3F3	/*主设备磁道驱动寄存器*/
#define S_TDR	0x373	/*从....*/
#define P_MSR	0x3F4	/*主设备主要状态寄存器*/
#define S_MSR	0x374	/*从....*/
#define P_MSR	0x3F4	/*主设备数据速率选择寄存器*/
#define S_MSR	0x374	/*从....*/
#define P_FIFO	0x3F5	/*主设备先入先出数据寄存器*/
#define S_FIFO	0x375	/*从....*/
#define P_RESERVE 0x3F6
#define S_RESERVE 0x376
#define P_DIR	0x3F7	/*主设备数字输入寄存器*/
#define S_DIR	0x377	/*从....*/
#define P_CCR	0x3F7	/*主设备配置控制寄存器*/
#define S_CCR	0x377	/*从....*/
#endif /*I82078_H_*/
