/*                                                         v1.2 - 02/Feb/2010
  ============================================================================

  RPEDEMO.C
  ~~~~~~~~~~

  Description:
  ~~~~~~~~~~~~

  Demonstration program for UGST/ITU-T RPE-LTP module. Takes the
  input file and processes by the GSM 06.10 Full Rate speech codec,
  depending on user's option: for encoding, input must be in
  either linear, A, mu law (G711); for decoding, in GSM 06.10 76-word
  format. The modules called have been written in Unix-C by Jutta
  Deneger and Carsten Borman, within the Communications and
  Operating Systems Research Group (KBS) of the Technishe Universitaet
  Berlin. This demo program has been written by Simao F.Campos Neto,
  from CPqD/Telebras based on previous UGST demo programs, as well on
  a driving program by the RPE-LTP program.

  Input data is supposed to be aligned at word boundaries, i.e.,
  organized in 16-bit words, following the operating system normal
  organization (low-byte first for VMS and DOS; high byte first for
  most Unix systems). G711 compressed data is supposed to be in the 8
  LEAST significant bits of the word and the RPE-LTP data will use a
  varied number of least significant bits. Both are without sign
  extension. Linear format, on its hand, expect data to be 16-bit
  aligned to the left, and the algorith will take only the 13 most
  significant bits. This complies to the general UGST integer data
  format.

  Output data will be generated in the same format as decribed
  above for the input data.

  Usage:
  ~~~~~~
  $ rpedemo [-l|-u|-A] [-enc|-dec]  InpFile OutFile BlockSize 1stBlock
             NoOfBlocks
  where:
   -l .......... input data for encoding and output data for decoding
                 are in linear format (DEFAULT).
   -A .......... input data for encoding and output data for decoding
                 are in A-law (G.711) format.
   -u .......... input data for encoding and output data for decoding
                 are in u-law (G.711) format.
   -enc ........ run the only the decoder (default: run enc+dec)
   -dec ........ run the only the encoder (default: run enc+dec)

   InpFile ..... is the name of the file to be processed;
   OutFile ..... is the name with the processed data;
   BlockSize ... is the block size, in number of samples (default = 160)
   1stBlock .... is the number of the first block of the input file
                 to be processed;
   NoOfBlocks .. is the number of blocks to be processed, starting on
                 block "1stBlock"

  Exit values:
  ~~~~~~~~~~~~
  0        success (all but VMS);
  1        success (only in VMS);
  2        error opening input file;
  3        error creating output file;
  4        error moving pointer to desired start of conversion;
  5        error creating gsm object (i.e., state variable);
  6        error reading input file;
  7        error writing to file;

  Original authors:
  ~~~~~~~~~~~~~~~~~
  Demo program:
  Simao Ferraz de Campos Neto   EMail : simao@cpqd.ansp.br
  CPqD/Telebras                 Rd. Mogi Mirim-Campinas Km.118
  DDS/Pr.11                     13.088-061 - Campinas - SP (Brazil)

  Module routines:
  Jutta Degener (jutta@cs.tu-berlin.de)
  Carsten Bormann (cabo@cs.tu-berlin.de)

  History:
  ~~~~~~~~
  28/Oct/92 r.1.0 pl.0 by  Carsten Bormann  (cabo at kubus)
        Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
        Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
        details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.

  30/Oct/92 r.1.0 pl.1 by Jutta Degener  (jutta at kraftbus)
        Switched uid/gid in toast's [f]chown calls.

  29/Jan/93 r.1.0 pl.2 by Jutta Degener  (jutta at kraftbus)
        fixed L_add(0,-1) in src/add.c and inc/private.h,
        thanks to Raphael Trommer at AT&T Bell Laboratories;
        * ANSI C compatibility details

  20/Mar/94 v1.0 Release of 1st version of demo program

  22/Feb/95 v1.1 Cleaned compilation warnings, modified for Alpha VMX/AXP
                 operation (as suggested by Kirchherr, FI/DBP Telekom)
                 <simao@ctd.comsat.com>
  02/Feb/10 v1.2 Modified maximum string length to avoid buffer overruns
                 (y.hiwasaki)
  ============================================================================
*/


