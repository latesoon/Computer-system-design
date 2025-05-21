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
  /*
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
  */
  const FLOAT zero = 0;
  if (b == zero) panic("Division by zero");

  // 预处理操作数
  FLOAT abs_dividend = Fabs(a);
  FLOAT abs_divisor  = Fabs(b);
  
  // 整数部分计算
  FLOAT quotient  = abs_dividend / abs_divisor;
  FLOAT remainder = abs_dividend % abs_divisor;

  // 小数部分计算（16位精度）
  for (uint8_t bit_pos = 0; bit_pos < 16; bit_pos++) {
    remainder <<= 1;  // 将余数左移一位
    quotient <<= 1;   // 将商左移一位
    
    if (remainder >= abs_divisor) {
      remainder -= abs_divisor;    // 从余数中减去除数
      quotient |= 1;               // 在商的最低位设置为1
    }
  }

  // 符号位处理
  if ((a ^ b) < zero) {
    quotient = -quotient;
  }

  return quotient;
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
  /*
  uint32_t b;
  char* dest = (char*)(&b),*src = (char*)(&a);
  for(int i = 0; i < sizeof(uint32_t); i++)
    dest[i] = src[i];
  uint32_t sign = b >> 31;
  int32_t e = ((b >> 23) & 0xFF) - 127 + 16 - 23;
  uint32_t m = (b & 0x7FFFFF) | 0x800000;
  int32_t A = (e >= 0) ? (m << e) : (m >> (-e));
  if(sign) A = -A;
  //printf("%d,%d\n",b,A);
  return A;*/
  union float_bits 
  {
    struct {
      uint32_t fraction : 23;
      uint32_t power : 8;
      uint32_t sgn : 1;
    };
    uint32_t raw;
  };
  union float_bits fb;
  fb.raw = *((uint32_t*)(void*)&a);

  FLOAT result = 0;
  // IEEE 754标准中，浮点数的指数部分（power）为全1（即255）时，表示该浮点数是NaN
  if (fb.power == 255) // 检查是否为NaN或无穷大
    assert(0);

  // IEEE 754标准中，浮点数的指数部分存储的是偏移值（power），实际的指数值需要减去偏移量127
  int shift = 127 - fb.power; // 计算位移量
  if (shift < 0) 
  {
    // 如果指数为正，进行右移或左移操作
    shift = -shift;
    if (shift <= 7)
      // 将尾数部分与隐含的最高位（1）拼接起来。IEEE 754标准中，尾数部分默认有一个隐含的最高位为1（规格化数）。
      result = (fb.fraction | (1 << 23)) >> shift;
    else
      result = (fb.fraction | (1 << 23)) << (shift - 8);
  }

  // 根据符号位返回结果
  return fb.sgn ? -result : result;
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
