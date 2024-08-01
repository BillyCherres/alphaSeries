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

## operation
echo "open session"
./control open $@

echo "authentication"
./control auth $@

sleep 2

echo "set the Dial mode to Host"
./control send --op=0x9205 --p1=0xD25A --size=1 --data=0x01 $@

sleep 1

value="00000001"


echo "change ExpMode to 0x$value (current is 0x$current)"
./control send --op=0x9205 --p1=0x500E --size=4 --data=0x$value

echo "waiting the changing"
cond="XXXX"
while [ "$cond" != "$value" ]
do
    ./control get 0x500E $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond=`get_device_property_value "CurrentValue: " "$out"`
done

echo "exp mode value"
./control get 0x500E $@

echo "close connection"
./control close $@
