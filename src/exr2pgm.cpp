#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <imfutil.hpp>
#include <OpenEXR/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfPixelType.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfFrameBuffer.h>


using namespace Imf;
using namespace Imath;

typedef struct {
	const char *name;
} PixelTypeName;

static PixelTypeName ptmap[] = {
	{"UINT"},
	{"HALF"},
	{"FLOAT"}
};

bool
fexists(const char *name)
{
	FILE *f;
	if ((f = fopen(name, "r"))) {
		fclose(f);
		return true;
	}
	return false;
}

template <typename T>
void process (InputFile &file, const char *oname)
{
	typedef typename imf_ttrait<T>::type type;
	constexpr PixelType ptype = imf_ttrait<T>::PIXEL_TYPE;
	Array2D<type> r, g, b;

	FrameBuffer fb;
	Box2i dw = file.header().dataWindow();

	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;

	r.resizeErase(h, w);
	g.resizeErase(h, w);
	b.resizeErase(h, w);

	fb.insert("R",
		Slice(ptype, (char*)(&r[0][0] - dw.min.x - dw.min.y * w),
			sizeof(r[0][0]) * 1, sizeof(r[0][0]) * w, 1, 1, 0.0));
	fb.insert("G",
		Slice(ptype, (char*)(&g[0][0] - dw.min.x - dw.min.y * w),
			sizeof(g[0][0]) * 1, sizeof(g[0][0]) * w, 1, 1, 0.0));
	fb.insert("B",
		Slice(ptype, (char*)(&b[0][0] - dw.min.x - dw.min.y * w),
			sizeof(b[0][0]) * 1, sizeof(b[0][0]) * w, 1, 1, 0.0));

	file.setFrameBuffer(fb);
	file.readPixels(dw.min.y, dw.max.y);

	FILE *pgm = fopen(oname, "wb");
	float bw;

	fprintf(pgm, "P5\n");
	fprintf(pgm, "%d %d\n", w, h);
	fprintf(pgm, "255\n");

	// convert to gray scale
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			bw = 0.30 * lin2srgb(r[j][i])
			   + 0.58 * lin2srgb(g[j][i])
			   + 0.12 * lin2srgb(b[j][i]);
			fputc((int)(bw * 255.0 + 0.5), pgm);
		}
	}
	fclose(pgm);
}

void
exr2pgm(const char *iname, const char *oname)
{
	InputFile file(iname);

	// peak into the R channel to find the image's PixelType
	PixelType ptype = file.header().channels().findChannel("R")->type;
	switch (ptype) {
	case FLOAT:
		process<float>(file, oname);
		break;
	case HALF:
		process<half>(file, oname);
		break;
	default:
		fprintf(stderr, "exr2pgm: unhandled pixel type %s\n", ptmap[ptype].name);
	}
}

int
main (int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "usage: exr2pgm <inputfile> <outputfile>\n");
		return EXIT_FAILURE;
	}
	if (!fexists(argv[1])) {
		fprintf(stderr, "exr2pgm: file '%s' not found\n", argv[1]);
		return EXIT_FAILURE;
	}

	exr2pgm(argv[1], argv[2]);
	return EXIT_SUCCESS;
}
