#include "scriscv_scalar.h"

////1clock delay
void riscv_dmemory::dcache(void){
    sc_uint<32> address;//////pc_ft
    sc_uint<3> mask;
    sc_uint<2> opType;
    sc_uint<32> dataIn;
    sc_uint<32> dataOut;///////instruction_ft
    sc_uint<32> addOut;
    // bool stall = 0;//////stallIm

    ////////dmem
    sc_uint<8> t8 = 0;
    sc_int<1> bit = 0;
    sc_uint<16> t16 = 0;
    sc_uint<32> addr = 0;

    sc_uint<32> d_data[DRAM_SIZE>>2]/*Cyber array=REG*/ = {
        #include "dmem.txt"
    };
    wait();
    while(1){

        addr = dmem_address.read();
        mask = dmem_mask.read();
        opType = dmem_opType.read();
        dataIn = dmem_dataIn.read();
        // std::cout<<" addr is "<<addr<<" optype is "<<opType<<"\n";
        switch(opType){           
            case 2:
                //store
                switch (mask) {
                    case 4:
                    case 0:
                        d_data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3)) =  dataIn.range(7,0);
                        break;
                    case 1:
                    case 5:
                        d_data[addr >> 2].range( (addr[1] ? 16 : 0)+16-1, (addr[1] ? 16 : 0)) = dataIn.range(15,0);
                        break;
                    case 2:
                    default:
                        d_data[addr >> 2] = dataIn;
                        break;
                    
                }
                stallDm.write(1);
                break;
            case 1:
                switch (mask) {
                    case 0://// signed byte
                        t8  = d_data[addr >> 2].range((((int)addr.range(1,0)) << 3)+8-1,(((int)addr.range(1,0)) << 3));
                        bit = t8.range(7,7);
                        dataOut.range(0+t8.length()-1,0) = t8;
                        dataOut.range(8+24-1,8) =  (sc_int<24>)bit;
                        break;
                    case 1:///// signed short
                        t16 = d_data[addr >> 2].range((addr[1] ? 16 : 0)+16-1,addr[1] ? 16 : 0);
                        bit = t16.range(15,15);
                        dataOut.range(0+t16.length()-1,0) =  t16;
                        dataOut.range(16+16-1,16) = (sc_int<16>)bit;
                        break;
                    case 4:///// u_int8
                        dataOut = d_data[addr >> 2].range( (((int)addr.range(1,0)) << 3)+8-1, (((int)addr.range(1,0)) << 3) ) & 0xff;
                        break;
                    case 5:///// u_int16
                        dataOut = d_data[addr >> 2].range((addr[1] ? 16 : 0)+15,(addr[1] ? 16 : 0))& 0xffff;
                        break;
                    case 2:
                    default:
                        dataOut = d_data[addr >> 2];
                        break;
                    }
                    stallDm.write(0);
                break;
                default:
                    stallDm.write(1);
            break;
        }
        // stallDm.write(1);
        dmem_dataOut.write(dataOut);
        wait();
    }
}

////1clock delay
void riscv_imemory::icache(void){
    //////input
    sc_uint<32> address;//////pc_ft
    sc_uint<3> mask;
    sc_uint<2> opType;
    sc_uint<32> dataIn;
    sc_uint<32> instruction;///////instruction_ft
    sc_uint<32> addOut;
    // bool stall;//////stallIm
    sc_uint<32> i_data[IRAM_SIZE>>2]/*Cyber array=REG*/  = {
        #include "imem.txt"
    };

    instruction_ft.write(0);///////instruction_ft
    imem_addOut.write(0);
    stallIm.write(0);//////stallIm  
    wait();

    while(1){
        
        address = imem_address.read().range(31,2);
        instruction = i_data[address];
        instruction_ft.write(instruction);
        wait();
    } 
}

