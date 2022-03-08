// #include <errno.h>
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/time.h>
// #include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "scriscv_core.h"


void riscv_imemory::icache(void){

std::cout<<"IRAM_SIZEis "<<std::dec<<IRAM_SIZE<<"\n";
    sc_uint<32> data[IRAM_SIZE>>2]/*Cyber array = REG*/ = {
        #include "imem.txt"
    };
std::cout<<"IRAM_SIZEis "<<std::dec<<IRAM_SIZE<<"\n";
    sc_uint<2> imem_opType_imm;
    sc_uint<32> dataOut = 0;
    sc_uint<32> dataIn = 0;

    sc_int<32> temp = 0;
    sc_uint<8> t8 = 0;
    sc_int<1> bit = 0;
    sc_uint<16> t16 = 0;
    sc_uint<32> addr = 0;
    sc_uint<3> mask;
    
    instruction_ft.write(0);
    stallIm.write(0);
    imem_addOut.write(PC_STARTADDR);
    wait();

    while(1){
        
        mask = WORD;
        addr = imem_address.read();
        dataIn  = imem_dataIn.read(); 
        imem_opType_imm = imem_opType.read();
        if(addr == RAADDRESS){
            #ifdef SYSCSIMULATION
            std::cout<<" finish the process return ra \n";
            sc_stop();
            exit(-1);
            #endif
            while(1) {
                stallIm.write(1);
                wait();
            }

        }

        switch(imem_opType_imm){            
            case STORE:
                
                switch (mask) {
                    case BYTE_U:
                    case BYTE:
                        data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3)) =  dataIn.range(7,0);
                        break;
                    case HALF:
                    case HALF_U:
                        data[addr >> 2].range( (addr[1] ? 16 : 0)+16-1, (addr[1] ? 16 : 0)) = dataIn.range(15,0);
                        break;
                    case WORD:
                        data[addr >> 2] = dataIn;
                        break;
                    case LONG:
                        for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
                        // data[(addr >> 2) + oneWord] = dataIn.template slc<32>(32 * oneWord);
                        data[(addr >> 2) + oneWord] = dataIn.range(32*oneWord+32-1,32*oneWord);
                        break;
                }
            break;
            case LOAD:
                switch (mask) {
                    case BYTE:

                        t8  = data[addr >> 2].range((((int)addr.range(1,0)) << 3)+8-1,(((int)addr.range(1,0)) << 3));
                        bit = t8.range(7,7);
                        dataOut.range(0+t8.length()-1,0) = t8;
                        dataOut.range(8+24-1,8) =  (sc_int<24>)bit;
                        break;
                    case HALF:
                        t16 = data[addr >> 2].range((addr[1] ? 16 : 0)+16-1,addr[1] ? 16 : 0);
                        bit = t16.range(15,15);
                        dataOut.range(0+t16.length()-1,0) =  t16;
                        dataOut.range(16+16-1,16) = (sc_int<16>)bit;

                        break;
                    case WORD:
                        dataOut = data[addr >> 2];
                        break;
                    case LONG:
                        for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
                        dataOut.range(32 * oneWord+31-1,32 * oneWord) = data[(addr >> 2) + oneWord];
                        break;
                    case BYTE_U:
                        dataOut = data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3) ) & 0xff;
                        break;
                    case HALF_U:
                        dataOut = data[addr >> 2].range((addr[1] ? 16 : 0)+15,(addr[1] ? 16 : 0))& 0xffff;
                        break;
                    }

            break;
        }


        instruction_ft.write(dataOut);
        stallIm.write(0);
        imem_addOut.write(addr+4);
        wait();

        
    }
}


void riscv_dmemory::dcache(void){
    sc_uint<32> data[DRAM_SIZE>>2]/*Cyber array = REG*/ = {
        #include "dmem.txt"
    };
    sc_uint<32> dataOut = 0;
    sc_uint<32> dataIn = 0;

    sc_int<32> temp = 0;
    sc_uint<8> t8 = 0;
    sc_int<1> bit = 0;
    sc_uint<16> t16 = 0;
    sc_uint<32> addr = 0;

    sc_uint<3> mask;
    sc_uint<2> opType = 0;

    dmem_dataOut.write(0);
    stallDm.write(0);
    wait();
    while(1){
        mask = dmem_mask.read();
        addr = dmem_address.read();
        dataIn  = dmem_dataIn.read(); 
        opType = dmem_opType.read();        
        switch(opType){           
            case STORE:
                
                switch (mask) {
                    case BYTE_U:
                    case BYTE:
                        data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3)) =  dataIn.range(7,0);
                        break;
                    case HALF:
                    case HALF_U:
                        data[addr >> 2].range( (addr[1] ? 16 : 0)+16-1, (addr[1] ? 16 : 0)) = dataIn.range(15,0);
                        break;
                    case WORD:
                        data[addr >> 2] = dataIn;
                        break;
                    case LONG:
                        for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
                        data[(addr >> 2) + oneWord] = dataIn.range(32*oneWord+32-1,32*oneWord);
                        break;
                }
            break;
            case LOAD:
                switch (mask) {
                    case BYTE:

                        t8  = data[addr >> 2].range((((int)addr.range(1,0)) << 3)+8-1,(((int)addr.range(1,0)) << 3));
                        bit = t8.range(7,7);
                        dataOut.range(0+t8.length()-1,0) = t8;
                        dataOut.range(8+24-1,8) =  (sc_int<24>)bit;
                        break;
                    case HALF:
                        t16 = data[addr >> 2].range((addr[1] ? 16 : 0)+16-1,addr[1] ? 16 : 0);
                        bit = t16.range(15,15);
                        dataOut.range(0+t16.length()-1,0) =  t16;
                        dataOut.range(16+16-1,16) = (sc_int<16>)bit;

                        break;
                    case WORD:
                        dataOut = data[addr >> 2];
                        break;
                    case LONG:
                        for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
                        dataOut.range(32 * oneWord+31-1,32 * oneWord) = data[(addr >> 2) + oneWord];
                        // dataOut.set_slc(32 * oneWord, data[(addr >> 2) + oneWord]);
                        break;
                    case BYTE_U:
                        dataOut = data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3) ) & 0xff;
                        // dataOut = data[addr >> 2].slc<8>(((int)addr.slc<2>(0)) << 3) & 0xff;
                        break;
                    case HALF_U:
                        dataOut = data[addr >> 2].range((addr[1] ? 16 : 0)+15,(addr[1] ? 16 : 0))& 0xffff;

                        // dataOut = data[addr >> 2].slc<16>(addr[1] ? 16 : 0) & 0xffff;
                        break;
                    }
                
                    // result_mem.write(dataOut);

            break;
        }

        stallDm.write(0);
        dmem_dataOut.write(dataOut);
        wait();
        //wait();
    }
}

//// cycle 0    1    2        3        5
////      pc   pc+4 pc+8     pc+12
////                ins(pc) ins(pc+4)
void riscv_core::fetch(void){
    sc_uint<32> pc = PC_STARTADDR;

    sc_uint<32> instruction = 0;
    sc_uint<64> cycle = 0;////dummy


    ////****imem****////
    //const 
    sc_uint<2> opType_i = LOAD;
    sc_uint<3> mask_i = WORD;
    ////input and output
    sc_uint<32> dataIn = 0;
    sc_uint<32> dataOut = 0;

    ////local parameters
    bool stallSignals_ft;
    bool localStall_ft;
    bool stallIm_ft;
    bool stallDm_ft;
    bool isBranch_dc_ft;
    bool isBranch_ex_ft;
    sc_uint<32> nextPC_ex_ft = PC_STARTADDR;
    sc_uint<32> nextPC_dc_ft = PC_STARTADDR;
    sc_uint<32> nextPC_ft_ft = PC_STARTADDR;
    sc_uint<2> imem_opType_ft=NONE;
    ////initial
    // instruction_ft.write(0);
    // stallIm.write(0);
    imem_opType.write(NONE);
    imem_address.write(pc);
    we_ft.write(0);
    // pc_ft.write(PC_STARTADDR);
    // nextPC_ft.write(PC_STARTADDR);
    ////localstall  and stallDm
    wait();

    while(1){

            #ifdef SYSCSIMULATION
                cycle++;
                std::cout<<"cycle is "<<cycle<<"\n";
            #endif
            // ////localstall  and stallDm
            stallSignals_ft = stallSignals[STALL_FETCH].read();
            localStall_ft = localStall.read();
            stallIm_ft = stallIm.read();
            stallDm_ft = stallDm.read();
            isBranch_dc_ft = isBranch_dc.read();
            isBranch_ex_ft = isBranch_ex.read();
            nextPC_ex_ft = nextPC_ex.read();
            nextPC_dc_ft = (nextPC_dc.read());
            nextPC_ft_ft = imem_addOut.read();//nextPC_ft.read();

            if(!stallSignals_ft && !localStall_ft && !stallIm_ft && !stallDm_ft){
                if(isBranch_ex_ft)  {
                    pc = nextPC_ex_ft;                    
                }
                else if(isBranch_dc_ft) {
                    pc = (nextPC_dc_ft);
                    
                }
                else{
                    pc = nextPC_ft_ft;
                }
            }

            
            /////
            if (!localStall_ft && !stallDm_ft){
                imem_opType_ft = LOAD; 
            }
            else imem_opType_ft = NONE; 


            imem_opType.write(imem_opType_ft);
            imem_address.write(pc);
            // pc_ft.write(pc);
            // nextPC_ft.write(pc+4);
            we_ft.write(true);
        

        wait();
    }
}


