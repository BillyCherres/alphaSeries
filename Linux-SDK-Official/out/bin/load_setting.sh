#! /bin/bash


## Load Setting           ##

## This setting loads content from setting.dat which was created by save_setting.sh. This program also checks if there is a setting.dat file in the directory, if not, then the program will prompt the user to run save_setting.sh first before closing the session##

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

sleep 3

## operation
: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment

# If 0xD271 is 0x01, please proceed.
# If 0xD271 is 0x00, it is not supported.
# If setting.dat file is not in the folder, please ask user to run the save_setting.sh first

fileName="setting.dat"

#get the Camera Save setting
./control get 0xD271 "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
exp=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

if [[ "$exp" == "01" ]]; then # checking id d271 is supported

	if [[ -f "$fileName" ]]; then # check if setting.dat exists
		echo "sendobject - Load setting file"
		./control send --op=0x100D --p=0xffffc004 --size=file --data=setting.dat
	else
		echo ""
		echo ""
		echo "Please run save_setting.sh first" # run save settinbefore running load_setting
		echo ""
		echo ""

	fi
else
	echo "Save Data Mode is Disabled/ Not supported"

fi

#echo "close connection"
#./control close $@
