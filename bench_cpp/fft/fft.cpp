#ifdef CHECKPRINT
    #include "stdio.h"
#endif

#define M 4
#define N 16
#define len 8
int main(void){
    
    int data_in[32][2] = {
        #include "fft_input.txt"
    };
    int data_out[16][2];

    int sample[16][2];
    int index = 0;
    
    int theta = 39;
    int w_real = 92;
    int w_imag = -38;

    int loops = 0;
    int W[7][2];
    int w_rec_real = 1;
    int w_rec_imag = 0;
    int w_temp;
    int  tmp_real, tmp_imag, tmp_real2, tmp_imag2;
    unsigned int stage, i, j,index2, windex, incr;
    int length;


    for(loops=0;loops<2;loops++){

        for(i=0;i<16;i++){
            sample[i][0] = data_in[i+loops*16][0];
            sample[i][1] = data_in[i+loops*16][1];
        }

        index = 0;
        w_rec_real = 1;
        w_rec_imag = 0;
        while(index<len-1){
            w_temp = w_rec_real*w_real - w_rec_imag*w_imag;
            w_rec_imag =  w_rec_real*w_imag + w_rec_imag*w_real;
            w_rec_real = w_temp;
            W[index][0] = w_rec_real;
            W[index][1] = w_rec_imag;
            index++;
        }

        length = N;
        incr = 1;
        stage = 0;
        
        while (stage < M)
        { 
            length = length>>1;
    
            //First Iteration :  With No Multiplies
            i = 0;
            while(i < N)
            {
                index =  i; index2 = index + length; 
                
                tmp_real = sample[index][0] + sample[index2][0];
                tmp_imag = sample[index][1] + sample[index2][1];

                sample[index2][0] = sample[index][0] - sample[index2][0];
                sample[index2][1] = sample[index][1] - sample[index2][1];

                sample[index][0] = tmp_real;
                sample[index][1] = tmp_imag;
        

                i = i + 2*length;          

            }
            //Remaining Iterations: Use Stored W

            
            j = 1; 
            windex = incr - 1;
            while (j < length) // This loop executes N/2 times at first stage, .. once at last stage.
            {
                i = j; 
                while (i < N)
                {
                index = i;
                index2 = index + length;

                tmp_real = sample[index][0] + sample[index2][0]; 
                tmp_imag = sample[index][1] + sample[index2][1];
                tmp_real2 = sample[index][0] - sample[index2][0];
                tmp_imag2 = sample[index][1] - sample[index2][1];
    
                sample[index2][0] = tmp_real2*W[windex][0] - tmp_imag2*W[windex][1];
                sample[index2][1] = tmp_real2*W[windex][1] + tmp_imag2*W[windex][0]; 

                sample[index][0] = tmp_real;
                sample[index][1] = tmp_imag;
            
                i = i + 2*length;
                
                }
                windex = windex + incr;
                j++;
            }
            

            stage++;
            incr = 2*incr;
        } 
        
        //////////////////////////////////////////////////////////////////////////
        //Writing out the normalized transform values in bit reversed order
        #ifdef CHECKPRINT
        for(i=0;i<16;i++){
            // data_out[i][0] = sample[i][0];
            // data_out[i][1] = sample[i][1];
                printf("data is %d ",sample[i][0]);
                printf("data is %d \n",sample[i][1]);
            
        }
        #endif


    }
    return 1;
}