void riscv_core::decode_comb(void){
    sc_uint<32> instruction_ft_dccb = instruction_ft.read();
    
    sc_uint<5> rs2 = instruction_ft_dccb.range(24,20);
    sc_uint<5> rs1 = instruction_ft_dccb.range(19,15);
    valueReg1_dc_pre.write( (sc_uint<32>) registerFile[rs1].read());
    valueReg2_dc_pre.write( (sc_uint<32>) registerFile[rs2].read());

}

void riscv_core::decode(void){
    ////input  dataOut_i,  pc_ft, stallIm,  registers[]  cycle?
    ////output:  all the signals
    ////input
    std::cout<<"start decode "<<std::endl;
    ////output
    sc_uint<7> opCode = 0;
    sc_uint<7> funct7 = 0;
    sc_uint<3> funct3 = 0;
    sc_int<32> lhs = 0;
    sc_int<32> rhs = 0;
    sc_int<32> datac = 0;
    sc_uint<32> pc = 0;
    sc_uint<32> instruction = 0;
    bool isBranch = 0;
    // bool crashFlag = 0;
    
    bool useRs1 = 0;
    bool useRs2 = 0;
    bool useRs3 = 0;
    bool useRd = 0;
    sc_uint<5> rs1 = 0;
    sc_uint<5> rs2 = 0;
    sc_uint<5> rs3 = 0;
    sc_uint<5> rd = 0;
    bool we = 0;


    sc_uint<32> nextPC = 0;

    isBranch_dc = false;
    // crashFlag_dc = false;

    useRs1_dc = false;
    useRs2_dc = false;
    useRs3_dc = false;
    useRd_dc = false;


    ////////buffer
    ////**************////I(immediate+load) instruction
    sc_uint<12> imm12_I = 0;
    sc_uint<12> imm12_S = 0;
    ////**************////S instruction
    sc_int<12> imm12_I_signed = 0;
    sc_int<12> imm12_S_signed = 0;
    ////**************////SB instruction
    sc_uint<13> imm13 = 0;
    sc_int<13> imm13_signed = 0;
    ////**************////U instruction
    sc_int<32> imm31_12 = 0;
    ////**************////UJ instruction
    sc_int<21> imm21_1 = 0;
    sc_int<21> imm21_1_signed = 0;

    ////**************//// ports initial

    // Register access
    sc_uint<32> valueReg1 = 0;
    sc_uint<32> valueReg2 = 0;

    ////local 
    bool stallSignals_ft_dc = 0;
    bool localStall_dc = 0;
    bool stallIm_dc = 0;
    bool stallDm_dc = 0;
    sc_uint<32> instruction_ft_dc = 0;
    sc_uint<32> pc_ft_dc = 0;
    bool isBranch_ex_dc = 0;
    bool isBranch_dc_dc = 0;
    bool we_ft_dc = 0;

    sc_int<32> registerFile_dc[32];
    for(int i=0;i<32;i++) registerFile_dc[i] = 0;

    pc_dc.write(0);
    instruction_dc.write(0);
    opCode_dc.write(0);
    funct7_dc.write(0);
    funct3_dc.write(0);
    nextPC_dc.write(0);
    
    lhs_dc.write(0);
    rhs_dc.write(0);
    // crashFlag_dc.write(0);
    isBranch_dc.write(0);
    datac_dc.write(0);

    useRs1_dc.write(0);
    useRs2_dc.write(0);
    useRs3_dc.write(0);
    useRd_dc.write(0);
    rs3_dc.write(0);
    rs2_dc.write(0);
    rs1_dc.write(0);
    rd_dc.write(0);

    wait();

    while(1)
    {       
            ////////input ports
            // Register access
            stallSignals_ft_dc = stallSignals[STALL_FETCH].read();
            localStall_dc = localStall.read();
            stallIm_dc = stallIm.read();
            stallDm_dc = stallDm.read();
            instruction_ft_dc = instruction_ft.read();
            pc_ft_dc = imem_address.read();//pc_ft.read();
            isBranch_dc_dc = isBranch;//isBranch_dc.read();
            isBranch_ex_dc = isBranch_ex.read();
            we_ft_dc = we_ft.read();
            ////////
            if(!stallSignals_ft_dc && !localStall_dc && !stallIm_dc && !stallDm_dc){
                instruction = instruction_ft_dc;
                pc = pc_ft_dc;
                ///////////////////////////////////////////////branch
                if(isBranch_ex_dc)  {
                    we = (0);
                }
                else if(isBranch_dc_dc) {
                    we = (0);
                }
                else{
                    we = we_ft_dc;////<<<----we_ft_br
                }

                valueReg1 = valueReg1_dc_pre.read();
                valueReg2 = valueReg2_dc_pre.read();
            }
            
            
            isBranch = 0;

            ////**************////I(immediate+load) instruction
            imm12_I = instruction.range(31,20);
            imm12_S = 0;

            ////**************////S instruction
            imm12_S.range(5+7-1,5) = instruction.range(31,25);
            imm12_S.range(0+5-1,0) = instruction.range(11,7);
            imm12_I_signed = (sc_int<12>) instruction.range(31,20);
            imm12_S_signed = 0;
            imm12_S_signed.range(0+12-1,0) = imm12_S.range(11,0);

            ////**************////SB instruction
            imm13[12]               = instruction[31];
            imm13.range(5+6-1,5) = instruction.range(30,25);
            imm13.range(1+4-1,1) = instruction.range(11,8);

            imm13[11] = instruction[7];
            imm13_signed.range(0+imm13.length()-1,0) = imm13;


            ////**************////U instruction
            imm31_12.range(12+20-1,12) = instruction.range(31,12);

            ////**************////UJ instruction        
            imm21_1.range(12+8-1,12) = instruction.range(20-1,12);
            imm21_1[11] = instruction[20];
            imm21_1.range(1+10-1,1) = instruction.range(30,21);
            imm21_1[20] = instruction[31];
            imm21_1_signed.range(0+imm21_1.length()-1,0) = imm21_1;

            
            funct7 = instruction.range(31,25);
            rs2 = instruction.range(24,20);
            rs1 = instruction.range(19,15);
            rs3 = rs2;
            funct3 = instruction.range(14,12);
            rd = instruction.range(11,7);
            opCode = instruction.range(6,0); // could be reduced to 5 bits because 1:0 is always 11

            // Initialization of control bits
            useRs1   = 0;
            useRs2   = 0;
            useRs3   = 0;
            useRd    = 0;
            // we       = we_ft.read();
            

            isBranch = 0;
            switch (opCode) {////opCode_dc
                case RISCV_LUI:
                    lhs = imm31_12;
                    useRs1 = (0);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);

                    break;
                case RISCV_AUIPC:
                    lhs = pc;
                    rhs = imm31_12;
                    useRs1 = (0);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);
                    break;
                case RISCV_JAL:
                    lhs = pc + 4;
                    rhs = 0;
                    nextPC = pc + imm21_1_signed;
                    useRs1 = (0);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);
                    isBranch = 1;

                    break;
                case RISCV_JALR:
                    lhs = valueReg1;
                    rhs = imm12_I_signed;
                    useRs1 = (1);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);
                    break;
                case RISCV_BR:

                    lhs = valueReg1;
                    rhs = valueReg2;
                    useRs1 = (1);
                    useRs2 = (1);
                    useRs3 = (0);
                    useRd = (0);

                    break;
                case RISCV_LD:

                    lhs = valueReg1;
                    rhs = imm12_I_signed;
                    useRs1 = (1);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);
                    break;

                case RISCV_ST:
                    lhs = valueReg1;
                    rhs = imm12_S_signed;
                    datac = valueReg2; // Value to store in memory
                    useRs1 = (1);
                    useRs2 = (0);
                    useRs3 = (1);
                    useRd = (0);
                    rd     = 0;
                    break;
                case RISCV_OPI:
                    lhs = valueReg1;
                    rhs = imm12_I_signed;
                    useRs1 = (1);
                    useRs2 = (0);
                    useRs3 = (0);
                    useRd = (1);
                    break;

                case RISCV_OP:

                    lhs = valueReg1;
                    rhs = valueReg2;
                    useRs1 = (1);
                    useRs2 = (1);
                    useRs3 = (0);
                    useRd = (1);

                    break;
                case RISCV_SYSTEM:////not used?
                

                    break;
                default:
                    useRs1 = (0);
                    useRs2 = (0);
                    // std::cout<<" instruction error and break out \n";
                    // std::cout<<std::hex<<instruction;
                    if (instruction != 0)
                        // crashFlag = true;
                        // sc_stop();

                    break;
            }

            // If dest is zero, useRd should be at zero
            if (rd == 0) {
                useRd = 0;
            }

            // If the instruction was dropped, we ensure that isBranch is at zero
            if (!we) {
                isBranch = 0;
                useRd    = 0;
                useRs1   = 0;
                useRs2   = 0;
                useRs3   = 0;
            }

            ////////output ports
            instruction_dc.write(instruction);
            pc_dc.write(pc);

            lhs_dc.write(lhs);
            rhs_dc.write(rhs);
            // crashFlag_dc.write(crashFlag);
            isBranch_dc.write(isBranch);
            datac_dc.write(datac);

            nextPC_dc.write(nextPC);

            useRs1_dc.write(useRs1);
            useRs2_dc.write(useRs2);
            useRs3_dc.write(useRs3);
            useRd_dc.write(useRd);

            funct7_dc.write(funct7);
            rs3_dc.write(rs3);
            rs2_dc.write(rs2);
            rs1_dc.write(rs1);
            funct3_dc.write(funct3);
            rd_dc.write(rd);
            opCode_dc.write(opCode); // could be reduced to 5 bits because 1:0 is always 11

        

            we_dc.write(we);
        
        wait();
    }
}


