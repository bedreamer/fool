#!/bin/sh
FOOLROOT='.'

	rm fool.img
	if [ -e $FOOLROOT/arch-i386/BOOTSECTOR ];
		then
			script/mvp fool.img arch-i386/BOOTSECTOR /0
		else
			echo check $FOOLROOT/arch-i386/BOOTSECTOR FAILE!
			exit 0
	fi
mount fool.img -o loop .tmp
	if [ -e arch-i386/FOOLLDR ];
		then
			cp arch-i386/FOOLLDR .tmp/
			echo "copy arch-i386/FOOLLDR done!"
		else
			echo "copy arch-i386/FOOLLDR faile!"
			exit 0
	fi
	if [ -e KSYSTEM ];
		then
			cp KSYSTEM .tmp/
			echo "copy KSYSTEM done!"
		else
			echo "copy KSYSTEM faile!"
			exit 0
	fi
sleep 1
if [ -e KFOOL ];
		then
			cp KFOOL .tmp/
			echo "copy KFOOL done!"
		else
			echo "copy KFOOL faile!"
			exit 0
	fi
sleep 1
umount .tmp
echo 'done!'