/* ..... General definitions for UGST demo programs ..... */
#include "ugstdemo.h"

/* ..... General include ..... */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(VMS)
#include <stat.h>
#else /* Unix/DOS */
#include <sys/stat.h>
#endif

/* ..... RPE-LTP module definitions ..... */
#include "private.h"
#include "gsm.h"
#include "rpeltp.h"

/* ..... G.711 module definitions ..... */
#include "g711.h"

/* ..... Local definitions ..... */
#define LINEAR 0                /* binary: 00 */
#define U_LAW 1                 /* binary: 01 */
#define A_LAW 3                 /* binary: 11 */

/* ..... Local function prototypes ..... */
void display_usage ARGS ((void));
int main ARGS ((int argc, char *argv[]));


/*
 -------------------------------------------------------------------------
 void display_usage(void);
 ~~~~~~~~~~~~~~~~~~
 Display proper usage for the demo program. Generated automatically from
 program documentation.

 History:
 ~~~~~~~~
 20.Mar.94 v1.0 Created.
 -------------------------------------------------------------------------
*/
void display_usage () {
  printf ("RPEDEMO: Version 1.2 of 02.Feb.2010 \n\n");

  printf ("  Demonstration program for UGST/ITU-T RPE-LTP based on \n");
  printf ("  module implemented  in Unix-C by Jutta Deneger and Carsten \n");
  printf ("  Borman, within the Communications and Operating Systems \n");
  printf ("  Research Group (KBS) of the Technishe Universitaet Berlin.\n");
  printf ("  This demo program has been written by Simao F.Campos Neto\n");
  printf ("\n");
  printf ("  Usage:\n");
  printf ("  $ rpedemo [-l|-u|-A] [-enc|-dec]  InpFile OutFile BlockSize 1stBlock\n");
  printf ("             NoOfBlocks \n");
  printf ("  where:\n");
  printf ("   -l .......... input data for encoding and output data for decoding\n");
  printf ("                 are in linear format (DEFAULT).\n");
  printf ("   -A .......... input data for encoding and output data for decoding\n");
  printf ("                 are in A-law (G.711) format.\n");
  printf ("   -u .......... input data for encoding and output data for decoding\n");
  printf ("                 are in u-law (G.711) format.\n");
  printf ("   -enc ........ run the only the decoder (default: run enc+dec)\n");
  printf ("   -dec ........ run the only the encoder (default: run enc+dec)\n");
  printf ("\n");
  printf ("   InpFile ..... is the name of the file to be processed;\n");
  printf ("   OutFile ..... is the name with the processed data;\n");
  printf ("   BlockSize ... is the block size, in number of samples (default = 160)\n");
  printf ("   1stBlock .... is the number of the first block of the input file\n");
  printf ("                 to be processed;\n");
  printf ("   NoOfBlocks .. is the number of blocks to be processed, starting on\n");
  printf ("                 block \"1stBlock\"\n");

  exit (-128);
}

