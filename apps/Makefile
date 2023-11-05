CC = gcc
CFLAGS = -export-dynamic -fno-stack-protector
SRC = company_app.c global_write.c
OUT = company_app.out global_write.out

all: global_write.out company_app.out

global_write.out: global_write.cpp
	$(CC) $(CFLAGS) $< -o $@

company_app.out: company_app.c
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f $(OUT)

.PHONY: clean
