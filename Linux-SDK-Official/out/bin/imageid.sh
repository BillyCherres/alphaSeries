#! /bin/bash


## ImageID Handling  		   ##

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

## operation

## MTP Open Session
: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment

sleep 2.0

## Image ID NUM setting ON
./control send --op=0x9205 --p1=0xD092 --data=0x02 --size=2

## Image ID String
./control send --op=0x9205 --p1=0xD04C --data=ImageIDString.txt --size=file

#sleep 1.0
## Image ID NUM
./control send --op=0x9205 --p1=0xD04B --data=0x01 --size=8

#sleep 2.0
## Take a picture
./control send --op=0x9207 --p1=0xD2C2 --data=0x02 --size=2
sleep 1.0
./control send --op=0x9207 --p1=0xD2C2 --data=0x01 --size=2
echo "waiting the event of adding a image"
COMPLETE=0x8000
cond="0x0000"
while [ $(($cond & $COMPLETE)) -ne $(($COMPLETE)) ]; do
	./control get 0xD215 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo $out
	cond=0x$(get_device_property_value "CurrentValue: " "$out")
done
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot.jpg

sleep 1.0
exiftool shoot.jpg


