#!/bin/bash


## ISO Settings                    ##

# In this file we are working with the ISO Sensitivity or Property Code 0xD21E, where we continuously get
# the current ISO Setting and offer the user to change the ISO setting

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

# Declare an associative array
declare -A iso_values
declare -A iso_map

iso_values["1"]="00FFFFFF"  # auto
iso_values["2"]="10000032"  # 50
iso_values["3"]="10000040"  # 64
iso_values["4"]="10000050"  # 80
iso_values["5"]="00000064"  # 100
iso_values["6"]="0000007D"  # 125
iso_values["7"]="000000A0"  # 160
iso_values["8"]="000000C8"  # 200
iso_values["9"]="000000FA"  # 250
iso_values["10"]="00000140" # 320
iso_values["11"]="00000190" # 400
iso_values["12"]="000001F4" # 500
iso_values["13"]="00000280" # 640
iso_values["14"]="00000320" # 800
iso_values["15"]="000003E8" # 1000
iso_values["16"]="000004E2" # 1250
iso_values["17"]="00000640" # 1600
iso_values["18"]="000007D0" # 2000
iso_values["19"]="000009C4" # 2500
iso_values["20"]="00000C80" # 3200
iso_values["21"]="00000FA0" # 4000
iso_values["22"]="00001388" # 5000
iso_values["23"]="00001900" # 6400
iso_values["24"]="00001F40" # 8000
iso_values["25"]="00002710" # 10000
iso_values["26"]="00003200" # 12800
iso_values["27"]="00003E80" # 16000
iso_values["28"]="00004E20" # 20000
iso_values["29"]="00006400" # 25600
iso_values["30"]="00007D00" # 32000
iso_values["31"]="10009C40" # 40000
iso_values["32"]="1000C800" # 51200
iso_values["33"]="1000FA00" # 64000
iso_values["34"]="10013880" # 80000
iso_values["35"]="10019000" # 102400

#holding hexadecimal values with their respective iso values
iso_map["00FFFFFF"]="AUTO"
iso_map["10000032"]=50
iso_map["10000040"]=64
iso_map["10000050"]=80
iso_map["00000064"]=100
iso_map["0000007D"]=125
iso_map["000000A0"]=160
iso_map["000000C8"]=200
iso_map["000000FA"]=250
iso_map["00000140"]=320
iso_map["00000190"]=400
iso_map["000001F4"]=500
iso_map["00000280"]=640
iso_map["00000320"]=800
iso_map["000003E8"]=1000
iso_map["000004E2"]=1250
iso_map["00000640"]=1600
iso_map["000007D0"]=2000
iso_map["000009C4"]=2500
iso_map["00000C80"]=3200
iso_map["00000FA0"]=4000
iso_map["00001388"]=5000
iso_map["00001900"]=6400
iso_map["00001F40"]=8000
iso_map["00002710"]=10000
iso_map["00003200"]=12800
iso_map["00003E80"]=16000
iso_map["00004E20"]=20000
iso_map["00006400"]=25600
iso_map["00007D00"]=32000
iso_map["10009C40"]=40000
iso_map["1000C800"]=51200
iso_map["1000FA00"]=64000
iso_map["10013880"]=80000
iso_map["10019000"]=102400

#terminal formatting
echo ""
echo ""
echo ""



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

# P automatic
var=00010002

echo "set the exposure mode to P automatic"
./control send --op=0x9205 --p1=0x500E --size=6 --data=0x$var $@

#proceed after the change is complete
echo "waiting the changing"
cond2="XXXX"
while [ "$cond2" != "$var" ]
do
    ./control get 0x500E $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond2=`get_device_property_value "CurrentValue: " "$out"`
done

#get the exposure mode setting
./control get 0x500E "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
exp=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')
echo "Current exposure mode: $exp"

if [[ $exp == "00010002" ]]; then
	bool=true
	while $bool; do
		echo ""
		echo ""
		echo ""
		# Get the current ISO setting
		./control get 0xD21E "$@" --of="$FIFO" &
		sleep 1
		out=$(cat "$FIFO")
		value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

		echo ""
		echo ""
		echo ""
		echo "Current ISO Setting: ISO ${iso_map[$value]}" #print the current setting
		echo ""
		echo ""

		echo "Please choose one of the following options:"
		for i in $(seq 1 ${#iso_values[@]}); do
			echo "$i) ISO ${iso_map[${iso_values[$i]}]}"
		done
		echo "0) Quit"

		read input

		# if the user chooses 0 then quit
		if [[ "$input" == 0 ]]; then
			echo "Quitting"
			bool=false
			break
		fi

		if [[ "$input" -ge 1 && "$input" -le ${#iso_values[@]} ]]; then
			hexvalue="${iso_values[$input]}" #set hex value based off iso_map

			#Change the ISO Setting
			./control send --op=0x9205 --p1=0xD21E --size=6 --data=0x$hexvalue "$@" #change iso value

			#re retrieve the ISO Setting
			./control get 0xD21E "$@" --of="$FIFO" &
			sleep 1
			out=$(cat "$FIFO")
			value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

		else
			echo "Invalid input. Please enter a number between 1 and ${#iso_values[@]}, or 0 to quit."
		fi
	done

else
	echo ""
	echo ""
	echo ""
	echo "Not in shutter priority"
	echo ""
	echo ""
	echo ""
fi

#terminal formatting
echo ""
echo ""
echo ""

#./control close "$@"
