#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <highgui.h>

#include <imfutil.hpp>
#include <OpenEXR/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfPixelType.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfFrameBuffer.h>

using namespace cv;
using namespace Imf;
using namespace Imath;

typedef struct {
	Array2D<half> r, g, b;
	FrameBuffer fb;
	int w, h;
} exr_t;

void
read_exr(const char *fname, exr_t &exr)
{
	InputFile file(fname);
	Box2i dw = file.header().dataWindow();

	exr.w = dw.max.x - dw.min.x + 1;
	exr.h = dw.max.y - dw.min.y + 1;
	exr.r.resizeErase(exr.h, exr.w);
	exr.g.resizeErase(exr.h, exr.w);
	exr.b.resizeErase(exr.h, exr.w);

	exr.fb.insert("R",
		Slice(HALF, (char*)(&exr.r[0][0] - dw.min.x - dw.min.y * exr.w),
			sizeof(exr.r[0][0]) * 1, sizeof(exr.r[0][0]) * exr.w, 1, 1, 0.0));
	exr.fb.insert("G",
		Slice(HALF, (char*)(&exr.g[0][0] - dw.min.x - dw.min.y * exr.w),
			sizeof(exr.g[0][0]) * 1, sizeof(exr.g[0][0]) * exr.w, 1, 1, 0.0));
	exr.fb.insert("B",
		Slice(HALF, (char*)(&exr.b[0][0] - dw.min.x - dw.min.y * exr.w),
			sizeof(exr.b[0][0]) * 1, sizeof(exr.b[0][0]) * exr.w, 1, 1, 0.0));

	file.setFrameBuffer(exr.fb);
	file.readPixels(dw.min.y, dw.max.y);
}

void
exrcvview(int n, char **fnames, bool to_srgb, bool loop)
{
	exr_t exr;
	IplImage *rgb = nullptr;

	cvNamedWindow("exrcvview", CV_WINDOW_AUTOSIZE);
	int i = 0;
	while (i < n) {
		read_exr(fnames[i], exr);
		if (!rgb) rgb = cvCreateImage(cvSize(exr.w, exr.h), IPL_DEPTH_32F, 3);

		for (int j = 0; j < exr.h; j++) {
			for (int i = 0; i < exr.w; i++) {
				if (to_srgb) {
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 0] = lin2srgb(exr.b[j][i]);
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 1] = lin2srgb(exr.g[j][i]);
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 2] = lin2srgb(exr.r[j][i]);
				}
				else {
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 0] = exr.b[j][i];
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 1] = exr.g[j][i];
					((float*)rgb->imageData)[i*3 + j * exr.w * 3 + 2] = exr.r[j][i];
				}
			}
		}

		cvShowImage("exrcvview", rgb);
		if ((cvWaitKey(10) & 255) == 27) goto exit;

		if (loop)
			i = (i + 1) % n;
		else
			i++;
	}

	// continue showing the last frame, even when not looping
	for (;;) {
		cvShowImage("exrcvview", rgb);
		if ((cvWaitKey(10) & 255) == 27) break;
	}

exit:
	if (rgb) cvReleaseImage(&rgb);
}


const char*
usage()
{
	return "Usage: exrcvview [options] <filename> [<filename>...]\n"
	       "Options:\n"
	       "  -s    convert input image(s) from linear RGB to sRGB\n"
	       "  -l    infinitely loop over all input images\n"
	       "  -h    show this help\n"
	       "note: exrcvview only supports HALF exr-files at the moment\n";
}


int
main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s", usage());
		return EXIT_FAILURE;
	}

	bool srgb = false;
	bool loop = false;

	int opt;
	while ((opt = getopt(argc, argv, "slh")) != -1) {
		switch (opt) {
		case 's':
			srgb = true;
			break;
		case 'l':
			loop = true;
			break;
		case 'h':
			printf("%s", usage());
			return EXIT_SUCCESS;
		default:
			fprintf(stderr, "%s", usage());
			return EXIT_FAILURE;
		}
	}
	if (optind >= argc) {
		fprintf(stderr, "no input images provided\n");
		fprintf(stderr, "%s", usage());
		return EXIT_FAILURE;
	}

	exrcvview(argc - optind, argv + optind, srgb, loop);
	return EXIT_SUCCESS;
}
