# 执行make
# 
FOOLROOT=/root/fool
OBJDIR=$FOOLROOT/.obj
# 输出文件及对应的源文件目录
CRT_DIR=$FOOLROOT/lib			# libcrt.a
CORE_DIR=$FOOLROOT/kernel		# core.a
MM_DIR=$FOOLROOT/mm				# mm.a
FS_DIR=$FOOLROOT/fs				# fs.a
DRIVERS_DIR=$FOOLROOT/drivers	# drivers.a
# 输出文件及对应的源文件目录
LIB_CRT=$OBJDIR/libcrt.a
DLIB_CRT=$OBJDIR/libdcrt.a
LIB_CORE=$OBJDIR/libcore.a
DLIB_CORE=$OBJDIR/libdcore.a
LIB_MM=$OBJDIR/libmm.a
DLIB_MM=$OBJDIR/libdmm.a
LIB_FS=$OBJDIR/libfs.a
DLIB_FS=$OBJDIR/libdfs.a
LIB_DRIVERS=$OBJDIR/libdrivers.a
DLIB_DRIVERS=$OBJDIR/libddrivers.a

for path in $CRT_DIR $MM_DIR $FS_DIR $DRIVERS_DIR $CORE_DIR
	do
		cd $path
		make
	done


