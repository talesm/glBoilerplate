#pragma once

/**
 * @brief Get an always positive module
 *
 * The number returned is the number you have to subtract from the quotient to
 * get the previous divisible quotient.
 *
 * @param q the quotient
 * @param d the divisor
 * @return int the module
 */
inline int
floorMod(int q, int d)
{
  return q > 0 ? q % d : d + q % d;
}

/**
 * @brief The classic fast inverse square root.
 *
 * @param number The number to get square root from
 * @return float the result
 */
inline float
rsqrt(float number)
{
  long        i;
  float       x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = *(long*)&y;            // evil floating point bit level hacking
  i  = 0x5f3759df - (i >> 1); // what the fuck?
  y  = *(float*)&i;
  y  = y * (threehalfs - (x2 * y * y)); // 1st iteration
  //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration,
  return y;
}