/*
   **************************************************************************
   ***                                                                    ***
   ***        Demo-Program for testing the correct implementation         ***
   ***               and to show how to use the programs                  ***
   ***                                                                    ***
   **************************************************************************
*/
int main (argc, argv)
     int argc;
     char *argv[];
{
  gsm rpe_enc_state, rpe_dec_state;
  gsm_signal rpe_frame[RPE_FRAME_SIZE];
  long N = 256, N1 = 1, N2 = 0, cur_blk, smpno, count = 0;
#ifdef STATIC_ALLOCATION
  short tmp_buf[256], inp_buf[256], out_buf[256];
#else
  short *tmp_buf, *inp_buf, *out_buf;
#endif

  /* G.711 Compression/expansion function pointers */
  void (*compress) (), (*expand) ();

  /* File variables */
  char FileIn[MAX_STRLEN], FileOut[MAX_STRLEN];
  FILE *Fi, *Fo;
  long start_byte;
  char format, run_encoder, run_decoder;
#ifdef VMS
  char mrs[15];
#endif


  /* SETTING DEFAULT OPTIONS */
  format = LINEAR;
  run_encoder = run_decoder = 1;

  /* GETTING OPTIONS */

  if (argc < 2)
    display_usage ();
  else {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp (argv[1], "-") == 0) {
        /* Oops, stop processing options! */
        break;
      } else if (strcmp (argv[1], "-l") == 0) {
        /* Input/output (uncoded) data format is linear */
        format = LINEAR;

        /* Move arg[cv] over the next valid option */
        argv++;
        argc--;
      } else if (strcmp (argv[1], "-A") == 0 || strcmp (argv[1], "-a") == 0) {
        /* Input/output (uncoded) data format is A-law (G.711) */
        format = A_LAW;

        /* Move arg[cv] over the next valid option */
        argv++;
        argc--;
      } else if (strcmp (argv[1], "-u") == 0) {
        /* Input/output (uncoded) data format is u-law (G.711) */
        format = U_LAW;

        /* Move arg[cv] over the next valid option */
        argv++;
        argc--;
      } else if (strcmp (argv[1], "-enc") == 0) {
        /* Run only the encoder */
        run_encoder = 1;
        run_decoder = 0;

        /* Move arg[cv] over the next valid option */
        argv++;
        argc--;
      } else if (strcmp (argv[1], "-dec") == 0) {
        /* Run only the encoder */
        run_encoder = 0;
        run_decoder = 1;

        /* Move arg[cv] over the next valid option */
        argv++;
        argc--;
      } else {
        fprintf (stderr, "ERROR! Invalid option \"%s\" in command line\n\n", argv[1]);
        display_usage ();
      }
  }

  GET_PAR_S (1, "_Input File: .................. ", FileIn);
  GET_PAR_S (2, "_Output File: ................. ", FileOut);
  FIND_PAR_L (3, "_Block Size: .................. ", N, RPE_WIND_SIZE);
  FIND_PAR_L (4, "_Starting Block: .............. ", N1, 1);
  FIND_PAR_L (5, "_No. of Blocks: ............... ", N2, 0);


  /* Find staring byte in file; all are 16-bit word-aligned =>short data type */
  start_byte = sizeof (short) * (long) (--N1) * (long) N;

  /* Check if is to process the whole file */
  if (N2 == 0) {
    struct stat st;

    /* ... find the input file size ... */
    stat (FileIn, &st);
    /* convert to block count, depending on whether the input file is a uncoded or coded file */
    if (run_encoder)
      N2 = (st.st_size - start_byte) / (N * sizeof (short));
    else
      N2 = (st.st_size - start_byte) / (RPE_FRAME_SIZE * sizeof (short));
  }

  /* Choose A/u law */
  if (format == A_LAW) {
    expand = alaw_expand;
    compress = alaw_compress;
  } else if (format == U_LAW) {
    expand = ulaw_expand;
    compress = ulaw_compress;
  }


/*
 * ...... MEMORY ALLOCATION .........
 */
#ifndef STATIC_ALLOCATION
  if ((inp_buf = (short *) calloc (N, sizeof (short))) == NULL)
    error_terminate ("Error in memory allocation!\n", 1);
  if ((out_buf = (short *) calloc (N, sizeof (short))) == NULL)
    error_terminate ("Error in memory allocation!\n", 1);
  if ((tmp_buf = (short *) calloc (N, sizeof (short))) == NULL)
    error_terminate ("Error in memory allocation!\n", 1);
#endif


/*
 * ......... FILE PREPARATION .........
 */

  /* Define stuff for VMS binary, fixed-record files */
#ifdef VMS
  sprintf (mrs, "mrs=%d", 512);
