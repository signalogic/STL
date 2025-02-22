/*
  ===========================================================================
   File: BASOP32.H                                       v.2.3 - 30.Nov.2009
  ===========================================================================
            ITU-T STL  BASIC OPERATORS
            GLOBAL FUNCTION PROTOTYPES
   History:
   26.Jan.00   v1.0     Incorporated to the STL from updated G.723.1/G.729 
                        basic operator library (based on basic_op.h) and 
                        G.723.1's basop.h.
   05.Jul.00   v1.1     Added 32-bit shiftless mult/mac/msub operators
   03 Nov 04   v2.0     Incorporation of new 32-bit / 40-bit / control
                        operators for the ITU-T Standard Tool Library as 
                        described in Geneva, 20-30 January 2004 WP 3/16 Q10/16
                        TD 11 document and subsequent discussions on the
                        wp3audio@yahoogroups.com email reflector.
                        norm_s()      weight reduced from 15 to 1.
                        norm_l()      weight reduced from 30 to 1.
                        L_abs()       weight reduced from  2 to 1.
                        L_add()       weight reduced from  2 to 1.
                        L_negate()    weight reduced from  2 to 1.
                        L_shl()       weight reduced from  2 to 1.
                        L_shr()       weight reduced from  2 to 1.
                        L_sub()       weight reduced from  2 to 1.
                        mac_r()       weight reduced from  2 to 1.
                        msu_r()       weight reduced from  2 to 1.
                        mult_r()      weight reduced from  2 to 1.
                        L_deposit_h() weight reduced from  2 to 1.
                        L_deposit_l() weight reduced from  2 to 1.
                        L_mls() weight of 5.
                        div_l() weight of 32.
                        i_mult() weight of 3.
   30 Nov 09   v2.3     round() function is now round_fx().
                        saturate() is not referencable from outside application
  ============================================================================
*/

/* 
Make thread-safe and optimize for codec usage. Initially tested with EVS and AMR-WB, but modifications can be expanded or edited for other ITU and 3GPP ETSI codecs, as needed

Copyright (C) Signalogic, 2017-2023

Revision History

  Mar 2017 CKJ, thread-safe modifications:
                -remove/disable usage of global flags
                -add "xxx_ovf()" versions of some functions and pass Overflow as a stack param; e.g. sub_ovf()
  Sep 2022 - Mar 2023 JHB, implement mods for codec builds:
                -USE_BASOPS_xxx" should be defined in Makefile or an include file such as options.h or similar
                -enable static inline basops in basop32.h if USE_BASOPS_INLINE is defined
                -enable/disable Overflow and Carry global variables with NO_BASOPS_OVERFLOW_GLOBAL_VAR and NO_BASOPS_CARRY_GLOBAL_VAR
                -disable abort() and enable error handling and descriptive error messages if NO_BASOPS_EXIT not defined
                -no indentation/formatting mods to STL 2017 file outside of mods described here 
  Mar 2023 JHB, improve comments
  Jun 2023 JHB, change define around L_deposit_x and extract_x to USE_BASOPS_INLINE
  Sep 2023 JHB, change USE_BASOP_EXIT, USE_BASOPS_OVERFLOW_GLOBAL_VAR, USE_BASOPS_CARRY_GLOBAL_VAR to NO_BASOP_EXIT, NO_BASOPS_OVERFLOW_GLOBAL_VAR, and NO_BASOPS_CARRY_GLOBAL_VAR, and reverse polarity of #if usage
*/

#ifndef _BASIC_OP_H
#define _BASIC_OP_H

#include "basop_platform.h"  /* include basop platform file, which will define _CODEC_TYPE_ */

#if _CODEC_TYPE_ == _EVS_CODEC_

/* defines added by 3GPP EVS authors */
  #define BASOP_OVERFLOW2
  #define BASOP_SATURATE_WARNING_ON
  #define BASOP_SATURATE_WARNING_OFF
  #define BASOP_SATURATE_ERROR_ON
  #define BASOP_SATURATE_ERROR_OFF
  #define BASOP_CHECK()
#endif

#ifndef NO_BASOPS_EXIT
  #include <stdlib.h>  /* exit(), abort() */
#endif

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 | $Id $
 |___________________________________________________________________________|
