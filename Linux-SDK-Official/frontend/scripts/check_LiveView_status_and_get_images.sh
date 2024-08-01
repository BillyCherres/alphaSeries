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

echo "set the Dial mode to Host"
./control send --op=0x9205 --p1=0xD25A --size=1 --data=0x01 $@

echo "waiting the operating mode API"
cond=""
while [ "$cond" != "01" ]
do
    ./control get 0x5013 $@ --of=$FIFO &
    out=`cat $FIFO`
    echo out=\"$out\"
    cond=`get_device_property_value "IsEnable: " "$out"`
done

echo "set the operating mode to still shooting mode"
./control send --op=0x9205 --p1=0x5013 --size=4 --data=0x00000001 $@

echo "waiting the changing"
cond=""
while [ "$cond" != "00000001" ]
do
    ./control get 0x5013 $@ --of=$FIFO &
    out=`cat $FIFO`
    echo out=\"$out\"
    cond=`get_device_property_value "CurrentValue: " "$out"`
done

echo "waiting live view"
cond=""
while [ "$cond" != "01" ]
do
    ./control get 0xD221 $@ --of=$FIFO &
    out=`cat $FIFO`
    echo out=\"$out\"
    cond=`get_device_property_value "CurrentValue: " "$out"`
done

echo "get live view image"
mkdir -p jpg
NUM=0
while [ $NUM -lt 10 ]
do
    ./control getliveview $@ --of=jpg/live$NUM.jpg
    if [ -s jpg/live$NUM.jpg ]; then
        NUM=`expr $NUM + 1`
    fi
done

echo "close connection"
./control close $@
