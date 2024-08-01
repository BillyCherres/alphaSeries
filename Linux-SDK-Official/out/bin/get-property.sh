#!/bin/bash


## Get Propert Code .txt           ##

# In this file we are getting information about a property code and putting the information inside a txt file
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

: << 'comment'
## Open Session
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

comment
boolean=true
while $boolean; do
	bool=true
	echo ""
	echo ""
	echo ""
	echo "Please choose one of the following options"
	echo "1) Get a single property"
	echo "2) Get all properties"
	echo "0) Quit"

	read opt

	case "$opt" in
	"1")
		echo "Please enter a valid property code starting with 0x"
		read code

		fileName="$code".txt
		touch "$fileName"
		./control get "$code" --of="$fileName"

		;;
	"2")
		fileName="allProperties.txt"
		touch "$fileName"
		./control getall --of="$fileName"
		;;
	"0")
		echo "Quitting"
		boolean=false #exit outerwhileloop
		bool=false    #exit inner whileloop
		;;
	*)
		"Invalid Input"
		bool=false
		;;
	esac

	while $bool; do
		echo ""
		echo ""
		echo "1) View file contents"
		echo "2) Quit and delete file"
		read option
		case "$option" in
		"1")
			cat "$fileName"
			;;
		"2")
			echo "Quitting"
			rm -f "$fileName"
			bool=false
			;;
		*) #default case
			echo "Invalid Option"
			;;
		esac

	done

done
## Close Session
#echo "[close session]"
#./control close $@
