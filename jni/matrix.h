/*  
	Copyright (C) 2006-2007 shash
	Copyright (C) 2007-2012 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <string.h>

#include "types.h"
#include "mem.h"

#ifdef ENABLE_SSE
#include <xmmintrin.h>
#endif

#ifdef ENABLE_SSE2
#include <emmintrin.h>
#endif

struct MatrixStack
{
	MatrixStack(int size, int type);
	s32		*matrix;
	s32		position;
	s32		size;
	u8		type;
};

void	MatrixInit				(float *matrix);
void	MatrixInit				(s32 *matrix);

//In order to conditionally use these asm optimized functions in visual studio
//without having to make new build types to exclude the assembly files.
//a bit sloppy, but there aint much to it

float	MatrixGetMultipliedIndex	(int index, float *matrix, float *rightMatrix);
s32	MatrixGetMultipliedIndex	(int index, s32 *matrix, s32 *rightMatrix);
void	MatrixSet				(s32 *matrix, int x, int y, s32 value);
void	MatrixCopy				(s32 * matrixDST, const s32 * matrixSRC);
int		MatrixCompare				(const s32 * matrixDST, const float * matrixSRC);
void	MatrixIdentity			(s32 *matrix);

void	MatrixStackInit				(MatrixStack *stack);
void	MatrixStackSetMaxSize		(MatrixStack *stack, int size);
void	MatrixStackPushMatrix		(MatrixStack *stack, const s32 *ptr);
void	MatrixStackPopMatrix		(s32 *mtxCurr, MatrixStack *stack, int size);
s32*	MatrixStackGetPos			(MatrixStack *stack, int pos);
s32*	MatrixStackGet				(MatrixStack *stack);
void	MatrixStackLoadMatrix		(MatrixStack *stack, int pos, const s32 *ptr);

void Vector2Copy(float *dst, const float *src);
void Vector2Add(float *dst, const float *src);
void Vector2Subtract(float *dst, const float *src);
float Vector2Dot(const float *a, const float *b);
float Vector2Cross(const float *a, const float *b);

float Vector3Dot(const float *a, const float *b);
void Vector3Cross(float* dst, const float *a, const float *b);
float Vector3Length(const float *a);
void Vector3Add(float *dst, const float *src);
void Vector3Subtract(float *dst, const float *src);
void Vector3Scale(float *dst, const float scale);
void Vector3Copy(float *dst, const float *src);
void Vector3Normalize(float *dst);

void Vector4Copy(float *dst, const float *src);

//these functions are an unreliable, inaccurate floor.
//it should only be used for positive numbers
//this isnt as fast as it could be if we used a visual c++ intrinsic, but those appear not to be universally available
FORCEINLINE u32 u32floor(float f)
{
#ifdef ENABLE_SSE2
	return (u32)_mm_cvtt_ss2si(_mm_set_ss(f));
/*#elif HAVE_NEON
	asm (
	        "vcvt.u32.f32   d0, d0                                  \n\t"
	        :::"d0"
	        );*/
#else
	return (u32)f;
#endif
}
FORCEINLINE u32 u32floor(double d)
{
#ifdef ENABLE_SSE2
	return (u32)_mm_cvttsd_si32(_mm_set_sd(d));
#else
	return (u32)d;
#endif
}

//same as above but works for negative values too.
//be sure that the results are the same thing as floorf!
FORCEINLINE s32 s32floor(float f)
{
#ifdef ENABLE_SSE2
	return _mm_cvtss_si32( _mm_add_ss(_mm_set_ss(-0.5f),_mm_add_ss(_mm_set_ss(f), _mm_set_ss(f))) ) >> 1;
/*#elif HAVE_NEON
	asm (
		        "vcvt.s32.f32   d0, d0                                  \n\t"
		        :::"d0"
		        );*/
#else
	return (s32)floorf(f);
#endif
}
FORCEINLINE s32 s32floor(double d)
{
	return s32floor((float)d);
}

//switched SSE2 functions
//-------------
#ifdef HAVE_NEON

template<int NUM>
FORCEINLINE void memset_u16_le(void* dst, u16 val)
{
	//ehhh add neon later
	for(int i=0;i<NUM;i++)
			T1WriteWord((u8*)dst,i<<1,val);
}


#else //no sse2

template<int NUM>
static FORCEINLINE void memset_u16_le(void* dst, u16 val)
{
	for(int i=0;i<NUM;i++)
		T1WriteWord((u8*)dst,i<<1,val);
}

#endif

// NOSSE version always used in gfx3d.cpp
void _NOSSE_MatrixMultVec4x4 (const float *matrix, float *vecPtr);
void MatrixMultVec3x3_fixed(const s32 *matrix, s32 *vecPtr);

//---------------------------
//switched SSE functions
#ifdef HAVE_NEON

