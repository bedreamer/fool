#!/bin/bash

make

for file in keyboard.o tty.o
	do
		if [ -e $PWD/$file ];
		then
			echo "check driver object file $PWD/$file OK..."
		else
			make $file
			if [ -e $PWD/$file ];
			then
				echo "check driver object file $PWD/$file OK..."
			else
				echo "check driver object file $PWD/$file FAILE..."
				exit 0
			fi
		fi
	done
