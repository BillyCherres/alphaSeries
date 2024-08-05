#! /bin/bash


## Date and time           ##

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

get_date_time() {
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

#date_time=$(date +"%Y%m%dT%H%M%S%z")
date_time=$(date +"%Y%m%dT%H%M%S.%S%z0")
#modified_date_time=$(echo "$date_time" | sed 's/\([0-9]\+\.[0-9]\)[0-9]-/\1-/')
#modified_date_time="20240425T074106.0-07000"
#modified_date_time="20240425T074106.0-00000"
modified_date_time="20240429T184304.0+00.00"
echo "$modified_date_time"

echo "$DATE"

echo "Set Date/Time"
./control send --op=0x9205 --p1=0xD223 --data=${modified_date_time} --size=string $@


