#!/bin/bash


## MENU           ##

# In this file we are integrating a menu like system in the terminal which allows the user to choose options to manipulate the a7R5 Camera
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
echo "[open session]"
./control open $@

## Authentication
echo "[authentication]"
./control auth $@

## ENSURING PERMISSION ##

for file in *.sh; do
     chmod +x "$file"
done

chmod +x control

: << 'Comment'
chmod +x ./change_camera_settings_expmode.sh
chmod +x ./change_camera_settings_zoom.sh
chmod +x ./shoot_an_image_and_get_it.sh
chmod +x ./mtp date-time.sh
chmod +x ./get-property.sh
chmod +x ./single_shoot_cont.sh
chmod +x ./imageid.sh
chmod +x ./iso-setting.sh
chmod +x ./shutter-speed-setting.sh
chmod +x ./still-file-format-setting.sh
chmod +x ./sd-card-format.sh
chmod +x ./movie_rec.sh
chmod +x ./save_setting
chmod +x ./load_setting.sh
Comment

# zoom setting enabled status
./control get 0xD25B "$@" --of="$FIFO" &

sleep 1
out=$(cat "$FIFO")
exp=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')


if [[ "$exp" -eq "01" ]]; then
	zoomV="D25A"
elif [[ "$exp" -eq "00" ]]; then
	# switch to digital zoom
	./control send --op=0x9205 --p1=0xD25F --size=2 --data=0x04 "$@"
fi

echo "waiting the changing"
cond1="XXXX"
while [ "$cond1" != "04" ]
do
    ./control get 0xD25F $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    cond1=`get_device_property_value "CurrentValue: " "$out"`
done


echo "waiting the changing"
exp="XXXX"
start_time=$(date +%s)

while [ "$exp" != "01" ]
do
    ./control get 0xD25B $@ --of=$FIFO &
    out=`cat $FIFO`
    echo $out
    exp=`get_device_property_value "CurrentValue: " "$out"`
    
    # Check if the loop has been running for more than 7 seconds
    current_time=$(date +%s)
    elapsed_time=$(( current_time - start_time ))
    if [ $elapsed_time -ge 7 ]; then
        echo "Time limit of 7 seconds reached, breaking the loop."
        break
    fi

done


echo "$exp"

if [[ "$exp" -eq "01" ]]; then
zoomV="D25A"
else 
zoomV="null"
fi

echo "$zoomV"


# device property value ---> .sh file
# this method is used to generate a menu specific for each cameras personal set up
declare -A props
props=(
	["500E"]="change_camera_settings_expmode"
	["$zoomV"]="change_camera_settings_zoom"
	["D24F"]="shoot_an_image_and_get_it mtp date-time get-property single_shoot_cont"
	["D092"]="imageid"
	["D21E"]="iso-setting"
	["D20D"]="shutter-speed-setting"
	["D253"]="still-file-format-setting"
	["D292"]="sd-card-format"
	["D261"]="movie_rec"
	["D271"]="save_setting load_setting"
	# D24F is a code that every device supports
)

# generate a file that contains getall output
touch allProps.txt
file="allProps.txt"
./control getall --of="$file"

# Device properties we are searching for in allProps.txt
hexProp=("500E" "D25A" "D24F" "D092" "D24F" "D24F" "D21E" "D20D" "D253" "D292" "D261" "D24F" "D271" "D271" "D24F")

# array that holds all device property values found in allProps.txt
found_hex=()

# Conducts the search for all values in hexProp, in the allProps.txt file, and adds all found values to found_hex array
check_hex() {
	for hex in "${hexProp[@]}"; do
		if grep -q "$hex" "$file"; then
			found_hex+=("$hex")
		fi
	done
}

check_hex

# generate the frontend terminal menu
generate_case_blocks() {
	local count=1                                    # Initialize count to keep track of the menu numbers
	local script                                     # Variable to hold the script names from props associative array
	declare -A unique_scripts                        # Associative array to keep track of displayed scripts to avoid duplicates
	for hex in "${found_hex[@]}"; do                 # Loop through each found hexadecimal value from the found_hex array
		for script in ${props["$hex"]}; do              # Loop through each script associated with the current hex key in the props array
			if [[ -z "${unique_scripts[$script]}" ]]; then # If the script has not been added to the menu yet (is not in unique_scripts)
				unique_scripts["$script"]=1                   # Mark the script as added to the menu
				echo "$count) $script"                        # Print the menu option with the current count and script name
				count=$((count + 1))                          # Increment the count for the next menu option
			fi
		done
	done
	echo "0) Quit" # Add a "Quit" option at the end of the menu
}

execute_option() {
	local count=1
	local selected_script
	local script
	declare -A unique_scripts
	#iterates through each found hexadecimal value and its associated scripts, ensuring each unique script is considered only once, then matches the selected menu option (passed as $1) with the corresponding script to be executed.
	for hex in "${found_hex[@]}"; do
		for script in ${props["$hex"]}; do
			if [[ -z "${unique_scripts[$script]}" ]]; then
				unique_scripts["$script"]=1
				if [[ "$count" -eq "$1" ]]; then
					selected_script="$script"
				fi
				count=$((count + 1))
			fi
		done
	done

	if [[ -n "$selected_script" ]]; then
		echo "Executing $selected_script.sh"
		./$selected_script.sh
	else
		echo "Invalid option, please choose a valid number"
	fi
}

#run the menu
menu1=true
while $menu1; do

	echo ""
	echo ""
	echo "Alpha Series Menu"
	echo " -----------------"
	echo "Bellow are compatible options for this specific alpha camera"
	echo " "
	echo "Choose one of the following options:"
	echo ""
	echo ""
	generate_case_blocks
	read -p "Enter your choice: " option
	echo ""

	if [[ "$option" -eq 0 ]]; then
		echo "Quitting"
		rm -f allProps.txt
		menu1=false
	else
		execute_option "$option"
	fi
done

## close session
./control close $@

