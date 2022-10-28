
all:
	mbed deploy
	mbed compile -t GCC_ARM -m MAX32630FTHR
clean:
	rm -rf BUILD
