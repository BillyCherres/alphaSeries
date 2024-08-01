#! /bin/bash


## Change Camera Settings           ##

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

: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment

#sleep 2

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

value="00000001" # set exposure program mode to Manual (M on the dial)

echo "change ExpMode to 0x$value (current is 0x$current)"
./control send --op=0x9205 --p1=0x500E --size=4 --data=0x$value

#proceed after the change is complete
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

#echo "close connection"
#./control close $@

