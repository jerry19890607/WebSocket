#!/bin/sh
#please install imagemagick first

clear

gcc websocketServer.c -o websocketServer

if [ "$?" != "0" ]
then
	echo "Compiler server error"
	exit -1
fi

./websocketServer &

while true
do
	sleep 0.1
	import -windows root $(pwd)/websocketPic.jpg
done

