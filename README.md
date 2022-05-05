# crc32

crc32 for embedded

# Reference


# Compile

make clean && make

# Test

head -c 1M </dev/urandom > ./testfile.bin

# Fast CRC Calculation

./crc32 testfile.bin 

# Slow CRC Calculation with Progressbar

./crc32 -p testfile.bin 

rm -rif testfile.bin 