FORCEINLINE void MatrixMultiply(float * matrix, const float * rightMatrix)
{
	asm (
	        "vld1.32                {d0, d1}, [%1]!                 \n\t"   //q0 = m1
	        "vld1.32                {d2, d3}, [%1]!                 \n\t"   //q1 = m1+4
	        "vld1.32                {d4, d5}, [%1]!                 \n\t"   //q2 = m1+8
	        "vld1.32                {d6, d7}, [%1]                  \n\t"   //q3 = m1+12
	        "vld1.32                {d16, d17}, [%0]!               \n\t"   //q8 = m0
	        "vld1.32                {d18, d19}, [%0]!               \n\t"   //q9 = m0+4
	        "vld1.32                {d20, d21}, [%0]!               \n\t"   //q10 = m0+8
	        "vld1.32                {d22, d23}, [%0]                \n\t"   //q11 = m0+12

	        "vmul.f32               q12, q8, d0[0]                  \n\t"   //q12 = q8 * d0[0]
	        "vmul.f32               q13, q8, d2[0]                  \n\t"   //q13 = q8 * d2[0]
	        "vmul.f32               q14, q8, d4[0]                  \n\t"   //q14 = q8 * d4[0]
	        "vmul.f32               q15, q8, d6[0]                  \n\t"   //q15 = q8 * d6[0]
	        "vmla.f32               q12, q9, d0[1]                  \n\t"   //q12 = q9 * d0[1]
	        "vmla.f32               q13, q9, d2[1]                  \n\t"   //q13 = q9 * d2[1]
	        "vmla.f32               q14, q9, d4[1]                  \n\t"   //q14 = q9 * d4[1]
	        "vmla.f32               q15, q9, d6[1]                  \n\t"   //q15 = q9 * d6[1]
	        "vmla.f32               q12, q10, d1[0]                 \n\t"   //q12 = q10 * d0[0]
	        "vmla.f32               q13, q10, d3[0]                 \n\t"   //q13 = q10 * d2[0]
	        "vmla.f32               q14, q10, d5[0]                 \n\t"   //q14 = q10 * d4[0]
	        "vmla.f32               q15, q10, d7[0]                 \n\t"   //q15 = q10 * d6[0]
	        "vmla.f32               q12, q11, d1[1]                 \n\t"   //q12 = q11 * d0[1]
	        "vmla.f32               q13, q11, d3[1]                 \n\t"   //q13 = q11 * d2[1]
	        "vmla.f32               q14, q11, d5[1]                 \n\t"   //q14 = q11 * d4[1]
	        "vmla.f32               q15, q11, d7[1]                 \n\t"   //q15 = q11 * d6[1]

	        "vst1.32                {d24, d25}, [%1]!               \n\t"   //d = q12
	        "vst1.32                {d26, d27}, [%1]!               \n\t"   //d+4 = q13
	        "vst1.32                {d28, d29}, [%1]!               \n\t"   //d+8 = q14
	        "vst1.32                {d30, d31}, [%1]                \n\t"   //d+12 = q15

	        : "+r"(matrix), "+r"(rightMatrix) :
	    : "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15",
	        "memory"
	        );
}



FORCEINLINE void MatrixMultVec4x4(const float *matrix, float *vecPtr)
{
	asm (
	        "vld1.32                {d0, d1}, [%1]                  \n\t"   //Q0 = v
	        "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
	        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
	        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
	        "vld1.32                {d24, d25}, [%0]!               \n\t"   //Q4 = m+12

	        "vmul.f32               q13, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
	        "vmla.f32               q13, q10, d0[1]                 \n\t"   //Q5 += Q1*Q0[1]
	        "vmla.f32               q13, q11, d1[0]                 \n\t"   //Q5 += Q2*Q0[2]
	        "vmla.f32               q13, q12, d1[1]                 \n\t"   //Q5 += Q3*Q0[3]

	        "vst1.32                {d26, d27}, [%1]                \n\t"   //Q4 = m+12
	        :
	        : "r"(matrix), "r"(vecPtr)
	    : "q0", "q9", "q10","q11", "q12", "q13", "memory"
	        );
}

FORCEINLINE void MatrixMultVec4x4_M2(const float *matrix, float *vecPtr)
{
	//there are hardly any gains from merging these manually
	MatrixMultVec4x4(matrix+16,vecPtr);
	MatrixMultVec4x4(matrix,vecPtr);
}

FORCEINLINE void MatrixMultVec3x3(const float * matrix, float * vecPtr)
{
	asm (
	        "vld1.32                {d0, d1}, [%1]                  \n\t"   //Q0 = v
	        "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
	        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
	        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
	        "vld1.32                {d24, d25}, [%0]!               \n\t"   //Q4 = m+12

	        "vmul.f32               q13, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
	        "vmla.f32               q13, q10, d0[1]                 \n\t"   //Q5 += Q1*Q0[1]
	        "vmla.f32               q13, q11, d1[0]                 \n\t"   //Q5 += Q2*Q0[2]
	        "vmla.f32               q13, q12, d1[1]                 \n\t"   //Q5 += Q3*Q0[3]

	        "vst1.32                {d26, d27}, [%1]                \n\t"   //Q4 = m+12
	        :
	        : "r"(matrix), "r"(vecPtr)
	    : "q0", "q9", "q10","q11", "q12", "q13", "memory"
	        );
}

