#!/bin/bash

# 用于生成文件备份

EXT='.tar.gz'
BASESTR=fool-$(date +%Y)$(date +%m)$(date +%d)

# 不存在当天的文件则创建之
if [ ! -e $BASESTR$EXT ]; then
	tar -czvf $BASESTR$EXT fool
	exit 0
fi

# 当天已经备份过，生成文件名后缀
for ((i=0;i<100;i++)) 
do
	str=$BASESTR'-'$i$EXT
	if [ ! -e $str ]; then
		tar -czvf $str fool
		exit 0
	fi
done

exit 1