#endif

  /* Opening input/output files; abort if there's any problem */
  if ((Fi = fopen (FileIn, RB)) == NULL)
    KILL (FileIn, 2);

  if ((Fo = fopen (FileOut, WB)) == NULL)
    KILL (FileOut, 3);

  /* Move pointer to 1st block of interest */
  if (fseek (Fi, start_byte, 0) < 0l)
    KILL (FileIn, 4);

  /* ......... CREATE AND INIT GSM OBJECT (STATE VARIABLE) ......... */
  if (!(rpe_enc_state = rpeltp_init ()))
    error_terminate ("Error creating state variable for encoder\n", 5);
  if (!(rpe_dec_state = rpeltp_init ()))
    error_terminate ("Error creating state variable for encoder\n", 5);


  /* ......... PROCESSING ACCORDING TO GSM 06.10 RPE-LTP CODEC ......... */

  for (cur_blk = 0; cur_blk < N2; cur_blk++) {
    /* Reset output sample vector */
    memset (out_buf, (int) 0, N);

    /* Read a block of samples */
    if (run_encoder) {
      /* Reset sample vector */
      memset (inp_buf, (int) 0, N);

      /* Read a block of uncoded samples */
      if ((smpno = fread (inp_buf, sizeof (short), (long) N, Fi)) <= 0)
        break;
    } else {
      /* Reset frame vector */
      memset (rpe_frame, (int) 0, (long) RPE_FRAME_SIZE);

      /* Read a unpacked frame */
      if ((smpno = fread (rpe_frame, sizeof (short), (long) RPE_FRAME_SIZE, Fi)) <= 0)
        break;
    }

    /* Carry out the desired operation */

    /* CODEC OPERATION */
    if (run_encoder && run_decoder) {
      /* Run both and save decoded samples */

      /* First, expand samples, if needed */
      if (format) {
        memcpy (tmp_buf, inp_buf, (long) (sizeof (short) * smpno));
        expand (smpno, tmp_buf, inp_buf);
      }

      /* Encode & decode ... */
      rpeltp_encode (rpe_enc_state, inp_buf, rpe_frame);
      rpeltp_decode (rpe_dec_state, rpe_frame, out_buf);

      /* Compress samples, if requested */
      if (format) {
        memcpy (tmp_buf, out_buf, (long) (sizeof (short) * N));
        compress (N, tmp_buf, out_buf);
      }

      /* Save samples to file */
      if (!(smpno = fwrite (out_buf, sizeof (short), (long) smpno, Fo)))
        break;
    }
    /* ENCODER-ONLY OPERATION */
    else if (run_encoder) {
      /* First, expand samples, if needed */
      if (format) {
        memcpy (tmp_buf, inp_buf, (long) (sizeof (short) * smpno));
        expand (smpno, tmp_buf, inp_buf);
      }

      /* Run only the encoder, unpack frame and save rpe-ltp frame */
      rpeltp_encode (rpe_enc_state, inp_buf, rpe_frame);
      if (!(smpno = fwrite (rpe_frame, sizeof (short), RPE_FRAME_SIZE, Fo)))
        break;
    }
    /* DECODER-ONLY OPERATION */
    else {
      /* Decode frame */
      rpeltp_decode (rpe_dec_state, rpe_frame, out_buf);

      /* Compress samples, if requested */
      if (format) {
        memcpy (tmp_buf, out_buf, (long) (sizeof (short) * N));
        compress (N, tmp_buf, out_buf);
      }

      /* Save the decoded samples */
      if (!(smpno = fwrite (out_buf, sizeof (short), (long) N, Fo)))
        break;
    }
    count += smpno;
  }

  /* Check for errors */
  if (ferror (Fi))
    KILL (FileIn, 6);
  else if (ferror (Fo))
    KILL (FileOut, 7);

  /* ......... FINALIZATIONS ......... */

  /* Close input and output files and state */
  fclose (Fi);
  fclose (Fo);
  rpeltp_delete (rpe_enc_state);
  rpeltp_delete (rpe_dec_state);

  /* Exit with success for non-vms systems */
#ifndef VMS
  return (0);
#endif
}

/* ............................. end of main() ............................. */
