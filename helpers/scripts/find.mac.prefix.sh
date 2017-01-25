#!/usr/bin/env bash

SEARCH="Espressif"

if [ "x$1" != "x" ]
then
    SEARCH="$1"
fi

LINE=""
for i in `grep $SEARCH * | \
					grep hex | \
					awk -F'[:\ ]' '{print $2}' | \
					sed -e "s/-/:/g" | \
					sed -e "s/:0/:/g" | \
					sed -e "s/A/a/g" | \
					sed -e "s/B/b/g" | \
					sed -e "s/C/c/g" | \
					sed -e "s/D/d/g" | \
					sed -e "s/E/e/g" | \
					sed -e "s/F/f/g"`
do
	LINE=$LINE" -e \"$i\" "
done
arp -a | grep $LINE
