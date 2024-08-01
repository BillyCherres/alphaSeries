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

echo "set savemedia to host device"
./control send --op=0x9205 --p1=0xD222 --size=2 --data=0x0001 $@

echo "waiting the savemedia changing"
cond=""
while [ "$cond" != "0001" ]
do
    ./control get 0xD222 $@ --of=$FIFO &
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

echo "Getting Focus Information:"
echo " "
echo " "
echo " "

./control get 0x500A "$@" --of="$FIFO" &
out=$(cat "$FIFO")
value=$(echo "$out"| grep -o 'dataset CurrentValue: [0-9]*'| awk '{print $NF}')


#Sleep levels differ based off camera model and focus mode
# Comments are the shortest sleep levels for a7R4 model
if [ "$value" -eq 0001 ]; then 
	AF="Manual(MF)"
	sleepNum=1.8 # Shortest sleep = 1.3
elif [ "$value" -eq 0002 ]; then 
	AF="Automatic (AF_S)"
	sleepNum=2 # Shortest sleep = 1.5
elif [ "$value" -eq 8004 ]; then 
	AF="Continuous AF(AF_C)"
	sleepNum=2 # Shortest sleep = 1.5
elif [ "$value" -eq 8005 ]; then 
	AF="Auto (AF_A)"
	sleepNum=2.3 # Shortest sleep = 1.8
elif [ "$value" -eq 8006 ]; then 
	AF="Direct Manual Focus (DMF)"
	sleepNum=2 # 1.5
	
fi

echo "Type of Focus: $AF; Sleep#: $sleepNum"

echo " "
echo " "
echo " "
echo "shooting"
./control send --op=0x9207 --p1=0xD2C1 --data=0x0002 --size=2 $@
sleep "$sleepNum"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep "$sleepNum"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
sleep "$sleepNum"
./control send --op=0x9207 --p1=0xD2C1 --data=0x0001 --size=2 $@
echo " "
echo " "
echo " "



echo "waiting the event of adding a image"
COMPLETE=0x8000
cond="0x0000"
while [ $(($cond & $COMPLETE)) -ne $(($COMPLETE)) ]
do
    ./control get 0xD215 $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond=0x`get_device_property_value "CurrentValue: " "$out"`
done
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xffffc001 $@
echo "getobject"
./control getobject 0xffffc001 $@ --of=shoot.jpg

sleep 1

echo "close connection"
./control close $@
