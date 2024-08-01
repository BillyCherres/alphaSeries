#! /bin/bash


## MTP  Transfer		    ##

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

# Function to display the main menu
function displayMainMenu {
	echo ""
	echo ""
	echo ""
	echo "Main Menu"
	echo "1. Get Contents List"
	echo "2. Exit"
}

# common function
get_event_code() {
	TMP=$(echo $2 | sed -e ':a; $!N; $!b a')
	echo $TMP | sed -e "s/^.*$1=\([^,.]\+\).*\$/\1/g"
}

get_device_property_value() {
	TMP=$(echo $2 | sed -e ':a; $!N; $!b a')
	echo $TMP | sed -e "s/^.*$1\([0-9A-F]\+\).*\$/\1/g"
}

# Function to convert hexadecimal data to folder ID
convert_to_folder_id() {
	local hex_data=$1
	local reversed_data=$(echo "$hex_data" | tac -rs '..' | tr -d '\n')
	local folder_id=$(printf "0x%08s" "$reversed_data")
	echo "$folder_id"
}

# Function to convert hexadecimal data to folder ID with the length of 4
convert_string_to_number_4() {
	string="$1"

	byte1=$(((16#${string:0:2}) << 8))
	formatted_byte1=$(printf "0x%04X" "$byte1")
	trimmed_data=$(echo "$formatted_byte1" | sed -E 's/0x//')
	reversed_data1=$(echo "$trimmed_data" | sed -E 's/(..)(..)/\2\1/')
	byte1="$reversed_data1"

	byte2=$((16#${string:2:2}))
	formatted_byte2=$(printf "0x%04X" "$byte2")
	trimmed_data2=$(echo "$formatted_byte2" | sed -E 's/0x//')
	reversed_data2=$(echo "$trimmed_data2" | sed -E 's/(..)(..)/\2\1/')
	byte2="$reversed_data2"

	formatted_number2=$(printf "0x%s%s" "$reversed_data2" "$reversed_data1")
	echo "$formatted_number2"
}

# Function to convert hexadecimal data to folder ID with the length of 6
convert_string_to_number_6() {
	string="$1"

	byte1=$((16#${string:0:4}))
	formatted_byte1=$(printf "0x%04X" "$byte1")
	trimmed_data=$(echo "$formatted_byte1" | sed -E 's/0x//')
	reversed_data1=$(echo "$trimmed_data" | sed -E 's/(..)(..)/\2\1/')
	byte1="$reversed_data1"

	byte2=$((16#${string:4:4}))
	formatted_byte2=$(printf "0x%04X" "$byte2")
	trimmed_data2=$(echo "$formatted_byte2" | sed -E 's/0x//')
	reversed_data2=$(echo "$trimmed_data2" | sed -E 's/(..)(..)/\2\1/')
	byte2="$reversed_data2"

	formatted_number2=$(printf "0x%s%s" "$reversed_data2" "$reversed_data1")
	echo "$formatted_number2"
}

# Function to convert hexadecimal data to folder ID with the length of 8
convert_string_to_number_8() {
	string="$1"

	byte1=$((16#${string:0:4}))
	formatted_byte1=$(printf "0x%04X" "$byte1")
	trimmed_data=$(echo "$formatted_byte1" | sed -E 's/0x//')
	reversed_data1=$(echo "$trimmed_data" | sed -E 's/(..)(..)/\2\1/')
	byte1="$reversed_data1"

	byte2=$((16#${string:4}))
	formatted_byte2=$(printf "0x%04X" "$byte2")
	trimmed_data2=$(echo "$formatted_byte2" | sed -E 's/0x//')
	reversed_data2=$(echo "$trimmed_data2" | sed -E 's/(..)(..)/\2\1/')
	byte2="$reversed_data2"

	formatted_number2=$(printf "0x%s%s" "$reversed_data2" "$reversed_data1")
	echo "$formatted_number2"
}

# Checking if SD card is in slot 1, if not, then send error message
slot1_check() {
	./control get 0xD292 "$@" --of="$FIFO" &
	sleep 1
	out=$(cat "$FIFO")
	slot1=$(echo "$out" | grep -o 'dataset CurrentValue: [0-9A-F]*' | awk '{print $NF}')

	if [[ "$slot1" != "01" ]]; then # if not enabled

		echo "false"
	else
		echo "true"
	fi
}

#################################################
#		 "Top Menu"			#
#################################################
# Main program logic
is_running=true

while [ "$is_running" = true ]; do
	displayMainMenu
	read -p "Enter your choice: " choice

	case $choice in
	1)
		echo "Get Contents List"
		# Add your code for Option 1 here

		## operation

		## MTP Open Session
		#echo "[open session]"
		#./socc open_mtp $@  > /dev/null 2>&1
		./control send --op=0x9210 --p1=0x01 --p2=0x00000001 --p3=0x00000001 >/dev/null 2>&1

		## Authentication
		#echo "[authentication]"
		./control auth $@ >/dev/null 2>&1

		checker=$(slot1_check)
		echo "$checker"
		if [[ "$checker" == "true" ]]; then
			echo ""
			echo ""
			echo " Please enter a SD card into slot 1"
			continue
		fi

		# MTP Enabled
		#echo "[set the operating mode to the contents transferring mode]"
		#./socc mtp_on $@ > /dev/null 2>&1
		./control send --op=0x9212 --p1=0x02 --p2=0x01 >/dev/null 2>&1

		sleep 1.0

		## Get Object Property Folder Lists
		#echo "[Get Object Property Folder Lists]"
		# Execute the command and capture the output
		./control recv --op=0x9805 --p1=0 --p2=0x3001 --p3=0xDC07 --p4=0 --p5=0x01 $@ --of=$FIFO >/dev/null 2>&1 &
		data=$(cat $FIFO | tr -d '\0') >/dev/null 2>&1

		sleep 1

		# Convert from string to hex
		hex=$(echo -n "$data" | xxd -p)
		#echo "$hex"

		# Extract the first 2 characters
		first_2_bytes="${hex:1:1}"
		#echo "$first_2_bytes"
		# Convert the first 2 bytes from hex to decimal

		# Check if the first 2 bytes are valid hexadecimal digits
		if [[ ! $first_2_bytes =~ ^[0-9A-Fa-f]+$ ]]; then
			echo "No folders found."
			num_folders=0
			./control close >/dev/null 2>&1
			handler_interrupt
			handler_exit
			exit 1

		else

			# Convert the first 2 bytes from hex to decimal
			num_folders=$((16#${first_2_bytes}))
		fi

		#echo "Number of folders: [$num_folders]"

		# Remove the leading 'xx' and split the data into chunks
		data1="${hex:2}"
		folder_data=$(echo "$data1" | tr -d '\n[:space:]')

		#echo "folder_data: $folder_data"
		count=1 #photo count
		FOLDER_ID=()
		DATES=()
		CONTENT_ID=()
		CONTENT_NAME=()
		CONTENT_SIZE=()

		#################################################
		#		 "Folder Lists"			#
		#################################################

		pattern="07dcffff0b"

		for ((j = 1; j < num_folders + 1; j++)); do

			# Find the position of the pattern '07dcffff0b'
			pattern_pos=$(awk -v data="$folder_data" -v pattern="$pattern" 'BEGIN{print index(data, pattern)}')

			# Check if the pattern exists in the data
			if [[ $pattern_pos -gt 0 ]]; then
				# Calculate the length of the data before the pattern
				length_data=$((pattern_pos - 1))
				#echo "Length of Folder ID data: $length_data"
			else
				echo "Pattern not found in the data"
			fi

			#echo "folder_data: $folder_data"
			#done
			###################

			#echo "folder_data: $folder_data"
			# Define the lengths of the chunks
			lengths=($length_data 10 20)
			#echo "lengths: $lengths"

			# Loop until the end of the data
			#while [ -n "$folder_data" ]; do
			for length in "${lengths[@]}"; do
				if [ ${#folder_data} -ge $length ]; then
					chunk="${folder_data:0:$length}"
					folder_data="${folder_data:$length}"

					# Determine the type of chunk based on its position
					if [ $length -eq $length_data ]; then
						chunk=$(echo "$chunk" | tr -d '\n[:space:]')
						#echo "Folder ID: $chunk"

						if [[ $length_data -eq 4 ]]; then
							folder_id=$(convert_string_to_number_4 "$chunk")
						elif [[ $length -eq 6 ]]; then
							folder_id=$(convert_string_to_number_6 "$chunk")
						elif [[ $length -eq 8 ]]; then
							folder_id=$(convert_string_to_number_8 "$chunk")
						else
							echo "Invalid string length: $chunk"
						fi

						#echo "Folder ID: [$folder_id]"

					elif [ $length -eq 10 ]; then
						chunk=$(echo "$chunk" | tr -d '\n[:space:]')
						#echo "Data: $chunk"
					elif [ $length -eq 20 ]; then
						chunk=$(echo "$chunk" | tr -d '\n[:space:]')
						#echo "Dates: $chunk"
						# Convert hex string to ASCII string
						dates=$(echo "$chunk" | xxd -r -p)
						#echo "Dates: [$ascii_string]"
						#echo "=====   $j:  Folder ID: [$folder_id] [$dates]    ====="
						#printf "===== %4d:   Folder ID: [%s] [%s] =====\n" "$j" "$folder_id" "$dates"

					fi
				fi
			done
			FOLDER_ID+=("$folder_id")
			DATES+=("$dates")
			printf "===== %4d:   Folder ID: [%s] [%s] ===== \n" "$j" "$dates" "$folder_id"

			#################################################
			#		"Content Lists"			#
			#################################################
			./control recv --op=0x1007 --p1=0x00010001 --p2=0 --p3="$folder_id" $@ --of=$FIFO >/dev/null 2>&1 &
			data=$(cat $FIFO | tr -d '\0') >/dev/null 2>&1

			# Convert from string to hex
			hex=$(echo -n "$data" | xxd -p)
			#echo "hex=$hex"

			# Extract the first 2 characters
			first_2_bytes2="${hex:1:1}"
			#echo "$first_2_bytes2"
			# Convert the first 2 bytes from hex to decimal
			num_contents=$((16#${first_2_bytes2}))

			#echo "Number of contents: [$num_contents]"

			# Remove the leading 'xx' and split the data into chunks
			data2="${hex:2}"
			content_data=$(echo "$data2" | tr -d '\n[:space:]')

			#echo "content_data: $content_data"

			last_byte=$(printf "%s" "$content_data" | awk '{print substr($0,length-1)}')
			#echo "Last byte: $last_byte"

			pattern2="$last_byte"

			for ((k = 1; k < num_contents + 1; k++)); do

				# Find the position of the pattern 'xx'
				pattern_pos=$(awk -v data="$content_data" -v pattern="$pattern2" 'BEGIN{print index(data, pattern)}')

				# Check if the pattern exists in the data
				if [[ $pattern_pos -gt 0 ]]; then
					# Calculate the length of the data before the pattern
					length_data2=$((pattern_pos + 1))
					#echo "Length of Content ID data: $length_data"
					#else
					#echo "Pattern not found in the data"
				fi

				#echo "content_data: $content_data"
				#done
				#echo "folder_data: $folder_data"
				# Define the lengths of the chunks
				lengths2=($length_data2)
				#echo "lengths: $lengths2"

				# Loop until the end of the data
				#while [ -n "$content_data" ]; do

				for length2 in "${lengths2[@]}"; do
					if [ ${#content_data} -ge $length2 ]; then
						chunk2="${content_data:0:$length2}"
						content_data="${content_data:$length2}"

						# Determine the type of chunk based on its position
						if [ $length2 -eq $length_data2 ]; then
							chunk2=$(echo "$chunk2" | tr -d '\n[:space:]')
							#echo "Folder ID: $chunk"
							#echo "length_data: $length_data2"
							if [[ $length_data2 -eq 4 ]]; then
								content_id=$(convert_string_to_number_4 "$chunk2")
							elif [[ $length2 -eq 6 ]]; then
								content_id=$(convert_string_to_number_6 "$chunk2")
							elif [[ $length2 -eq 8 ]]; then
								content_id=$(convert_string_to_number_8 "$chunk2")
							else
								echo "Invalid string length: $chunk2"
							fi

						fi
					fi
				done

				#################################################
				#		"Content Name"			#
				#################################################
				#echo "content id: $content_id"
				./control recv --op=0x9803 --p1=$content_id --p2=0xDC07 $@ --of=$FIFO >/dev/null 2>&1 &
				content_name=$(cat $FIFO | tr -d '\0') >/dev/null 2>&1

				content_name=$(echo "$content_name" | sed 's/^[[:space:]]*//') # Remove newline character from content_name
				content_name=$(echo "$content_name" | tr -d '\n')              # Remove newline character from content_name

				printf "%4d:   Content ID: [%s]	Content Name: [%s] \n" "$count" "$content_id" "$content_name"

				#################################################
				#		"Content Size"			#
				#################################################
				#echo "content id: $content_id"
				./control recv --op=0x9803 --p1=$content_id --p2=0xDC04 $@ --of=$FIFO >/dev/null 2>&1 &
				content_size=$(cat $FIFO | tr -d '\0') >/dev/null 2>&1

				# Convert from string to hex
				content_size=$(echo -n "$content_size" | xxd -p)
				content_size=$(printf "%s" "$content_size" | tac -rs .. | tr -d '\n')

				#echo "content_size: $content_size"

				CONTENT_ID+=("$content_id")
				CONTENT_NAME+=("$content_name")
				CONTENT_SIZE+=("$content_size")

				count=$((count + 1))
			done
		done

		read -p "Select a folder to be downloaded: " number
		echo "The selected number is: $number"
		#################################################
		#		"Content Transfer"		#
		#################################################
		./control recv --op=0x9211 --p1="${CONTENT_ID[$((number - 1))]}" --p2=0 --p3=0 --p4=0x"${CONTENT_SIZE[$((number - 1))]}" --of="${CONTENT_NAME[$((number - 1))]}" >/dev/null 2>&1

		rm -f $FIFO

		;;

	2)
		echo "Exiting..."
		# ./control close >/dev/null 2>&1
		handler_interrupt
		handler_exit
		break
		;;
	esac
done
