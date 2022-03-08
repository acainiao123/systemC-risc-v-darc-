RISCC := riscv32-unknown-elf-gcc 
RISFLAG :=  -S -fno-asynchronous-unwind-tables -fno-exceptions -DCHECKPRINT
##flat    -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti
# -fverbose-asm -masm=intel
CC := g++
CCFLAGS := -Wall -DCHECKPRINT -DCHECKWRITE
#  -DCHECKPRINT -DCHECKWRITE
NAME := fft

all: $(NAME).cpp
	$(RISCC) $(RISFLAG) $(NAME).cpp -o $(NAME)assemb.S
	$(CC) $(CCFLAGS) $(NAME).cpp -o test.exe
	./test.exe | tee output.txt

# $(filter %.o,$(obj_files)): %.o: %.c


.PHONY: all, clean

clean:
	rm -f *.exe *.S 