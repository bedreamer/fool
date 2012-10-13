/*
 *	fool.h
 *	bedreamer@163.com
 *	Tuesday, May 29, 2012 10(void)26(void)25 CST 
 *	内核操作函数集=>fool.asm
 */
#ifndef _FOOL_
#define _FOOL_

/*kroute.asm*/
extern void divide_error(void);
extern void single_step_exception(void);
extern void nmi(void);
extern void breakpoint_exception(void);
extern void overflow(void);
extern void bounds_check(void);
extern void inval_opcode(void);
extern void copr_not_available(void);
extern void double_fault(void);
extern void copr_seg_overrun(void);
extern void inval_tss(void);
extern void segment_not_present(void);
extern void stack_exception(void);
extern void general_protection(void);
extern void page_fault(void);
extern void copr_error(void);
extern void clock_hwint00(void);
extern void keyboard_hwint01(void);
extern void cascade_hwint02(void);
extern void second_serial_hwint03(void);
extern void first_serial_hwint04(void);
extern void XT_winchester_hwint05(void);
extern void floppy_hwint06(void);
extern void printer_hwint07(void);
extern void realtime_clock_hwint08(void);
extern void irq_2_redirected_hwint09(void);
extern void irq_10_hwint10(void);
extern void irq_11_hwint11(void);
extern void PS_2_mourse_hwint12(void);
extern void FPU_exception_hwint13(void);
extern void IDE0_hwint14(void);
extern void IDE1_hwint15(void);
extern void asm_sys_call(void);
extern void asm_sys_call(void);
extern void asm_restart(void);

/*fool.asm*/
extern void asm_set_ds(_u16 d);
#define setds asm_set_ds
extern void asm_set_es(_u16 d);
#define setes asm_set_es
extern void asm_set_gs(_u16 d);
#define setfs asm_set_fs
extern void asm_set_fs(_u16 d);
#define setgs asm_set_gs
extern void asm_lidt(_u8 *idt_ptr);
#define lidt asm_lidt
extern void asm_ltr(_u8 *str);
#define ltr asm_ltr
extern void asm_sgdt(_u8 *gdt_ptr);
#define sgdt asm_sgdt
extern void asm_sti(void);
extern void sti(void);
extern void asm_cli(void);
extern void cli(void);
extern void asm_hlt(void);
#define hlt asm_hlt
extern  _u8 asm_inb(_u16 port);
#define inb asm_inb
extern _u16 asm_inw(_u16 port);
#define inw asm_inw
extern _u32 asm_ind(_u16 port);
#define ind asm_ind
extern void asm_insb(_u16 port,_u8 *ptr);
#define insb asm_insb
extern void asm_insw(_u16 port,_u16 *ptr);
#define insw asm_insw
extern void asm_insd(_u16 port,_u32 *ptr);
#define insd asm_insd
extern void asm_outb(_u16 port,_u8 data);
#define outb asm_outb
extern void asm_outw(_u16 port,_u16 data);
#define outw asm_outw
extern void asm_outd(_u16 port,_u32 data);
#define outd asm_outd
extern void asm_outsb(_u16 port,_u8 *pdata);
#define outsb asm_outsb
extern void asm_outsw(_u16 port,_u16 *pdata);
#define outsw asm_outsw
extern void asm_outsd(_u16 port,_u32 *pdata);
#define outsd asm_outsd
extern void asm_inputs(int portnum,_u32* desnation,int count);
#define inputs asm_inputs
extern void asm_outputs(int portnum,_u32* source,int count);
#define outputs asm_outputs
extern void asm_save_exp_irq(void);
extern void asm_save_hw_irq(void);
extern _u64 asm_rdtsc(void);
#define rdtsc asm_rdtsc
extern _u32 asm_cr3(_u32);
extern void asm_enable_paging(void);
extern _u8 asm_readcmos(_u8);

/*_start.asm*/
extern byte *kstacktop;
#endif /*_FOOL*/
