#ifndef __CORE_H__
#define __CORE_H__

#include "systemc.h"

#include "riscvISA.h"

// all the possible memories
#define PC_STARTADDR 0
#define DRAM_SIZE 2048*4
#define IRAM_SIZE 2048*4
#define STACK_INIT 2048*4
#define RAADDRESS 1073741824


typedef enum { BYTE = 0, HALF, WORD, BYTE_U, HALF_U, LONG } memMask;

typedef enum { NONE = 0, LOAD, STORE } memOpType;




#define INTERFACE_SIZE 4


/******************************************************************************************
 * Stall signals enum
 * ****************************************************************************************
 */
typedef enum { STALL_FETCH = 0, STALL_DECODE = 1, STALL_EXECUTE = 2, STALL_MEMORY = 3, STALL_WRITEBACK = 4 } StallNames;


#endif // __CORE_H__

SC_MODULE(riscv_imemory){
    sc_in<bool> clk;
    sc_in<bool> rst;

    ////****i memory****////
    ////input
    sc_in<sc_uint<32> > imem_address;//////pc_ft
    sc_in<sc_uint<3> > imem_mask;
    sc_in<sc_uint<2> > imem_opType;
    sc_in<sc_uint<32> > imem_dataIn;
    ////output
    sc_out<sc_uint<32> > instruction_ft;///////instruction_ft
    sc_out<sc_uint<32> > imem_addOut;
    sc_out<bool> stallIm;//////stallIm  

    void icache(void);
    SC_CTOR(riscv_imemory){
        SC_CTHREAD(icache,clk.neg());
        reset_signal_is(rst,false);
        sensitive << clk.neg();
    }

};

SC_MODULE(riscv_dmemory){
    sc_in<bool> clk;
    sc_in<bool> rst;
    // ////****d memory****////
    ////input
    sc_in<sc_uint<32> > dmem_address;
    sc_in<sc_uint<3> > dmem_mask;
    sc_in<sc_uint<2> > dmem_opType;
    sc_in<sc_uint<32> > dmem_dataIn;
    ////output
    sc_out<sc_uint<32> > dmem_dataOut;/////////result_mem
    sc_out<sc_uint<32> > dmem_addOut;
    sc_out<bool> stallDm;/////////stallDm 

    void dcache(void);
    SC_CTOR(riscv_dmemory){
        SC_CTHREAD(dcache,clk.neg());
        reset_signal_is(rst,false);
        sensitive << clk.neg();
    }

};


