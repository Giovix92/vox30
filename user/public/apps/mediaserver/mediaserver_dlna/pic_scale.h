#ifndef __PIC_SCALE_H__
#define	__PIC_SCALE_H__

#define	JPEG_MEDIUM_WIDTH		1024
#define	JPEG_MEDIUM_HEIGHT		768

#define	JPEG_SMALL_WIDTH		640
#define	JPEG_SMALL_HEIGHT		480

#define	JPEG_TN_WIDTH			160
#define	JPEG_TN_HEIGHT			160

int CreateThumb(char *pFileName, char *pThumbName, char *resolution);
int GetScaledPictureResolution(int org_width, int org_height, int *new_width, int *new_height);
#endif

