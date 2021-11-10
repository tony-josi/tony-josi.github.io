#include <stdlib.h>

#include <math.h>
#include <stdio.h>

#include "lti.h"

static int __get_delay_mem(LTI_Sys *lti_sys_handle, int index, float *op_val);
static int __update_delay_mem(LTI_Sys *lti_sys_handle, float data_value);

int LTI_Sys_Init(LTI_Sys *lti_sys_handle, int M, int N) {

    if (M == 0 && N == 0)
        return -1;

    lti_sys_handle->delay_mem_size = M >= N ? M : N;
    lti_sys_handle->M = M;
    lti_sys_handle->N = N;
    lti_sys_handle->__delay_buffer_head_index = 0;
    lti_sys_handle->__delay_mem_buffer = NULL;
    lti_sys_handle->coeff_buffer_a = NULL;
    lti_sys_handle->coeff_buffer_b = NULL;

    if((lti_sys_handle->__delay_mem_buffer = malloc(sizeof(float) * lti_sys_handle->delay_mem_size)) == NULL) 
        goto free_n_exit_error;

    for(int i = 0; i < lti_sys_handle->delay_mem_size; ++i) {
        lti_sys_handle->__delay_mem_buffer[i] = 0.;
    }

    if(lti_sys_handle->N > 0) {
        if((lti_sys_handle->coeff_buffer_a = malloc(sizeof(float) * lti_sys_handle->N)) == NULL)
            goto free_n_exit_error;
        
        for(int i = 0; i < lti_sys_handle->N; ++i) {
            lti_sys_handle->coeff_buffer_a[i] = 0.;
        }
    }

    if(lti_sys_handle->M > 0) {
        if((lti_sys_handle->coeff_buffer_b = malloc(sizeof(float) * lti_sys_handle->M)) == NULL)
            goto free_n_exit_error;

        for(int i = 0; i < lti_sys_handle->M; ++i) {
            lti_sys_handle->coeff_buffer_b[i] = 0.;
        }
    }

    return 0;

    free_n_exit_error:
        free(lti_sys_handle->__delay_mem_buffer);
        free(lti_sys_handle->coeff_buffer_a);
        free(lti_sys_handle->coeff_buffer_b);
        return -1;

}

int LTI_Sys_Set_Initial_State(LTI_Sys *lti_sys_handle, float *init_state, int size) {

    if(size > lti_sys_handle->delay_mem_size)
        return -1;

    for(int i = 0; i < size; ++i) {
        lti_sys_handle->__delay_mem_buffer[lti_sys_handle->delay_mem_size - i - 1] = init_state[i];
    }

    return 0;

}

int LTI_Sys_Set_Coeff(LTI_Sys *lti_sys_handle, float *coeff_a, float *coeff_b, int size_a, int size_b) {

    if(size_a > lti_sys_handle->N || size_b > lti_sys_handle->M)
        return -1;

    for(int i = 0; i < size_a; ++i)
        lti_sys_handle->coeff_buffer_a[i] = coeff_a[i];
    for(int i = 0; i < size_b; ++i)
        lti_sys_handle->coeff_buffer_b[i] = coeff_b[i];

    return 0;

}

int LTI_Sys_Process(LTI_Sys *lti_sys_handle, float ip, float *op) {

    float op_val = ip, tmp_coeff = 0., tmp_op_val = 0.;

    for(int i = 0; i < lti_sys_handle->N; ++i) {
        if(!__get_delay_mem(lti_sys_handle,i, &tmp_coeff))
            op_val += lti_sys_handle->coeff_buffer_a[i] * tmp_coeff;
        else
            return -1;
    }

    tmp_op_val = op_val;

    for(int i = 0; i < lti_sys_handle->M; ++i) {
        if(i == 0)
            op_val = lti_sys_handle->coeff_buffer_b[i] * op_val;
        else if(!__get_delay_mem(lti_sys_handle, i, &tmp_coeff))
            op_val += lti_sys_handle->coeff_buffer_b[i] * tmp_coeff;
        else
            return -1;
    }

    if(__update_delay_mem(lti_sys_handle, tmp_op_val))
        return -1;

    *op = op_val;

    return 0;

}

