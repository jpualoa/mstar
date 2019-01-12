/*=========================================================================
 *=========================================================================
  == [DISCLAIMER]: THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION IS   ==
  == RELEASED "AS IS".  THE U.S. GOVERNMENT MAKES NO WARRANTY OF ANY     ==
  == KIND, EXPRESS OR IMPLIED, CONCERNING THIS SOFTWARE AND ANY          ==
  == ACCOMPANYING DOCUMENTATION, INCLUDING, WITHOUT LIMITATION, ANY      ==
  == WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  ==
  == IN NO EVENT WILL THE U.S. GOVERNMENT BE LIABLE FOR ANY DAMAGES      ==
  == ARISING OUT OF THE USE, OR INABILITY TO USE, THIS SOFTWARE OR ANY   ==
  == ACCOMPANYING DOCUMENTATION, EVEN IF INFORMED IN ADVANCE OF THE      ==
  == POSSIBLITY OF SUCH DAMAGES.                                         ==
  =========================================================================
  =========================================================================*/

/*-------------------------------------------------------------------------
 *                Routine: mstar2ras (Version 2.0)
 *                 Author: John F. Querns, Veridian Engineering
 *                      Date: 24 September 1998                            
 *
 * What's new:
 *
 * (1)  Recoded auto-enhancement section to use a better algorithm for target
 *      chips.  Use same enhancement scheme for full scenes.
 *
 * (2)  Added code to check current CPU for byte ordering. If little-endian,
 *      code automatically byteswaps input data before any further processing
 *      is done. 
 *
 * (3)  Corrected potential bug that occurs in some X viewer packages when
 *      reading images with an ODD number of columns.  This correction
 *      zero pads to 16-bit alignment.  It does not affect the original
 *      image width, but may increase the overall image length.
 *
 *-------------------------------------------------------------------------
 *
 * Purpose: This routine converts MSTAR fullscene and target chip images   
 *          to 8-bit Sunraster images.  The MSTAR imagery is outputted as 
 *          8-bit to better facilitate VIEWING of the imagery using standard
 *          commercial software.
 *
 *-------------------------------------------------------------------------
 *
 * [Calls]:
 *
 *     char *read_switch()   -- Utility to extract arguments from 
 *                              input switches.  This was extracted
 *                              from code developed by Otto Milvang,
 *                              U. of Oslo and is part of the B-LAB
 *                              XITE software.
 *
 *                              Copyright 1990, Blab, UiO 
 *                              Image processing lab, 
 *                              Department of Informatics 
 *                              University of Oslo
 *
 *     float
 *     byteswap_SR_IR()      -- Does big-endian to little-endian float
 *                              byteswap..this is specifically for the
 *                              case of Sun big-endian to PC-Intel
 *                              little-endian data.
 *
 *     unsigned short
 *     byteswap_SUS_IUS()    -- Does big-endian to little-endian swap for
 *                              unsigned short (16-bit) numbers. This is
 *                              specifically for the case of Sun big-
 *                              endian to PC-Intel little-endian data.
 *
 *     int
 *     CheckByteOrder()      -- This checks the byte order for the CPU that
 *                              this routine is compiled run on. If the
 *                              CPU is little-endian, it will return a 0
 *                              value (LSB_FIRST); else, it will return a 1
 *                              (MSB_FIRST).
 *
 *                              Taken from:                     
 *                                 
 *                                Encyclopedia of Graphic File  
 *                                Formats, Murray & Van Ryper, 
 *                                O'Reilly & Associates, 1994,  
 *                                pp. 114-115. 
 *
 *------------------------------------------------------------------------
 *
 * [Methodology]: 
 *
 *     This routine converts ONLY the MAGNITUDE data portion of MSTAR
 *     files (i.e. it ignores the PHASE data).  The output SunRaster file 
 *     represents the compressed 8-bit value of the original MAGNITUDE
 *     data portion of the MSTAR files:
 * 
 *          [FULLSCENE IMAGES]: Unsigned 16-bit data linearly scaled to
 *                              unsigned 8-bit.
 *
 *          [CHIP IMAGES]     : 32-bit floating point linearly scaled to 
 *                              unsigned 8-bit.                             
 *
 *     Scaling Method:
 *     --------------
 *
 *          Given: OP     = Output pixel
 *                 IP     = Input pixel
 *                 RANGE  = Range of gray values in input image (MAX-MIN)
 *                 MIN    = Minimum pixel value in input image
 *                 T      = Temporary value
 *
 *          Then:  T   =   ((FLOAT) (IP - MIN) / RANGE) * 255.0
 *                 OP  = (unsigned char) (T + 0.5);
 *
 *     Zero-Padding (16-bit Alignment) Method:
 *     --------------------------------------
 *
 *          To ensure certain odd-rowed Sunraster images are readable by
 *          common X viewers (like XV and Khoros), a 16-bit row alignment
 *          scheme is employed.  Although the image dimensions remain the
 *          same the actual image size may increase.  For example, an
 *          additional column may be added.
 *
 *            1) Set 'linesize' = image width x image depth
 *
 *               linesize = w * d;      (where d = 8 [bit])
 *
 *            2) Mod the linesize by 16.  That is, divide it by size. Based
 *               on the results, increase the size of 'linesize'.
 *
 *               if (linesize%16) linesize += (16 - (linesize%16));
 *
 *            3) Divide 'linesize' by 8 to get final linesize value to use
 *               in output write.
 *
 *               linesize /= 8;
 *
 *            4) If the final value of 'linesize' is > original value of
 *               'linesize', output is zero-padded.
 *
 *          EXAMPLE: Given an image of width 755, height 755, depth 8, is
 *                   output image padded?
 *
 *                   linesize = 755 * 8 = 6040
 *
 *                   'linesize%16' = 8
 *
 *                   'if' statement is true; 'linesize' is adjusted:
 *
 *                   linesize += (16 - (linesize%16));
 *                   linesize = 6040 + (16-8) = 6048;
 *
 *                   linesize /= 8 = 6048/8 = 756;
 *
 *                   Since 756 > original 755, output is zero-padded..
 *
 *------------------------------------------------------------------------
 *
 * [Enhancement]:
 *
 *    The MSTAR data is very dark and will need contrast enhancement to
 *    increase the dynamic range for suitable VIEWING.  This routine 
 *    includes a switch option (-e) to automatically increase the image
 *    contrast, producing a "nicer" looking image.  The default mode
 *    is NOT to enhance the image, but to allow the user to enhance
 *    the image using an appropriate software tool.
 *
 *    [ALGORITHM]:
 *
 *      TARGET CHIPS/
 *      CLUTTER FULLSCENES : New histogram-based method applied....
 *
 *      TARGET FULLSCENES  : Same. Multiplies pixel value by constant and
 *                           clips to 255.
 *
 *------------------------------------------------------------------------
 *
 * [ASCII Header]:  
 * 
 *    This routine includes a switch option (-h) that allows the dumping
 *    of the MSTAR Phoenix header to an output file.  The output header
 *    will use name of input MSTAR file as its basename and will have the
 *    extension: ".hdr".  The default mode is NOT to dump header.
 *
 *------------------------------------------------------------------------
 *
 * [INFORMATION]:
 *
 *    This routine includes a switch option (-v) that turns on the 
 *    verbose mode.  The code normally runs in quiet mode, but will,
 *    if the -v switch is entered, echo what processes are being run.
 *
 *------------------------------------------------------------------------
 *
 * [Syntax/Usage]:  
 *
 *    mstar2ras -i <MSTAR input> -o <SunRaster output> [-e] [-h] [-v] 
 *                                    
 *            Options: [-e] -- Apply automatic contrast enhancement
 *                     [-h] -- Dump Phoenix header
 *                     [-v] -- verbose mode
 *
 *-----------------------------------------------------------------------
 *
 * [Contacts]: 
 * 
 *   John F. Querns
 *   Veridian Engineering (Dayton Group) 
 *   5200 Springfield Pike, Dayton, OH 45431  
 *   email: jquerns@dytn.veridian.com 
 *  
 *   Veridian Contractor Area  
 *   Area B  Bldg 23  Rm 115
 *   2010 Fifth Street 
 *   Wright Patterson AFB, OH  45433-7001  
 *   Work : (937) 255-1116, Ext 2818  
 *   email: jquerns@mbvlab.wpafb.af.mil 
 * 
 *------------------------------------------------------------------------- 
 * 
 * [Versions]:
 *
 *   1.0: 13 Aug 1997
 * 
 *   2.0: 24 September 1998 - 
 *        Improved auto-enhancement algorithm. [JQuerns]
 *        Added on-the-fly auto-byteswapping for PCs. [JQuerns]
 *        Added code to 16-bit align output for odd-width images. [JQuerns]
 *
 *------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "rasterfile.h"  /* Note: you could use one in /usr/include */