void riscv_core::exec(void){
    ////input:
    ////input::
        // forwardRegisters   stallSignals  stallIm=waitOut_i  stallDm=waitOut_d  localStall
    // std::cout<<"start exec "<<std::endl;
    bool stallSignals_dc_ex = 0;
    bool stallSignals_ex_ex = 0;
    bool stallSignals_ft_ex = 0;

    bool localStall_ex = 0;
    bool stallIm_ex = 0;
    bool stallDm_ex = 0;
    bool ForwardReg_EXval1_ex = 0;
    bool ForwardReg_EXval2_ex = 0;
    bool ForwardReg_EXval3_ex = 0;
    
    bool ForwardReg_MMval1_ex = 0;
    bool ForwardReg_MMval2_ex = 0;
    bool ForwardReg_MMval3_ex = 0;
    
    bool ForwardReg_WBval1_ex = 0;
    bool ForwardReg_WBval2_ex = 0;
    bool ForwardReg_WBval3_ex = 0;
    

    // bool we_ex_ex = 0;
    bool we_mem_ex = 0;
    bool we_wb_ex = 0;
    sc_int<32> result_ex_ex_dc = 0;
    sc_int<32> result_mem_mem_dc = 0;
    sc_int<32> value_wb_dc = 0;
    sc_int<32> lhs_dc_dc = 0;
    sc_int<32> rhs_dc_dc = 0;
    

    sc_uint<7> opCode_dc_ex = 0;
    sc_uint<7> funct7_dc_ex = 0;
    sc_uint<3> funct3_dc_ex = 0;
    sc_int<32> datac_dc_cd = 0;
    sc_uint<32> pc_dc_ex = 0;
    sc_uint<32> instruction_dc_ex = 0;
    bool we_dc_ex = 0;
    sc_uint<5> rd_dc_ex = 0;
    bool useRd_dc_ex = 0;
    bool isBranch_ex_ex = 0;
    bool we_ex_dc = 0;

    
    ////local
    sc_uint<32> pc = 0;////extoMem.pc
    sc_uint<32> instruction=0;
    sc_uint<7> opCode = 0;
    sc_uint<5> rd = 0;
    sc_uint<3> funct3 = 0;
    sc_uint<7> funct7 = 0;
    sc_int<32> lhs = 0;   //  left hand side : operand 1
    sc_int<32> rhs = 0;   // right hand side : operand 2
    sc_int<32> datac = 0;

    // bool isBranch = 0;
    // bool useRs3 = 0;
    bool useRd = 0;    
    bool we = 0;


    ////output
    pc_ex_comb.write(0);////extoMem.pc
    instruction_ex_comb.write(0);
    useRd_ex_comb.write(false);
    we_ex_comb.write(false);
    lhs_ex_comb.write(0);   //  left hand side : operand 1
    rhs_ex_comb.write(0);   // right hand side : operand 2
    datac_ex_comb.write(0);
    opCode_ex_comb.write(0);
    rd_ex_comb.write(0);
    funct3_ex_comb.write(0);
    funct7_ex_comb.write(0);
    


    
    
    wait();
    
    while(1){
        stallSignals_dc_ex = stallSignals[STALL_DECODE].read();
        stallSignals_ex_ex = stallSignals[STALL_EXECUTE].read();
        stallSignals_ft_ex = stallSignals[STALL_FETCH].read();

        localStall_ex = localStall.read();
        stallIm_ex = stallIm.read();
        stallDm_ex = stallDm.read();
        ForwardReg_EXval1_ex = ForwardReg_EXval1.read();
        ForwardReg_EXval2_ex = ForwardReg_EXval2.read();
        ForwardReg_EXval3_ex = ForwardReg_EXval3.read();
        ForwardReg_MMval1_ex = ForwardReg_MMval1.read();
        ForwardReg_MMval2_ex = ForwardReg_MMval2.read();
        ForwardReg_MMval3_ex = ForwardReg_MMval3.read();
        ForwardReg_WBval1_ex = ForwardReg_WBval1.read();
        ForwardReg_WBval2_ex = ForwardReg_WBval2.read();
        ForwardReg_WBval3_ex = ForwardReg_WBval3.read();

        we_ex_dc = we_ex.read();
        we_mem_ex = we_mem.read();
        we_wb_ex = we_wb.read();

        result_ex_ex_dc = result_ex_ex.read();
        result_mem_mem_dc = result_mem_mem.read();

        value_wb_dc = value_wb.read();
        lhs_dc_dc = lhs_dc.read();
        rhs_dc_dc = rhs_dc.read();
        datac_dc_cd = datac_dc.read();

        opCode_dc_ex = (opCode_dc.read());
        funct7_dc_ex = (funct7_dc.read());
        funct3_dc_ex = (funct3_dc.read());
        rd_dc_ex = (rd_dc.read());
        pc_dc_ex = (pc_dc.read());
        instruction_dc_ex = (instruction_dc.read());
        we_dc_ex = (we_dc.read());
        useRd_dc_ex = (useRd_dc.read());
        isBranch_ex_ex = isBranch_ex.read();


        ////output
        if (!stallSignals_dc_ex && !localStall_ex && !stallIm_ex && !stallDm_ex){
            
            ////////*****************forwarding*****************/////////////
            if (ForwardReg_EXval1_ex && we_ex_dc){
                // std::cout<<" forwardExtoVal1 "<<"\n";
                lhs = result_ex_ex_dc;
            }
            else if (ForwardReg_MMval1_ex && we_mem_ex){
                // std::cout<<" forwardMemtoVal1 "<<"\n";
                lhs = result_mem_mem_dc;
            }
            else if (ForwardReg_WBval1_ex && we_wb_ex){
                // std::cout<<" forwardWBtoVal1 "<<"\n";
                lhs = value_wb_dc;
            }
            else{
                lhs = lhs_dc_dc;
            }           
            if (ForwardReg_EXval2_ex && we_ex_dc){
                rhs = result_ex_ex_dc;
            }
            else if (ForwardReg_MMval2_ex && we_mem_ex){
                rhs = result_mem_mem_dc;
            }
            else if (ForwardReg_WBval2_ex && we_wb_ex){
                rhs = value_wb_dc;
            }
            else{
                rhs = rhs_dc_dc;
            }
            if (ForwardReg_EXval3_ex && we_ex_dc)
                datac = result_ex_ex_dc;
            else if (ForwardReg_MMval3_ex && we_mem_ex)
                datac = result_mem_mem_dc;
            else if (ForwardReg_WBval3_ex && we_wb_ex)
                datac = value_wb_dc;
            else
                datac = datac_dc_cd;

            opCode = (opCode_dc_ex);
            funct7 = (funct7_dc_ex);
            funct3 = (funct3_dc_ex);

            rd = (rd_dc_ex);

            pc = (pc_dc_ex);
            instruction = (instruction_dc_ex);
            
            we = (we_dc_ex);

            useRd = (useRd_dc_ex);
        }	
        else{
            pc = pc;
            instruction = instruction;
            useRd = useRd;
            we = we;

        }
        if (stallSignals_dc_ex && !stallSignals_ex_ex && !localStall_ex && !stallIm_ex && !stallDm_ex){
            we = 0;
            useRd = 0;
            instruction = 0;
            pc = 0;
            // isBranch = 0;

        }

        ///////////////////////////////////////////////branch
        if(!stallSignals_ft_ex && !localStall_ex && !stallIm_ex && !stallDm_ex){
            ////////branch
            if(isBranch_ex_ex)  we = (0);
        }


        pc_ex_comb.write(pc);////extoMem.pc
        instruction_ex_comb.write(instruction);
        useRd_ex_comb.write(useRd);
        we_ex_comb.write(we);
        lhs_ex_comb.write(lhs);   //  left hand side : operand 1
        rhs_ex_comb.write(rhs);   // right hand side : operand 2
        datac_ex_comb.write(datac);
        opCode_ex_comb.write(opCode);
        rd_ex_comb.write(rd);
        funct3_ex_comb.write(funct3);
        funct7_ex_comb.write(funct7);




        wait();
        //wait();
    }

}

