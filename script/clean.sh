FOOLROOT=/root/fool
# 输出文件及对应的源文件目录
CRT_DIR=$FOOLROOT/lib			# libcrt.a
CORE_DIR=$FOOLROOT/kernel		# core.a
MM_DIR=$FOOLROOT/mm				# mm.a
FS_DIR=$FOOLROOT/fs				# fs.a
DRIVERS_DIR=$FOOLROOT/drivers	# drivers.a

for path in $CORE_DIR $MM_DIR $FS_DIR $DRIVERS_DIR $CRT_DIR
	do
		cd $path
		make clean
	done

