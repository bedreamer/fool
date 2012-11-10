/*
 *	8259a.c
 *	bedreamer@163.com
 *	Thursday, May 31, 2012 07:10:55 CST 
 */
#include <kernel/kernel.h>
#include <kernel/int.h>

/*
 * Initialize the 8259a contraller
 */
void init8259A(void)
{
//	printk("initialize the Intel 8259A contraller...");
	outb(INT_M_CTL,		0x11);				// Master 8259, ICW1.
	outb(INT_S_CTL,		0x11);				// Slave  8259, ICW1.
	outb(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	// Master 8259, ICW2.
	outb(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	// Slave  8259, ICW2.
	outb(INT_M_CTLMASK,	0x4);				// Master 8259, ICW3. 
	outb(INT_S_CTLMASK,	0x2);				// Slave  8259, ICW3. 
	outb(INT_M_CTLMASK,	0x1);				// Master 8259, ICW4.
	outb(INT_S_CTLMASK,	0x1);				// Slave  8259, ICW4.

	outb(INT_M_CTLMASK,	0x01);				// Master 8259, OCW1. 
	outb(INT_S_CTLMASK,	0x00);				// Slave  8259, OCW1. 
	return;
}

void _8259Asti(_u8 vector)
{
	if ( vector < INT_VECTOR_IRQ8 ){
		outb(INT_M_CTLMASK, inb(INT_M_CTLMASK) & ~(1 << vector));
	} else {
		outb(INT_S_CTLMASK, inb(INT_M_CTLMASK) & ~(1 << (vector-8)));
	}
}

void _8259Acli(_u8 vector)
{
	if ( vector < INT_VECTOR_IRQ8 ) {
		outb(INT_M_CTLMASK, inb(INT_M_CTLMASK) | (1 << vector));
	} else {
		outb(INT_S_CTLMASK, inb(INT_M_CTLMASK) | (1 << (vector-8)));
	}
}

