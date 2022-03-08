#include "systemc.h"
#include <stdio.h>
#include <iostream>
#include "riscvISA.h"

#define PC_STARTADDR 0
#define IRAM_SIZE 2048*4
#define DRAM_SIZE 2048*4
#define STACK_INIT 2048*4
#define RAADDRESS 1073741824

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
public:
    sc_in<bool> clk;
    sc_in<bool> rst;

    void core();
    ////input
    sc_out<sc_uint<32> > imem_address;//////pc_ft
    sc_out<sc_uint<3> > imem_mask;
    sc_out<sc_uint<2> > imem_opType;
    sc_out<sc_uint<32> > imem_dataIn;
    ////output
    sc_in<sc_uint<32> > instruction_ft;///////instruction_ft
    sc_in<sc_uint<32> > imem_addOut;
    sc_in<bool> stallIm;//////stallIm


    // void cache_dmem();
    sc_out<sc_uint<32> > dmem_address;
    sc_out<sc_uint<3> > dmem_mask;
    sc_out<sc_uint<2> > dmem_opType;
    sc_out<sc_uint<32> > dmem_dataIn;
    ////output
    sc_in<sc_uint<32> > dmem_dataOut;/////////result_mem
    sc_in<sc_uint<32> > dmem_addOut;
    sc_in<bool> stallDm;/////////stallDm

    SC_CTOR(riscv_core){
        SC_CTHREAD(core,clk.pos());
        reset_signal_is(rst,false);
        sensitive<<clk.pos();
    }
    
    ~riscv_core(){}
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
int sc_main(int argc, char** argv){
    sc_clock                clk("clk", 25, SC_NS, 0.5, 12.5, SC_NS, true);
    sc_signal<bool>         rst;
    
    riscv_top u_riscv_top("riscvtop");

    u_riscv_top.clk(clk);
    u_riscv_top.rst(rst);

    std::cout<<"start rest"<<std::endl;
    sc_start( 25, SC_NS );
    rst.write(0);

    std::cout<<"end rest"<<std::endl;

    sc_start( 25, SC_NS );
    rst.write(1);

    std::cout<<"start first clk "<<std::endl;
    sc_start();
    return 0;
}
#endif