void riscv_core::exe_wecomb(void){
    //// all the signals
    sc_uint<32> pc = 0;////extoMem.pc
    sc_uint<32> instruction=0;
    sc_uint<7> opCode = 0;
    sc_uint<5> rd = 0;
    sc_uint<3> funct3 = 0;
    sc_uint<7> funct7 = 0;
    sc_int<32> lhs = 0;   //  left hand side : operand 1
    sc_int<32> rhs = 0;   // right hand side : operand 2
    sc_int<32> datac = 0;

    bool isBranch = 0;
    // bool useRs3 = 0;
    bool useRd = 0;    
    bool we = 0;

    bool jalrcus = 0;
    sc_uint<3> jalcustype = 0;
    
    bool isLongInstruction = 0;

    sc_uint<32> nextPC = 0;////extoMem.nextpc
    sc_int<32> result = 0;
      
    ////buffer
    sc_uint<13> imm13 = 0;
    sc_int<13> imm13_signed = 0;
    sc_uint<5> shamt = 0;

    ////buffer
    // bool stallSignals_dc_ex = 0;
    // bool stallSignals_ex_ex = 0;
    // bool stallSignals_ft_ex = 0;

    // bool localStall_ex = 0;
    // bool stallIm_ex = 0;
    // bool stallDm_ex = 0;
    // bool ForwardReg_EXval1_ex = 0;
    // bool ForwardReg_EXval2_ex = 0;
    // bool ForwardReg_EXval3_ex = 0;
    
    // bool ForwardReg_MMval1_ex = 0;
    // bool ForwardReg_MMval2_ex = 0;
    // bool ForwardReg_MMval3_ex = 0;
    
    // bool ForwardReg_WBval1_ex = 0;
    // bool ForwardReg_WBval2_ex = 0;
    // bool ForwardReg_WBval3_ex = 0;
    

    // bool we_ex_ex = 0;
    // bool we_mem_ex = 0;
    // bool we_wb_ex = 0;
    sc_int<32> result_ex_ex_dc = 0;
    sc_int<32> result_mem_mem_dc = 0;
    sc_int<32> value_wb_dc = 0;
    sc_int<32> lhs_dc_dc = 0;
    sc_int<32> rhs_dc_dc = 0;
    

    sc_uint<7> opCode_dc_ex = 0;
    sc_uint<7> funct7_dc_ex = 0;
    sc_uint<3> funct3_dc_ex = 0;
    sc_int<32> datac_dc_cd = 0;
    sc_uint<32> pc_dc_ex = 0;
    sc_uint<32> instruction_dc_ex = 0;
    // bool we_dc_ex = 0;
    sc_uint<5> rd_dc_ex = 0;
    // bool useRd_dc_ex = 0;
    // bool isBranch_ex_ex = 0;
    // bool we_ex_dc = 0;
    

    ////comb
    jalrcus = 0;
    jalcustype = 0;

    pc = pc_ex_comb.read();////extoMem.pc
    instruction = instruction_ex_comb.read();
    useRd = useRd_ex_comb.read();
    we = we_ex_comb.read();
    lhs = lhs_ex_comb.read();   //  left hand side : operand 1
    rhs = rhs_ex_comb.read();   // right hand side : operand 2
    datac = datac_ex_comb.read();
    opCode = opCode_ex_comb.read();
    rd = rd_ex_comb.read();
    funct3 = funct3_ex_comb.read();
    funct7 = funct7_ex_comb.read();


    isLongInstruction = 0;
    isBranch = 0;

    imm13[12]               = instruction[31];
    imm13.range(5+6-1,5) = instruction.range(25+6-1,25);
    imm13.range(1+4-1,1) = instruction.range(4+8-1,8);
    imm13[11] = instruction[7];

    imm13_signed.range(0+imm13.length()-1,0) = imm13;

    shamt = instruction.range(24,20);


    // case
    switch (opCode) {
    case RISCV_LUI:
        result = lhs;
        break;
    case RISCV_AUIPC:
        result = lhs + rhs;
        break;
    case RISCV_JAL:
        result = lhs;
        break;
    case RISCV_JALR:
        if(funct3 == 0){
            // Note: in current version, the addition is made in the decode stage
            // The value to store in rd (pc+4) is stored in lhs

            nextPC   = rhs + lhs;

            isBranch = 1;

            result = pc + 4;
        }
        else if(funct3==1){
            jalrcus = 1;
            isBranch = 0;
            jalcustype = 1;
        }
        else if(funct3==2){
            jalrcus = 1;
            isBranch = 0;
            jalcustype = 2;
        }
        
        break;
    case RISCV_BR:
        nextPC = pc + imm13_signed;

        // std::cout<<" RISCV_BR imm13_signed is "<<imm13_signed<<" "<<pc<<" "<<nextPC<<"\n";


        switch (funct3) {
        case RISCV_BR_BEQ:
            isBranch = (lhs == rhs);
            break;
        case RISCV_BR_BNE:
            isBranch = (lhs != rhs);
            break;
        case RISCV_BR_BLT:
            isBranch = (lhs < rhs);
            break;
        case RISCV_BR_BGE:
            isBranch = (lhs >= rhs);
            break;
        case RISCV_BR_BLTU:
            // extoMem.isBranch = ((ac_int<32, false>)dctoEx.lhs < (ac_int<32, false>)dctoEx.rhs);
            isBranch = ((sc_uint<32>)lhs < (sc_uint<32>)rhs);
            break;
        case RISCV_BR_BGEU:
            // extoMem.isBranch = ((ac_int<32, false>)dctoEx.lhs >= (ac_int<32, false>)dctoEx.rhs);
            isBranch = ((sc_uint<32>)lhs >= (sc_uint<32>)rhs);
            break;
        }

        break;
    case RISCV_LD:
        isLongInstruction = 1;
        result            = lhs + rhs;
        break;
    case RISCV_ST:
        datac  = datac;
        result = lhs + rhs;
        break;
    case RISCV_OPI:
        switch (funct3) {
        case RISCV_OPI_ADDI:
            result = lhs + rhs;
            break;
        case RISCV_OPI_SLTI:
            result = lhs < rhs;
            break;
        case RISCV_OPI_SLTIU:
            // extoMem.result = (ac_int<32, false>)dctoEx.lhs < (ac_int<32, false>)dctoEx.rhs;
            result = (sc_uint<32>)lhs < (sc_uint<32>)rhs;
            break;
        case RISCV_OPI_XORI:
            result = lhs ^ rhs;
            break;
        case RISCV_OPI_ORI:
            result = lhs | rhs;
            break;
        case RISCV_OPI_ANDI:
            result = lhs & rhs;
            break;
        case RISCV_OPI_SLLI: // cast rhs as 5 bits, otherwise generated hardware is 32 bits
            result = lhs << (sc_uint<5>)rhs;
            break;
        case RISCV_OPI_SRI:
            if (funct7.range(5,5)) // SRAI
            result = lhs >> (sc_uint<5>)shamt;
            else // SRLI
            result = (sc_uint<32>)lhs >> (sc_uint<5>)shamt;
            break;
        }
        break;
    case RISCV_OP:
        if (funct7.range(0,0)) // M Extension
        {
        } else {
        switch (funct3) {
            case RISCV_OP_ADD:
            if (funct7.range(5,5)) // SUB
                result = lhs - rhs;
            else // ADD
                result = lhs + rhs;
            break;
            case RISCV_OP_SLL:
            result = lhs << (sc_uint<5>)rhs;
            break;
            case RISCV_OP_SLT:
            result = lhs < rhs;
            break;
            case RISCV_OP_SLTU:
            result = (sc_uint<32>)lhs < (sc_uint<32>)rhs;
            break;
            case RISCV_OP_XOR:
            result = lhs ^ rhs;
            break;
            case RISCV_OP_SR:
            if (funct7.range(5,5)) // SRA
                result = lhs >>  (sc_uint<5>)rhs;
            else // SRL
                result = (sc_uint<32>)lhs >>  (sc_uint<5>)rhs;
            break;
            case RISCV_OP_OR:
            result = lhs | rhs;
            break;
            case RISCV_OP_AND:
            result = lhs & rhs;
            break;
        }
        }
        break;
    case RISCV_MISC_MEM: // this does nothing because all memory accesses are
                            // ordered and we have only one core
        break;

    case RISCV_SYSTEM:
        switch (funct3) { // case 0: mret instruction, dctoEx.memValue
                                // should be 0x302
        case RISCV_SYSTEM_ENV:
            break;
        case RISCV_SYSTEM_CSRRW:       // lhs is from csr, rhs is from reg[rs1]
            datac  = rhs; // written back to csr
            result = lhs; // written back to rd
            break;
        case RISCV_SYSTEM_CSRRS:
            datac  = lhs | rhs;
            result = lhs;
            break;
        case RISCV_SYSTEM_CSRRC:
            // extoMem.datac  = dctoEx.lhs & ((ac_int<32, false>)~dctoEx.rhs);
            datac  = lhs & ((sc_uint<32>)~rhs);
            
            result = lhs;
            break;
        case RISCV_SYSTEM_CSRRWI:
            datac  = rhs;
            result = lhs;
            break;
        case RISCV_SYSTEM_CSRRSI:
            datac  = lhs | rhs;
            result = lhs;
            break;
        case RISCV_SYSTEM_CSRRCI:
            // extoMem.datac  = dctoEx.lhs & ((ac_int<32, false>)~dctoEx.rhs);
            datac  = lhs & ((sc_uint<32>)~rhs);
            result = lhs;
            break;
        }
        break;
    }

    // If the instruction was dropped, we ensure that isBranch is at zero
    if (!we) {
        isBranch = (0);
        useRd = (0);
    }


    isBranch_ex.write(isBranch);
    useRd_ex.write(useRd);
    we_ex.write(we);
    
    result_ex.write(result);    
    datac_ex.write(datac);
    nextPC_ex.write(nextPC);
    pc_ex.write(pc);
    instruction_ex.write(instruction);
    rd_ex.write(rd);
    funct3_ex.write(funct3);
    opCode_ex.write(opCode);   
    isLongInstruction_ex.write(isLongInstruction);


    jalrcus_ex.write(jalrcus);
    jalcustype_ex.write(jalcustype);

}

