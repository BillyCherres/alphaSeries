#! /bin/bash


# fifo
FIFO=fifo$$
handler_interrupt() {
    kill $(jobs -p) >& /dev/null
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
    TMP=`echo $2 | sed -e ':a; $!N; $!b a'`
    echo $TMP | sed -e "s/^.*$1=\([^,.]\+\).*\$/\1/g"
}

get_device_property_value() {
    TMP=`echo $2 | sed -e ':a; $!N; $!b a'`
    echo $TMP | sed -e "s/^.*$1\([0-9A-F]\+\).*\$/\1/g"
}

echo "open session"
#./control open $@
./control send --op=0x1002 --p1=1 $@

echo "authentication"
#./control auth $@
./control recv --op=0x9201 --p1=1 --p2=0x00000000 --p3=0x00000000 $@
./control recv --op=0x9201 --p1=2 --p2=0x00000000 --p3=0x00000000 $@
cond="0"
while [ "$cond" != "0x2C01" ] # little endian
do
    ./control recv --op=0x9202 --p1=0x012C $@ --log=$FIFO &
    out=`cat $FIFO`
    echo out=\"$out\"
    cond=`get_event_code "data" "$out" | cut -c 1-6`
done
./control recv --op=0x9201 --p1=3 --p2=0x0000000 --p3=0x0000000 $@




