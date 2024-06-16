#!/bin/bash

output_dir="res_h"

mkdir -p "$output_dir"

for file in res/*; do
    filename=$(basename -- "$file")
    
    header_name="${filename//./_}.h"
    
    xxd -i "$file" > "$output_dir/$header_name"
done