void riscv_core::mult_res(void){
    if(multUsed.read()) result_ex_ex.write( (sc_int<32>) multResult.read());
    else result_ex_ex.write(result_ex.read());
}

////cmethod registers <- stall when exe/mult/div take multiple cycles
void riscv_core::glbmet_regs(void){
    ////sensitive list <-  stallMultAlu 
    bool glb_localStall = 0;
    glb_localStall = 0;
    glb_localStall |= stallMultAlu.read();
    localStall.write(glb_localStall);
}

void riscv_core::mult(void){
    ////input

    sc_uint<7> opCode = 0;
    sc_uint<7> funct7 = 0;
    sc_int<32> lhs = 0;
    sc_int<32> rhs = 0;
    sc_uint<3> funct3 = 0;

    ////output
    bool stall = false;
    sc_uint<32> result = 0;
    bool valRet = false;

    ////////buffer
    sc_uint<32> quotient = 0,remainder = 0; 
    sc_uint<6> state = 0;

    bool resIsNeg = 0;
    int i = 0;
    sc_uint<32> dataAUnsigned=0, dataBUnsigned=0;
    bool forward_minus_minus = false, forward_minus_plus = false, forward_plus_minus = false;
    bool forward_result_unsigned = false, forward_result_signed = false;

    ////////local
    bool stallSignals_dc_mul = 0;
    bool localStall_mul = 0;
    bool stallIm_mul = 0;
    bool stallDm_mul = 0;
    bool ForwardReg_EXval1_mul = 0;
    bool ForwardReg_EXval2_mul = 0;
    // bool ForwardReg_EXval3_mul = 0;
    bool ForwardReg_MMval1_mul = 0;
    bool ForwardReg_MMval2_mul = 0;
    // bool ForwardReg_MMval3_mul = 0;
    bool ForwardReg_WBval1_mul = 0;
    bool ForwardReg_WBval2_mul = 0;
    // bool ForwardReg_WBval3_mul = 0;
    sc_int<32> result_ex_mul = 0;
    sc_int<32> result_mem_mem_mul = 0;
    sc_int<32> value_wb_mul = 0;
    sc_int<32> lhs_dc_mul = 0;
    sc_int<32> rhs_dc_mul = 0;
    sc_uint<7> opCode_dc_mul = 0;
    sc_uint<7> funct7_dc_mul = 0;
    sc_uint<3> funct3_dc_mul = 0;
    bool we_ex_mul = 0;
    bool we_mem_mul = 0;
    bool we_wb_mul = 0;
    
    
        

    ////////initial
    stallMultAlu.write(0);
    multResult.write(0);
    multUsed.write(0);
    wait();

    while(1){
        valRet = false;
        stall = false;

        stallSignals_dc_mul = stallSignals[STALL_DECODE].read();
        localStall_mul = localStall.read();
        stallIm_mul = stallIm.read();
        stallDm_mul = stallDm.read();
        ForwardReg_EXval1_mul = ForwardReg_EXval1.read();
        ForwardReg_EXval2_mul = ForwardReg_EXval2.read();
        // ForwardReg_EXval3_mul = ForwardReg_EXval3.read();
        ForwardReg_MMval1_mul = ForwardReg_MMval1.read();
        ForwardReg_MMval2_mul = ForwardReg_MMval2.read();
        // ForwardReg_MMval3_mul = ForwardReg_MMval3.read();
        ForwardReg_WBval1_mul = ForwardReg_WBval1.read();
        ForwardReg_WBval2_mul = ForwardReg_WBval2.read();
        // ForwardReg_WBval3_mul = ForwardReg_WBval3.read();
        result_ex_mul = result_ex.read();
        result_mem_mem_mul = result_mem_mem.read();
        value_wb_mul = value_wb.read();
        lhs_dc_mul = lhs_dc.read();
        rhs_dc_mul = rhs_dc.read();        
        opCode_dc_mul = (opCode_dc.read());
        funct7_dc_mul = (funct7_dc.read());
        funct3_dc_mul = (funct3_dc.read());
        we_ex_mul = we_ex.read();
        we_mem_mul = we_mem.read();
        we_wb_mul = we_wb.read();
        
        if (!stallSignals_dc_mul && !localStall_mul && !stallIm_mul && !stallDm_mul){
            opCode = opCode_dc_mul;
            funct7 = funct7_dc_mul;
            funct3 = funct3_dc_mul;
            ////////*****************forwarding*****************/////////////
            if (ForwardReg_EXval1_mul && we_ex_mul){
                lhs = result_ex_mul;
            }
            else if (ForwardReg_MMval1_mul && we_mem_mul){
                lhs = result_mem_mem_mul;
            }
            else if (ForwardReg_WBval1_mul && we_wb_mul){
                lhs = value_wb_mul;
            }
            else{
                lhs = lhs_dc;
            }           
            if (ForwardReg_EXval2_mul && we_ex_mul){
                rhs = result_ex_mul;
            }
            else if (ForwardReg_MMval2_mul && we_mem_mul){
                rhs = result_mem_mem_mul;
            }
            else if (ForwardReg_WBval2_mul && we_wb_mul){
                rhs = value_wb_mul;
            }
            else{
                rhs = rhs_dc_mul;
            }
        }

        
        if (opCode == RISCV_OP && funct7 == RISCV_OP_M) {
            if (state == 0) {
            
                forward_result_unsigned |= (dataAUnsigned ^ lhs) | (dataBUnsigned ^ rhs);
                forward_result_unsigned = !forward_result_unsigned;
                
                forward_minus_minus |= (dataAUnsigned ^ -lhs) | (dataBUnsigned ^ -rhs);
                forward_minus_minus = !forward_minus_minus;
                
                forward_minus_plus |= (dataAUnsigned ^ -lhs) | (dataBUnsigned ^ rhs);
                forward_minus_plus = !forward_minus_plus;
                
                forward_plus_minus |= (dataAUnsigned ^ lhs) | (dataBUnsigned ^ -rhs);
                forward_plus_minus = !forward_plus_minus;
                forward_result_signed = forward_minus_minus | forward_minus_plus | forward_plus_minus | forward_result_unsigned;
            
                //   dataAUnsigned.set_slc(0, dctoEx.lhs);
                //   dataBUnsigned.set_slc(0, dctoEx.rhs);
                dataAUnsigned.range(0+lhs.length()-1,0) = lhs;
                dataBUnsigned.range(0+rhs.length()-1,0) = rhs;

                // mult results
                //   ac_int<64, false> resultU  = dataAUnsigned * dataBUnsigned;
                //   ac_int<64, false> resultS  = dctoEx.lhs * dctoEx.rhs;
                //   ac_int<64, false> resultSU = dctoEx.lhs * dataBUnsigned;
                // sc_uint<64> resultU  = dataAUnsigned * dataBUnsigned;
                // sc_uint<64> resultS  = lhs * rhs;
                // sc_uint<64> resultSU = lhs * dataBUnsigned;
                resIsNeg                   = lhs[31] ^ rhs[31];

                switch (funct3) {
                    case RISCV_OP_M_MUL:{
                    //   result = resultS.slc<32>(0);
                        sc_uint<64> resultS  = lhs * rhs;
                        result = resultS.range(31,0);
                        valRet = true;
                    }
                    break;
                    case RISCV_OP_M_MULH:{
                    //   result = resultS.slc<32>(32);
                        sc_uint<64> resultS  = lhs * rhs;
                        result = resultS.range(63,32);
                        valRet = true;
                    }
                    break;
                    case RISCV_OP_M_MULHSU:{
                        sc_uint<64> resultSU = lhs * dataBUnsigned;

                        result = resultSU.range(63,32);
                        valRet = true;
                    }
                    break;
                    case RISCV_OP_M_MULHU:{
                        sc_uint<64> resultU  = dataAUnsigned * dataBUnsigned;
                        result = resultU.range(63,32);
                        valRet = true;
                    }
                    break;
                    case RISCV_OP_M_DIV:
                        if (dataBUnsigned == 0) { // division by zero
                            result = -1;
                            valRet = true;
                        } else if (dataAUnsigned == 0x80000000 && dataBUnsigned == 0xfffffffc){ // Overflow 32 bits
                            result = 0x80000000;
                            valRet = true;
                        } 
                        else {
                            #ifdef SYSCSIMULATION
                                std::cout<<"start div "<<dataAUnsigned<<" div "<<dataBUnsigned<<"\n";
                            #endif

                            if (resIsNeg)
                                result = -quotient;
                            else
                                result = quotient;
                            valRet = true;

                            state     = 32;
                            quotient  = 0;
                            remainder = 0;
                            if (lhs[31]) {
                                dataAUnsigned = -lhs;///convert from negative to postive
                            }
                            if (rhs[31]) {
                            dataBUnsigned = -rhs;///convert from negative to postive
                            }
                        }
                    break;
                    case RISCV_OP_M_DIVU:
                        if (dataBUnsigned == 0) { // division by zero
                            result = -1;
                            valRet = true;
                        } else if (forward_result_unsigned){
                            result = quotient;
                            valRet = true;
                        } else {
                            state     = 32;
                            quotient  = 0;
                            remainder = 0;
                        }
                    break;
                    case RISCV_OP_M_REM:
                        if (dataBUnsigned == 0) { // division by zero
                            result = dataAUnsigned;
                            valRet = true;
                        } else if (dataAUnsigned == 0x80000000 && dataBUnsigned == 0xfffffffc){ // Overflow
                            result = 0;
                            valRet = true;
                        } else if(forward_result_signed){
                            if (lhs[31])
                                result = -remainder;
                            else
                                result = remainder;
                            valRet = true;
                        } else {
                            state     = 32;
                            quotient  = 0;
                            remainder = 0;
                            if (lhs[31]) {
                                dataAUnsigned = -lhs;
                            }
                            if (rhs[31]) {
                                dataBUnsigned = -rhs;
                            }
                        }
                    break;
                    case RISCV_OP_M_REMU:
                        if (dataBUnsigned == 0) { // division by zero
                            result = dataAUnsigned;
                            valRet = true;
                        } else if(forward_result_unsigned) {
                            result = remainder;
                            valRet = true;
                        } else {
                            state     = 32;
                            quotient  = 0;
                            remainder = 0;
                        }
                    break;
                    default:
                    break;
                }
            } 
            else 
            {   
                // #ifdef SYSCSIMULATION
                //     std::cout<<"div rem "<<dataAUnsigned<<" div "<<dataBUnsigned<<"\n";
                // #endif
                // Loop for the division
                for (i = 0; i < 4; i++) {
                    state--;
                    remainder    = remainder << 1;
                    remainder[0] = dataAUnsigned[state];
                    if (remainder >= dataBUnsigned) {
                    remainder       = remainder - dataBUnsigned;
                    quotient[state] = 1;
                    }
                }
                if (state == 0) {
                switch (funct3) {
                    case RISCV_OP_M_DIV:
                        if (resIsNeg)
                        result = -quotient;
                        else
                        result = quotient;
                        valRet = true;
                        #ifdef SYSCSIMULATION
                            std::cout<<"div rem "<<dataAUnsigned<<" div "<<dataBUnsigned<<"div res "<<result<<"\n";
                        #endif
                        break;
                    case RISCV_OP_M_DIVU:
                        result = quotient;
                        valRet = true;
                        break;
                    case RISCV_OP_M_REM:
                        if (lhs[31])
                        result = -remainder;
                        else
                        result = remainder;
                        valRet = true;
                        break;
                    case RISCV_OP_M_REMU:
                        result = remainder;
                        valRet = true;
                        break;
                    default:
                    break;
                }
                }
            }
            ////assume multiplication is 1 latency
            stall |= (state != 0);
        }
        else{
            stall = 0;
            valRet = 0;
        }

        
        multUsed.write(valRet);
        stallMultAlu.write(stall);
        multResult.write(result);
        // std::cout<<" state is "<<state<<" funct3 is "<<funct3<<" opCode is  "<<opCode<<" funct7 "<<funct7<<" multUsed "<<valRet<<" stallMultAlu "<<stall<<" result "<<result<<"\n";
        wait();
    }
}




