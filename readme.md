# RISCV SYNTHESIZABLE DESIGN IN SYSTEMC

## pre - req
### Building RISC-V toolchain -- [official web](https://github.com/riscv-collab/riscv-gnu-toolchain)
In this project, only M(mul) extension is availiable. The following are the commands. Before installing, create a folder for the source code. (~10GB total size)
(please read the official github website for the neweast version)

    git clone https://github.com/riscv/riscv-gnu-toolchain

    sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev

    ./configure --prefix=/opt/riscv --with-arch=rv32im --with-abi=ilp32

    make

Once finish all the steps without error, add bin folder to the PATH.

    export PATH = $PATH:$RISCV/bin


## files arch
bin: top level to run the systemC simulation of the risc-v core.
    ** how to use :**
        python3 update_mem.py fft 
        python3 update_mem.py fft 2400
        python3 update_mem.py jpegen 3600
    ** "fft" is the name of the c program running on risc-v core. "2400" is the word size of L1 cache for ins/data. "jpegen" need at least 3600 word size. ** 

    output: under "pipeline" and "scalar" folder, there will be files "ppoutput.txt" and "scoutput.txt" to print out the varables in "printf" function.


ass2bin:  convert the c code to risc-v binary code and run the simulation in systemc.

bench_cpp: example c program to run on risc-v processor. If the new c program added, the name of folder should be same as the c program file with main function. Please create corresponding make file. ("char,double, long long" are not allowed. Dynamic memory is not supported now. To use "printf" function, please follow the exisited program)

pipeline: 5 stage pipeline risc-v core

scalar: scalar risc-v core

asip: application specific processor
            



