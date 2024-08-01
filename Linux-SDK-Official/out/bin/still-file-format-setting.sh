#!/bin/bash


## Still File Format Setting       ##

# In this file we are working with the Still File Format Setting or Property Code 0xD253, where we continuously get
# the current Still File Format Setting and offer the user to change the setting

# fifo
FIFO=fifo$$

handler_interrupt() {
	kill $(jobs -p) >&/dev/null
	exit
}
handler_exit() {
	rm -f $FIFO
	exit
}
trap handler_interrupt INT
trap handler_exit EXIT
mkfifo -m 644 $FIFO

# common function
get_event_code() {
	TMP=$(echo $2 | sed -e ':a; $!N; $!b a')
	echo $TMP | sed -e "s/^.*$1=\([^,.]\+\).*\$/\1/g"
}

get_device_property_value() {
	TMP=$(echo $2 | sed -e ':a; $!N; $!b a')
	echo $TMP | sed -e "s/^.*$1\([0-9A-F]\+\).*\$/\1/g"
}

## Open Session
: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment

# 1) raw / 2) raw + jpeg / 3) jpeg

bool=true

while $bool; do
	#retrieve the current Still format setting Setting
	./control get 0xD253 "$@" --of="$FIFO" &
	sleep 1
	out=$(cat "$FIFO")
	value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

	#print out the current still file format settings
	if [[ $value == "01" ]]; then
		echo " "
		echo " "
		echo "Still File Format: Raw"
		echo " "
		echo " "
	elif [[ $value == "02" ]]; then
		echo " "
		echo " "
		echo "Still File Format: Raw+JPEG"
		echo " "
		echo " "
	else
		echo " "
		echo " "
		echo "Still File Format: JPEG"
		echo " "
		echo " "
	fi

	echo ""
	echo ""
	echo "Please Choose one of the following options"
	echo "1) Raw"
	echo "2) Raw+JPEG"
	echo "3) JPEG"
	echo "0) Quit"
	read input

	case "$input" in
	"1")
		./control send --op=0x9205 --p1=0xD253 --size=2 --data=0x01
		;;
	"2")
		./control send --op=0x9205 --p1=0xD253 --size=2 --data=0x02
		;;
	"3")
		./control send --op=0x9205 --p1=0xD253 --size=2 --data=0x03
		;;
	"0")
		echo "Quitting"
		bool=false
		;;
	*) #default case
		echo "Invalid Option"
		;;
	esac

	#re retrieve the current Still format setting Setting
	./control get 0xD253 "$@" --of="$FIFO" &
	sleep 1
	out=$(cat "$FIFO")
	value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')
done

#./control close "$@"