void riscv_core::core(void){
    #ifdef CHECKCYCLES
    int cycles = 0;
    #endif
    
    sc_uint<32> pc = PC_STARTADDR;
    sc_uint<32> instruction = 0;

    
    ////////registers for different stages
    bool insbranch = 0;
    ////////decode
    sc_uint<7> opCode = 0;
    sc_uint<7> funct7 = 0;
    sc_uint<3> funct3 = 0;
    // sc_int<32> lhs = 0;
    // sc_int<32> rhs = 0;
    // sc_int<32> datac = 0;
    // bool useRs1 = 0;
    // bool useRs2 = 0;
    // bool useRs3 = 0;
    // bool useRd = 0;
    sc_uint<5> rs1 = 0;
    sc_uint<5> rs2 = 0;
    // sc_uint<5> rs3 = 0;
    sc_uint<5> rd = 0;
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
    // Register access
    sc_uint<32> valueReg1 = 0;
    sc_uint<32> valueReg2 = 0;
    
    ////////exe
    // sc_int<32> result;
    sc_uint<5> shamt = 0;
    
    ////////dmem

    ////////wb
    sc_int<32> regs[32] = {
        0
    };
    regs[2] = STACK_INIT;
    regs[1] = RAADDRESS;

    imem_address.write(PC_STARTADDR);
    wait();

    while(1){
        insbranch = 0;
        regs[0] = 0;

        if(pc == RAADDRESS){
            #ifdef SYSCSIMULATION
            std::cout<<" end program";
            sc_stop();
            exit(-1);
            #endif
            while(1) wait();
        }
        ////fetch
        //Cyber scheduling_block
        {
            
            imem_address.write(pc);
            wait();
            instruction = instruction_ft.read();
            wait();
        }

        // instruction = i_data[pc>>2];
        
        ////decode
        opCode = instruction.range(6,0); // could be reduced to 5 bits because 1:0 is always 11

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
        // rs3 = rs2;
        funct3 = instruction.range(14,12);
        rd = instruction.range(11,7);
        // opCode = instruction.range(6,0); // could be reduced to 5 bits because 1:0 is always 11
        // Register access
        valueReg1 = sc_uint<32> (regs[rs1]); //(sc_uint<32>) 
        valueReg2 = sc_uint<32> (regs[rs2]);//(sc_uint<32>) 

        shamt = rs2;

        ////wb


        switch(opCode){
            ////////UJ type
            case RISCV_LUI:
                regs[rd] = ( (sc_int<32>) imm31_12);
                break;
            case RISCV_AUIPC:
                // result = pc + imm31_12;
                // regs[rd]=( (sc_int<32>) result);
                regs[rd]=( (sc_int<32>) (pc + imm31_12));
                break;
            ////////I type
            case RISCV_OPI://///itype  ////1 adder 
                // lhs = valueReg1;
                // rhs = imm12_I_signed;
                switch (funct3) {
                    case RISCV_OPI_ADDI:
                        // result = valueReg1 + imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( ((sc_int<32>) valueReg1) + imm12_I_signed);
                        break;
                    case RISCV_OPI_SLTI:
                        // result = valueReg1 < imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (((sc_int<32>) valueReg1) < imm12_I_signed));
                        break;
                    case RISCV_OPI_SLTIU:
                        // result = (sc_uint<32>)valueReg1 < (sc_uint<32>)imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (sc_int<32>) (valueReg1 < (sc_uint<32>)imm12_I_signed ));
                        break;
                    case RISCV_OPI_XORI:
                        // result = valueReg1 ^ imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (((sc_int<32>) valueReg1) ^ imm12_I_signed));
                        break;
                    case RISCV_OPI_ORI:
                        // result = valueReg1 | imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (((sc_int<32>) valueReg1) | imm12_I_signed));
                        break;
                    case RISCV_OPI_ANDI:
                        // result = valueReg1 & imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (((sc_int<32>) valueReg1) & imm12_I_signed) );
                        break;
                    case RISCV_OPI_SLLI: // cast rhs as 5 bits, otherwise generated hardware is 32 bits
                        // result = valueReg1 << (sc_uint<5>)imm12_I_signed;
                        // regs[rd]=( (sc_int<32>) result);
                        regs[rd]=( (((sc_int<32>) valueReg1)<< (sc_uint<5>)imm12_I_signed) );
                        break;
                    case RISCV_OPI_SRI:
                    {
                        // if (dctoEx.funct7.slc<1>(5)) // SRAI
                        sc_int<32> result;
                        if (funct7.range(5,5)) // SRAI
                        {result = ((sc_int<32>) valueReg1) >> (sc_uint<5>)shamt;}
                        else // SRLI
                        {result = (sc_uint<32>)valueReg1 >> (sc_uint<5>)shamt;}

                        regs[rd]=( (sc_int<32>) result);
                        break;
                    }
                        
                }
            break;
            case RISCV_JALR: ////2 adder 
                if(funct3 == 0){
                    // Note: in current version, the addition is made in the decode stage
                    // The value to store in rd (pc+4) is stored in lhs
                    regs[rd]=( (sc_int<32>) (pc+4) );
                    pc   = imm12_I_signed + ((sc_int<32>) valueReg1);
                    insbranch = 1;
                }
                else if(funct3==1){
                    #ifdef SYSCSIMULATION
                    std::cout<<" print is "<<std::dec<<regs[11]<<std::endl;
                    #endif
                }
                else if(funct3==2){////accelerator
                    #ifdef SYSCSIMULATION
                    std::cout<<" accelerator start "<<std::endl;
                    #endif
                }
                break;
            case RISCV_LD:{ ////1 adder 
                sc_int<32> result;
                result = ((sc_int<32>) valueReg1) + imm12_I_signed;                
                sc_uint<32> dataOut = 0;
                //Cyber scheduling_block
                {
                    dmem_address.write( (sc_uint<32>) result);
                    dmem_mask.write( funct3.range(2,0) );
                    dmem_opType.write(1);
                    // while(stallDm.read()) wait();
                    do{wait();} while(stallDm.read());
                    regs[rd]=( (sc_int<32>) dmem_dataOut.read());
                    dmem_opType.write(0);
                }
            }
            break;
            // //////S TYPE
            case RISCV_ST:////1 adder               
                {
                    sc_int<32> result;
                    result = ((sc_int<32>) valueReg1) + imm12_S_signed;

                    //Cyber scheduling_block
                    {
                        dmem_address.write( (sc_uint<32>) result);
                        dmem_dataIn.write(valueReg2);
                        dmem_mask.write( funct3.range(2,0) );
                        dmem_opType.write( 2);
                        wait();
                        dmem_opType.write(0);
                    }
                }
                break;
            
            //////J TYPE
            case RISCV_JAL:////2 adder 
                regs[rd] = ( (sc_int<32>) pc+4);
                pc = pc + imm21_1_signed;
                insbranch = 1;
                break;
            //////SB TYPE
            case RISCV_BR: ////1 adder 
                switch (funct3) {
                case RISCV_BR_BEQ:
                    // insbranch = (valueReg1 == valueReg2);
                    if(((sc_int<32>) valueReg1)==((sc_int<32>) valueReg2)) {pc = pc + imm13_signed; insbranch = 1;}
                    break;
                case RISCV_BR_BNE:
                    // insbranch = (valueReg1 != valueReg2);
                    if(((sc_int<32>) valueReg1)!=((sc_int<32>) valueReg2)) {pc = pc + imm13_signed; insbranch = 1;}
                    break;
                case RISCV_BR_BLT:
                    // insbranch = (valueReg1 < valueReg2);
                    if(((sc_int<32>) valueReg1) < ((sc_int<32>) valueReg2)) {pc = pc + imm13_signed; insbranch = 1;}
                    break;
                case RISCV_BR_BGE:
                    // insbranch = (valueReg1 >= valueReg2);
                    if(((sc_int<32>) valueReg1) >= ((sc_int<32>) valueReg2)) {pc = pc + imm13_signed; insbranch = 1;}
                    break;
                case RISCV_BR_BLTU:
                    if((sc_uint<32>)valueReg1 < (sc_uint<32>)valueReg2) {
                        pc = pc + imm13_signed;
                        insbranch = 1;
                    }
                    break;
                case RISCV_BR_BGEU:
                    if ((sc_uint<32>)valueReg1 >= (sc_uint<32>)valueReg2) {
                        pc = pc + imm13_signed;
                        insbranch = 1;
                    }
                    break;
                }
                break;
            ////////R TYPE
            case RISCV_OP:
                // lhs = valueReg1;
                // rhs = valueReg1;

                if (funct7.range(0,0)) // M Extension
                {
                    if(funct7 == RISCV_OP_M){
                        sc_int<32> result;
                        sc_uint<32> quotient = 0,remainder = 0; 
                        sc_uint<32> dataAUnsigned=0, dataBUnsigned=0;
                        dataAUnsigned.range(31,0) = valueReg1;
                        dataBUnsigned.range(31,0) = valueReg2;
                        // sc_uint<64> resultU  = dataAUnsigned * dataBUnsigned;
                        // sc_uint<64> resultS  = valueReg1 * valueReg2;
                        // sc_uint<64> resultSU = valueReg1 * dataBUnsigned;
                        sc_uint<64> resultU;
                        sc_uint<64> resultS;
                        sc_uint<64> resultSU;
                        bool resIsNeg = valueReg1[31] ^ valueReg2[31];
                    
                        switch (funct3) {  
                        case RISCV_OP_M_MUL:  ////1 mult 
                        //   result = resultS.slc<32>(0);
                            resultS  = valueReg1 * valueReg2;
                            result = resultS.range(31,0);
                            regs[rd]=( (sc_int<32>) result);
                            // valRet = true;
                        break;
                        case RISCV_OP_M_MULH:   ////1 mult 
                        //   result = resultS.slc<32>(32);
                            resultS  = valueReg1 * valueReg2;
                            result = resultS.range(63,32);
                            regs[rd]=( (sc_int<32>) result);
                            // valRet = true;
                        break;
                        case RISCV_OP_M_MULHSU:  ////1 mult 
                            resultSU = valueReg1 * dataBUnsigned;
                            result = resultSU.range(63,32);
                            regs[rd]=( (sc_int<32>) result);
                            // valRet = true;
                        break;
                        case RISCV_OP_M_MULHU:  ////1 mult 
                            resultU  = dataAUnsigned * dataBUnsigned;
                            result = resultU.range(63,32);
                            regs[rd]=( (sc_int<32>) result);
                            // valRet = true;
                        break;
                        case RISCV_OP_M_DIV:
                            if (dataBUnsigned == 0) { // division by zero
                                result = -1;
                                regs[rd]=( (sc_int<32>) result);
                            } else if (dataAUnsigned == 0x80000000 && dataBUnsigned == 0xfffffffc){ // Overflow 32 bits
                                result = 0x80000000;
                                regs[rd]=( (sc_int<32>) result);
                            } else {
                                quotient = 0;
                                remainder = 0;
                                if (valueReg1[31]) {
                                    dataAUnsigned = -valueReg1;
                                }
                                if (valueReg2[31]) {
                                    dataBUnsigned = -valueReg2;
                                }
                                //Cyber unroll_times = 4 
                                for(int state=31;state>=0;state--){
                                    // state--;
                                    remainder    = remainder << 1;
                                    remainder[0] = dataAUnsigned[state];
                                    if (remainder >= dataBUnsigned) {
                                    remainder       = remainder - dataBUnsigned;
                                    quotient[state] = 1;
                                    }
                                }
                                
                                if (resIsNeg)
                                    result = -quotient;
                                else
                                    result = quotient;
                                regs[rd]=( (sc_int<32>) result);
                                // #ifdef SYSCSIMULATION
                                //     std::cout<<" div is "<<std::dec<<dataAUnsigned<<" div "<<dataBUnsigned\
                                //     <<" res is "<<result<<std::endl;
                                // #endif
                            }
                            break;
                        case RISCV_OP_M_DIVU:
                            if (dataBUnsigned == 0) { // division by zero
                                result = -1;
                                regs[rd]=( (sc_int<32>) result);
                            } else{
                                quotient  = 0;
                                remainder = 0;
                                //Cyber unroll_times = 4 
                                for(int state=31;state>=0;state--){
                                    // state--;
                                    remainder    = remainder << 1;
                                    remainder[0] = dataAUnsigned[state];
                                    if (remainder >= dataBUnsigned) {
                                    remainder       = remainder - dataBUnsigned;
                                    quotient[state] = 1;
                                    }
                                }
                                result = quotient;
                                regs[rd]=( (sc_int<32>) result);
                            }
                        break;
                        case RISCV_OP_M_REM:
                            if (dataBUnsigned == 0) { // division by zero
                                result = dataAUnsigned;
                                regs[rd]=( (sc_int<32>) result);
                            } else if (dataAUnsigned == 0x80000000 && dataBUnsigned == 0xfffffffc){ // Overflow
                                result = 0;
                                regs[rd]=( (sc_int<32>) result);
                            } else{
                                if (valueReg1[31]) {
                                    dataAUnsigned = -valueReg1;
                                }
                                if (valueReg2[31]) {
                                    dataBUnsigned = -valueReg2;
                                }
                                quotient  = 0;
                                remainder = 0;
                                //Cyber unroll_times = 4 
                                for(int state=31;state>=0;state--){
                                    // state--;
                                    remainder    = remainder << 1;
                                    remainder[0] = dataAUnsigned[state];
                                    if (remainder >= dataBUnsigned) {
                                    remainder       = remainder - dataBUnsigned;
                                    quotient[state] = 1;
                                    }
                                }
                                
                                if (valueReg1[31])
                                    result = -remainder;
                                else
                                    result = remainder;
                                regs[rd]=( (sc_int<32>) result);
                                // #ifdef SYSCSIMULATION
                                //     std::cout<<" end rem is "<<std::dec<<dataAUnsigned<<" rem "<<dataBUnsigned\
                                //     <<" res is "<<result<<std::endl;
                                // #endif
                            }
                        break;
                        case RISCV_OP_M_REMU:
                            if (dataBUnsigned == 0) { // division by zero
                                result = dataAUnsigned;
                                regs[rd]=( (sc_int<32>) result);
                            } else{
                                quotient  = 0;
                                remainder = 0;
                                //Cyber unroll_times = 4 
                                for(int state=31;state>=0;state--){
                                    // state--;
                                    remainder    = remainder << 1;
                                    remainder[0] = dataAUnsigned[state];
                                    if (remainder >= dataBUnsigned) {
                                    remainder       = remainder - dataBUnsigned;
                                    quotient[state] = 1;
                                    }
                                }
                                result = remainder;
                                regs[rd]=( (sc_int<32>) result);
                            }
                        break;
                        default:
                        break;
                        }
                    }
                    
                } 
                else 
                {
                    sc_int<32> result;
                    switch (funct3) {
                        case RISCV_OP_ADD:
                        if (funct7.range(5,5)) // SUB
                        {
                            result = valueReg1 - valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        }
                        else // ADD
                        {
                            result = valueReg1 + valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        }                            
                        break;
                        case RISCV_OP_SLL:
                            result = ((sc_int<32>) valueReg1) << (sc_uint<5>)valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        break;
                        case RISCV_OP_SLT:
                            result = ((sc_int<32>) valueReg1) < ((sc_int<32>) valueReg2);
                            regs[rd]=( (sc_int<32>) result);
                        break;
                        case RISCV_OP_SLTU:
                            result = (sc_uint<32>)valueReg1 < (sc_uint<32>)valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        break;
                        case RISCV_OP_XOR:
                            result = valueReg1 ^ valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        break;
                        case RISCV_OP_SR:
                        if (funct7.range(5,5)) // SRA
                        {
                            result = ((sc_int<32>) valueReg1) >>  (sc_uint<5>)valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        }
                        else // SRL
                        {
                            result = (sc_uint<32>)valueReg1 >>  (sc_uint<5>)valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        }
                        break;
                        case RISCV_OP_OR:
                            result = valueReg1 | valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        break;
                        case RISCV_OP_AND:
                            result = valueReg1 & valueReg2;
                            regs[rd]=( (sc_int<32>) result);
                        break;
                    }
                }
                break;
            #ifdef CHECKCYCLES
            // case RISCV_SYSTEM:////not used?
            // //     if (funct3 != 0) {
            // //     if (imm12_I == RISCV_CSR_MCYCLE)
            // //         lhs = cycle_ft.read().range(31,0);
            // //         // dctoEx.lhs = cycle.slc<32>(0);
            // //     else if (imm12_I == RISCV_CSR_MCYCLEH)
            // //         lhs = cycle_ft.read().range(63,0);
            // //         // dctoEx.lhs = cycle.slc<64>(0);
            // //     }

            //     break;
            default:
                //     useRs1 = (0);
                //     useRs2 = (0);
                std::cout<<" instruction error and break out \n";
            #endif
        }
        
        if(!insbranch)  pc += 4;
        
        #ifdef CHECKCYCLES
            std::cout << " instruction_dc_fw is "<<std::setfill('0') << std::setw(8) << std::right << std::hex << instruction;
            cycles++;
            for(int i=0;i<32;i++)  std::cout<<std::dec<<"  "<<regs[i]<<" ";
            std::cout<<" the cycles are "<<std::dec<<cycles<<"\n";
            // for(int i=0;i<32;i++){
            //     std::cout<<std::dec<<" reg["<<i<<"] "<<regs[i];
            // }
            // std::cout <<"\n current cycles is "<<std::dec<<cycles<<" pc is "<<pc<<std::endl;
        #endif

        wait();
    }

}

