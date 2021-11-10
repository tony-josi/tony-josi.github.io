#ifndef _LTI_SYS__HEADER
#define _LTI_SYS__HEADER

typedef struct _lti_sys {

    /* data */
    int M, N, delay_mem_size;
    float *coeff_buffer_a, *coeff_buffer_b;
    float *__delay_mem_buffer;
    int __delay_buffer_head_index;

} LTI_Sys;

int LTI_Sys_Init(LTI_Sys *lti_sys_handle, int M, int N);
int LTI_Sys_Set_Initial_State(LTI_Sys *lti_sys_handle, float *init_state, int size);
int LTI_Sys_Set_Coeff(LTI_Sys *lti_sys_handle, float *coeff_a, float *coeff_b, int size_a, int size_b);
int LTI_Sys_Process(LTI_Sys *lti_sys_handle, float ip, float *op);
int LTI_Sys_Reset(LTI_Sys *lti_sys_handle);
int LTI_Sys_Free(LTI_Sys *lti_sys_handle);


#endif /* _LTI_SYS__HEADER */

