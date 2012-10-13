-----------------
fool

致歉
	很抱歉写出这样一个不伦不类的东西出来，我当初写这个内核也仅仅是为了学习X86CPU的一些特性，后来渐渐的有了自己实现一个内核
的想法，但因为自己的大意没有写出一个标准的配置文件来对整个工程进行配置，因此内核的编译,调试，运行可能有一些困难。

配置
	编译环境需要安装过gcc 4.6.3,ld 2.22,nasm 2.09.10,GNU Make 3.81
	调试使用VirtualBox功能，也可使用bochs进行调试，若使用bochs进行调试则需另外在工程目录中新创建一个100m的硬盘镜像.
	若使用VirtualBox进行调试则需要新建一个虚拟机并将虚拟机的路径在文件 debug 中进行修改.

如何编译
	1. 编译BOOTLOADER
	>  cd fool/arch-i386
	>  make
	2. 编译基本库文件
	>  cd fool/lib
	>  make
	3. 编译基本驱动模块
	>  cd fool/drivers
	>  make
	4. 编译内存管理模块
	>  cd fool/mm
	>  make
	5. 编译基本文件系统模块
	>  cd fool/fs/mfs
	>  make
	6. 编译内核文件
	>  cd fool/kernel
	>  make alldone
	7. 编译Init
	>  cd foo/app
	>  make

生成调试文件
	>  cd fool/
	>  ./mkimg

如何调试运行内核
	>  cd fool/
	>  ./debug 或 bochs