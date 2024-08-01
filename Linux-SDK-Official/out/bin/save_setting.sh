#! /bin/bash


## Save Settings           ##

##In this Data Mode, we are getting the object information, but first we ensure that the Save Data mode is enabled##

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

#get the Camera Save setting
./control get 0xD271 "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
exp=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')
echo "Current Save Data mode: $exp"

if [[ "$exp" == "01" ]]; then # checking id d271 is supported

	echo "getobjectinfo"
	./control recv --op=0x1008 --p1=0xffffc004 $@
	echo "getobject"
	./control getobject 0xffffc004 $@ --of=setting.dat

else
	echo ""
	echo ""
	echo "Save Data Mode is Disabled/ Not Supported"
	echo ""
	echo ""
fi

#echo "close connection"
#./control close $@