FORCEINLINE void MatrixTranslate(float *matrix, const float *ptr)
{
	asm (
		        "vld1.32                {d0, d1}, [%1]                  \n\t"   //Q0 = v
		        "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
		        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
		        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
		        "vld1.32                {d24, d25}, [%0]               \n\t"   //Q4 = m+12

		        "vmla.f32               q12, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
		        "vmla.f32               q12, q10, d0[1]                 \n\t"   //Q5 += Q1*Q0[1]
		        "vmla.f32               q12, q11, d1[2]                 \n\t"   //Q5 += Q2*Q0[2]

		        "vst1.32                {d24, d25}, [%0]                \n\t"   //Q4 = m+12
		        :
		        : "r"(matrix), "r"(ptr)
		    : "q0", "q9", "q10","q11", "q12", "memory"
		        );
}

FORCEINLINE void MatrixScale(float *matrix, const float *ptr)
{
	asm (
			        "vld1.32                {d0, d1}, [%1]                  \n\t"   //Q0 = v
			        "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
			        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
			        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8

			        "vmul.f32               q9, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
			        "vmul.f32               q10, q10, d0[1]                 \n\t"   //Q5 += Q1*Q0[1]
			        "vmul.f32               q11, q11, d1[2]                 \n\t"   //Q5 += Q2*Q0[2]

					"vld1.32                {d22, d23}, [%0]!               \n\t"   //Q1 = m
					"vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
					"vld1.32                {d18, d19}, [%0]!               \n\t"   //Q3 = m+8
			        :
			        : "r"(matrix), "r"(ptr)
			    : "q0", "q9", "q10","q11", "memory"
			        );

}

template<int NUM_ROWS>
FORCEINLINE void vector_fix2float(float* matrix, const float divisor)
{
	CTASSERT(NUM_ROWS==3 || NUM_ROWS==4);

	if(NUM_ROWS==3)
	{
		asm (
					"vrecpe.f32				d0, d0							\n\t"
			        "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
			        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
			        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8


			        "vmul.f32               q9, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
					"vmul.f32               q10, q10, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
					"vmul.f32               q11, q11, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]

					"vst1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
					"vst1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
					"vst1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
			        :
			        : "r"(matrix)
			    : "q0", "q9", "q10","q11", "memory"
			        );
	}
	else
	{
		asm (
					        "vrecpe.f32				d0, d0							\n\t"
							"vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
					        "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
					        "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
							"vld1.32                {d24, d25}, [%0]!               \n\t"   //Q3 = m+8


					        "vmul.f32               q9, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
							"vmul.f32               q10, q10, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
							"vmul.f32               q11, q11, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
							"vmul.f32               q12, q12, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]

							"vst1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
							"vst1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
							"vst1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
							"vst1.32                {d24, d25}, [%0]!               \n\t"   //Q3 = m+8
					        :
					        : "r"(matrix)
					    : "q0", "q9", "q10","q11", "q12", "memory"
					        );
	}
}

//WARNING: I do not think this is as fast as a memset, for some reason.
//at least in vc2005 with sse enabled. better figure out why before using it
template<int NUM>
static FORCEINLINE void memset_u8(void* _dst, u8 val)
{
	memset(_dst,val,NUM);
	//const u8* dst = (u8*)_dst;
	//u32 u32val = (val<<24)|(val<<16)|(val<<8)|val;
	//const __m128i temp = _mm_set_epi32(u32val,u32val,u32val,u32val);
	//MACRODO_N(NUM/16,_mm_store_si128((__m128i*)(dst+(X)*16), temp));
}

#else //no sse

void MatrixMultVec4x4 (const float *matrix, float *vecPtr);
void MatrixMultVec3x3(const float * matrix, float * vecPtr);
void MatrixMultiply(float * matrix, const float * rightMatrix);
void MatrixTranslate(float *matrix, const float *ptr);
void MatrixScale(float * matrix, const float * ptr);

FORCEINLINE void MatrixMultVec4x4_M2(const float *matrix, float *vecPtr)
{
	//there are hardly any gains from merging these manually
	MatrixMultVec4x4(matrix+16,vecPtr);
	MatrixMultVec4x4(matrix,vecPtr);
}

template<int NUM_ROWS>
FORCEINLINE void vector_fix2float(float* matrix, const float divisor)
{
	for(int i=0;i<NUM_ROWS*4;i++)
		matrix[i] /= divisor;
}

template<int NUM>
static FORCEINLINE void memset_u8(void* dst, u8 val)
{
	memset(dst,val,NUM);
}

#endif //switched SSE functions

void MatrixMultVec4x4 (const s32 *matrix, s32 *vecPtr);

void MatrixMultVec4x4_M2(const s32 *matrix, s32 *vecPtr);

void MatrixMultiply(s32* matrix, const s32* rightMatrix);
void MatrixScale(s32 *matrix, const s32 *ptr);
void MatrixTranslate(s32 *matrix, const s32 *ptr);

#endif
