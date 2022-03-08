# import glob
import re
import math
import os

# class ass2bin:
#     def __init__(self):
class ass2bin():
    def __init__(self,name='fftassemb.S',stack_size=2048):
        self.filename = name
        self.stack_size = stack_size
        self.ra = 0x40000000####reg[1]   <--- if pc==x1: exit
        ####pre zero instruction in imem to prevent the negative triggered imem module run faster than fetch
        self.pre_zero = 2
        self.cusins_set = {'printf':'1','putchar':'2','acc1':'3'}###0 -> normal jalr
        self.r_map,self.instr_data = self.__pre()
        self.temp_scname = 'template_scriscv_scalar.h'
        self.temp_ppname = 'template_scriscv_core.h'

        
    def __premap__(self):
        # self.r_map = {}
        # self.instr_data = {}
        self.R_instr = [
            "add","sub", "sll", 
            "slt","sltu", "xor", "srl", 
            "sra", "or", "and",
            "addw", "subw", "sllw",
            "slrw", "sraw", "mul",
            "mulh", "mulu", "mulsu",
            "div", "divu", "rem",
            "remu"
        ]
        self.I_instr = [
            "addi", "lb", "lh","lw",
            "ld", "lbu", "lhu",
            "lwu", "fence", "fence.i", 
            "slli", "slti", "sltiu", 
            "xori", "srli", "srai",
            "ori", "andi", "addiw",
            "slliw", "srliw", "sraiw", 
            "jalr", "ecall", "ebreak", 
            "CSRRW", "CSRRS","CSRRC", 
            "CSRRWI", "CSRRSI", "CSRRCI" 
        ]
        ####fence fenc not working now
        self.S_instr = [
            "sw", "sb", "sh", 
            "sd"
        ]
        self.SB_instr = [
            "beq", "bne", "blt", 
            "bge", "bltu", "bgeu"
        ]
        self.U_instr = ["auipc", "lui"]
        self.UJ_instr = ["jal"]
        self.pseudo_instr = [
            "beqz", "bnez", "li", 
            "mv", "j", "jr", 
            "la", "neg", "nop", 
            "not", "ret", "seqz", 
            "snez", "bgt", "ble",
            "snez","bleu","bgtu"
        ]
        self.customized_instr = ['jalrcus']

        self.all_instr = self.flatten([
            self.R_instr, self.I_instr, self.S_instr,
            self.SB_instr, self.U_instr, self.UJ_instr, 
            self.pseudo_instr,self.customized_instr
        ])

    ####flat 2d array to 1d array
    def flatten(self,x):
        arr = []
        for e in x:
            if not isinstance(e, list):
                arr.append(e)
            else:
                arr.extend(e)
        return arr


    ###### convert int to binary string
    ###### boarder = big endien(high bit -> low bit) = little endien(low bit -> high bit)
    def __binary(self,x:int, size:int, border = 'big'):
        x = int (x)
        size = int(size)
        byte_num = math.ceil(size/8)
        b_num = x.to_bytes(byte_num, byteorder = 'big', signed = True)

        fin_bin = ''.join(format(byte, '08b') for byte in b_num)
        
        if(border=='big'):
            if byte_num*8 == size:
                return fin_bin
            return fin_bin[len(fin_bin)-size:len(fin_bin)]
        else:
            fin_bin = fin_bin[::-1]
            if byte_num*8 == size:
                return fin_bin
            return fin_bin[:size]

    def R_type(
                self,instr, rs1, 
                rs2, rd):
        if instr not in self.R_instr:
            print("wrong R instruction type")
            return
            # raise WrongInstructionType()
        opcode = 0;f3 = 1;f7 = 2
        return "".join([
            self.instr_data[instr][f7],
            # __reg_to_bin(rs2),
            self.__binary(rs2,5),
            # __reg_to_bin(rs1),
            self.__binary(rs1,5),
            self.instr_data[instr][f3],
            # __reg_to_bin(rd),
            self.__binary(rd,5),
            self.instr_data[instr][opcode]
        ])

    def I_type(
            self,instr, rs1, 
            imm, rd):

        non_imm_ins = {'slli','srli','srai'}
        if instr not in self.I_instr:
            # raise WrongInstructionType()
            print("wrong I instruction type")
            return
        opcode = 0;f3 = 1;f7 = 2
        # mod_imm = int(imm) - ((int(imm)>>12)<<12) # imm[11:0]
        if(imm[:2]=='0x'):
            imm = int(imm[2:],base=16)
        lit_imm = self.__binary(imm,12,'little')

        if(instr in non_imm_ins):
            return "".join([self.instr_data[instr][f7],
            self.__binary(imm,5),
            self.__binary(rs1,5),
            self.instr_data[instr][f3],
            self.__binary(rd,5),
            self.instr_data[instr][opcode],
            ])
        return "".join([
            # self.__binary(mod_imm,12),
            lit_imm[::-1],
            # self.__reg_to_bin(rs1),
            self.__binary(rs1,5),
            self.instr_data[instr][f3],
            # self.__reg_to_bin(rd),
            self.__binary(rd,5),
            self.instr_data[instr][opcode]
        ])

    #### 31    12                11   7    6    0
    #### imm[20|10:1|11|19:12]   rd        opCode
    def UJ_type(
            self,instr, 
            imm, rd):

        if instr not in self.UJ_instr:
            # raise WrongInstructionType()
            print("wrong UJ instruction type")
            return

        opcode = 0;f3 = 1;f7 = 2

        # mod_imm = ((int(imm) - ((int(imm) >> 20) << 20)) >> 19) << 19 # imm[20]
        # mod_imm += (int(imm) - ((int(imm) >> 10) << 10)) >> 1 # imm[20|10:1]
        # mod_imm += (int(imm) - ((int(imm) >> 11) << 11)) >> 10 # imm[20|10:1|11]
        # mod_imm += (int(imm) - ((int(imm) >> 19) << 19)) >> 12 # imm[20|10:1|11|19:12]
        lit_imm = self.__binary(imm,21,'little')
        
        return  "".join([		
            # __binary(mod_imm,20),
            lit_imm[20] + lit_imm[10:0:-1] + lit_imm[11]+lit_imm[19:11:-1],   # imm[31 -> 20|30:21 -> 10:1|20 -> 11|19:12]  11..0
            self.__binary(rd,5),
            self.instr_data[instr][opcode]
        ])

    def S_type(
            self,instr, rs1, 
            rs2, imm):

        if instr not in self.S_instr:
            print("wrong S instruction type")
            return '1'*32

        opcode = 0;f3 = 1;f7 = 2
        lit_imm = self.__binary(imm,12,'little')
        mod_imm = lit_imm[5:12][::-1]
        # mod_imm = (int(imm) - ((int(imm) >> 12) << 12)) >> 5 # imm[11:5]
        mod_imm_2 = lit_imm[4::-1]
        # mod_imm_2 = int(imm) - ((int(imm) >> 5) << 5) # imm[4:0]
        return "".join([
            #self.__binary(int(imm),12)[::-1][5:12][::-1],
            mod_imm, # imm[11:5]
            # self.__reg_to_bin(rs2),
            self.__binary(rs2,5),
            # self.__reg_to_bin(rs1),
            self.__binary(rs1,5),
            self.instr_data[instr][f3],
            #self.__binary(int(imm),12)[::-1][0:5][::-1],
            mod_imm_2, # imm[4:0]
            self.instr_data[instr][opcode]
        ])

    def CUSTI_type(
            self,f3):
        f3 = int(f3)
        if f3<=0 or f3>=8:
            # raise WrongInstructionType()
            print("wrong CUSTI_type instruction type")
            return
        opcode = 0
        
        
        return "".join([
            # self.__binary(mod_imm,12),
            self.__binary(0,12,'little')[::-1],
            # self.__reg_to_bin(rs1),
            self.__binary(0,5),
            self.__binary(f3,3),
            # self.__reg_to_bin(rd),
            self.__binary(0,5),
            self.instr_data['jalr'][opcode]
        ])

    def SB_type(
            self,instr, rs1, 
            rs2, imm):

        if instr not in self.SB_instr:
            print("wrong SB instruction type")
            return '1'*32

        opcode = 0;f3 = 1;f7 = 2
        return "".join([
            "".join([
                self.__binary(int(imm),13)[::-1][12][::-1],
                self.__binary(int(imm),13)[::-1][5:11][::-1]
            ]),
            self.__binary(rs2,5),
            self.__binary(rs1,5),
            self.instr_data[instr][f3],
            "".join([
                self.__binary(int(imm),13)[::-1][1:5][::-1],
                self.__binary(int(imm),13)[::-1][11][::-1]
            ]),
            self.instr_data[instr][opcode]
        ])

    def U_type(
            self,instr, 
            imm, rd):
        # "auipc", "lui"
        if instr not in self.U_instr:
            print("wrong U instruction type")
            return '1'*32
        opcode = 0;f3 = 1;f7 = 2

        # mod_imm = (int(imm) >> 12)
        mod_imm = int(imm)
        return "".join([
            #self.__binary(int(imm),32)[::-1][12:32][::-1],
            self.__binary(mod_imm,20),
            self.__binary(rd,5),
            self.instr_data[instr][opcode]
        ])

    ######read the insruction and register map
    def __pre(self):
        r_p = {}
        f = open('../doc/reg_map.txt',"r")
        #f = open("riscinterpreter/data/reg_map.dat", "r")
        #f = open("src/data/reg_map.dat","r")
        line = f.readline()

        #assign mapping 
        while line != "":
            elems = line.split()
            r_p[elems[0]] = int(elems[1][1:]) 
            line = f.readline()

        f.close()
        #index for instr_data
        # opcode = 0
        # f3 = 1
        # f7 = 2

        #order is [opcode, f3, f7]
        i_data = {}
        instr_path = "../doc/instr_map.txt"
        f = open(instr_path,"r")
        #f = open("riscinterpreter/data/instr_data.dat", "r")
        #f = open("src/data/instr_data.dat","r")
        line = f.readline()

        #assign data
        while line != "":
            elems = line.replace("\n","").split()
            i_data[elems[0]] = elems[1::]
            line = f.readline()
        f.close()
        return r_p,i_data

    

    ######calculate the distance between the current line and the destination line
    def calcJump(self,x,line_num):
        if(x in self.ip):
            return (self.ip[x]-line_num)*4
        
        print('error to find ',x)
        return -10





    def __valid_line(self,x, allow_colon = False):
        if x[0][0] == "#" or x[0][0] == "\n" or x[0][0] == "" or x[0][0] == ".":
            return False

        if not allow_colon and x[0][-1] == ":" :
            return False
        return True

    def expand_ins(self,line):
        # "li": #need to consider larger than 12 bits
        # if int(clean[2]) > 2**11:
        #     res.append(U_type(instr='lui', imm=clean[2], rd=r_map[clean[1]] ))
        # res.append(I_type("addi", r_map[clean[1]], clean[2], r_map[clean[1]] ))

        # call offset   tail offset 
        # auipc x6, offset[31:12] Call far-away subroutine   -> auipc x6, %H20(offset)
        # jalr x1, x6, offset[11:0]    -> jalr x1, x6x %L12(offset)

        # call putchar/acc0
        # jalrcus imm
        newline = []
        clean = self.flatten([elem.replace("\n","").split(",") for elem in line.strip().split()])
        if(clean[0]=='li'):
            
            check_val = int (clean[2])
            ####check_val   0x00000 1
            binary_clean = self.__binary(check_val,32)
            if(binary_clean[:21]=='0'*21):
                #### pos< 2**11
                newline.append(f'addi {clean[1]},zero,{str( (int(binary_clean[20:],base=2) ))  }\n')
            elif(binary_clean[:21]=='1'*21):
                #### neg> -2**11
                newline.append(f'addi {clean[1]},zero,{str( (int(binary_clean[20:],base=2) ))  }\n')
            elif(binary_clean[0]=='0' and binary_clean[20]=='1') or\
                (binary_clean[0]=='1' and binary_clean[:20]!='1'*20 and binary_clean[20]=='1'):
                ####
                ####corner case   0xffff e   e ff   -> 0xffff f(e+1) 000   +    0xfffffeff    <----neg[31:12]  neg[11:0]
                ####                    12  11
                ####corner case   0x0000 0   e ff   -> 0x0000 1(0+1) 000   +    0xfffffeff    <----pos[31:12]  neg[11:0]
                ####                    12  11
                binary_clean_list = [int(_) for _ in binary_clean]
                
                for i in range(19,-1,-1):
                    binary_clean_list[i] ^= 1
                    if(binary_clean_list[i] == 1):
                        break
                binary_clean = ''.join([str(_) for _ in binary_clean_list])
                newline.append(f'lui {clean[1]},{str( int(binary_clean[:20],base=2) )}\n' )
                newline.append(f'addi {clean[1]},{clean[1]},{str( (int(binary_clean[20:],base=2) ))  }\n')
            else:
                ####<----pos[31:12]  pos[11:0]
                ####<----neg[31:12]  pos[11:0]
                newline.append(f'lui {clean[1]},{str( int(binary_clean[:20],base=2) )}\n' )
                newline.append(f'addi {clean[1]},{clean[1]},{str( (int(binary_clean[20:],base=2) ))  }\n')
        elif(clean[0]=='call'):
            if(clean[1] in self.cusins_set):
                newline.append(f'jalrcus {self.cusins_set[line.strip().split()[1]]}\n')
                return newline
            newline.append(f'auipc t1,%h20({clean[1]})\n' )
            newline.append(f'jalr ra,t1,%l12({clean[1]})\n' )
        else:
            return [line]
        return newline
        
    ####input: contents(list)
    ####output: dmem_contents
    def __pre_mapfilt(self,contents):
        #### value under .LC0 .LC1 ...
        dmem_contents = []
        #### postion of .LC0  .LC1 ...
        dmem_position = {}
        #### position of main  subfunc1 ...
        imem_position = {}
        #### valid instruction ("." without word/byte/char  or # start)
        imem_contents = []

        #### need to complement
        dmem_type = {'.word':4,'.half':2,'.byte':1}
        i_idx = 0

        new_contents = []
        

        # with open('memcpy_test.txt','r') as f:
        #     new_contents += f.readlines()

        for i,v in enumerate(contents):
            # print(expand_ins(v))
            new_contents += self.expand_ins(v)
        with open('../doc/memcpy_test.txt','r') as f:
            new_contents += f.readlines()
        # new_contents = ['addi zero,zero,0\n']*pre_zero
        for i,n in enumerate(new_contents):
            buf_n = n.strip().split()
            if('main:' in buf_n[0]):
                new_contents = new_contents[:i+1]+ ['addi zero,zero,0\n']*self.pre_zero + new_contents[i+1:]
                # print(new_contents[i])
                break
        

        with open('output_as.txt','w') as f:
            f.writelines(new_contents)
        
        byte_num = 0
        half_num = 0
        for i,v in enumerate(new_contents):
            v = v.strip().split()
            if(len(v)==1 and v[0][-1]==':'):####postion
                if re.match(r'.LC\d+',v[0]):
                    dmem_position[v[0].replace(':','')] = len(dmem_contents)
                    imem_position[v[0][:-1]] = len(dmem_contents)
                elif v[0][-1]==':':
                    imem_position[v[0][:-1]] = len(imem_contents)#############################################
                else:
                    print('error with the :',i,v)
            elif v[0] in dmem_type:
                if(len(v)!=2):
                    print('error with the dmem_type',i,v)
                    continue
                if dmem_type[v[0]] == 4:
                    ####word
                    dmem_contents.append( self.__binary(int(v[1]),32) )
                ###################################basic riscv: little endien
                ###### mos          lea
                ######word eg: 0x 87 65 43 21
                ###### ->0x 87 65 43 21
                ######half eg: 0x 87 65, 0x 43 21
                ###### ->0x 43 21,87 65 
                ######byte eg: 0x 87, 0x 65, 0x 43, 0x 21
                ###### ->0x 21,43,65,87 

                elif dmem_type[v[0]] == 2:
                    ####half
                    if(half_num==0):
                        half_num += 1
                        dmem_contents.append( '0'*16+self.__binary(int(v[1]),16) )
                    elif(half_num==1):
                        half_num = 0
                        dmem_contents[-1] = self.__binary(int(v[1]),16) + dmem_contents[-1][16:]
                    
                elif dmem_type[v[0]] == 1:
                    ####byte
                    if(byte_num==0):
                        byte_num += 1
                        dmem_contents.append( '0'*24+self.__binary(int(v[1]),8) )
                    elif(byte_num==1):
                        byte_num += 1
                        dmem_contents[-1] = '0'*16 + self.__binary(int(v[1]),8) + dmem_contents[-1][24:]
                    elif(byte_num==2):
                        byte_num += 1
                        dmem_contents[-1] = '0'*8 + self.__binary(int(v[1]),8) + dmem_contents[-1][16:]
                    elif(byte_num==3):
                        byte_num = 0
                        dmem_contents[-1] = self.__binary(int(v[1]),8) + dmem_contents[-1][8:]
                else:
                    print('the dmem_type is missing')
            elif not self.__valid_line(v):
                print(' current line is invalid',i,v)
            else:
                ####
                imem_contents.append(new_contents[i])
        return imem_contents,imem_position,dmem_contents,dmem_position

    def subsidmem(self,line,i):
        # lui	a5,%hi(.LC0)    ->    lui	a5,###
        # addi	a4,a5,%lo(.LC0)    ->    addi	a4,a5,###
        line = line.strip()
        if('%hi' in line):
            dmem_flag = re.match(r'.*?%hi\((.*?)\)',line).groups()[0]
            if dmem_flag not in self.dp:
                print('could not find the hi at subsdmem in line ',line,dmem_flag)
            else:
                new_line = line.split(',')
                # new_line[-1] = str(dp[dmem_flag]>>16)####mask lower bits
                # return ','.join(new_line)
                binary_clean = self.__binary( self.dp[dmem_flag] *4 ,32)

                if(binary_clean[0]=='0' and binary_clean[16]=='1') or\
                    (binary_clean[0]=='1'  and binary_clean[16]=='1'):###and binary_clean[:16]!='1'*20    need two lines
                    ####
                    ####corner case   0xffff e   e ff   -> 0xffff f(e+1) 000   +    0xfffffeff    <----neg[31:12]  neg[11:0]
                    ####                    12  11
                    ####corner case   0x0000 0   e ff   -> 0x0000 1(0+1) 000   +    0xfffffeff    <----pos[31:12]  neg[11:0]
                    ####                    12  11
                    binary_clean_list = [int(_) for _ in binary_clean]
                    
                    for i in range(15,-1,-1):
                        binary_clean_list[i] ^= 1
                        if(binary_clean_list[i] == 1):
                            break
                    binary_clean = ''.join([str(_) for _ in binary_clean_list])
                # return f'auipc t1,{str( int(binary_clean[:20],base=2) )}'
                new_line[-1] = str(int(binary_clean[:16],base=2) )####mask lower bits
                return ','.join(new_line)

        elif('%lo' in line):
            dmem_flag = re.match(r'.*?%lo\((.*?)\)',line).groups()[0]
            if dmem_flag not in self.dp:
                print('could not find the hi at subsdmem in line ',line,dmem_flag)
            else:
                new_line = line.split(',')
                new_line[-1] = str( (self.dp[dmem_flag]*4) & ((1<<16) -1))####mask higher bits
                return ','.join(new_line)
        elif('%h20' in line):###auipc
            imem_flag = re.match(r'.*?%h20\((.*?)\)',line).groups()[0]  

            if imem_flag not in self.ip:
                print('could not find the h20 at subsdmem in line ',line,imem_flag)
            else:
                ####check corner   0x00000 1
                binary_clean = self.__binary(self.calcJump(imem_flag,i) ,32)

                if(binary_clean[0]=='0' and binary_clean[20]=='1') or\
                    (binary_clean[0]=='1'and binary_clean[20]=='1'):### and binary_clean[:20]!='1'*20 
                    ####
                    ####corner case   0xffff e   e ff   -> 0xffff f(e+1) 000   +    0xfffffeff    <----neg[31:12]  neg[11:0]
                    ####                    12  11
                    ####corner case   0x0000 0   e ff   -> 0x0000 1(0+1) 000   +    0xfffffeff    <----pos[31:12]  neg[11:0]
                    ####                    12  11
                    binary_clean_list = [int(_) for _ in binary_clean]
                    
                    for i in range(19,-1,-1):
                        binary_clean_list[i] ^= 1
                        if(binary_clean_list[i] == 1):
                            break
                    binary_clean = ''.join([str(_) for _ in binary_clean_list])
                return f'auipc t1,{str( int(binary_clean[:20],base=2) )}'
        elif('%l12' in line):####jalr
            imem_flag = re.match(r'.*?%l12\((.*?)\)',line).groups()[0]
            if imem_flag not in self.ip:
                print('could not find the l12 at subsdmem in line ',line,imem_flag)
            else:
                return f'jalr ra, t1, {str( int(self.__binary(self.calcJump(imem_flag,i-1) ,32) [20:],base=2) )}'
        else:
            return line



    def __interpret(self,line,i):
        res = []
        # line = __handle_inline_comments(line)
        line = line.strip()
        #print(line)
        clean = self.flatten([elem.replace("\n","").split(",") for elem in line.split()])

        while "" in clean:
            clean.remove("")
        # print(clean)
        #check if line is comment, empty space, .global .text
        if not self.__valid_line(clean):
            print('not valid')
            return ['-1']

        if clean[0] == "ecall":
            print('ecall')
            return ['-1']
        
        if clean[0] == "sw" or clean[0] == "lw" or clean[0] == "lb" or clean[0] == "lbu"  or clean[0] == "lh" or clean[0] == "sb" or clean[0] == "sh" or clean[0] == 'lhu' or clean[0] == "lwu" :
            #sw s0, 0(sp)
            w_spl = clean[2].split("(")
            clean[2] = w_spl[0]
            clean.append(w_spl[1].replace(")",""))
        # print(clean)
        ####Rtype   op dest,rs0,rs1
        if clean[0] in self.R_instr:
            res.append(self.R_type(clean[0], self.r_map[clean[2]], self.r_map[clean[3]], self.r_map[clean[1]] ) )
        
        elif clean[0] in self.I_instr:
            if clean[0] == "jalr":
                if len(clean) == 4:
                    res.append(self.I_type(clean[0], self.r_map[clean[2]], clean[3], self.r_map[clean[1]] ) )
                    # res.append(self.I_type(clean[0], self.__reg_map(clean[2]), self.calcJump(clean[3],i),self.__reg_map(clean[1])))
                else:
                    res.append(self.I_type(clean[0], self.r_map[clean[1]], "0", self.r_map["ra"]) )
                    # res.append(self.I_type(clean[0], self.__reg_map(clean[1]), "0", self.__reg_map("x1")))
            elif clean[0] == "lw" or clean[0] == "lh" or clean[0] == "lb" or clean[0] == "ld" or clean[0] == "lbu" or clean[0] == "lhu" or clean[0] == "lwu" :####  lw rd imm(rs)
                # print(clean)
                res.append(self.I_type(clean[0], self.r_map[clean[3]], clean[2], self.r_map[clean[1]]) )
            else:####  addi rd imm rs
                # print(clean,i,line)
                res.append(self.I_type(clean[0], self.r_map[clean[2]], clean[3], self.r_map[clean[1]] ) )
        
        elif clean[0] in self.UJ_instr:####jal
            if len(clean) == 3:      
                res.append( self.UJ_type(clean[0], self.calcJump(clean[2],i), self.r_map[clean[1]] ) )
            else:
                res.append( self.UJ_type(clean[0], self.calcJump(clean[1],i), self.r_map["ra"] ))
        elif clean[0] in self.S_instr:
            res.append(self.S_type(clean[0], self.r_map[clean[3]], self.r_map[clean[1]], clean[2]) )
        
        elif clean[0] in self.U_instr:
            res.append(self.U_type(clean[0], clean[2], self.r_map[clean[1]] ))

        elif clean[0] in self.SB_instr:
            res.append(self.SB_type(clean[0], self.r_map[clean[1]], self.r_map[clean[2]], self.calcJump(clean[3],i) ) )
        elif clean[0] in self.pseudo_instr:
            if clean[0] == "j":
                #### 31    12                11   7    6    0
                #### imm[20|10:1|11|19:12]   rd        opCode
                # print(clean)
                res.append(self.UJ_type("jal", self.calcJump(clean[1],i), self.r_map["zero"] ))
                # print(UJ_type("jal", calcJump(clean[1],i), r_map["zero"] ),calcJump(clean[1],i),clean[1],i)
            
            elif clean[0] == "li": #need to consider larger than 12 bits
                # if int(clean[2]) > 2**11:###done in advance
                #     res.append(U_type(instr='lui', imm=clean[2], rd=r_map[clean[1]] ))
                res.append(self.I_type("addi", self.r_map[clean[1]], clean[2], self.r_map[clean[1]] ))
            elif clean[0] == "nop":
                res.append(self.I_type("addi", self.r_map["zero"], "0", self.r_map["zero"] ))
            elif clean[0] == "mv":
                res.append(self.I_type("addi", self.r_map[clean[2]], "0", self.r_map[clean[1]] ) )
            # elif clean[0] == "not":
            #     res.append(self.I_type("xori", self.__reg_map(clean[2]), "-1", self.__reg_map(clean[1])))
            elif clean[0] == "neg":
                res.append(self.R_type("sub", self.r_map["zero"], self.r_map[clean[2]], self.r_map[clean[1]] ))
            # elif clean[0] == "la":
            #     res.append(self.U_type("auipc", self.calcJump(clean[2],i), self.__reg_map(clean[1])))
            # elif clean[0] == "j":
            #     res.append(self.UJ_type("jal", self.calcJump(clean[1],i), self.__reg_map("zero")))
            elif clean[0] == "jr":
                res.append(self.I_type("jalr", self.r_map[clean[1]], "0", self.r_map["zero"]))
            elif clean[0] == "ret":
                res.append(self.I_type("jalr", self.r_map["ra"], "0", self.r_map["zero"]))
            elif clean[0] == "bgt" or clean[0] == "bgtu":
                res.append(self.SB_type("blt", self.r_map[clean[2]], self.r_map[clean[1]], self.calcJump(clean[3],i)))
            elif clean[0] == "ble" or clean[0] == "bleu":
                res.append(self.SB_type("bge", self.r_map[clean[2]], self.r_map[clean[1]], self.calcJump(clean[3], i)))
            else:
                print('not found in psedo_inst',line,i)
                res.append(line)
            
        elif clean[0] in self.customized_instr:
            print(clean[1])
            res.append(self.CUSTI_type(clean[1]))
        else:
            res.append(line)
        return res
        
    ####filt the dummy instruction
    def __read_in_advance(self,filename):
        code = []
        file = open(filename, "r")

        #store the lines in the arr
        line = file.readline()
        # i = 0
        while line != "":
            line = line.strip()
            clean = self.flatten([elem.replace("\n","").split(",") for elem in line.split()])##split  \n  \t \s
            
            if line == "" or not self.__valid_line(clean, True):
                print(line)
                line = file.readline()
                continue
            # print(line.split())
            # print(clean)
            code.append(line.strip())
            line = file.readline()
            # i+=1
            # if(i==5):
            #     return 0

        return code
    # ###################################################################################
    # ###################################################################################
    # start to run
    def run(self):
        with open(self.filename,'r') as f:
            contents = f.readlines()

        self.__premap__()
        #### expand pseudocode to multiple lines
        self.ic,self.ip,self.dc,self.dp = self.__pre_mapfilt(contents)

        #### substitute the %hi %lo %h20 %l12 to real number
        with open('filtered_imem.txt','w') as f:
            for i,_ in enumerate(self.ic):
                _ = self.subsidmem(_,i)
                f.write(_+'\n')

        #### filter the dummy instructions
        ins_filt = self.__read_in_advance('filtered_imem.txt')


        with open('bimem.txt','w') as f:
            for i in range(len(ins_filt)):
                f.write(''.join(self.__interpret(ins_filt[i],i))+'\n')


        with open('imem.txt','w') as f:
            with open('bimem.txt','r') as ff:
                b_contents = ff.readlines()
            
            # for i in range(pre_zero):
            #     f.write('0x'+'0'*8+',\n')
            for i in range(len(b_contents)):
                line = hex( int(b_contents[i].strip() , 2) )
                line = '0x'+(10-len(line))*'0'+line[2:]+',\n'
                f.write(line)
            if(len(b_contents)>=self.stack_size):
                print('error report! the stack is overflow \n')
                
            for i in range(len(b_contents),self.stack_size):
                f.write('0x'+'0'*8+',\n')


        with open('dmem.txt','w') as f:
            for i in range(len(self.dc)):
                line = hex( int(self.dc[i],2) )
                line = '0x'+(10-len(line))*'0'+line[2:]+',\n'
                f.write(line)
            for i in range(len(self.dc),self.stack_size):
                f.write('0x'+'0'*8+',\n')

        # pc = self.ip['main']

        with open(f'../doc/{self.temp_ppname}','r') as f:
            pipe_contents = f.readlines()
            for i in range(len(pipe_contents)):
                if ('PC_STARTADDR' in pipe_contents[i]):
                    pipe_contents[i] = f'#define PC_STARTADDR '+ str(self.ip['main']*4) +'\n'
                
                if('RAADDRESS' in pipe_contents[i]):
                    pipe_contents[i] = f'#define RAADDRESS '+ str(self.ra) +'\n'
                
                if('DRAM_SIZE' in pipe_contents[i]):
                    pipe_contents[i] = f'#define DRAM_SIZE '+ str(self.stack_size*4) +'\n'
                
                if('IRAM_SIZE' in pipe_contents[i]):
                    pipe_contents[i] = f'#define IRAM_SIZE '+ str(self.stack_size*4) +'\n'
                
                if('STACK_INIT' in pipe_contents[i]):
                    pipe_contents[i] = f'#define STACK_INIT '+ str(self.stack_size*4) +'\n'

        with open('scriscv_core.h','w') as f:
            f.writelines(pipe_contents)

        with open(f'../doc/{self.temp_scname}','r') as f:
            scalar_contents = f.readlines()
            for i in range(len(scalar_contents)):
                if ('PC_STARTADDR' in scalar_contents[i]):
                    print('the pc will start from '+ str(self.ip['main']*4) )
                    scalar_contents[i] = f'#define PC_STARTADDR '+ str(self.ip['main']*4) +'\n'
                
                if('RAADDRESS' in scalar_contents[i]):
                    scalar_contents[i] = f'#define RAADDRESS '+ str(self.ra) +'\n'
                
                if('IRAM_SIZE' in scalar_contents[i]):
                    scalar_contents[i] = f'#define IRAM_SIZE '+ str(self.stack_size*4) +'\n'

                if('DRAM_SIZE' in scalar_contents[i]):
                    scalar_contents[i] = f'#define DRAM_SIZE '+ str(self.stack_size*4) +'\n'
                
                if('STACK_INIT' in scalar_contents[i]):
                    scalar_contents[i] = f'#define STACK_INIT '+ str(self.stack_size*4) +'\n'

        with open('scriscv_scalar.h','w') as f:
            f.writelines(scalar_contents)





