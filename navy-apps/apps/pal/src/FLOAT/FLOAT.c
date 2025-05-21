#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //assert(0);
  //return 0;
  //return (a * b) >> 16; 
  uint32_t sign = ((a >> 31) & 0x1 ) ^ ((b >>31) & 0x1);
  a = Fabs(a);
  b = Fabs(b);
  int32_t a_hi = a >> 16;
  int32_t a_lo = a & 0xFFFF;
  int32_t b_hi = b >> 16;
  int32_t b_lo = b & 0xFFFF;
  int32_t r = ((a_hi * b_hi) << 16) + a_hi * b_lo + b_hi * a_lo + ((a_lo * b_lo)>> 16);
  if(sign) r = -r;
  return r;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  //assert(0);
  //return 0;
  //return ((((int64_t)a) << 16) / (int64_t)b);
  uint32_t sign = ((a >> 31) & 0x1 ) ^ ((b >>31) & 0x1);
  a = Fabs(a);
  b = Fabs(b);
  int32_t q = 0, r = a;
  for(int i = 0; i < 16; i++){
    r <<= 1,q <<= 1;
    if(r >= b)
      r -= b,q |= 1;
  }
  if(sign) q = -q;
  return q;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  //assert(0);
  //return 0;
  uint32_t b;
  char* dest = (char*)(&b),*src = (char*)(&a);
  for(int i = 0; i < sizeof(uint32_t); i++)
    dest[i] = src[i];
  uint32_t sign = b >> 31;
  int32_t e = ((b >> 23) & 0xFF) - 127 + 16 - 23;
  uint32_t m = (b & 0x7FFFFF) | 0x800000;
  int32_t A = (e >= 0) ? (m << e) : (m >> (-e));
  if(sign) A = -A;
  printf("%f,%d\n",a,A);
  return A;
}

FLOAT Fabs(FLOAT a) {
  //assert(0);
  //return 0;
  return a>=0 ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