void riscv_core::mems(void){

    bool we = 0;
    bool useRd = 0;
    sc_int<32> result = 0;
    sc_uint<5> rd = 0;
    sc_uint<3> funct3 = 0;
    sc_uint<7> opCode = 0;
    sc_int<32> datac = 0;
    ////output
    sc_int<32> address = 0;
    bool isLoad = 0;
    bool isStore = 0;
    sc_uint<32> valueToWrite = 0;
    sc_uint<4> byteEnable = 0;

    bool jalrcus=0;
    sc_uint<3> jalcustype=0;
    ////////buffer

    sc_uint<32> mem_read = 0;
    ////dmem
    sc_uint<2> opType_d = 0;
    sc_uint<32> dataIn = 0;
    sc_uint<32> dataOut = 0;
    sc_uint<32> addr = 0;
    
    /////localbuffer
    bool stallSignals_ex_mem = 0;
    bool stallSignals_mem_mem = 0;
    bool localStall_mem = 0;
    bool stallIm_mem = 0;
    bool stallDm_mem = 0;
    sc_int<32> result_ex_ex_mem = 0;
    sc_uint<5> rd_ex_mem = 0;
    bool useRd_ex_mem = 0;
    sc_uint<7> opCode_ex_mem = 0;
    sc_uint<3> funct3_ex_mem = 0;
    sc_int<32> datac_ex_mem = 0;
    bool we_ex_mem = 0;
    bool jalrcus_ex_mem;
    sc_uint<3> jalcustype_mem_mem;
    
    

    sc_uint<3> mask = 0;
    ////
    ////initial
    we_mem.write(0);
    result_mem.write(0);
    useRd_mem.write(0);

    dmem_flag.write(0);

    bool mem_flag = 0;

    rd_mem.write(0);
    address_mem.write(0);
    isLoad_mem.write(0);
    isStore_mem.write(0);
    valueToWrite_mem.write(0);
    byteEnable_mem.write(0);

    dmem_address.write(0);
    dmem_mask.write(0);
    dmem_opType.write(0);
    dmem_dataIn.write(0);

    // stallDm.write(false);
    jalrcus_mem.write(0);
    jalcustype_mem.write(0);
    // std::cout<<"end run mem inital"<<addr<<std::endl;

    wait(); 

    while(1){

            stallSignals_ex_mem = stallSignals[STALL_EXECUTE].read();
            stallSignals_mem_mem = stallSignals[STALL_MEMORY].read();
            localStall_mem = localStall.read();
            stallIm_mem = stallIm.read();
            stallDm_mem = stallDm.read();
            result_ex_ex_mem = (result_ex_ex.read());
            rd_ex_mem = (rd_ex.read());;
            useRd_ex_mem = (useRd_ex.read());
            opCode_ex_mem = (opCode_ex.read());
            funct3_ex_mem = (funct3_ex.read());
            datac_ex_mem = (datac_ex.read());
            we_ex_mem = (we_ex.read());
            jalrcus_ex_mem =  jalrcus_ex.read();
            jalcustype_mem_mem = jalcustype_ex.read();

            // if(!stallSignals[STALL_EXECUTE].read() && !localStall.read() && !stallIm.read() && !stallDm.read()){
            //     result = (result_ex_ex.read());
            //     rd = (rd_ex.read());
            //     useRd = (useRd_ex.read());
            //     opCode = (opCode_ex.read());
            //     funct3 = (funct3_ex.read());
            //     datac = (datac_ex.read());
            //     we = (we_ex.read());

            //     jalrcus=jalrcus_ex.read();
            //     jalcustype=jalcustype_ex.read();
            // }
            if(!stallSignals_ex_mem && !localStall_mem && !stallIm_mem && !stallDm_mem){
                result = result_ex_ex_mem;
                rd = (rd_ex_mem);
                useRd = useRd_ex_mem;
                opCode = (opCode_ex_mem);
                funct3 = (funct3_ex_mem);
                datac = (datac_ex_mem);
                we = (we_ex_mem);

                jalrcus=jalrcus_ex_mem;
                jalcustype=jalcustype_mem_mem;
            }

            address = 0;
            isLoad = 0;
            isStore = 0;
            switch (opCode) {
                case RISCV_LD:
                    
                    rd = rd;
                    address = result;
                    isLoad  = 1;
                    isStore = 0;
                    //    formatread(extoMem.result, datasize, signenable, mem_read); //TODO

                    
                    break;
                case RISCV_ST:
                    isLoad = 0;
                    isStore      = 1;
                    address      = result;
                    valueToWrite = datac;
                    byteEnable   = 0xf;

                    // dmem_flag.write(1);
                    break;
                default:
                    // dmem_flag.write(0);
                    break;
            }

            ////stallSignals    STALL_MEMORY     localStall    we   stallIm
            dataIn = valueToWrite;
            dataOut = result;
            opType_d = NONE;
            
            mem_flag = 0;
            
            if (!stallSignals_mem_mem && !localStall_mem && we && !stallIm_mem)
            {

                
                // TODO: carry the data size to memToWb
                switch (funct3) {
                case 0:
                    mask = BYTE;
                    break;
                case 1:
                    mask = HALF;
                    break;
                case 2:
                    mask = WORD;
                    break;
                case 4:
                    mask = BYTE_U;
                    break;
                case 5:
                    mask = HALF_U;
                    break;
                // Should NEVER happen
                default:
                    mask = WORD;
                    break;
                }
                                
                mask = mask;
                opType_d = isLoad ? LOAD : (isStore ? STORE : NONE);
                
                if(opType_d == LOAD) mem_flag = (1);


            }

            dmem_address.write((sc_uint<32>) address);
            dmem_mask.write(mask);
            dmem_opType.write(opType_d);
            dmem_dataIn.write(dataIn);

            dmem_flag.write(mem_flag);

            we_mem.write(we);
            result_mem.write(dataOut);
            useRd_mem.write(useRd);

            rd_mem.write(rd);
            address_mem.write(address);
            isLoad_mem.write(isLoad);
            isStore_mem.write(isStore);
            valueToWrite_mem.write(valueToWrite);
            byteEnable_mem.write(byteEnable);

            jalrcus_mem.write(jalrcus);
            jalcustype_mem.write(jalcustype);

        wait();
        //wait();
    }
}



