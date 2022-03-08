import glob
import sys
import os
sys.path.insert(0,os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
# sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
# for p in sys.path:
#     print(p)
# sys.exit('exit')
from ass2bin.ass2bin_wrap import ass2bin

import multiprocessing
import time

class update(ass2bin):
    def __init__(self, name='fft', stack_size=2048):
        super().__init__(name+'assemb.S', stack_size)
        self.bench_name = name#.replace('assemb.S','')
    ####run make file under the specific folder
    def run_make(self,folder_name):
        rootpath = os.getcwd()
        os.chdir(folder_name)
        if(os.system('make clean') ):
            print('error reported to run MAKE under '+folder_name)
            sys.exit("some error message")
        if(os.system('make') ):
            print('error reported to run MAKE under '+folder_name)
            sys.exit("some error message")
        
        os.chdir(rootpath)

    def run_ass2bin(self):
        ####run make file to for benchmark
        self.run_make(f'../bench_cpp/{self.bench_name}')
        ####copy S file to ass2bin folder
        os.system(f'cp ../bench_cpp/{self.bench_name}/{self.bench_name}assemb.S  ../ass2bin/{self.bench_name}assemb.S')

        ####generate the ins and data
        rootpath = os.getcwd()
        os.chdir('../ass2bin')
        super().run()
        os.chdir(rootpath)

        os.system(f'cp ../ass2bin/imem.txt  ../pipeline/imem.txt')
        os.system(f'cp ../ass2bin/dmem.txt  ../pipeline/dmem.txt')
        os.system(f'cp ../ass2bin/scriscv_core.h  ../pipeline/scriscv_core.h')

        os.system(f'cp ../ass2bin/imem.txt  ../scalar/imem.txt')
        os.system(f'cp ../ass2bin/dmem.txt  ../scalar/dmem.txt') 
        os.system(f'cp ../ass2bin/scriscv_scalar.h  ../scalar/scriscv_scalar.h')
    
    def run_scsim(self):
        rootpath = os.getcwd()
        os.chdir('../scalar')
        self.run_make('./')
        # if(os.system('make') ):
        #     print('error reported to run MAKE under scalar')
        #     sys.exit("some error message")
        os.system('./test.exe &> scoutput.txt')
        os.chdir(rootpath)

        os.chdir('../pipeline')
        self.run_make('./')
        # if(os.system('make') ):
        #     print('error reported to run MAKE under pipeline')
        #     sys.exit("some error message")
        os.system('./test.exe &> ppoutput.txt')
        os.chdir(rootpath)

        return
    def run(self):
        self.run_ass2bin()

        ####make to sc simulation for riscv
        self.run_scsim()


if __name__ == '__main__':
    # Start foo as a process
    if len(sys.argv)==1:
        u_update0 = update()
    elif len(sys.argv)==2:
        u_update0 = update(name = sys.argv[1], )
    elif len(sys.argv)==3:
        u_update0 = update(name = sys.argv[1], stack_size=int(sys.argv[2]))
    p = multiprocessing.Process(target=u_update0.run, name="u_update0")
    p.start()

    # Wait 10 seconds for foo
    time.sleep(30)
    if p.is_alive():
        # Terminate foo
        p.terminate()
        # Cleanup
        p.join()
        os.system('pkill -f "test.exe"')
        print('error ! the program could not stop in required time')
    
    
