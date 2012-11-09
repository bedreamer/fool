/*
 *	keyboard.c
 *	bedreamer@163.com
 *	Thursday, May 31, 2012 07:49:22 CST 
 */
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/fool.h>
#include <kernel/int.h>
#include <kernel/kmodel.h>
#include <drivers/keyboard.h>
#include <kernel/8259a.h>
#include <kernel/8042.h>

char *keyboardname="keyboard";
_u32 keystatus=0x00000000;

void initializekeyboard(void)
{
	if (0xEE==_8042detect())
		if (hwregistehandle(KEYBOARD_HWINT1,keyboardname,keyboardhandle))
			_8259Asti(KEYBOARD_HWINT1);
}

/*keyboard interrupt procedure.*/
void keyboardhandle(void)
{
	_u8 ch;
	_u32 dd;

	if ((ch=_8042readscancode())<0x80)
	{
		dd = keymap[ch*3];
		switch(dd)
		{
		case CTRL_L:
			LCTL_HOLD;
			goto end;
		case SHIFT_L:
			LSHIFT_HOLD;
			goto end;
		case SHIFT_R:
			RSHIFT_HOLD;
			goto end;
		case ALT_L:
			LALT_HOLD;
			goto end;
		case ALT_R:
			RALT_HOLD;
			goto end;
		case F1:
			//tty_switch(1);
			break;
		case F2:
			//tty_switch(2);
			break;
		case F3:
			//tty_switch(3);
			break;
		case F4:
			//tty_switch(4);
			break;
		default:
			ch=dd;
			break;
		}
		//tty_getkeyinput(keystatus|dd);
	}
end:
	eoi_m();
}
