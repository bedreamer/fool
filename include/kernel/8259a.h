/*8259a.h*/
#ifndef _8259A__
#define _8259A__

/* 8259A interrupt controller ports. */
#define	INT_M_CTL		0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL		0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */
#define EOI				0x20	/* command of end of interrupt*/

#define	INT_VECTOR_IRQ0			0x20	/*start vector number of 8259A master contraller*/
#define	INT_VECTOR_IRQ8			0x28	/*start vector number of 8259A slave contraller*/

void init8259A(void);
#define eoi_m() outb(INT_M_CTL,EOI)
#define eoi_s() outb(INT_S_CTL,EOI)
void _8259Asti(_u8 vector);
void _8259Acli(_u8 vector);

#endif /*_8259A__*/
