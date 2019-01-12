/***************************************************************************
*xvmstar.c *
***************************************************************************
**
*Author:John F.Querns, Veridian Inc., Veda Operations *
*Date:03 Mar 98 *
**
*Description:This is the LOAD routine for MSTAR
  format files.It *
      *allows XV to display the magnitude portion of MSTAR *
      *data files.It will process MSTAR target chips and *
      *fullscenes, including those with only a binary C4PL *
      *header.Magnitude data is linearly scaled to 8 - bit for *
      *display
    . *
	***************************************************************************
	*THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION IS RELEASED "AS IS." *
	*THE U.S.GOVERNMENT MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, *
	*CONCERNING THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION, INCLUDING, *
	*WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A *
	*PARTICULAR PURPOSE.IN NO EVENT WILL THE U.S.GOVERNMENT BE LIABLE FOR *
	*ANY DAMAGES, INCLUDING ANY LOST PROFITS, LOST SAVINGS OR OTHER *
	*INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE, OR INABILITY *
	*TO USE, THIS SOFTWARE OR ANY ACCOMPANYING DOCUMENTATION, EVEN IF *
	*INFORMED IN ADVANCE OF THE POSSIBILITY OF SUCH DAMAGES."                *
	***************************************************************************/


/* xvmstar.c - load routine for 'mstar' format magnitude data pictures

 * LoadMSTAR(fname, pinfo)  -  loads magnitude data for an MSTAR file NO WRITE
 * MODULE.... */

#include "copyright.h"

#include "xv.h"



/* comments on error handling: a truncated file is not considered a Major
 * Error.  The file is loaded, the rest of the pic is filled with 0's.
 * 
 * a file with garbage characters in it is an unloadable file.  All allocated
 * stuff is tossed, and LoadMSTAR returns non-zero
 * 
 * not being able to malloc is a Fatal Error.  The program is aborted. */


#define TRUNCSTR "File appears to be truncated."

#define MAX(x,y)   (((x) > (y)) ? (x) : (y))

#define MIN(x,y)   (((x) < (y)) ? (x) : (y))

	static int garbage;
static long numgot;

static int loadmstar PARM ((FILE *, PICINFO *, int, int));
static long little2bigL PARM ((unsigned char *));
static int mstarError PARM ((char *, char *));

static char *bname;

/*******************************************/
int
LoadMSTAR (fname, pinfo)
     char *fname;
     PICINFO *pinfo;

/*******************************************/
{
  /* returns '1' on success */

  FILE *fp;
  int c, c1, i;
  int maxv, rv;
  int phlen, nhlen, native_rows, native_cols;
  int mstartype, normal = 1, nchunks;
  long magloc;
  char *tptr = NULL;
  unsigned char tbuff[512];


  garbage = maxv = rv = 0;
  bname = BaseName (fname);

  pinfo->pic = (byte *) NULL;
  pinfo->comment = (char *) NULL;


  /* open the file for reading (binary) */
  fp = xv_fopen (fname, "rb");
  if (!fp)
    return (mstarError (bname, "can't open file"));

  /****************************************************
   * Read first 512 bytes to figure out some header   *
   * parameters....                                   *
   ****************************************************/
  fread (tbuff, sizeof (char), 512, fp);

  /* Extract Phoenix Summary header length */
  tptr = (char *) strstr (tbuff, "PhoenixHeaderLength= ");
  if (tptr != (char *) NULL)
    {
      sscanf ((tptr + 20), "%d", &phlen);
    }
  else
    {
      normal = -1;
      phlen = 0;		/* No Phoenix header ... */
      nhlen = 512;		/* Assume fscene with native header */
    }

  /* Check for and extract native header length */
  if (normal == 1)
    {
      tptr = (char *) strstr (tbuff, "native_header_length= ");
      if (tptr != (char *) NULL)
	{
	  sscanf ((tptr + 21), "%d", &nhlen);
	}
    }
  /* Extract MSTAR column width */
  if (normal == 1)
    {
      tptr = (char *) strstr (tbuff, "NumberOfColumns= ");
      if (tptr == (char *) NULL)
	{
	  fprintf (stderr,
		   "Error: Can not determine MSTAR image width");
	  fclose (fp);
	  exit (1);
	}
      else
	{
	  sscanf ((tptr + 16), "%d", &pinfo->w);
	  pinfo->normw = pinfo->w;
	}
    }
  else
    {
      pinfo->w = little2bigL (tbuff + 12);
      pinfo->normw = pinfo->w;
    }

  /* Extract MSTAR row height */
  if (normal == 1)
    {
      tptr = (char *) strstr (tbuff, "NumberOfRows= ");
      if (tptr == (char *) NULL)
	{
	  fprintf (stderr,
		   "Error: Can not determine MSTAR image height!");
	  fclose (fp);
	  exit (1);
	}
      else
	{
	  sscanf ((tptr + 13), "%d", &pinfo->h);
	  pinfo->normh = pinfo->h;
	}
    }
  else
    {
      pinfo->h = little2bigL (tbuff + 8);
      pinfo->normh = pinfo->h;
    }

  /* Set MSTAR image type */
  if (nhlen == 0)
    {				/* Implies FLOAT MSTAR chip image */
      mstartype = 0;
    }
  else
    {
      mstartype = 1;		/* UnShort Fullscene */
    }

  /******************************************************
   * Set up location to point to MSTAR magnitude data.. *
   ******************************************************/

  switch (mstartype)
    {
    case 0:			/* MSTAR Chip image */
      magloc = phlen;
      fseek (fp, magloc, 0);
      nchunks = pinfo->w * pinfo->h;
      printf ("\nLoading MSTAR Chip image into XV...\n");
      break;
    case 1:			/* MSTAR FullScene image */
      magloc = phlen + nhlen;	/* nhlen = 512 */
      fseek (fp, magloc, 0);
      nchunks = pinfo->h * pinfo->w;
      printf ("\nLoading MSTAR FullScene image into XV...\n");
      break;
    }

  /* Load image */
  rv = loadmstar (fp, pinfo, mstartype, nchunks);

  if (garbage)
    {
      fclose (fp);
      if (pinfo->comment)
	free (pinfo->comment);
      pinfo->comment = (char *) NULL;
      return (mstarError (bname, "Garbage characters in header."));
    }
  pinfo->frmType = -1;		/* No write mode */

  /* note:  pic, type, r,g,b, frmInfo, shortFrm, and colorType fields of
   * picinfo struct are filled in in the format-specific loaders */


  fclose (fp);

  if (!rv)
    {
      if (pinfo->pic)
	free (pinfo->pic);
      if (pinfo->comment)
	free (pinfo->comment);
      pinfo->pic = (byte *) NULL;
      pinfo->comment = (char *) NULL;
    }
  return rv;
}



