#ifndef _DSPF_SP_fftSPxSP_CN_H_
#define _DSPF_SP_fftSPxSP_CN_H_

#include "droneID_utils.h"

void DSPF_sp_fftSPxSP_cn (int N, float *ptr_x, float *ptr_w, float *ptr_y,
    int n_min, int offset, int n_max);
    
void tw_gen (float *w, int n);
void fft(Cplxf_t* x, int N);

#endif /* _DSPF_SP_fftSPxSP_CN_H_ */