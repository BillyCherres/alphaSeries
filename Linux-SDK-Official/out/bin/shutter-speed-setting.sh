#! /bin/bash


## Shutter Speed Settings  	   ##

# In this file we are working with the Shutter Speed or Property Code 0xD20D, where we continuously get
# the current Shutter Speed Setting and offer the user to change the Shutter Speed

# the sleep function is necessary for the camera to process the change in dial mode and change in exposure settings

# Declare associative arrays
declare -A shutter_speeds
declare -A shutter_speeds_indices

# Add fraction numbers
shutter_speeds["00011F40"]="1/8000"
shutter_speeds["00011900"]="1/6400"
shutter_speeds["00011388"]="1/5000"
shutter_speeds["00010FA0"]="1/4000"
shutter_speeds["00010C80"]="1/3200"
shutter_speeds["000109C4"]="1/2500"
shutter_speeds["000107D0"]="1/2000"
shutter_speeds["00010640"]="1/1600"
shutter_speeds["000104E2"]="1/1250"
shutter_speeds["000103E8"]="1/1000"
shutter_speeds["00010320"]="1/800"
shutter_speeds["00010280"]="1/640"
shutter_speeds["000101F4"]="1/500"
shutter_speeds["00010190"]="1/400"
shutter_speeds["00010140"]="1/320"
shutter_speeds["000100FA"]="1/250"
shutter_speeds["000100C8"]="1/200"
shutter_speeds["000100A0"]="1/160"
shutter_speeds["0001007D"]="1/125"
shutter_speeds["00010064"]="1/100"
shutter_speeds["00010050"]="1/80"
shutter_speeds["0001003c"]="1/60"
shutter_speeds["00010032"]="1/50"
shutter_speeds["00010028"]="1/40"
shutter_speeds["0001001E"]="1/30"
shutter_speeds["00010019"]="1/25"
shutter_speeds["00010014"]="1/20"
shutter_speeds["0001000F"]="1/15"
shutter_speeds["0001000D"]="1/13"
shutter_speeds["0001000A"]="1/10"
shutter_speeds["00010008"]="1/8"
shutter_speeds["00010006"]="1/6"
shutter_speeds["00010005"]="1/5"
shutter_speeds["00010004"]="1/4"
shutter_speeds["00010003"]="1/3"
# Add real numbers (denominator is fixed to 000A)
shutter_speeds["0004000A"]="0.4"
shutter_speeds["0005000A"]="0.5"
shutter_speeds["0006000A"]="0.6"
shutter_speeds["0008000A"]="0.8"
shutter_speeds["000A000A"]="1"
shutter_speeds["000D000A"]="1.3"
shutter_speeds["0010000A"]="1.6"
shutter_speeds["0014000A"]="2"
shutter_speeds["0019000A"]="2.5"
shutter_speeds["0020000A"]="3.2"
shutter_speeds["0028000A"]="4"
shutter_speeds["0032000A"]="5"
shutter_speeds["003C000A"]="6"
shutter_speeds["0050000A"]="8"
shutter_speeds["0064000A"]="10"
shutter_speeds["0082000A"]="13"
shutter_speeds["0096000A"]="15"
shutter_speeds["00C8000A"]="20"
shutter_speeds["00FA000A"]="25"
shutter_speeds["012C000A"]="30"

shutter_speeds_indices=(
	[1]="00011F40"
	[2]="00011900"
	[3]="00011388"
	[4]="00010FA0"
	[5]="00010C80"
	[6]="000109C4"
	[7]="000107D0"
	[8]="00010640"
	[9]="000104E2"
	[10]="000103E8"
	[11]="00010320"
	[12]="00010280"
	[13]="000101F4"
	[14]="00010190"
	[15]="00010140"
	[16]="000100FA"
	[17]="000100C8"
	[18]="000100A0"
	[19]="0001007D"
	[20]="00010064"
	[21]="00010050"
	[22]="0001003c"
	[23]="00010032"
	[24]="00010028"
	[25]="0001001E"
	[26]="00010019"
	[27]="00010014"
	[28]="0001000F"
	[29]="0001000D"
	[30]="0001000A"
	[31]="00010008"
	[32]="00010006"
	[33]="00010005"
	[34]="00010004"
	[35]="00010003"
	[36]="0004000A"
	[37]="0005000A"
	[38]="0006000A"
	[39]="0008000A"
	[40]="000A000A"
	[41]="000D000A"
	[42]="0010000A"
	[43]="0014000A"
	[44]="0019000A"
	[45]="0020000A"
	[46]="0028000A"
	[47]="0032000A"
	[48]="003C000A"
	[49]="0050000A"
	[50]="0064000A"
	[51]="0082000A"
	[52]="0096000A"
	[53]="00C8000A"
	[54]="00FA000A"
	[55]="012C000A"

)

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


# Manual M
var=00000001
echo "set the exposure mode to Manual M"
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

if [[ $exp == "00000001" ]]; then

	bool=true
	while $bool; do
		echo ""
		echo ""
		echo ""
		# Get the current Shutter Speed setting
		./control get 0xD20D "$@" --of="$FIFO" &
		sleep 1
		out=$(cat "$FIFO")
		value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

		echo ""
		echo ""
		echo ""
		echo "Current Shutter Speed Setting: ${shutter_speeds[$value]}" #print the current setting
		echo ""
		echo ""

		echo "Please choose one of the following options:"
		for i in $(seq 1 ${#shutter_speeds_indices[@]}); do
			echo "$i) ${shutter_speeds[${shutter_speeds_indices[$i]}]}"
		done
		echo "0) Quit"

		read input

		# if the user chooses 0 then quit
		if [[ "$input" == 0 ]]; then
			echo "Quitting"
			bool=false
			break
		fi

		if [[ "$input" -ge 1 && "$input" -le ${#shutter_speeds_indices[@]} ]]; then
			hexvalue="${shutter_speeds_indices[$input]}" #set hex value based off shutter_speeds

			#Change the shutter speed Setting
			./control send --op=0x9205 --p1=0xD20D --size=6 --data=0x$hexvalue "$@" #change shutter value

			#re retrieve the shutter Setting
			./control get 0xD20D "$@" --of="$FIFO" &
			sleep 1
			out=$(cat "$FIFO")
			value=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

		else
			echo ""
			echo ""
			echo ""
			echo "Invalid input. Please enter a number between 1 and ${#shutter_speeds_indices[@]}, or 0 to quit."
			echo ""
			echo ""
			echo ""
		fi
	done

else
	echo ""
	echo ""
	echo ""
	echo "Not correct exposure setting"
	echo ""
	echo ""
	echo ""
fi

#closing the session would change the dial mode host the camera and not the PC, changing it from Manual (M) to whatever the dial is at
# this could affect the shutter speed setting depending on what mode the dial is on 
# ./control close "$@"


