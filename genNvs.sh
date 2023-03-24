#!/bin/bash

if [[ -z $IDF_PATH ]]; then
	echo 'Varible $IDF_PATH is not defined'
	echo "Execute the ./export.sh script in the esp directory before generating the nvs partitions"
	exit
fi

# copy from template
cp nvs/nvs-template.csv nvs/nvs.csv

# generate uuid
serial=$(uuidgen)

# generate default password as 'admin' salted with the serial number 
pass=$(echo -n "admin""$serial" | sha256sum | tr -cd '[:alnum:]')

# subtitute values into the nvs partition template file
sed -i -r 's/\$serial/'"$serial"'/g' nvs/nvs.csv
sed -i -r 's/\$pass/'"$pass"'/g' nvs/nvs.csv

# uses python util to create the binary image with size 0x6000, #! change this if another parition table is used
python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate nvs/nvs.csv nvs/nvs.bin 0x6000