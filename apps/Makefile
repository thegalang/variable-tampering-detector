CC = gcc
CFLAGS = -export-dynamic -fno-stack-protector
SRC = company_app.c
OUT = company_app.out

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OUT)

.PHONY: clean
