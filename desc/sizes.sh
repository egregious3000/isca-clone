#! /bin/sh

# Assuming a forum info of the proper format, sets the length to the proper value
for file in room*; do
    rawfile=`stat -c %s $file`
    size=$(( ($rawfile - 20 + 3) / 4 * 4))
    # high 6 bits are always "1001 01??", although that doesn't make sense as a hlen
    bits=$((94 + $size % 4))
    size=$(($size / 4))
    size_big=$(($size / 256))
    size_sml=$(($size % 256))
    hex_big=`printf "%02x" $size_big`
    hex_sml=`printf "%02x" $size_sml`
    /bin/echo -ne "\x${bits}\x${hex_sml}\x${hex_big}" | dd of=$file bs=1 seek=5 conv=notrunc
done