/* MSTAR Image type */
#define MSTAR_CHIP     0           /* Implies MSTAR target chip file..     */
#define MSTAR_FSCENE   1           /* Implies MSTAR fullscene target file  */
#define MSTAR_CLUT     2           /* Implies MSTAR fullscene clutter file */

/* ByteOrder definitions */
#define LSB_FIRST    0             /* Implies little-endian CPU... */
#define MSB_FIRST    1             /* Implies big-endian CPU...    */

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   1
#endif

#ifndef MAX
#define MAX(x,y)   (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)   (((x) < (y)) ? (x) : (y))
#endif

/* Function declarations */
extern char          *read_switch();
static float          byteswap_SR_IR();
static unsigned short byteswap_SUS_IUS();
static int            CheckByteOrder();


main(argc, argv)

  int   argc;
  char *argv[];

{
  FILE *MSTARfp=NULL;        /* Ptr to input MSTAR file....    */
  FILE *RASfp=NULL;          /* Ptr to output Sunraster file.. */
  FILE *HDRfp=NULL;          /* Ptr to opt output header file..*/

  struct rasterfile ohdr;    /* Output SunRaster header struct */
  struct rasterfile *hptr;

  unsigned char *pic8;       /* Ptr to output 8-bit image...   */
  unsigned char *linebuf;    /* Ptr to 8-bit row...            */

  int   i, j, w, h, n, d, numgot;
  int   linesize, bytesperpixel;

  char *MSTARname=NULL;      /* Ptr to input MSTAR filename...   */
  char *RASname=NULL;        /* Ptr to output SRAS filename...   */
  char  HDRname[40];         /* Buffer to hold opt. hdr filename */

  int   HDRflag=FALSE;       /* If TRUE, dump Phoenix header..   */
  int   ENHANCEflag=FALSE;   /* If TRUE, apply enhancement..     */
  int   VERBOSEflag=FALSE;   /* If TRUE, keep user informed...   */

  /* Input argument ptr variables...  */
  char *HDRopt, *ENHopt, *VERBopt;

  int            phlen, nhlen, mstartype, radarmode; 
  long           magloc, bytesPerImage, nchunks;
  char          *tptr=NULL, *phdr;
  unsigned char  tbuff[2048], ival;

  unsigned short *FSCENEdata=NULL, smax, smin, srange;
  float          *CHIPdata=NULL;

  /* 8-bit linear scaling variables */
  float  fscale, f1, f2, fmax, fmin, frange;

  /* Byte Order Variables */
  int            byteorder;
  unsigned char  bigfloatbuf[4];   /* BigEndian float buffer... */
  float          littlefloatval;   /* LittleEndian float value  */
  unsigned char  bigushortbuf[2];  /* BigEndian ushort buffer...*/ 
  unsigned short littleushortval;  /* LittleEndian ushort value.*/

  /* Enhancement Algorithm Variables */
  int            hist8[256];
  int            histmax, histmin, histrange, numcolors=0; 
  int            maxPixelCountBin, minPixelCountBin;
  int            CountBinRange;
  float          fsum, t1, hval; 
  float          scaleAdj, threshVal, threshLim;


/************************ B E G I N  C O D E ****************************/

if (argc < 2)
   {
    fprintf(stderr,
           "\n\nUsage: mstar2ras -i <MSTAR File> -o <SunRaster File> [-e] [-h] [-v]");
    fprintf(stderr,"\n  Options:  [-e]   =  Auto-contrast enhance [Def: none]");
    fprintf(stderr,"\n            [-h]   =  Dump Phoenix (ASCII) header [Def: none]");
    fprintf(stderr,"\n            [-v]   =  Verbose mode [Def: Quiet]\n\n");
    exit(1);
   } 

/* Extract Input & Output filenames */
MSTARname = read_switch(&argc, argv, "-i", 1, NULL);
RASname  = read_switch(&argc, argv, "-o", 1, NULL);

/* Check for output Phoenix header dump option */
HDRopt  = read_switch(&argc, argv, "-h", 0, NULL);
if (HDRopt != (char *) NULL)
   {
    HDRflag = TRUE;
   }

/* Check for Enhancement option */
ENHopt  = read_switch(&argc, argv, "-e", 0, NULL);
if (ENHopt != (char *) NULL)
   {
    ENHANCEflag = TRUE;
   }
 
/* Check for VERBOSE option */
VERBopt = read_switch(&argc, argv, "-v", 0, NULL);
if (VERBopt != (char *) NULL)
   {
    VERBOSEflag = TRUE;
   }
 

/* Form output header filename, if header option selected */
if (HDRflag == TRUE)
   {
    HDRname[0] = '\0';
    strcat(HDRname, MSTARname);
    strcat(HDRname, ".hdr");
   }

if (VERBOSEflag == TRUE)
   {
    printf("\n\nMSTAR to SUNRASTER conversion: started!\n");
   }


/***************** MAIN MSTAR PROCESSING AREA ********************/

/* Open input MSTAR file for reading */
MSTARfp = fopen(MSTARname,"rb");
if (MSTARfp == NULL)
   {
    fprintf(stderr,
      "\n\nError: Unable to open [%s] for reading!\n\n", MSTARname);
    exit(1);
   }

/****************************************************
 * Read first 2K bytes to figure out some header    *
 * parameters....                                   *
 ****************************************************/ 

if (VERBOSEflag == TRUE)
   {
    printf("Determining input image/hdr information...\n");
   }

fread(tbuff, sizeof(char), 2048, MSTARfp);
rewind(MSTARfp);

/* Extract Phoenix Summary header length */
tptr = (char *) strstr(tbuff,"PhoenixHeaderLength= ");
if (tptr == (char *) NULL)
   {
    fprintf(stderr,"Can not determine Phoenix header length!\n");
    fclose(MSTARfp);
    exit(1);
   } else
        {
         sscanf((tptr+20), "%d", &phlen);
        }

/* Check for and extract native header length */
tptr = (char *) strstr(tbuff,"native_header_length= ");
if (tptr == (char *) NULL)
   {
    fprintf(stderr,"Can not determine native header length!\n");
    fclose(MSTARfp);
    exit(1);
   } else
        {
         sscanf((tptr+21), "%d", &nhlen);
        }

/* Extract MSTAR column width */
tptr = (char *) strstr(tbuff,"NumberOfColumns= ");
if (tptr == (char *) NULL)
   {
    fprintf(stderr,
           "Error: Can not determine MSTAR image width");
    fclose(MSTARfp);
    exit(1);
   } else
        {
         sscanf((tptr+16), "%d", &w);
        }

/* Extract MSTAR row height */
tptr = (char *) strstr(tbuff,"NumberOfRows= ");
if (tptr == (char *) NULL)
   {
    fprintf(stderr,
           "Error: Can not determine MSTAR image height!");
    fclose(MSTARfp);
    exit(1);
   } else
        {
         sscanf((tptr+13), "%d",&h); 
        }

/* Set MSTAR image type */
tptr = (char *) strstr(tbuff,"RadarMode= ");
if (tptr == (char *) NULL)
   {
    fprintf(stderr,
           "Error: Can not determine MSTAR image radar mode!\n\n");
    fclose(MSTARfp);
    exit(1);
   } else
        {
         sscanf((tptr+16), "%d", &radarmode);
        }
 
if (nhlen == 0)
   {/* Implies FLOAT MSTAR chip image */
    mstartype = MSTAR_CHIP;
    if (VERBOSEflag == TRUE)
       { 
        printf("==> MSTAR target chip image detected...\n");
       }
   } else
        {
         switch (radarmode)
           {
            case 4: /* Strip mode */
              mstartype = MSTAR_CLUT;
              break;
            case 5: /* Spotlight */
            default:
              mstartype = MSTAR_FSCENE;
              break;
           }
 
         if (VERBOSEflag == TRUE)
            {
             switch (mstartype)
               { 
                case MSTAR_CLUT:
                  printf("==> MSTAR fullscene clutter image detected...\n");
                  break;
                case MSTAR_FSCENE:
                  printf("==> MSTAR fullscene target image detected...\n");
                  break;
               } 
            }
        }

/*******************************************************
 * If HEADER FLAG = TRUE:                              *
 * Allocate memory to header buffer, read Phoenix hdr, *
 * and write out to output file...free header buffer.  *
 *******************************************************/
if (HDRflag == TRUE)
   {
    HDRfp = fopen(HDRname, "w");
    if (HDRfp == NULL)
       {
        fprintf(stderr,
                "Error: unable to open header file [%s] for writing!\n",
                HDRname);
        fclose(MSTARfp);
        exit(1);
       }

   phdr = (char *) malloc(phlen+1);
   if (phdr == (char *) NULL)
      {
       fprintf(stderr,
              "Error: unable to allocate header memory!\n");
       fclose(MSTARfp);
       fclose(HDRfp);
       unlink(HDRname);
       exit(1);
      }

   fread(phdr, sizeof(char), phlen, MSTARfp);
   fwrite(phdr, sizeof(char), phlen, HDRfp);
   fclose(HDRfp);
   free(phdr);

   if (VERBOSEflag == TRUE)
      {
       printf("Phoenix header written to [%s]...\n", HDRname);
      }
  }

/***********************************************************
 * Set up file location to point to MSTAR magnitude data.. *
 ***********************************************************/
switch (mstartype)
  {
   case MSTAR_CHIP:
     magloc  = phlen;
     fseek(MSTARfp, magloc, 0);
     break;
   case MSTAR_CLUT:
   case MSTAR_FSCENE:
     magloc  = phlen + nhlen; /* nhlen = 512 */
     fseek(MSTARfp, magloc, 0);
     break;
  }

/******************************************************
 * Check byte-order, swap bytes if necessary...       *
 * Allocate memory, read data,  & convert to 8-bit    *
 * based on 'mstartype'                               *
 ******************************************************/

/* Check byteorder */
byteorder = (int) CheckByteOrder();
switch (byteorder)
  {
   case LSB_FIRST:
     
     if (VERBOSEflag == TRUE)
        {
         printf("==> Little-Endian CPU detected: Will byteswap before scaling data!\n");
        }
     break;

   case MSB_FIRST:
   
     if (VERBOSEflag == TRUE)
        {
         printf("==> Big-Endian CPU detected: No byteswap needed!\n");
        }
     break;
  }


/******************************************************
 * Allocate memory, read data,  & convert to 8-bit    *
 * based on 'mstartype'                               *
 ******************************************************/
numgot = 0;
nchunks = w * h;
switch (mstartype)
  {
   case MSTAR_CHIP:
     bytesPerImage = nchunks * sizeof(float);
     CHIPdata = (float *) malloc(bytesPerImage);
     if (CHIPdata == (float *) NULL)
        {
         fprintf(stderr,
                 "Error: Unable to malloc CHIP memory!\n");
         fclose(MSTARfp);
         exit(1);
        }

     pic8 = (unsigned char *) calloc((size_t) w * h, (size_t) 1);
     if (pic8 == (unsigned char *) NULL) 
        { 
         fprintf(stderr, 
                 "Error: Unable to malloc output 8-bit memory!\n");
         fclose(MSTARfp);
         free(CHIPdata);
         exit(1);
        }
 
     if (VERBOSEflag == TRUE)
        {
         printf("Reading MSTAR Chip magnitude data...\n");
        }

     switch (byteorder)
       {
        case LSB_FIRST: /* Little-endian..do byteswap */

          if (VERBOSEflag == TRUE)
             {
              printf("Performing auto-byteswap...\n"); 
             }
          for (i = 0; i < nchunks; i++)
              {
               fread(bigfloatbuf, sizeof(char), 4, MSTARfp);
               littlefloatval = byteswap_SR_IR(bigfloatbuf);
               CHIPdata[i] = littlefloatval;
              }
          break; 

        case MSB_FIRST: /* Big-endian..no swap */

          numgot = fread(CHIPdata, sizeof(float), nchunks, MSTARfp);
          break;
       }

     if (VERBOSEflag == TRUE)
        {
         printf("Scaling MSTAR Chip magnitude data to 8-bit...\n");
        }

     fmax = 0.0;  
     fmin = 100000.0;
     for (i = 0; i < nchunks; i++)
       {
        fmax = MAX(CHIPdata[i], fmax);
        fmin = MIN(CHIPdata[i], fmin);
       }

     frange = fmax - fmin;
     fscale =  255.0/frange;

     for (i = 0; i < nchunks; i++)
       {
        f1 = (float) CHIPdata[i] - fmin;
        f2 = f1 * fscale;
        pic8[i] = (unsigned char) (f2 + 0.5);
       }
     free(CHIPdata);
     break;

     case MSTAR_CLUT:
     case MSTAR_FSCENE:
       bytesPerImage = nchunks * sizeof(short);
       FSCENEdata = (unsigned short *) malloc( bytesPerImage );
       if (FSCENEdata == (unsigned short *) NULL)
          {
           fprintf(stderr,
                   "Error: Unable to malloc fullscene memory!\n");
           fclose(MSTARfp);
           exit(1);
          }

       pic8 = (unsigned char *) calloc((size_t) w * h, (size_t) 1);
       if (pic8 == (unsigned char *) NULL) 
          { 
           fprintf(stderr, 
                   "Error: Unable to malloc output 8-bit memory!\n");
           fclose(MSTARfp);
           free(FSCENEdata);
           exit(1);
          }

       if (VERBOSEflag == TRUE)
          {
           printf("Reading MSTAR fullscene magnitude data...\n");
          }

       switch (byteorder)
        {
         case LSB_FIRST: /* Little-endian..do byteswap */

           if (VERBOSEflag == TRUE)
              {
               printf("Performing auto-byteswap...\n"); 
              }
           for (i = 0; i < nchunks; i++)
               {
                fread(bigushortbuf, sizeof(char), 2, MSTARfp);
                littleushortval = byteswap_SUS_IUS(bigushortbuf);
                FSCENEdata[i] = littleushortval;
               }
           break; 

         case MSB_FIRST: /* Big-endian..no swap */

           numgot = fread(FSCENEdata, sizeof(short), nchunks, MSTARfp);
           break;
       }

       if (VERBOSEflag == TRUE)
          {
           printf("Scaling MSTAR fullscene magnitude data to 8-bit...\n");
          }

       smax = 0;  
       smin = 65535;
       for (i = 0; i < nchunks; i++)
           {
            smax = MAX(FSCENEdata[i], smax);
            smin = MIN(FSCENEdata[i], smin);
           }
       srange = smax - smin;
       fscale = 255.0 / (float) srange;

       for (i = 0; i < nchunks; i++)
           {
            f1 = (float) (FSCENEdata[i] - smin);
            f2 = f1 * fscale;
            pic8[i] = (unsigned char) (f2 + 0.5);
           }            

       free(FSCENEdata);
       break;
     }

fclose(MSTARfp);   


/************* MAIN ENHANCEMENT PROCESSING AREA ****************/

if (ENHANCEflag == TRUE)
   {
    if (VERBOSEflag == TRUE)
       {
        printf("Contrast-Adjusting 8-bit data...\n");
       }

    /*********** NEW ENHANCEMENT ALGORITHM *******************/

    /* Initialize histogram array */
    for (i = 0; i < 256; i++)
      {  
       hist8[i] = 0;
      }  
 
    /* Calculate histogram of MSTAR 8-bit data */
    for (i = 0; i < nchunks; i++)
      {  
       ival = pic8[i];
       hist8[ival]++;
      }  
 
    /******************************************
     * Calculate number of grayscale values.. *
     ******************************************/
    for (i = 0; i < 256; i++)
      {  
       if (hist8[i] > 0)
          {
           numcolors++;
          }
      }
   
    /* Find MAX & MIN COUNTS of Histogram BINS */
    histmax = 0;
    histmin = nchunks;
    for (i = 0; i < 256; i++)
      {
       histmax = MAX(hist8[i], histmax);
       histmin = MIN(hist8[i], histmin);
      }
 
    /* Find BIN where MAX & MIN Pixel Counts are */
    maxPixelCountBin = 0;      /* Assume Bin#0   has maximum pixel count.. */
    minPixelCountBin = 255;    /* Assume Bin#255 has minimum pixel count.. */
    for (i = 0; i < 256; i++)
      {
       if (hist8[i] == histmax)
          {/* Do this just in case > 1 bin has histmax values */
           maxPixelCountBin = MAX(maxPixelCountBin, i);
          }
 
       if (hist8[i] == histmin)
          {/* Do this just in case > 1 bin has histmin values */
           minPixelCountBin = MIN(minPixelCountBin, i);
          }
      }

    CountBinRange = abs(maxPixelCountBin - minPixelCountBin);
 
    /* Adjust Pixel Contrast */
    for (i = 0; i < nchunks; i++)
      {
       switch (mstartype)
         {
          case MSTAR_CHIP:
          case MSTAR_CLUT:
 
            ival = pic8[i];
            if (minPixelCountBin > maxPixelCountBin)
               { 
                threshVal = (float) (minPixelCountBin - maxPixelCountBin);
                if (ival > threshVal)
                   {
                    f1 = 255.0;
                   } else
                        {
                         scaleAdj = 255.0 /threshVal;
                         f1 = scaleAdj * (float) ival;
                        }
              } else
                   {
                    f1 = 3.0 * (float) ival;
                   }
            break;
 
 
          case MSTAR_FSCENE:
            f1 = (float) pic8[i] * 20.0;
            break;
         }
   
     /*********** NEW ENHANCEMENT ALGORITHM *******************/

       if (f1 > 255.0)
          {
           pic8[i] = 255;
          } else
               {
                pic8[i] = (unsigned char) (f1 + 0.5);
               }
      }

   }


/************* MAIN SUNRASTER PROCESSING AREA ********************/

if (VERBOSEflag == TRUE)
   {
    printf("Setting up SUNRASTER header parameters....\n");
   }

/* Open output Sunraster file for writing */
RASfp = fopen(RASname, "wb");
if (RASfp == NULL)
   {
    fprintf(stderr,
            "Error: unable to open [%s] for writing!\n\n",
            RASname);
    free(pic8);
    exit(1);
   }

/* Perform Zero-padding check of image row */
d = 8; /* 8-bit depth */
linesize = w * d;
if (linesize%16) linesize += (16 - (linesize%16));
linesize /= 8;

/* Allocate memory to line buffer */
linebuf = (unsigned char *) calloc((size_t) linesize, (size_t) 1);
if (linebuf == (unsigned char *) NULL)
   {
    fprintf(stderr,
            "Error: unable to allocate space for line buffer!\n");
    free(pic8);
    fclose(RASfp);
    unlink(RASname);  /* This deletes output file */
    exit(1);
   }

/* Set up some Sunraster header attributes */
ohdr.ras_magic     = RAS_MAGIC;     /* 0x59a66a95          */
ohdr.ras_width     = w;             /* Image width  (cols) */
ohdr.ras_height    = h;             /* Image height (rows) */
ohdr.ras_depth     = 8;             /* 8-bits per pixel    */
ohdr.ras_length    = (w*h);         /* Num bytes in image  */
ohdr.ras_type      = RT_STANDARD;   /* Std image...        */
ohdr.ras_maptype   = RMT_NONE;      /* No color palette    */
ohdr.ras_maplength = 0;             /* Colormap length     */

/* Write out the Sunraster header */
if (VERBOSEflag == TRUE)
   {
    printf("Writing header data to Sunraster file: [%s]\n", RASname);
   }

hptr = &ohdr;
n = fwrite(hptr, sizeof(struct rasterfile), 1, RASfp);
if (n != 1)
   {
    fprintf(stderr,
           "Error: in writing out Sunraster header!\n\n");
    free(pic8);
    fclose(RASfp);
    unlink(RASname);
    exit(1);
   }

/* Write the image data */
if (VERBOSEflag == TRUE)
   {
    printf("Writing image data to Sunraster file: [%s]\n", RASname);
   }

for (j = 0; j < h; j++)
  {
   for (i = 0; i < w; i++)
     {
      linebuf[i] = pic8[i + (w*j)];
     }
   fwrite(linebuf, sizeof(unsigned char), linesize, RASfp);
  }

/* Free and close stuff */
free(pic8);    /* Frees ptr to 8-bit image data....    */
free(linebuf); /* Free line buffer space               */
fclose(RASfp); /* Closes output file...                */

if (VERBOSEflag == TRUE)
   {
    printf("\nMSTAR to SUNRASTER conversion: completed!\n\n");
   }

exit(0);
} /* End of mstar2ras() */