void riscv_core::result_mem_arhb(void){
    if(dmem_flag.read()){
        result_mem_mem.write(dmem_dataOut.read());
    }
    else{
        result_mem_mem.write(result_mem.read());
    }
}



void riscv_core::wrbk(void){
    ////input
    ////output

    bool we;
    sc_uint<5> rd;
    bool useRd;
    sc_uint<32> result;
    sc_uint<32> value;

    ////initial
    for(int i=0;i<32;i++) registerFile[i].write(0);
    
    bool jalrcus = 0;
    sc_uint<3> jalcustype = 0;

    //////local
    sc_int<32> result_mem_mem_wb = 0;
    bool stallSignals_mem_wb = 0;
    bool localStall_wb = 0;
    bool stallIm_wb = 0;
    bool stallDm_wb = 0;
    bool we_mem_wb = 0;
    bool useRd_mem_wb = 0;
    sc_uint<5> rd_mem_wb = 0;

    jalrcus_wb.write(0);
    jalcustype_wb.write(0);

    registerFile[2] = STACK_INIT;
    registerFile[1] = RAADDRESS;
    wait();
    while(1){
        stallSignals_mem_wb = stallSignals[STALL_MEMORY].read();
        localStall_wb = localStall.read();
        stallIm_wb = stallIm.read();
        stallDm_wb = stallDm.read();
        rd_mem_wb = (rd_mem.read());
        useRd_mem_wb = (useRd_mem.read());
        we_mem_wb = (we_mem.read());

        jalrcus = jalrcus_mem.read();
        jalcustype = jalcustype_mem.read();
        result_mem_mem_wb = result_mem_mem.read();

        if (!stallSignals_mem_wb && !localStall_wb && !stallIm_wb && !stallDm_wb){
            rd = rd_mem_wb;
            useRd = useRd_mem_wb;
            we = we_mem_wb;
        }


        if ((rd != 0) && (we) && useRd) {
            value = result_mem_mem_wb;
            rd    = rd;
            value = value;
            useRd = 1;
        }
        else{
            useRd = 0;
            rd = 0;
        }


        value_wb.write(value);
        rd_wb.write(rd);
        useRd_wb.write(useRd);
        we_wb.write(we);

        jalrcus_wb.write(jalrcus);
        jalcustype_wb.write(jalcustype);
        
        if (we && useRd && !localStall_wb && !stallIm_wb && !stallDm_wb)
        {   
            #ifdef SYSCSIMULATION
                std::cout<<" write register is "<<value<<" \n";
            #endif
            registerFile[rd].write( (sc_int<32>) value);
            // std::cout<<" instruction_ft is"<<instruction_ft.read()<<" write back register "<<std::dec<<rd<<" value "<<value<<"  ";
        }
        #ifdef SYSCSIMULATION
        if(jalrcus && !localStall_wb && !stallIm_wb && !stallDm_wb){
            if(jalcustype == 1){
                std::cout<<" print is "<<std::dec<<registerFile[11]<<std::endl;
            }
        }
        #endif
        // we_wb.write(we);
        wait();
        //wait();
    }
}



void riscv_core::forwardUnit(){
    ////input:  localStall   useRs1_dc   rs1_dc   useRs2_dc rs2_dc useRs3_dc rs3_dc useRd_ex rd_ex  useRd_mem rd_mem useRd_wb rd_wb isLongInstruction_ex
    ////all temp should solve stalls in same cycle!!!
    ////output: stallSignals    forwardRegisters.forwardExtoVal1


    bool forwardExtoVal1 = 0;
    bool forwardExtoVal2 = 0;
    bool forwardExtoVal3 = 0;
    bool forwardMemtoVal1 = 0;
    bool forwardMemtoVal2 = 0;
    bool forwardMemtoVal3 = 0;
    bool forwardWBtoVal1  = 0;
    bool forwardWBtoVal2  = 0;
    bool forwardWBtoVal3  = 0;
    
    bool stallSignals_fw[5] = {0};

    if(!localStall.read()){
    
        ////CMETHOD   combinational circuit
        if (useRs1_dc.read()) {
            if (useRd_ex.read() && rs1_dc.read() == rd_ex.read()) {
                if (isLongInstruction_ex.read()) {
                stallSignals_fw[0] = 1;
                stallSignals_fw[1] = 1;
                } else {
                forwardExtoVal1 = 1;
                }
            } else if (useRd_mem.read() && rs1_dc.read() == rd_mem.read()) {
                forwardMemtoVal1 = 1;
            } else if (useRd_wb.read() && rs1_dc.read() == rd_wb.read()) {
                forwardWBtoVal1 = 1;
            }
            else{
                forwardExtoVal1 = 0;
                forwardMemtoVal1 = 0;
                forwardWBtoVal1 = 0;
            }
        }
        else{
            forwardExtoVal1 = 0;
            forwardMemtoVal1 = 0;
            forwardWBtoVal1 = 0;
        }

        if (useRs2_dc.read()) {
            if (useRd_ex.read() && rs2_dc.read() == rd_ex.read()) {
                if (isLongInstruction_ex.read()) {
                stallSignals_fw[0] = (1);
                stallSignals_fw[1] = (1);
                } else {
                forwardExtoVal2 = 1;
                }
            } else if (useRd_mem.read() && rs2_dc.read() == rd_mem.read())
                forwardMemtoVal2 = 1;
            else if (useRd_wb.read() && rs2_dc.read() == rd_wb.read())
                forwardWBtoVal2 = 1;
            else{
                forwardExtoVal2 = 0;
                forwardMemtoVal2 = 0;
                forwardWBtoVal2 = 0;
            }
        }
        else{
            forwardExtoVal2 = 0;
            forwardMemtoVal2 = 0;
            forwardWBtoVal2 = 0;
        }

        if (useRs3_dc.read()) {
            if (useRd_ex.read() && rs3_dc.read() == rd_ex.read()) {
                if (isLongInstruction_ex.read()) {
                stallSignals_fw[0] = 1;
                stallSignals_fw[1] = 1;
                } else {
                forwardExtoVal3 = 1;
                }
            } else if (useRd_mem.read() && rs3_dc.read() == rd_mem.read())
                forwardMemtoVal3 = 1;
            else if (useRd_wb.read() && rs3_dc.read() == rd_wb.read())
                forwardWBtoVal3 = 1;
            else{
                forwardExtoVal3 = 0;
                forwardMemtoVal3 = 0;
                forwardWBtoVal3 = 0;
            }
        }
        else{
            forwardExtoVal3 = 0;
            forwardMemtoVal3 = 0;
            forwardWBtoVal3 = 0;
        }


    }
    ////output:
    stallSignals[0].write(stallSignals_fw[0]);
    stallSignals[1].write(stallSignals_fw[1]);
    stallSignals[2].write(stallSignals_fw[2]);
    stallSignals[3].write(stallSignals_fw[3]);
    stallSignals[4].write(stallSignals_fw[3]);

    ////every cycle to intialize 
    
    ForwardReg_EXval1.write(forwardExtoVal1);
    ForwardReg_EXval2.write(forwardExtoVal2);
    ForwardReg_EXval3.write(forwardExtoVal3);
    ForwardReg_MMval1.write(forwardMemtoVal1);
    ForwardReg_MMval2.write(forwardMemtoVal2);
    ForwardReg_MMval3.write(forwardMemtoVal3);
    ForwardReg_WBval1.write(forwardWBtoVal1);
    ForwardReg_WBval2.write(forwardWBtoVal2);
    ForwardReg_WBval3.write(forwardWBtoVal3);
    

}


