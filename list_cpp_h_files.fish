#!/usr/bin/env fish

# Create a temporary file to hold the final output
set temp_file (mktemp)

# Loop through all .cpp and .h files in the src directory
for file in src/*.cpp src/*.h
    # Append the filename in the required format
    echo "$file" >> $temp_file
    echo "```c" >> $temp_file
    
    # Append the content of the file
    cat $file >> $temp_file
    echo "```" >> $temp_file
    echo "" >> $temp_file
end

# Copy the content to the macOS clipboard
pbcopy < $temp_file

# Clean up the temporary file
rm $temp_file