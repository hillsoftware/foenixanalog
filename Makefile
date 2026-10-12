CC = mos-f256-clang
CFLAGS = -T f256.ld -I/home/michael/llvmcc/include/ -Os -Wall -lm

SRC = joytest.c
HEADERS = f256.h
TMP_PGZ = joytest.tmp.pgz
FINAL_PGZ = joytest.pgz

.PHONY: all clean upload send deploy run go

all: $(FINAL_PGZ)

$(TMP_PGZ): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC)

$(FINAL_PGZ): $(TMP_PGZ)
	pgzbuild $(SRC) $(TMP_PGZ) $@
	rm -f $(TMP_PGZ).elf
	rm -f $(TMP_PGZ)

clean:
	rm -f $(TMP_PGZ) $(TMP_PGZ).elf $(FINAL_PGZ)

upload send deploy run go: $(FINAL_PGZ)
	fsend.exe $<