int LTI_Sys_Reset(LTI_Sys *lti_sys_handle) {

    lti_sys_handle->__delay_buffer_head_index = 0;

    for(int i = 0; i < lti_sys_handle->delay_mem_size; ++i) {
        lti_sys_handle->__delay_mem_buffer[i] = 0.;
    }

    for(int i = 0; i < lti_sys_handle->N; ++i) {
        lti_sys_handle->coeff_buffer_a[i] = 0.;
    }

    for(int i = 0; i < lti_sys_handle->M; ++i) {
        lti_sys_handle->coeff_buffer_b[i] = 0.;
    }

    return 0;

}

int LTI_Sys_Free(LTI_Sys *lti_sys_handle) {

    LTI_Sys_Reset(lti_sys_handle);

    free(lti_sys_handle->__delay_mem_buffer);
    free(lti_sys_handle->coeff_buffer_a);
    free(lti_sys_handle->coeff_buffer_b);
    return 0;

}

static int __get_delay_mem(LTI_Sys *lti_sys_handle, int index, float *op_val) {

    *op_val = lti_sys_handle->__delay_mem_buffer[(index + lti_sys_handle->__delay_buffer_head_index) % lti_sys_handle->delay_mem_size];
    return 0;

}

static int __update_delay_mem(LTI_Sys *lti_sys_handle, float data_value) {

    if(lti_sys_handle->__delay_buffer_head_index == 0)
        lti_sys_handle->__delay_buffer_head_index = lti_sys_handle->delay_mem_size - 1;
    else
        lti_sys_handle->__delay_buffer_head_index -= 1;

    lti_sys_handle->__delay_mem_buffer[lti_sys_handle->__delay_buffer_head_index] = data_value;
    return 0;

}

#define IP_SIZE         1000
#define FREQ_1          1.2
#define FREQ_2          0.01
#define SAMPLING_FREQ   5
#define LPF_WINDOW      4

#define SAMPLING_PERIOD (1./SAMPLING_FREQ)


int main() {

    float ip_buffer[IP_SIZE];

    LTI_Sys test_lti;
    if(LTI_Sys_Init(&test_lti, LPF_WINDOW, 1))
        printf("Error Init\n");

    for(int i = 0; i < IP_SIZE; ++i) {
        ip_buffer[i] = sinf(2 * 3.1415926 * FREQ_1 * i * SAMPLING_PERIOD) + sinf(2 * 3.1415926 * FREQ_2 * i * SAMPLING_PERIOD);
    }

    float initial_state[LPF_WINDOW];
    for(int i = 0; i < LPF_WINDOW; ++i) {
        initial_state[i] = ip_buffer[i] * (i + 1);
    }

    if(LTI_Sys_Set_Initial_State(&test_lti, initial_state, LPF_WINDOW))
        printf("Error Init State\n");

    float coeff_a[] = {1.0};
    float coeff_b[LPF_WINDOW];

    //coeff_b[0] = 1;
    for(int i = 0; i < LPF_WINDOW; ++i) {
        if(i == 0)
            coeff_b[i] = 1./LPF_WINDOW;
        else if(i == (LPF_WINDOW - 1))
            coeff_b[i] = -1./LPF_WINDOW;
        else
            coeff_b[i] = 0.;
    }

    if(LTI_Sys_Set_Coeff(&test_lti, coeff_a, coeff_b, 1, LPF_WINDOW))
        printf("Error Set Coeff\n");

    FILE *ptr = fopen("op_data.txt","w");

    float op_val = 0.;
    for(int i = LPF_WINDOW; i < IP_SIZE; ++i) {

        if(LTI_Sys_Process(&test_lti, ip_buffer[i], &op_val))
            printf("Error Process\n");

        fprintf(ptr, "%d %f\n", i, op_val);
        printf("%d %f\n", i, ip_buffer[i]);

    }

    fclose(ptr);

    return 0;

}