#ifdef CHECKCYCLES
void riscv_core::print_cycle()
{
    int cycles = 0;
    wait();
    while(1){
        std::cout << " instruction_ft is "<<std::setfill('0') << std::setw(8) << std::right << std::hex << instruction_ft.read();//sc_signal<sc_uint<32>> instruction_ft;
        std::cout << " pc_ft is "<<std::dec<<imem_address.read()<<" nextPC_ft is "<<nextPC_ft.read()<<" we_ft is "<<we_ft.read()<<" stallIm is "<<stallIm.read()<<"\n";

//         // ////wrbk////
//         // // std::cout<<" wrbk ";
//         // std::cout<<" value_wb "<<value_wb.read()<<" rd_wb "<<rd_wb.read()<<" useRd_wb "<<useRd_wb.read()<<" we_wb "<<we_wb.read()<<"\n";
//         // std::cout<<" multUsed "<<multUsed.read()<<" result_ex_ex "<<result_ex_ex.read()<<" localStall "<<localStall.read()<<" result_mem_mem "\
//         // <<result_mem_mem.read()<<" result_mem "<<result_mem.read()<<\
//         // " rd_mem"<<rd_mem.read()<<" rd_ex"<<rd_ex.read()<<" we_mem"<<we_mem.read()<<"\n";


        // std::cout<<" cycle is "<<std::dec<<cycles<<" \nregisterFiles are ";
        // for(int i=0;i<32;i++)  std::cout<<"  "<<registerFile[i].read()<<" ";
        // std::cout<<"\n";
//         // stall
//         ////output signal/ports
//         ////ft_de_ex////
        
//         std::cout<<" fetch_module ";
//         std::cout << " instruction_ft is "<<std::setfill('0') << std::setw(8) << std::right << std::hex << instruction_ft.read();//sc_signal<sc_uint<32>> instruction_ft;
//         std::cout << " pc_ft is "<<std::dec<<imem_address.read()<<" nextPC_ft is "<<nextPC_ft.read()<<" we_ft is "<<we_ft.read()<<" stallIm is "<<stallIm.read()<<"\n";
//         // sc_signal<sc_uint<32>> pc_ft;
//         // sc_signal<sc_uint<32>> nextPC_ft;
//         // sc_signal<bool> we_ft;
//         // sc_uint<64> cycle_ft;////done after branchUnit
//         // sc_signal<bool> stallIm;

//         ////dc_2_exe////
//         std::cout<<" decode_module ";
//         std::cout << " instruction_dc is "<<std::setfill('0') << std::setw(8) << std::right << std::hex << instruction_dc.read();
//         std::cout<<std::dec<<" opCode_dc is "<<opCode_dc.read()<<" funct7_dc is "<<funct7_dc.read()<<" lhs_dc is "<<lhs_dc.read()<<" rhs_dc is "<<rhs_dc.read()<<" datac_dc is "<<datac_dc.read();
//         std::cout<<" pc_dc is "<<pc_dc.read()<<" nextPC_dc is "<<nextPC_dc.read()<<" isBranch_dc is "<<isBranch_dc.read();
//         std::cout<<" useRs123d_dc is "<<useRs1_dc.read()<<useRs2_dc.read()<<useRs3_dc.read()<<useRd_dc.read()<<" rs1_dc is "<<rs1_dc.read()<<" rs2_dc is "<<rs2_dc.read();
//         std::cout<<" rs3_dc is "<<rs3_dc.read()<<" rd_dc is "<<rd_dc.read()<<" we_dc is "<<we_dc.read()<<"\n";

        
//         ////ex_mem////
//         std::cout<<" execute ";
//         std::cout << " instruction_ex is "<<std::setfill('0') << std::setw(8) << std::right << std::hex << instruction_ex.read();
//         std::cout<<" pc_ex is "<<std::dec<<pc_ex.read()<<" nextPC_ex "<<nextPC_ex.read()<<" result_ex is "<<result_ex_ex.read()<<" rd_ex is "<<rd_ex.read()<<" useRd_ex is "<<useRd_ex.read();
//         std::cout<<" isLongInstruction_ex "<<isLongInstruction_ex.read()<<" opCode_ex "<<opCode_ex.read()<<" funct3_ex "<<funct3_ex.read();
//         std::cout<<" datac_ex "<<datac_ex.read()<<" isBranch_ex "<<isBranch_ex.read()<<" we_ex "<<we_ex.read()<<"\n";

//         ////mem_2_wb////
//         std::cout<<" mem ";
//         std::cout<<" result_mem "<<result_mem_mem.read()<<" rd_mem "<<rd_mem.read()<<" useRd_mem "<<useRd_mem.read();//<<" stallDm "<<stallDm_fw
//         std::cout<<" address_mem "<<address_mem.read()<<" valueToWrite_mem "<<valueToWrite_mem.read()<<" byteEnable_mem "<<byteEnable_mem.read()<<" dmem_address is "<<dmem_address.read();
//         std::cout<<" isStore_mem "<<isStore_mem.read()<<" isLoad_mem "<<isLoad_mem.read()<<" we_mem "<<we_mem.read()<<" \n";


//         ////wrbk////
//         std::cout<<" wrbk ";
//         std::cout<<" value_wb "<<value_wb.read()<<" rd_wb "<<rd_wb.read()<<" useRd_wb "<<useRd_wb.read()<<" we_wb "<<we_wb.read()<<"\n";
//         // sc_signal<sc_uint<32>> value_wb;
//         // sc_signal<sc_uint<5> > rd_wb;
//         // sc_signal<bool> useRd_wb;
//         // sc_signal<bool> we_wb;

//         // ////mult////
//         std::cout<<" mult ";
//         std::cout<<" multResult "<<multResult.read()<<" multUsed "<<multUsed.read()<<"\n";
//         std::cout<<" opCode_dc is "<<opCode_dc.read()<<" funct7 "<<funct7_dc.read()<<" funct3 "\
//   <<funct3_dc.read()<<" lhs "<<lhs_dc.read()<<" rhs "<<rhs_dc.read()<<" localstall "<<localStall.read()<<" stallMultAlu "<<stallMultAlu<<"\n";
//         // sc_signal<sc_uint<32>> multResult;//multResult result_mul
//         // sc_signal<bool> stallMultAlu; 
//         // sc_signal<bool> multUsed;//multUsed
//         // sc_signal<bool > localStall;


//         // ////forwardUnit////
//         std::cout<<" forwardUnit ";
//         std::cout<<"stallSignals are "<<stallSignals[0].read()<<stallSignals[1].read()<<stallSignals[2].read()<<stallSignals[3].read()<<stallSignals[4].read();
//         std::cout<<" forwardRegistersEX1 2 3 "<<ForwardReg_EXval1.read()<<ForwardReg_EXval2.read()<<ForwardReg_EXval3.read();
//         std::cout<<" forwardRegistersMEM1 2 3 "<<ForwardReg_MMval1.read()<<ForwardReg_MMval2.read()<<ForwardReg_MMval3.read();
//         std::cout<<" forwardRegistersWB1 2 3 "<<ForwardReg_WBval1.read()<<ForwardReg_WBval2.read()<<ForwardReg_WBval3.read()<<"\n";

        
//         ////dmem_mod////
//         std::cout<<" dmem_mod ";
//         std::cout <<" dmem_address is "<<std::dec<<dmem_address.read();
//         std::cout<<" dmem_mask "<<dmem_mask.read()<<" dmem_opType "<<dmem_opType.read()<<" dmem_dataIn "<<dmem_dataIn.read()<<" dmem_dataOut "<<dmem_dataOut.read()<<" dmem_flag "<<dmem_flag.read();
//         std::cout<<"  \n";

//         ////imem_mod////
//         std::cout<<" imem_mod ";
//         std::cout <<" imem_address is "<<std::dec<<imem_address.read();
//         std::cout<<" imem_dataOut "<<std::setfill('0') << std::setw(8) << std::right << std::hex <<instruction_ft.read()<<std::dec;
//         std::cout<<"  \n";
        cycles++;  
        wait();
    }
}
#endif