SC_MODULE(riscv_core){
    sc_in<bool> clk;
    sc_in<bool> rst;

    // ////****i memory****////
    ////input
    sc_out<sc_uint<32> > imem_address;//////pc_ft
    sc_out<sc_uint<3> > imem_mask;
    sc_out<sc_uint<2> > imem_opType;
    sc_out<sc_uint<32> > imem_dataIn;
    ////output
    sc_in<sc_uint<32> > instruction_ft;///////instruction_ft
    sc_in<sc_uint<32> > imem_addOut;
    sc_in<bool> stallIm;//////stallIm

    ////****d memory****////
    ////input
    sc_out<sc_uint<32> > dmem_address;
    sc_out<sc_uint<3> > dmem_mask;
    sc_out<sc_uint<2> > dmem_opType;
    sc_out<sc_uint<32> > dmem_dataIn;
    ////output
    sc_in<sc_uint<32> > dmem_dataOut;/////////result_mem
    sc_in<sc_uint<32> > dmem_addOut;
    sc_in<bool> stallDm;/////////stallDm

    ////****dmem_dataOut and result_mem****////
    sc_signal<sc_uint<32> > result_mem_mem;

    ////global variables
    sc_signal<sc_int<32> > registerFile[32];

    // stall
    sc_signal<bool> ften_mod;
    sc_signal<bool> dcen_mod;
    sc_signal<bool> exen_mod;
    sc_signal<bool> memen_mod;
    sc_signal<bool> wben_mod;

    ////output signal/ports
    ////****fetch2decode****////
    // sc_signal<sc_uint<32> > instruction_ft;

    sc_signal<sc_uint<32> > pc_ft;/////<---pc_br
    sc_signal<sc_uint<32> > nextPC_ft;
    sc_signal<bool> we_ft;
    sc_uint<64> cycle_ft;////done after branchUnit

    ////****fetch2decode_forward****////

    ////****dc_2_exe****////
    sc_signal<sc_uint<32> > instruction_dc;
    sc_signal<sc_uint<7> > opCode_dc;
    sc_signal<sc_uint<7> > funct7_dc;
    sc_signal<sc_uint<3> > funct3_dc;
    sc_signal<sc_int<32> > lhs_dc;
    sc_signal<sc_int<32> > rhs_dc;
    sc_signal<sc_int<32> > datac_dc;
    sc_signal<sc_uint<32> > pc_dc;
    sc_signal<sc_uint<32> > nextPC_dc;
    sc_signal<bool> isBranch_dc;
    sc_signal<bool> crashFlag_dc;
    sc_signal<bool> useRs1_dc;
    sc_signal<bool> useRs2_dc;
    sc_signal<bool> useRs3_dc;
    sc_signal<bool> useRd_dc;
    sc_signal<sc_uint<5> > rs1_dc;
    sc_signal<sc_uint<5> > rs2_dc;
    sc_signal<sc_uint<5> > rs3_dc;
    sc_signal<sc_uint<5> > rd_dc;
    sc_signal<bool> we_dc;


    ////****dc_2_comb****////
    sc_signal<sc_uint<32> > valueReg1_dc_pre;
    sc_signal<sc_uint<32> > valueReg2_dc_pre;

    ////****dc_2_exe_forward****////
    

    ////****ex_mem****////
    sc_signal<sc_uint<32> > instruction_ex;
    sc_signal<sc_uint<32> > pc_ex;
    sc_signal<sc_int<32> > result_ex;
    sc_signal<sc_uint<5> > rd_ex;
    sc_signal<bool> useRd_ex;
    sc_signal<bool> isLongInstruction_ex;
    sc_signal<sc_uint<7> > opCode_ex;
    sc_signal<sc_uint<3> > funct3_ex;
    sc_signal<sc_int<32> > datac_ex;
    sc_signal<sc_uint<32> > nextPC_ex;
    sc_signal<bool> isBranch_ex;
    sc_signal<bool> we_ex;

    sc_signal<bool> jalrcus_ex;
    sc_signal<sc_uint<3> > jalcustype_ex;
    ////****ex_mem_comb****////
    sc_signal<sc_uint<32> > pc_ex_comb;////extoMem.pc
    sc_signal<sc_uint<32> > instruction_ex_comb;
    sc_signal<bool> useRd_ex_comb;
    sc_signal<bool> we_ex_comb;
    sc_signal<sc_int<32> > lhs_ex_comb;   //  left hand side : operand 1
    sc_signal<sc_int<32> > rhs_ex_comb;   // right hand side : operand 2
    sc_signal<sc_int<32> > datac_ex_comb;
    sc_signal<sc_uint<7> > opCode_ex_comb;
    sc_signal<sc_uint<5> > rd_ex_comb;
    sc_signal<sc_uint<3> > funct3_ex_comb;
    sc_signal<sc_uint<7> > funct7_ex_comb;


    sc_signal<bool> stallSignals_dc_ex_comb;
    sc_signal<bool> stallSignals_ex_ex_comb;
    sc_signal<bool> stallSignals_ft_ex_comb;
    sc_signal<bool> localStall_ex_comb;
    sc_signal<bool> stallIm_ex_comb ;
    sc_signal<bool> stallDm_ex_comb;
    sc_signal<bool> ForwardReg_EXval1_ex_comb ;
    sc_signal<bool> ForwardReg_EXval2_ex_comb;
    sc_signal<bool> ForwardReg_EXval3_ex_comb ;
    sc_signal<bool> ForwardReg_MMval1_ex_comb ;
    sc_signal<bool> ForwardReg_MMval2_ex_comb ;
    sc_signal<bool> ForwardReg_MMval3_ex_comb ;
    sc_signal<bool> ForwardReg_WBval1_ex_comb ;
    sc_signal<bool> ForwardReg_WBval2_ex_comb;
    sc_signal<bool> ForwardReg_WBval3_ex_comb;
    sc_signal<bool> we_ex_dc_comb ;
    sc_signal<bool> we_mem_ex_comb ;
    sc_signal<bool> we_wb_ex_comb;
    sc_signal<sc_int<32> > result_ex_ex_dc_comb;
    sc_signal<sc_int<32> > result_mem_mem_dc_comb;

    sc_signal<sc_int<32> > value_wb_dc_comb ;
    sc_signal<sc_int<32> > lhs_dc_dc_comb;
    sc_signal<sc_int<32> > rhs_dc_dc_comb;
    sc_signal<sc_int<32> > datac_dc_cd_comb;
    
    sc_signal<sc_uint<7> > opCode_dc_ex_comb;
    sc_signal<sc_uint<7> > funct7_dc_ex_comb;
    sc_signal<sc_uint<3> > funct3_dc_ex_comb;
    sc_signal<sc_uint<5> > rd_dc_ex_comb;
    sc_signal<sc_uint<32> > pc_dc_ex_comb;
    sc_signal<sc_uint<32> > instruction_dc_ex_comb;
    sc_signal<bool> we_dc_ex_comb ;
    sc_signal<bool> useRd_dc_ex_comb;
    sc_signal<bool> isBranch_ex_ex_comb;


    ////****mem_2_wb****////
    sc_signal<sc_uint<32> > result_mem; // Result to be written back
    sc_signal<sc_uint<5> > rd_mem;      // destination register
    sc_signal<bool> useRd_mem;
    
    // sc_signal<bool> stallDm;

    sc_signal<sc_int<32> > address_mem;
    sc_signal<sc_uint<32> > valueToWrite_mem;
    sc_signal<sc_uint<4> > byteEnable_mem;
    sc_signal<bool> isStore_mem;
    sc_signal<bool> isLoad_mem;
    sc_signal<bool> we_mem;

    sc_signal<bool> dmem_flag;

    sc_signal<bool> jalrcus_mem;
    sc_signal<sc_uint<3> > jalcustype_mem;

    ////****mem_2_wb_forward****////
    

    ////****mult****////
    sc_signal<sc_uint<32> > multResult;//multResult result_mul
    sc_signal<bool> stallMultAlu; 
    sc_signal<bool> multUsed;//multUsed
    sc_signal<bool > localStall;

    sc_signal<sc_int<32> > result_ex_ex;

    ////****forwardUnit****////
    sc_signal<bool> stallSignals[5];
    ////////pass struct
    // ForwardReg forwardRegisters;
    sc_signal<bool> ForwardReg_WBval1;
    sc_signal<bool> ForwardReg_WBval2;
    sc_signal<bool> ForwardReg_WBval3;

    sc_signal<bool> ForwardReg_MMval1;
    sc_signal<bool> ForwardReg_MMval2;
    sc_signal<bool> ForwardReg_MMval3;

    sc_signal<bool> ForwardReg_EXval1;
    sc_signal<bool> ForwardReg_EXval2;
    sc_signal<bool> ForwardReg_EXval3;


    ////****solveSyscall****////
    sc_signal<sc_uint<32> > result_mem_sys; // Result to be written back
    sc_signal<sc_uint<5> > rd_mem_sys;      // destination register
    sc_signal<bool> useRd_mem_sys;
    
    ////****wrbk****////
    sc_signal<sc_uint<32> > value_wb;
    sc_signal<sc_uint<5> > rd_wb;
    sc_signal<bool> useRd_wb;
    sc_signal<bool> we_wb;
    
    sc_signal<bool> jalrcus_wb;
    sc_signal<sc_uint<3> > jalcustype_wb;


    ////seq pos_clk
    void fetch(void);
    void decode(void);
    void exec(void);
    void mems(void);
    void wrbk(void);
    void mult(void);
    ////seq neg_clk
    void imem_mod(void);
    void dmem_mod(void);

    ////comb
    void forwardUnit(void);////comb
    void glbmet_regs(void);////comb
    void result_mem_arhb(void);////comb
    void mult_res(void);

	void exe_wecomb(void);

    void decode_comb(void);////comb to decode the register
    ////inline
    #ifdef CHECKCYCLES
        void print_cycle(void);
    #endif
    //     //hardware
    //   void imem(void);
    //   void dmem(void);

    SC_CTOR(riscv_core){
        SC_CTHREAD(fetch,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();

        SC_CTHREAD(decode,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();

        SC_CTHREAD(exec,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();

        SC_CTHREAD(mems,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();

        SC_CTHREAD(wrbk,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();

        SC_CTHREAD(mult,clk.pos());
        reset_signal_is(rst,false);
        sensitive << clk.pos();


        #ifdef CHECKCYCLES
            SC_CTHREAD(print_cycle,clk.neg());
            reset_signal_is(rst,false);
            sensitive << clk.neg();
        #endif

        SC_METHOD(result_mem_arhb); 
        sensitive<<dmem_flag<<dmem_dataOut<<result_mem;

        SC_METHOD(mult_res); 
        sensitive<<multUsed<<multResult<<result_ex;

        SC_METHOD(glbmet_regs); 
        sensitive<<stallMultAlu;//<<clk;

        SC_METHOD(exe_wecomb); 
        sensitive<<pc_ex_comb<<instruction_ex_comb<<useRd_ex_comb<<we_ex_comb\
        <<lhs_ex_comb<<rhs_ex_comb<<datac_ex_comb<<opCode_ex_comb<<rd_ex_comb\
        <<funct3_ex_comb<<funct7_ex_comb;
        // <<stallSignals_dc_ex_comb<<stallSignals_ex_ex_comb<<stallSignals_ft_ex_comb\
        // <<localStall_ex_comb<<stallIm_ex_comb<<stallDm_ex_comb\
        // <<ForwardReg_EXval1_ex_comb<<ForwardReg_EXval2_ex_comb<<ForwardReg_EXval3_ex_comb\
        // <<ForwardReg_MMval1_ex_comb<<ForwardReg_MMval2_ex_comb<<ForwardReg_MMval3_ex_comb\
        // <<ForwardReg_WBval1_ex_comb<<ForwardReg_WBval2_ex_comb<<ForwardReg_WBval3_ex_comb\
        // <<we_ex_dc_comb<<we_mem_ex_comb<<we_wb_ex_comb\
        // <<result_ex_ex_dc_comb<<result_mem_mem_dc_comb\
        // <<value_wb_dc_comb<<lhs_dc_dc_comb<<rhs_dc_dc_comb<<datac_dc_cd_comb\
        // <<opCode_dc_ex_comb<<funct7_dc_ex_comb<<funct3_dc_ex_comb<<rd_dc_ex_comb<<pc_dc_ex_comb<<instruction_dc_ex_comb\
        // <<we_dc_ex_comb<<useRd_dc_ex_comb<<isBranch_ex_ex_comb\

        SC_METHOD(forwardUnit);  // register do_nand2 with kernel
        sensitive<<localStall<<useRs1_dc<<rs1_dc<<useRs2_dc<<rs2_dc<<useRs3_dc<<rs3_dc<<useRd_ex<<rd_ex<<useRd_mem<<rd_mem<<useRd_wb<<rd_wb<<isLongInstruction_ex;//<<result_mem_mem;  // sensitivity list

        SC_METHOD(decode_comb);  // register do_nand2 with kernel
        sensitive<<instruction_ft<<registerFile[0]<<registerFile[1]<<registerFile[2]<<registerFile[3]<<registerFile[4]<<registerFile[5]\
        <<registerFile[6]<<registerFile[7]<<registerFile[8]<<registerFile[9]<<registerFile[10]\
        <<registerFile[11]<<registerFile[12]<<registerFile[13]<<registerFile[14]<<registerFile[15]\
        <<registerFile[16]<<registerFile[17]<<registerFile[18]<<registerFile[19]<<registerFile[20]\
        <<registerFile[21]<<registerFile[22]<<registerFile[23]<<registerFile[24]<<registerFile[25]\
        <<registerFile[26]<<registerFile[27]<<registerFile[28]<<registerFile[29]<<registerFile[30]\
        <<registerFile[31];  // sensitivity list     
  }

};


SC_MODULE(riscv_top){
    sc_in<bool> clk;
    sc_in<bool> rst;

    ////****i memory****////
    ////input
    sc_signal<sc_uint<32> > imem_address;//////pc_ft
    sc_signal<sc_uint<3> > imem_mask;
    sc_signal<sc_uint<2> > imem_opType;
    sc_signal<sc_uint<32> > imem_dataIn;
    ////output
    sc_signal<sc_uint<32> > instruction_ft;///////instruction_ft
    sc_signal<sc_uint<32> > imem_addOut;
    sc_signal<bool> stallIm;//////stallIm  

    // ////****d memory****////
    ////input
    sc_signal<sc_uint<32> > dmem_address;
    sc_signal<sc_uint<3> > dmem_mask;
    sc_signal<sc_uint<2> > dmem_opType;
    sc_signal<sc_uint<32> > dmem_dataIn;
    ////output
    sc_signal<sc_uint<32> > dmem_dataOut;/////////result_mem
    sc_signal<sc_uint<32> > dmem_addOut;
    sc_signal<bool> stallDm;/////////stallDm 

    riscv_core *u_riscv;
    riscv_imemory *u_imem;
    riscv_dmemory *u_dmem;

    SC_CTOR(riscv_top){
        u_riscv = new riscv_core("riscv");
        u_riscv->clk(clk);
        u_riscv->rst(rst);
        u_riscv->imem_address(imem_address);
        u_riscv->imem_mask(imem_mask);
        u_riscv->imem_opType(imem_opType);
        u_riscv->imem_dataIn(imem_dataIn);
        u_riscv->instruction_ft(instruction_ft);
        u_riscv->imem_addOut(imem_addOut);
        u_riscv->stallIm(stallIm);

        u_riscv->dmem_address(dmem_address);
        u_riscv->dmem_mask(dmem_mask);
        u_riscv->dmem_opType(dmem_opType);
        u_riscv->dmem_dataIn(dmem_dataIn);
        u_riscv->dmem_dataOut(dmem_dataOut);
        u_riscv->dmem_addOut(dmem_addOut);
        u_riscv->stallDm(stallDm);

        u_imem = new riscv_imemory("icache");
        u_imem->clk(clk);
        u_imem->rst(rst);
        u_imem->imem_address(imem_address);
        u_imem->imem_mask(imem_mask);
        u_imem->imem_opType(imem_opType);
        u_imem->imem_dataIn(imem_dataIn);
        u_imem->instruction_ft(instruction_ft);
        u_imem->imem_addOut(imem_addOut);
        u_imem->stallIm(stallIm);

        u_dmem = new riscv_dmemory("dcache");
        u_dmem->clk(clk);
        u_dmem->rst(rst);
        u_dmem->dmem_address(dmem_address);
        u_dmem->dmem_mask(dmem_mask);
        u_dmem->dmem_opType(dmem_opType);
        u_dmem->dmem_dataIn(dmem_dataIn);
        u_dmem->dmem_dataOut(dmem_dataOut);
        u_dmem->dmem_addOut(dmem_addOut);
        u_dmem->stallDm(stallDm);
    }
    

};

#ifdef SYSCSIMULATION
int sc_main(int argc, char** argv)
{
    sc_clock                clk("clk", 25, SC_NS, 0.5, 12.5, SC_NS, true);
    sc_signal<bool>         rst;

    riscv_top u_riscv_top("riscvtop");

    u_riscv_top.clk(clk);
    u_riscv_top.rst(rst);

    std::cout<<"start rest"<<std::endl;
    sc_start( 2.5, SC_NS );
    rst.write(0);

    std::cout<<"end rest"<<std::endl;

    sc_start( 200, SC_NS );
    rst.write(1);

    std::cout<<"start first clk "<<std::endl;
    sc_start();


    return 0;

};
#endif
