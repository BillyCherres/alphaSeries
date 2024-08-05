#! /bin/bash


## Single Shoot Continuously     ##

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
echo "set the Dial mode to Host"
./control send --op=0x9205 --p1=0xD25A --size=1 --data=0x01 $@

echo "waiting the operating mode API"
cond=""
while [ "$cond" != "01" ]; do
	./control get 0x5013 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond=$(get_device_property_value "IsEnable: " "$out")
done

echo "set the operating mode to still shooting mode"
./control send --op=0x9205 --p1=0x5013 --size=4 --data=0x00000001 $@

echo "waiting the changing"
cond=""
while [ "$cond" != "00000001" ]; do
	./control get 0x5013 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond=$(get_device_property_value "CurrentValue: " "$out")
done

echo "set savemedia to host device"
./control send --op=0x9205 --p1=0xD222 --size=2 --data=0x0001 $@

echo "waiting the savemedia changing"
cond=""
while [ "$cond" != "0001" ]; do
	./control get 0xD222 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond=$(get_device_property_value "CurrentValue: " "$out")
done

echo "waiting live view"
cond=""
while [ "$cond" != "01" ]; do
	./control get 0xD221 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo out=\"$out\"
	cond=$(get_device_property_value "CurrentValue: " "$out")
done

start_time0=$(date +%s.%6N)
echo "shooting #1"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time1=$(date +%s.%6N)

echo "shooting #2"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time2=$(date +%s.%6N)

echo "shooting #3"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time3=$(date +%s.%6N)

echo "shooting #4"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time4=$(date +%s.%6N)

echo "shooting #5"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time5=$(date +%s.%6N)

echo "shooting #6"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time6=$(date +%s.%6N)

echo "shooting #7"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time7=$(date +%s.%6N)

echo "shooting #8"
./control send --op=0x9207 --p1=0xD2C2 --data=0x0002 --size=2 $@
sleep .6
./control send --op=0x9207 --p1=0xD2C2 --data=0x0001 --size=2 $@
start_time8=$(date +%s.%6N)

echo "waiting the event of adding a image"
COMPLETE=0x8000
cond="0x0000"

#start_time=$(date +%s.%6N)
COMPLETE=0x8000
cond="0x0000"
while [ $(($cond & $COMPLETE)) -ne $(($COMPLETE)) ]; do
	./control get 0xD215 $@ --of=$FIFO &
	out=$(cat $FIFO)
	echo $out
	cond=0x$(get_device_property_value "CurrentValue: " "$out")
done
end_time=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time - $start_time" | bc)
#echo "**************"
#echo $elapsed
#printf "\n"
#echo "**************"

echo "****#1***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot1.jpg

end_time1=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time1 - $start_time1" | bc)
echo "***#1*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#2***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot2.jpg

end_time2=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time2 - $start_time2" | bc)
echo "***#2*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#3***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot3.jpg

end_time3=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time3 - $start_time3" | bc)
echo "***#3*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#4***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot4.jpg

end_time4=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time4 - $start_time4" | bc)
echo "***#4*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#5***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot5.jpg

end_time5=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time5 - $start_time5" | bc)
echo "***#5*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#6***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot6.jpg

end_time6=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time6 - $start_time6" | bc)
echo "***#6*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#7***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot7.jpg

end_time7=$(date +%s.%6N)
elapsed=$(echo "scale=6; $end_time7 - $start_time7" | bc)
echo "***#7*******"
echo $elapsed
printf "\n"
echo "**************"

echo "****#8***"
echo "getobjectinfo"
./control recv --op=0x1008 --p1=0xFFFFC001 $@
echo "getobject"
./control getobject 0xFFFFC001 $@ --of=shoot8.jpg

end_time8=$(date +%s.%6N)

elapsed=$(echo "scale=6; $end_time8 - $start_time0" | bc)
echo "***Total time to take 8 single shots*******"
echo $elapsed
printf "\n"
echo "**************"