/********************** STATIC FUNCTIONS ************************/

/************************************************
 * Function:    byteswap_SR_IR                  *
 *   Author:    Dave Hascher (Veridian Inc.)    *
 *     Date:    06/05/97                        *
 *    Email:    dhascher@dytn.veridian.com      *
 ************************************************
 * 'SR' --> Sun 32-bit float value              *
 * 'IR' --> PC-Intel 32-bit float value         *
 ************************************************/

static float byteswap_SR_IR(pointer)
unsigned char *pointer;
{
  float *temp;
  unsigned char iarray[4], *charptr;

  iarray[0] = *(pointer + 3);
  iarray[1] = *(pointer + 2);
  iarray[2] = *(pointer + 1);
  iarray[3] = *(pointer );
  charptr = iarray;
  temp    = (float *) charptr;
  return *(temp);
}


/************************************************
 * Function:    byteswap_SUS_IUS                *
 *   Author:    John Querns (Veridian Inc.)     *
 *     Date:    06/05/97                        *
 *    Email:    jquerns@dytn.veridian.com       *
 ************************************************
 * 'SUS' --> Sun 16-bit uns short value         *
 * 'IUS' --> PC-Intel 16-bit uns short value    *
 ************************************************/

static unsigned short byteswap_SUS_IUS(pointer)
unsigned char *pointer;
{
  unsigned short *temp;
  unsigned char iarray[2], *charptr;

  iarray[0] = *(pointer + 1);
  iarray[1] = *(pointer );
  charptr = iarray;
  temp    = (unsigned short *) charptr;
  return *(temp);
}



/**********************************
 *   checkByteOrder()             *
 **********************************
 * Taken from:                    *
 *                                *
 *   Encyclopedia of Graphic File *
 *   Formats, Murray & Van Ryper, *
 *   O'Reilly & Associates, 1994, *
 *   pp. 114-115.                 *
 *                                *
 * Desc: Checks byte-order of CPU.*
 **********************************/

static int CheckByteOrder(void)

{
  short   w = 0x0001;
  char   *b = (char *) &w;

  return(b[0] ? LSB_FIRST : MSB_FIRST);
}
/************************ LAST LINE OF mstar2ras.c ************************/