/*******************************************/
static int
loadmstar (fp, pinfo, mtype, npixels)
     FILE *fp;
     PICINFO *pinfo;
     int mtype;
     int npixels;
{
  byte *pic8;
  byte *pix;
  int i, j, bitshift, w, h, holdmaxv;
  long bytesPerImage;

  unsigned short *FSCENEdata = NULL, smax, smin, srange;
  float *CHIPdata = NULL;
  float fscale, f1, f2, fmax, fmin, frange;


  w = pinfo->w;
  h = pinfo->h;
  pic8 = (byte *) calloc ((size_t) w * h, (size_t) 1);
  if (!pic8)
    return mstarError (bname, "couldn't malloc 'pic8'");

  /******************************************************
   * Allocate memory, read data,  & convert to 8-bit    *
   * based on 'mstartype'                               *
   ******************************************************/
  numgot = 0;
  switch (mtype)
    {
    case 0:			/* MSTAR (Float) Chip data */
      bytesPerImage = npixels * sizeof (float);

      CHIPdata = (float *) malloc (bytesPerImage);

      printf ("Reading float CHIP data...\n");

      numgot = fread (CHIPdata, sizeof (float), npixels, fp);

      printf ("Converting to 8-bit...\n");

      fmax = 0.0;
      fmin = 100000.0;
      for (i = 0; i < npixels; i++)
	{
	  fmax = MAX (CHIPdata[i], fmax);
	  fmin = MIN (CHIPdata[i], fmin);
	}
      frange = fmax - fmin;
      fscale = 255.0 / frange;

      for (i = 0; i < npixels; i++)
	{
	  f1 = (float) CHIPdata[i] - fmin;
	  f2 = f1 * fscale;
	  pic8[i] = (byte) (f2 + 0.5);
	}
      free (CHIPdata);
      break;

    case 1:			/* MSTAR FS (UnShort) data */
      bytesPerImage = npixels * sizeof (short);

      FSCENEdata = (unsigned short *) malloc (bytesPerImage);

      printf ("Reading unsigned short FSCENE data...\n");

      numgot = fread (FSCENEdata, sizeof (short), npixels, fp);

      printf ("Converting to 8-bit...\n");

      smax = 0;
      smin = 65535;
      for (i = 0; i < npixels; i++)
	{
	  smax = MAX (FSCENEdata[i], smax);
	  smin = MIN (FSCENEdata[i], smin);
	}
      srange = smax - smin;
      fscale = 255.0 / (float) srange;

      for (i = 0; i < npixels; i++)
	{
	  f1 = (float) (FSCENEdata[i] - smin);
	  f2 = f1 * fscale;
	  pic8[i] = (byte) (f2 + 0.5);
	}

      free (FSCENEdata);
      break;
    }

  pinfo->pic = pic8;
  pinfo->type = PIC8;
  if (mtype == 0)
    {
      sprintf (pinfo->fullInfo, "MSTAR CHIP Image");
      sprintf (pinfo->shrtInfo, "%dx%d MSTAR Chip Magnitude.", pinfo->w, pinfo->h);
    }
  else
    {
      sprintf (pinfo->fullInfo, "MSTAR FULLSCENE Image");
      sprintf (pinfo->shrtInfo, "%dx%d MSTAR FS  Magnitude.", pinfo->w, pinfo->h);
    }
  pinfo->colType = F_GREYSCALE;


  printf ("Building colormap...\n");

  /* fill in a greyscale colormap that maps to 255 */
  for (i = 0; i <= 255; i++)
    pinfo->r[i] = pinfo->g[i] = pinfo->b[i] = i;

  if (numgot != w * h)
    mstarError (bname, TRUNCSTR);	/* warning only */

  return 1;
}


/*******************************************/
static long
little2bigL (pointer)
     unsigned char *pointer;
{
  long *temp;
  unsigned char iarray[4], *charptr;

  iarray[0] = *(pointer + 3);
  iarray[1] = *(pointer + 2);
  iarray[2] = *(pointer + 1);
  iarray[3] = *(pointer);
  charptr = iarray;
  temp = (long *) charptr;
  return *(temp);
}

/*******************************************/
static int
mstarError (fname, st)
     char *fname, *st;
{
  SetISTR (ISTR_WARNING, "%s:  %s", fname, st);
  return 0;
}
