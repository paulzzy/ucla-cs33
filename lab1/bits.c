/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Our checker requires that you do NOT define a variable after 
     a statement that does not define a variable.

     For example, this is NOT allowed:

     int illegal_function_for_this_lab(int x, int y) {
      // this statement doesn't define a variable
      x = x + y + 1;
      
      // The checker for this lab does NOT allow the following statement,
      // because this variable definition comes after a statement 
      // that doesn't define a variable
      int z;

      return 0;
     }
     
  2. VERY IMPORTANT: Use the dlc (data lab checker) compiler (described in the handout)
     to check the legality of your solutions.
  3. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  4. Use the btest to check your functions for correctness.
  5. The maximum number of ops for each function is given in the
     header comment for each function. 

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest to verify that your solutions produce 
 *      the correct answers.
 */

#endif
/* Copyright (C) 1991-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* We do support the IEC 559 math functionality, real and complex.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
// 1
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  // Attempts to overflow x, which will only occur if x is Tmax. Note that Tmax
  // + 1 = Tmin.
  int try_overflow = x + 1;
  // Checks that x actually did overflow. Note that the expression (a ^ b)
  // yields -1 (all 1s in the binary representation) if a == Tmax and b ==
  // Tmin. ~(-1) yields 0. !0 yields 1.
  //
  // There is an edge case where -1 + 1 == 0 and (-1 ^ 0) yields -1. If x is -1,
  // then `!(x + 1)` will yield 1; if x is Tmax, then `!(x + 1)` yields 0. This
  // allows x == -1 to be distinguished from x == Tmax.
  int is_t_max = !(~((x + !try_overflow) ^ try_overflow));
  return is_t_max;
}
/*
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
  // Manually sets the even bits to 1, doing 8 bits at a time to avoid using any
  // variable with a value greater than 255.

  const int even_bits = 0x55;
  const int left_shift = 8;
  int word = even_bits;
  word = word << left_shift;
  word = word + even_bits;
  word = word << left_shift;
  word = word + even_bits;
  word = word << left_shift;
  word = word + even_bits;
  return word;
}
// 2
/*
 * isEqual - return 1 if x == y, and 0 otherwise
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
  // x XOR y returns 0 if x == y, since 1 ^ 1 = 0 and 0 ^ 0 = 0. If x != y,
  // there will be at least 1 in the binary representation of x XOR y, since
  // XOR-ing different bits yields 1.
  return !(x ^ y);
}
/*
 * fitsBits - return 1 if x can be represented as an
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  // If x fits in n bits, then the resulting number will be -1 (all 1 bits) or 0
  // (all 0 bits). Since one bit is used as the sign bit, the magnitude of x is
  // actually stored in n - 1 bits, so x is right shifted n - 1.

  const int neg_one = ~0;
  int constrained = x >> (n + neg_one);
  int is_neg_one = !(constrained ^ neg_one);
  return !(constrained + is_neg_one);
}
// 3
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  // If x is "true" (nonzero value), the mask is set such that y is unaffected
  // but z is zeroed. If x is "false" (x == 0), the mask is set such that y is
  // zeroed but z is unaffected. Note that a | b = a if b == 0 and a | b = b if
  // a == 0.

  int x_bool = !!x;
  int mask = ~x_bool + 1;
  return (mask & y) | (~mask & z);
}
/*
 * isGreater - if x > y  then return 1, else return 0
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  const int max_shift = 31;
  const int t_min = 1 << max_shift;
  const int t_max = ~t_min;
  // Capture bits that differ between x and y.
  int diff = x ^ y;
  // "Smear" the leftmost 1 bit. E.g. 0b0101 becomes 0b0111.
  int smeared_diff = diff | diff >> 1;
  // Data lab checker does not allow definitions after non-definition
  // statements, so these variables must be defined in advance.
  int only_left_diff = 0;
  int x_flip_sign = 0;
  int y_flip_all_except_sign = 0;
  int greater_bits_x = 0;
  int pos_x_neg_y = 0;
  int x_has_greater_diff = 0;
  smeared_diff = smeared_diff | smeared_diff >> 2;
  smeared_diff = smeared_diff | smeared_diff >> 4;
  smeared_diff = smeared_diff | smeared_diff >> 8;
  smeared_diff = smeared_diff | smeared_diff >> 16;
  // Only keep the leftmost 1 bit
  only_left_diff = smeared_diff ^ (smeared_diff >> 1);
  x_flip_sign = x ^ t_min;
  y_flip_all_except_sign = y ^ t_max;
  // Does two useful things:
  //
  // 1. If x is positive and y is negative, the MSB of this variable is set
  // to 1. If x is negative, or if y is positive, the MSB of this variable is
  // set to 0.
  //
  // 2. Captures the bits in x greater than the corresponding bits in y. For
  // example, if the LSB in x is 1 while in y it's 0, the LSB of this variable
  // will be set to 1.
  greater_bits_x = x_flip_sign & y_flip_all_except_sign;
  // x >= 0 and y < 0 guarantees x > y.
  pos_x_neg_y = t_min & greater_bits_x;
  // If the bit with the leftmost bit difference is greater in x than y, this
  // variable is set to a nonzero value. If the bit in y is greater, it's set to
  // zero.
  //
  // Note that if x is negative and y is positive, the leftmost bit difference
  // will always be the MSB. But since this implies the MSB of x < MSB of y,
  // then this variable will always be set to zero, which makes sense given that
  // x < 0 and y >= 0 guarantees x < y.
  x_has_greater_diff = only_left_diff & greater_bits_x;
  return !!(pos_x_neg_y | x_has_greater_diff);
}
/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
  // Multiples x by 5 using repeated addition, then divides by 8 using a right
  // shift of 3. Since the result is rounded toward 0 instead of rounded down,
  // one is added if x is negative and dividing by 8 leaves a remainder.

  const int divide_8 = 3;
  const int max_shift = 31;
  const int remainder_mask = 7;
  int x_mult_5 = x + x + x + x + x;
  int is_neg = x >> max_shift;
  int has_remainder = x_mult_5 & remainder_mask;
  return (x_mult_5 >> divide_8) + !!(is_neg & has_remainder);
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  // Performs an equality check, since a positive number does not equal its
  // negative value. However, 0 == -0, so 1 is returned if x == 0.
  //
  // The edge case Tmin = -Tmin is addressed by returning 0 if x is negative,
  // regardless of the equality check.

  const int max_shift = 31;
  int is_neg_mask = x >> max_shift;
  int neg_x = ~x + 1;
  int is_zero = ((neg_x ^ x) >> max_shift) + 1;
  // Conditional
  return (is_neg_mask & 0) | (~is_neg_mask & is_zero);
}
/*
 * twosComp2SignMag - Convert from two's complement to sign-magnitude
 *   where the MSB is the sign bit
 *   You can assume that x > TMin
 *   Example: twosComp2SignMag(-5) = 0x80000005.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int twosComp2SignMag(int x) {
  // Returns x if x is positive or zero. When x is negative, converts it to
  // positive and sets the MSB to 1.

  const int max_shift = 31;
  int is_pos = !(x >> max_shift);
  int convert_to_pos = ~x + 1;
  int sign_mag = convert_to_pos | (1 << max_shift);
  // Conditional
  int mask = ~is_pos + 1;
  return (mask & x) | (~mask & sign_mag);
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) {
  // All 1s in the binary representation is -1 in decimal.
  const int neg_one = ~0;
  const int max_shift = 31;
  // Ensures that 0 is returned if x < 0.
  int is_pos_mask = ~(x >> max_shift);
  // Ensures that 0 is returned if x == 0.
  int is_nonzero_mask = ~!!x + 1;
  // Ensures that 1 is returned if x == 1, since 2^0 == 1 is an edge case that
  // is not detected normally.
  int is_one = !(x ^ 1);
  // When x is a power of 2 (except for 2^0), subtracting one yields all 1s in
  // the bits less than the single 1 bit in x. The 1 bit in x is guaranteed to
  // be in a different position from any of the 1s in x - 1, so `x & (x - 1)`
  // always yields 0. Other values of x will have overlapping 1s with x - 1.
  int is_pow_2 = !(x & (x + neg_one));
  return (is_one | is_pow_2) & is_pos_mask & is_nonzero_mask;
}
