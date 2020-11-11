/* author: kylin 
* e-mail: kylin_kang@sdc.sercomm.com 
* gif_tn.c
* 2007年07月31日 星期二 10时02分25秒
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"GifRSize"

#define MAX_SCALE	16.0			  /* Maximum scaling factor. */


/* Make some variables global, so we could access them faster: */
static GifPixelType
    BackGroundColor = 0;
static double
    XScale = 0.5,
    YScale = 0.5;
static int
    XSize = 0,
    YSize = 0;

static void ResizeLine(GifRowType LineIn, GifRowType LineOut,
		      int InLineLen, int OutLineLen);
static void QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut);
int create_gif_thumbnail(const char *in_file, const char *out_file, int maxsize); 


int
main(void) {
	
	int p = 0;
	create_gif_thumbnail("aa.gif", "bbb.gif", 50); 
	exit(0);
}



int create_gif_thumbnail(const char *in_file, const char *out_file, int maxsize) {
    int	i, iy, last_iy, l, t, w, h, Error, NumFiles, ExtCode,
	ImageNum = 0,
	SizeFlag = FALSE,
	ScaleFlag = FALSE,
	XScaleFlag = FALSE,
	YScaleFlag = FALSE,
	HelpFlag = FALSE;
    double Scale, y;
    GifRecordType RecordType;
    char s[80];
    GifByteType *Extension;
    GifRowType LineIn, LineOut;
    char **FileName = NULL;
    GifFileType *GifFileIn = NULL, *GifFileOut = NULL;

	SizeFlag = 1;
	XSize = 50;
	YSize = 50;
	ScaleFlag = 0;
	Scale = 0.0;
	XScaleFlag = 0;
	YScaleFlag = 0;


	fprintf(stderr, "mark 000, in_file: %s\n", in_file);
	if ((GifFileIn = DGifOpenFileName(in_file)) == NULL)
	    QuitGifError(GifFileIn, GifFileOut);

	fprintf(stderr, "mark 001, SWidth: %d, SHeight: %d\n", GifFileIn->SWidth, GifFileIn->SHeight);
	XSize = (GifFileIn->SHeight > GifFileIn->SWidth) ? ( (float)GifFileIn->SWidth/GifFileIn->SHeight * maxsize) : maxsize;
	YSize = (GifFileIn->SWidth > GifFileIn->SHeight) ? ((float)GifFileIn->SHeight / GifFileIn->SWidth * maxsize) : maxsize;

	XScale = XSize / ((double) GifFileIn->SWidth);
	YScale = YSize / ((double) GifFileIn->SHeight);

    /* As at this time we know the Screen size of the input gif file, and as */
    /* all image(s) in file must be less/equal to it, we can allocate the    */
    /* scan lines for the input file, and output file. The number of lines   */
    /* to allocate for each is set by ScaleDown & XScale & YScale:	     */
    LineOut = (GifRowType) malloc(XSize * sizeof(GifPixelType));
    LineIn = (GifRowType) malloc(GifFileIn->SWidth * sizeof(GifPixelType));

	/* Open output file */
	if  ((GifFileOut = EGifOpenFileName(out_file, 0)) == NULL)
		QuitGifError(GifFileIn, GifFileOut);

    /* And dump out its new scaled screen information: */
    if (EGifPutScreenDesc(GifFileOut, XSize, YSize,
	GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
	GifFileIn->SColorMap) == GIF_ERROR)
	QuitGifError(GifFileIn, GifFileOut);

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
	if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
	    QuitGifError(GifFileIn, GifFileOut);

	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
		fprintf(stderr, "IMAGE_DESC_RECORD_TYPE\n");
		if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
	fprintf(stderr, "InLinLen: %d, OutLineLen: %d\n", GifFileIn->Image.Width, GifFileIn->Image.Height);
		/* Put the image descriptor to out file: */
		l = (int) (GifFileIn->Image.Left * XScale + 0.5);
		w = (int) (GifFileIn->Image.Width * XScale + 0.5);
		t = (int) (GifFileIn->Image.Top * YScale + 0.5);
		h = (int) (GifFileIn->Image.Height * YScale + 0.5);
		if (l < 0) l = 0;
		if (t < 0) t = 0;
		if (l + w > XSize) w = XSize - l;
		if (t + h > YSize) h = YSize - t;

	fprintf(stderr, "l: %d, t: %d, w: %d, h: %d\n", l, t ,w ,h);
		if (EGifPutImageDesc(GifFileOut, l, t, w, h,
		    GifFileIn->Image.Interlace,
		    GifFileIn->Image.ColorMap) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);

		if (GifFileIn->Image.Interlace) {
		    GIF_EXIT("Cannt resize interlaced images - use GifInter first.");
		}
		else {
		    GifQprintf("\n%s: Image %d at (%d, %d) [%dx%d]:     ",
			PROGRAM_NAME, ++ImageNum,
			GifFileOut->Image.Left, GifFileOut->Image.Top,
			GifFileOut->Image.Width, GifFileOut->Image.Height);

		    for (i = GifFileIn->Image.Height, y = 0.0, last_iy = -1;
			 i-- > 0;
			 y += YScale) {
			if (DGifGetLine(GifFileIn, LineIn,
					GifFileIn->Image.Width) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

			iy = (int) y;
			if (last_iy < iy && last_iy < YSize) {
			    ResizeLine(LineIn, LineOut,
				       GifFileIn->Image.Width, GifFileOut->Image.Width);

			    for (;
				 last_iy < iy && last_iy < GifFileOut->Image.Height - 1;
				 last_iy++) {
				GifQprintf("\b\b\b\b%-4d", last_iy + 1);
				if (EGifPutLine(GifFileOut, LineOut,
						GifFileOut->Image.Width) ==
								    GIF_ERROR)
				    QuitGifError(GifFileIn, GifFileOut);
			    }
			}
		    }

		    /* If scale is not dividable - dump last lines: */
		    while (++last_iy < GifFileOut->Image.Height) {
			GifQprintf("\b\b\b\b%-4d", last_iy);
			if (EGifPutLine(GifFileOut, LineOut,
					GifFileOut->Image.Width) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
		    }
		}
		break;
	    case EXTENSION_RECORD_TYPE:
		fprintf(stderr, "EXTENSION_RECORD_TYPE\n");
		/* Skip any extension blocks in file: */
		if (DGifGetExtension(GifFileIn, &ExtCode, &Extension) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		if (EGifPutExtension(GifFileOut, ExtCode, Extension[0],
							Extension) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);

		/* No support to more than one extension blocks, so discard: */
		while (Extension != NULL) {
		    if (DGifGetExtensionNext(GifFileIn, &Extension) == GIF_ERROR)
			QuitGifError(GifFileIn, GifFileOut);
		}
		break;
	    case TERMINATE_RECORD_TYPE:
		fprintf(stderr, "TERMINATE_RECORD_TYPE\n");
		break;
	    default:		    /* Should be traps by DGifGetRecordType. */
		break;
	}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

    if (DGifCloseFile(GifFileIn) == GIF_ERROR)
	QuitGifError(GifFileIn, GifFileOut);
    if (EGifCloseFile(GifFileOut) == GIF_ERROR)
	QuitGifError(GifFileIn, GifFileOut);

    free(LineOut);
    free(LineIn);

    return 0;
}

/******************************************************************************
* Close both input and output file (if open), and exit.			      *
******************************************************************************/
static void QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut)
{
    PrintGifError();
    if (GifFileIn != NULL) DGifCloseFile(GifFileIn);
    if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
    exit(EXIT_FAILURE);
}
/******************************************************************************
* Line resizing routine - scale given lines as follows:			      *
* Scale (by pixel duplication/elimination) from InLineLen to OutLineLen.      *
******************************************************************************/
static void ResizeLine(GifRowType LineIn, GifRowType LineOut,
		      int InLineLen, int OutLineLen)
{
    int i, ix, last_ix;
    double x;

    OutLineLen--;

    for (i = InLineLen, x = 0.0, last_ix = -1;
	 i-- > 0;
	 x += XScale, LineIn++)
    {
	ix = (int) x;
    	for (; last_ix < ix && last_ix < OutLineLen; last_ix++)
	    *LineOut++ = *LineIn;
    }

    /* Make sure the line is complete. */
    for (LineIn--; last_ix < OutLineLen; last_ix++)
	*LineOut++ = *LineIn;

}
