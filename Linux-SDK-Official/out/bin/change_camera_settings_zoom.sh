#! /bin/bash

## Change Zoom Settings           ##

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

: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@
comment

# set image quality to jpeg
./control send --op=0x9205 --p1=0xD253 --size=2 --data=0x03


echo "set the Dial mode to Host"
./control send --op=0x9205 --p1=0xD25A --size=2 --data=0x01 $@

#proceed after the change is complete
echo "waiting the changing"
cond1="XXXX"
while [ "$cond1" != "01" ]
do
    ./control get 0xD25A $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond1=`get_device_property_value "CurrentValue: " "$out"`
done

echo "set the operating mode to Normal shooting mode"
./control send --op=0x9205 --p1=0x5013 --size=4 --data=0x00000001 $@

echo "waiting the changing"
cond2=""
while [ "$cond2" != "00000001" ]; do
	./control get 0x5013 $@ --of=$FIFO &
	out=$(cat $FIFO)
        echo out=\"$out\"
	cond2=$(get_device_property_value "CurrentValue: " "$out")
done

echo "set zoom setting to Digital Image Zoom"
./control send --op=0x9205 --p1=0xD25F --size=1 --data=0x04

echo "waiting zoom info"
cond3=""
while [ "$cond3" != "01" ]; do
	./control get 0xD25C $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond3=$(get_device_property_value "IsEnable: " "$out")
	echo "cond 3"
done

sleep 1

echo "set zoom position"
value=01 ##FF for wide zoom ##01 for tele zoom
./control send --op=0x9207 --p1=0xD2DD --size=1 --data=0x$value $@

sleep 6

echo "stop zoom"4
value=00
./control send --op=0x9207 --p1=0xD2DD --size=1 --data=0x$value $@

echo "set zoom position"
value=FF ##FF for wide zoom ##01 for tele zoom
./control send --op=0x9207 --p1=0xD2DD --size=1 --data=0x$value $@

sleep 6

echo "stop zoom"
value=00
./control send --op=0x9207 --p1=0xD2DD --size=1 --data=0x$value $@


#echo "close connection"
#./control close $@
