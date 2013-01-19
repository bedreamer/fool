# !/bin/bash
# Run this script will do a system config.
# such as, you can config MMU as build-in(recomand) or moudle,
# and you can config without some filesystem module also.
#
# this script read default config file. defconfig and do some 
# modify and output an useable config file .config in current
# workspace directory.
#
# format of .defconfig
# every config item has 3 lines as follow:
# eg. :
# line 1. -> MM-CONFIG
# line 2. -> [TAB]comment=Config MMU as build-in?(Y/m/n)
# line 3. -> [TAB]default=y
# line 1 means the config name of item, line 2 means config 
# information of this item, line 3 means the defaule value of 
# configed item.
LINE1=
LINE2=
LINE3=
i=1
OUTPUTFILE=".config"

# doread(name,default,comment,mode)
function __doconfig(){
	printf "$3"
	default=`echo $2 | sed 's/.\+default=//'`
	case $default in
	[yY]) printf "(Y/m/n):";;
	[mM]) printf "(y/M/n):";;
	[nN]) printf "(y/m/N):";;
	*   ) exit 1;;
	esac

# mode != 0 use manual config mode.
	case $4 in
	1)
		read ch
		case $ch in
		[yY]) default=y ;;
		[mM]) default=m ;;
		[nN]) default=n ;;
		esac
	;;
	0)
		printf "$default\n"
	;;
	esac
	echo "$1=$default" >> .config
}

# this value used to contral read a whole line
IFS="
"
function printahead(){
printf "************************************\n"
printf "*  Config program for fool kernel  *\n"
printf "*  Copyright (C) 2012, 2013 Lijie  *\n"
printf "*  contact me: cuplision@163.com   *\n"
printf "*                                  *\n"
printf "*  m/M: module     y/Y: build-in   *\n"
printf "*  n/N: don't use  other: default  *\n"
printf "************************************\n"
}
if [ -e .config ] ; then
	rm -f .config
fi

# doconfig(mode)
function doconfig(){
	for line in `sed 's/^.\+comment=//' .defconfig` ; do
		case $i in
		1) LINE1="$line"; ;;
		2) LINE2="$line"; ;;
		3) LINE3="$line";
			__doconfig $LINE1 $LINE2 $LINE3 $1
		   i=0; ;;
		esac
		i=`expr $i + 1`;
	done
}

case $1 in
	"--default") printahead;doconfig 0;;
	"--manual" ) printahead;doconfig 1;;
	* ) printf "Wrong param $1\n!!!";;
esac
printf "\nSave config into `pwd`/.config done.\n\n"


