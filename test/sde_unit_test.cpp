// Usage:
// Compile with -DFMA_AVX2 to test AVX-2 FMA instructions.
// Compile with -DFMA_AVX512 to test AVX-512 FMA instructions.
// Compile with -DBF16 to test DPBF16 instructions.
// Compile with -DFMA4 to test 4FMA instructions.
//
// E.g.:
// $ icpc sde_unit_test.cpp -o sde_unit_test -O3 -DFMA_AVX512 -DBF16
// $ sde64 -cpx -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat
//         -stop_ssc_mark DEAD:repeat -- ./sde_unit_test
// $ python ../intel_sde_flops.py
// Warning: BF16 is currently experimental!
// TID: 0 (OS-TID: 18844):
//      Unmasked single prec. FLOPs: 96
//      Masked single prec. FLOPs: 48
//      Unmasked double prec. FLOPs: 0
//      Masked double prec. FLOPs: 0
//      Instructions executed: 40
//      FMA instructions executed: 2
// =============================================
// Sum:
//      Single prec. FLOPs: 144
//      Double prec. FLOPs: 0
//      Total instructions executed: 40

#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>

#ifndef __SSC_MARK
#define __SSC_MARK(tag)                                                        \
        __asm__ __volatile__("movl %0, %%ebx; .byte 0x64, 0x67, 0x90 "         \
                             ::"i"(tag) : "%ebx")
#endif

// Validate AVX-2 FMA for single precision FP (unmasked) execution.
// This computes 2 x 8 single precision FP operations.
#if defined(FMA_AVX2)
__attribute__((noinline))
float fma_avx2(float *aval, float *bval, float *cval, float *dval, float *memval)
{
    float rval[16];

    __m256 a = _mm256_load_ps(aval);
    __m256 b = _mm256_load_ps(bval);
    __m256 c = _mm256_load_ps(cval);

    __m256 result = _mm256_fnmsub_ps(a, b, c);

    _mm256_store_ps(rval, result);
    return rval[0];
}
#endif

// Validate FMA for single precision FP with masked and unmasked execution.
// Unmasked FMA computes 2 x 16 single precision FP operations.
// Masked FMA computes 1/2 x 2 x 16 single precision FP operations.
#if defined(FMA_AVX512)
__attribute__((noinline))
float fma_avx512(float *aval, float *bval, float *cval, float *dval, float *memval)
{
    float rval[16];

    __m512 a = _mm512_load_ps(aval);
    __m512 b = _mm512_load_ps(bval);
    __m512 c = _mm512_load_ps(cval);

    __m512 d = _mm512_fmaddsub_ps(a, b, c);

    uint16_t mask16 = 0x00FF;
    __mmask32 k16 = _load_mask32(&mask16);
    __m512 result = _mm512_mask3_fmadd_ps(d, a, b, k16);

    _mm512_store_ps(rval, result);
    return rval[0];
}
#endif

// Validate FMA4 for single precision FP with masked and unmasked execution.
// Unmasked FMA4 computes 4 x 2 x 16 single precision FP operations.
// Masked FMA4 computes 1/2 x 4 x 2 x 16 single precision FP operations.
#if defined(FMA4)
__attribute__((noinline))
float fma4(float *aval, float *bval, float *cval, float *dval, float *memval)
{
    float rval[16];

    __m512 a = _mm512_load_ps(aval);
    __m512 b = _mm512_load_ps(bval);
    __m512 c = _mm512_load_ps(cval);
    __m512 d = _mm512_load_ps(dval);
    __m512 src = _mm512_load_ps(aval);

    __m512 e = _mm512_4fmadd_ps(src, a, b, c, d, memval);

    uint16_t mask16 = 0x00FF;
    __mmask32 k16 = _load_mask32(&mask16);
    __m512 result = _mm512_maskz_4fmadd_ps(k16, d, a, b, c, d, memval);

    _mm512_store_ps(rval, result);
    return rval[0];
}
#endif

// Validate DP (dot product) for BF16 for single precision FP with masked and
// unmasked execution.
// Unmasked DPBF16 computes 2 x 2 x 16 single precision(!) FP operations.
// Masked DPBF16 computes 1/2 x 2 x 2 x 16 single precision(!) FP operations.
// Note:
// DPBF16 up-converts BF16 operands to single precition FP needed for
// multiplication. The converts are not counted as FLOPS!
#if defined(BF16)
__attribute__((noinline))
float bf16(float *aval, float *bval, float *cval, float *dval, float *memval)
{
    float rval[16];

    __m512 a = _mm512_load_ps(aval);
    __m512 b = _mm512_load_ps(bval);
    __m512 c = _mm512_load_ps(cval);

    uint32_t mask32 = 0x0000FFFF;
    __mmask32 k32 = _load_mask32(&mask32);
    __m512bh c1 =  _mm512_maskz_cvtne2ps_pbh(k32, a, b);

    __m512bh c2 = _mm512_cvtne2ps_pbh(b, a);
    __m512 d = _mm512_dpbf16_ps(c, c1, c2);

    uint16_t mask16 = 0x00FF;
    __mmask32 k16 = _load_mask32(&mask16);
//    __m512 result = _mm512_maskz_dpbf16_ps(k16, d, c2, c1);
    __m512 result = _mm512_mask_dpbf16_ps(d, k16, c2, c1);

    _mm512_store_ps(rval, result);

    return rval[0];
}
#endif
float dispatch(float (*func)(float *, float *, float *, float *, float *),
               float *aval, float *bval, float *cval, float *dval, float *memval)
{
    float ret;
    __SSC_MARK(0xFACE);
    ret = func(aval, bval, cval, dval, memval);
    __SSC_MARK(0xDEAD);

    return ret;
}

int main(int argc, char **argv)
{
    float memval[4];
    float aval[16], bval[16], cval[16], dval[16];
    for(int i = 0; i < 16; i++)
    {
        aval[i] = 1.00f / (float)(i + 1);
        bval[i] = 0.50f / (float)(i + 1);
        cval[i] = 0.10f / (float)(i + 1);
        dval[i] = 0.05f / (float)(i + 1);
    }
    for(int i = 0; i < 4; i++)
    {
        memval[4] = 0.9f / (float)(i + 1);
    }

    float ret = 0.0, this_ret;
#if defined(FMA_AVX2)
    ret += dispatch(fma_avx2, aval, bval, cval, dval, memval);
#endif

#if defined(FMA_AVX512)
    ret += dispatch(fma_avx512, aval, bval, cval, dval, memval);
#endif

#if defined(FMA4)
    ret += dispatch(fma4, aval, bval, cval, dval, memval);
#endif

#if defined(BF16)
    ret += dispatch(bf16, aval, bval, cval, dval, memval);
#endif

    return (int)ret; // Make sure compiler does not optimize away
}