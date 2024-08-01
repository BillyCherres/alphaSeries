#!/bin/bash


## SD Card Format Setting       ##

## In this file we are working with the SD Card Format Setting or Property Code 0xD222, where we are able to format the SD cards in slot 1 and 2##
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

## Open Session
: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment

#get the sd card status slot 1
./control get 0xD292 "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
slot1=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

#get the sd card status slot 2
./control get 0xD293 "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
slot2=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

echo "set Save Destination Mode to Host Device and Camera"
./control send --op=0x9205 --p1=0xD222 --size=4 --data=0x0011 $@

#proceed after the change is complete
echo "waiting the changing"
cond1="XXXX"
while [ "$cond1" != "0011" ]
do
    ./control get 0xD222 $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond1=`get_device_property_value "CurrentValue: " "$out"`
done

#get the save destination mode setting
./control get 0xD222 "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
exp=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')
echo "Current Destination mode: $exp"

# Check to ensure still image save destination mode is either set to Camera (Memory Card) or Host Device and Camera Memory Card
if [[ $exp == "0010" || $exp == "0011" ]]; then
	bool=true
	while $bool; do
		echo ""
		echo ""
		echo ""

		echo "Please choose one of the following options"
		echo "1) Format Slot 1"
		echo "2) Format Slot 2"
		echo "0) Quit"
		read input

		case "$input" in
		"1")
			if [[ "$slot1" == "01" ]]; then
				echo ""
				echo ""
				echo ""
				echo "Please choose one of the following options"
				echo "1) Full Format (Slot 1)"
				echo "2) Quick Format (Slot 1)"
				read opt1

				case "$opt1" in
				"1")
					echo "Full Format slot 1"
					./control send --op=0x9207 --p1=0xd2e2 --size=2 --data=0x0001
					;;
				"2")
					echo "Quick Format slot 1"
					./control send --op=0x9207 --p1=0xd2e2 --size=2 --data=0x0011
					;;
				"0")
					echo "Quitting"
					;;
				esac
				bool=false
			elif [[ "$slot1" == "00" ]]; then
				echo ""
				echo "no  SD card"

			fi
			;;
		"2")
			if [[ "$slot2" == "01" ]]; then
				echo ""
				echo ""
				echo ""
				echo "Please choose one of the following options"
				echo "1) Full Format (Slot 2)"
				echo "2) Quick Format (Slot 2)"
				read opt1

				case "$opt1" in
				"1")
					echo "Full Format slot 2"
					./control send --op=0x9207 --p1=0xd2e2 --size=2 --data=0x0002
					;;
				"2")
					echo "Quick Format slot 2"
					./control send --op=0x9207 --p1=0xd2e2 --size=2 --data=0x0012
					;;
				"0")
					echo "Quitting"
					;;

				esac
				bool=false
			elif [[ "$slot2" == "00" ]]; then
				echo ""
				echo "no SD card"

			fi
			;;
		"0")
			echo "Quitting"
			bool=false
			;;
		esac

	done

fi

## close session
#./control close $@
