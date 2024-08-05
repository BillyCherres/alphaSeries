#! /bin/bash


## Movie Record          ##

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
: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment
echo "****************************************************"
echo "************Movie Rec ******************************"
echo "****************************************************"

./control send --op=0x9207 --p1=0xD2C8 --data=0x0002 --size=2 $@
start_time=$(date +%s.%6N)
cond="0"
while [ "$cond" != "00000005" ]; do
	./control get 0xD261 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond=$(get_device_property_value "CurrentValue: " "$out")
done

./control send --op=0x9207 --p1=0xD2C8 --data=0x0001 --size=2 $@
end_time=$(date +%s.%6N)
date +"%T"
echo "****************************************************"
echo "************Movie Rec Stop**************************"
echo "****************************************************"
elapsed=$(echo "scale=6; $end_time - $start_time" | bc)
echo "***Total time of recording movie operations*******"
echo $elapsed