*/
/* Remove global flags, replaced with stack vars as needed - CKJ Mar 2017 */
/* Note - Chris demonstrates here that no EVS floating-point sources read Overflow, Overflow2, or Carry; i.e. these vars are "write-only" global flags written only by basop32.c, JHB Mar 2023 */
#ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR
extern Flag Overflow;
#endif
#ifndef NO_BASOPS_CARRY_GLOBAL_VAR
extern Flag Carry;
#endif

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

#include <stdio.h>  /* make available printf() and fprintf(stderr ...) */

#ifdef USE_BASOPS_INLINE  /* defined in options.h, JHB Mar 2023 */

static inline Word32 L_deposit_l(Word16 var1) { return (Word32)var1; }
static inline Word32 L_deposit_h(Word16 var1) { return (Word32)var1 << 16; }
static inline Word16 extract_l(Word32 L_var1) { return (Word16)L_var1; }
static inline Word16 extract_h(Word32 L_var1) { return (Word16)(L_var1 >> 16); }

static inline Word16 saturate(Word32 L_var1) {
  Word16 var_out;

  if (L_var1 > 0x00007fffL) {
    #ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* not needed; see above comments, JHB Mar 2023 */
    Overflow = 1;
    #endif
    var_out = MAX_16;
  }
  else if (L_var1 < (Word32) 0xffff8000L) {
    #ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* not needed, see above comments, JHB Mar 2023 */
    Overflow = 1;
    #endif
    var_out = MIN_16;
  }
  else var_out = extract_l(L_var1);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 negate(Word16 var1) { return (var1 == MIN_16) ? MAX_16 : -var1; }

static inline Word16 sub(Word16 var1, Word16 var2) {
  Word16 var_out;
  Word32 L_diff;

  L_diff = (Word32)var1 - var2;
  var_out = saturate(L_diff);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return (var_out);
}

static inline Word16 add(Word16 var1, Word16 var2) {
  Word16 var_out;
  Word32 L_sum;

  L_sum = (Word32)var1 + var2;
  var_out = saturate(L_sum);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word32 L_sub(Word32 L_var1, Word32 L_var2) {
  Word32 L_var_out;

  L_var_out = L_var1 - L_var2;

  if (((L_var1 ^ L_var2) & MIN_32) != 0) {

    if ((L_var_out ^ L_var1) & MIN_32) {

       L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
       #ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* not needed; see above comments, JHB Mar 2023 */
       Overflow = 1;
       #endif
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return L_var_out;
}

static inline Word32 L_add(Word32 L_var1, Word32 L_var2) {
  Word32 L_var_out;

  L_var_out = L_var1 + L_var2;

  if (((L_var1 ^ L_var2) & MIN_32) == 0) {

    if ((L_var_out ^ L_var1) & MIN_32) {

      L_var_out = (L_var1 < 0) ? MIN_32 : MAX_32;
      #ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* not needed; see above comments, JHB Mar 2023 */
      Overflow = 1;
      #endif
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return L_var_out;
}

static inline Word16 div_s(Word16 var1, Word16 var2) {
  Word16 var_out = 0, iteration;
  Word32 L_num, L_denom;

  if (var2 == 0) {

    #ifndef NO_BASOPS_EXIT  /* allow exit() unless defined in options.h or Makefile, JHB Mar 2023 */
    printf ("Division by 0, Fatal error \n"); printStack();
    abort(); /* exit (0); */
    #endif
    #ifdef ENABLE_BASOPS_ERROR_DISPLAY
    fprintf(stderr, "Division by 0 in divs_s in basop32 \n");
    #endif
    #if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
    BASOP_CHECK();
    #endif
    #ifdef NO_BASOPS_EXIT
    return MAX_16;  /* return max possible value, JHB Mar 2023 */
    #endif
  }

  if ((var1 > var2) || (var1 < 0) || (var2 < 0)) {

    #ifndef NO_BASOPS_EXIT  /* allow exit() unless defined in options.h or Makefile, JHB Mar 2023 */
    printf ("Division Error var1=%d  var2=%d\n", var1, var2); printStack();
    abort(); /* exit (0); */
    #endif
    #ifdef ENABLE_BASOPS_ERROR_DISPLAY
    fprintf(stderr, "Division error in div_s in basop32, var1 = %d var2 = %d \n", var1, var2);
    if (var1 < 0) var1 = -var1;  /* make positive values and proceed, JHB Mar 2023 */
    if (var2 < 0) var2 = -var2;
    #endif
  }

  if (var1) {

    if (var1 >= var2) var_out = MAX_16;  /* use >= check here, in case "make positive" above happened, JHB Mar 2023 */
    else {

       L_num = L_deposit_l(var1);
       L_denom = L_deposit_l(var2);

       for (iteration = 0; iteration < 15; iteration++) {

          var_out <<= 1;
          L_num <<= 1;

          if (L_num >= L_denom) {
             L_num = L_sub(L_num, L_denom);
             var_out = add(var_out, 1);
          }
       }
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 mult_r(Word16 var1, Word16 var2) {
  Word16 var_out;
  Word32 L_product_arr;

  L_product_arr = (Word32) var1 *(Word32) var2;  /* product */
  L_product_arr += (Word32) 0x00004000L;         /* round */
  L_product_arr &= (Word32) 0xffff8000L;
  L_product_arr >>= 15;                          /* shift */

  if (L_product_arr & (Word32) 0x00010000L) L_product_arr |= (Word32) 0xffff0000L;  /* sign extend when necessary */

  var_out = saturate(L_product_arr);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 shl_pos(Word16 var1, Word16 var2) {
  Word16 var_out;
  Word32 result;

  result = (Word32) var1 *((Word32) 1 << var2);

  if ((var2 > 15 && var1 != 0) || (result != (Word32)((Word16)result))) {

    #ifndef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* not needed; see above comments, JHB Mar 2023 */
    Overflow = 1;
    #endif
    var_out = (var1 > 0) ? MAX_16 : MIN_16;
  }
  else var_out = extract_l(result);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 shr(Word16 var1, Word16 var2) {
  Word16 var_out;

  if (var2 < 0) {
    if (var2 < -16) var2 = -16;
    var2 = -var2;
    var_out = shl_pos(var1, var2);  /* shl_pos() assumes positive input, avoids circular inline dependency, JHB Mar 2023 */
  } else {
    if (var2 >= 15) var_out = (var1 < 0) ? -1 : 0;
    else {
      if (var1 < 0) var_out = ~((~var1) >> var2);
      else var_out = var1 >> var2;
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 shl(Word16 var1, Word16 var2) {
  Word16 var_out;

  if (var2 < 0) {
    if (var2 < -16) var2 = -16;
    var2 = -var2;
    var_out = shr(var1, var2);
  }
  else var_out = shl_pos(var1, var2);

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 norm_l(Word32 L_var1) {
  Word16 var_out;

  if (L_var1 == 0) var_out = 0;
  else {

    if (L_var1 == (Word32) 0xffffffffL) var_out = 31;
    else {
      if (L_var1 < 0) L_var1 = ~L_var1;
      for (var_out = 0; L_var1 < (Word32) 0x40000000L; var_out++) L_var1 <<= 1;
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}

static inline Word16 norm_s(Word16 var1) {
  Word16 var_out;

  if (var1 == 0) var_out = 0;
  else {
    if (var1 == (Word16) 0xffff) var_out = 15;
    else {
      if (var1 < 0) var1 = ~var1;
      for (var_out = 0; var1 < 0x4000; var_out++) var1 <<= 1;
    }
  }

#if _CODEC_TYPE == _EVS_CODEC_  /* EVS authors added BASOP_CHECK() */
  BASOP_CHECK();
#endif
  return var_out;
}
#endif  /* USE_BASOPS_INLINE */

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

#ifndef USE_BASOPS_INLINE
Word16 add (Word16 var1, Word16 var2);  /* Short add, 1 */
Word16 sub (Word16 var1, Word16 var2);  /* Short sub, 1 */
Word16 shl (Word16 var1, Word16 var2);  /* Short shift left, 1 */
Word16 shr (Word16 var1, Word16 var2);  /* Short shift right, 1 */
Word16 negate (Word16 var1);    /* Short negate, 1 */
Word32 L_sub (Word32 L_var1, Word32 L_var2);    /* Long sub, 1 */
Word16 mult_r (Word16 var1, Word16 var2);       /* Mult with round, 1 */
Word16 norm_l (Word32 L_var1);  /* Long norm, 1 */
Word16 div_s (Word16 var1, Word16 var2);        /* Short division, 18 */
Word16 norm_s (Word16 var1);    /* Short norm, 1 */
#endif

#ifdef NO_BASOPS_OVERFLOW_GLOBAL_VAR  /* Identical to functions without _ovf, but do not use global Overflow flag to be thread-safe - CJ MAR2017 */
Word16 add_ovf (Word16 var1, Word16 var2, Flag *Overflow);
Word16 sub_ovf (Word16 var1, Word16 var2, Flag *Overflow);
Word16 shl_ovf (Word16 var1, Word16 var2, Flag *Overflow);
Word32 L_mult_ovf (Word16 var1, Word16 var2, Flag *Overflow);
Word32 L_add_ovf (Word32 L_var1, Word32 L_var2, Flag *Overflow);
Word32 L_sub_ovf (Word32 L_var1, Word32 L_var2, Flag *Overflow);
Word32 L_shl_ovf (Word32 L_var1, Word16 var2, Flag *Overflow);
#endif

Word16 abs_s (Word16 var1);     /* Short abs, 1 */
Word16 mult (Word16 var1, Word16 var2); /* Short mult, 1 */
Word32 L_mult (Word16 var1, Word16 var2);       /* Long mult, 1 */

#ifndef USE_BASOPS_INLINE  /* otherwise declared above as static inline, JHB Mar 2023 */
Word16 extract_h (Word32 L_var1);       /* Extract high, 1 */
Word16 extract_l (Word32 L_var1);       /* Extract low, 1 */
#endif

Word16 round_fx (Word32 L_var1);        /* Round, 1 */
Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2); /* Mac, 1 */
Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2); /* Msu, 1 */
Word32 L_macNs (Word32 L_var3, Word16 var1, Word16 var2);       /* Mac without sat, 1 */
Word32 L_msuNs (Word32 L_var3, Word16 var1, Word16 var2);       /* Msu without sat, 1 */
Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add, 1 */

#ifndef EXCLUDE_BASOPS_NOT_USED
/* Exclude from build as they read/set global Overflow/Carry vars, but are not called anywhere in 3GPP EVS floating-point code - CJ March 2017 */
Word32 L_add_c (Word32 L_var1, Word32 L_var2);  /* Long add with c, 2 */
Word32 L_sub_c (Word32 L_var1, Word32 L_var2);  /* Long sub with c, 2 */
Word32 L_sat (Word32 L_var1);   /* Long saturation, 4 */
#endif

Word32 L_negate (Word32 L_var1);        /* Long negate, 1 */
Word32 L_shl (Word32 L_var1, Word16 var2);      /* Long shift left, 1 */
Word32 L_shr (Word32 L_var1, Word16 var2);      /* Long shift right, 1 */
Word16 shr_r (Word16 var1, Word16 var2);        /* Shift right with round, 2 */
Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2); /* Mac with rounding, 1 */
Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2); /* Msu with rounding, 1 */

#ifndef USE_BASOPS_INLINE  /* otherwise declared above as static inline, JHB Mar 2023 */
Word32 L_deposit_h (Word16 var1);       /* 16 bit var1 -> MSB, 1 */
Word32 L_deposit_l (Word16 var1);       /* 16 bit var1 -> LSB, 1 */
#endif

Word32 L_shr_r (Word32 L_var1, Word16 var2);    /* Long shift right with round, 3 */
Word32 L_abs (Word32 L_var1);   /* Long abs, 1 */


/*
 * Additional G.723.1 operators
*/
Word32 L_mls (Word32, Word16);  /* Weight FFS; currently assigned 5 */
Word16 div_l (Word32, Word16);  /* Weight FFS; currently assigned 32 */
Word16 i_mult (Word16 a, Word16 b);     /* Weight FFS; currently assigned 3 */

/*
 *  New shiftless operators, not used in G.729/G.723.1
*/
Word32 L_mult0 (Word16 v1, Word16 v2);  /* 32-bit Multiply w/o shift 1 */
Word32 L_mac0 (Word32 L_v3, Word16 v1, Word16 v2);      /* 32-bit Mac w/o shift 1 */
Word32 L_msu0 (Word32 L_v3, Word16 v1, Word16 v2);      /* 32-bit Msu w/o shift 1 */


#endif /* ifndef _BASIC_OP_H */


/* end of file */
