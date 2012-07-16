#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <unistd.h>

#include <algorithm>

#include <OpenEXR/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfPixelType.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfFrameBuffer.h>

#include "math.hpp"


using namespace Imf;
using namespace Imath;

typedef struct {
	float f;	// focal length
	int n;
	char *csvname;
	char **exrfiles;
	bool longuet;
} config_t;

typedef struct {
	Array2D<float> z;
	FrameBuffer fb;
	int w, h;
} exr_t;

static const char* usage =
"Usage: exrflow [options] <csv> <exr> [<exr> ...]\n"
"csv contains the camera position for each exr file per row in the\n"
"format a11,a21,a31,a41,a12,...,a44 as exported with the script\n"
"camlocpos for blender\n"
"Options: \n"
"  -h           Show this help\n"
"  -l           Use Longuet-Higgins' coordinate system\n"
"  -f <value>   Set the focal length to <value> (default 2.1875)\n"
"The output is stored as Middleburry FLO files, names starting at 00001.flo.\n"
"For more information, see http://github.com/rochus/exrutils\n";

int
parse_args(config_t &config, int &argc, char **&argv)
{
	config.n = 0;
	config.csvname = nullptr;
	config.exrfiles = nullptr;
	config.f = 2.1875;
	config.longuet = false;

	int opt;
	while ((opt = getopt(argc, argv, "hlf:")) != -1) {
		switch (opt) {
		case 'f':
			config.f = atof(optarg);
			break;
		case 'l':
			config.longuet = true;
			break;
		case 'h':
			return 1;
		default:
			return 1;
		}
	}
	config.n = argc - (optind + 1);
	if (config.n <= 0) {
		fprintf(stderr, "insufficient arguments\n");
		return 1;
	}
	config.csvname = argv[optind];
	config.exrfiles = argv + optind + 1;
	return 0;
}

void
exr_read_z(char *name, exr_t &exr)
{
	InputFile file(name);
	Box2i dw = file.header().dataWindow();

	exr.w = dw.max.x - dw.min.x + 1;
	exr.h = dw.max.y - dw.min.y + 1;
	exr.z.resizeErase(exr.h, exr.w);

	exr.fb.insert("Z",
		Slice(FLOAT, (char*)(&exr.z[0][0] - dw.min.x - dw.min.y * exr.w),
			sizeof(exr.z[0][0]), sizeof(exr.z[0][0]) * exr.w, 1, 1, FLT_MAX));

	file.setFrameBuffer(exr.fb);
	file.readPixels(dw.min.y, dw.max.y);
}

// TODO: find real ZBUF_MAX value. this one is just a random value < what's
// really in there
#define ZBUF_MAX 10000000.0
void
compute_flow(char *fname, exr_t *exr, vec3 t, vec3 r, config_t &config)
{
	float x, y, z, u, v;
	// blender has a sensor size of 32mm (in the greater direction. assume
	// width >= height) and a default focal length of 35mm. the horizontal
	// opening angle is then
	//
	//	alpha = atan2(16.0, 35.0) * 2                           (1)
	//
	// to get the focal length in a canonical camera model, where the image
	// plane has an extent of 2.0 (from -1.0 to 1.0) in the largest extent,
	// we have to calculate:
	//
	//      f = b / tan(a/2)                                        (2)
	//
	// where b = 1.0 (because it is half the image plane width). but as we
	// have the 'lens focal length' and the sensor size, we can calculate:
	//
	//      tan(a/2) = (.5 * sensor size) / lens focal length       (3)
	//
	// insert (3) into (2) yields
	//
	//	f = 1.0 / ((.5s) / l)                                   (4)
	//
	// where s is the sensor size and l the lens focal length.
	// Setting s = 32mm and l = 35mm as in the default blender setup yields
	//
	//      f = 2.1875
	//
	FILE *flo = fopen(fname, "wb");
	fprintf(flo, "PIEH");
	fwrite(&(exr->w), sizeof(int), 1, flo);
	fwrite(&(exr->h), sizeof(int), 1, flo);

	float ratio = (float)exr->w / (float)exr->h;
	float pixel_size = 2.0 / (float)exr->w;

	y = 1.0 / ratio;
	const float f = config.f;
	for (int j = 0; j < exr->h; j++, y -= pixel_size) {

		x = -1.0;
		for (int i = 0; i < exr->w; i++, x += pixel_size)  {

			u = 10e9;
			v = 10e9;
			z = exr->z[j][i];

			if (z > 0.0 && z < ZBUF_MAX && z != FLT_MAX) {

				float d = sqrt(f * f + x * x + y * y);
				// z coming from exr file is positive!
				z = (z / d) * f;
				if (config.longuet) {
					// original Longuet-Higgins, Prazdny (plus focal
					// length)

					u = (1.0 / z) * (- t.x * f + t.z * x);
					v = (1.0 / z) * (- t.y * f + t.z * y);

					u += (1.0 / f) * (x * y * r.x - (f * f + x * x) * r.y + f * y * r.z);
					v += (1.0 / f) * ((f * f + y * y) * r.x - x * y * r.y - f * x * r.z);
				}
				else {
					// formulas adapted to reflect that z has to be
					// negative!
					z *= -1.0; // z coming from exr file is positive, map to negative!

					u = (1.0 / z) * (t.x * f + t.z * x);
					v = (1.0 / z) * (t.y * f + t.z * y);

					u += (1.0 / f) * (- x * y * r.x + (f * f + x * x) * r.y + f * y * r.z);
					v += (1.0 / f) * (- (f * f + y * y) * r.x + x * y * r.y - f * x * r.z);
				}
				// rescale to pixel size
				u /= pixel_size;
				v /= pixel_size;

				// make Middleburry flow
				v *= -1.0;
			}
			fwrite(&u, sizeof(float), 1, flo);
			fwrite(&v, sizeof(float), 1, flo);
		}
	}
	fclose(flo);
}
#undef ZBUF_MAX

void
scan_line(FILE *f, int *fn, mat4x4 *m)
{
	// the script writes column vectors! so, the first 4 floats are a11 to
	// a41, the next are a12 to a42 and so on...
	fscanf(f, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", fn,
			&m->a11, &m->a21, &m->a31, &m->a41,
			&m->a12, &m->a22, &m->a32, &m->a42,
			&m->a13, &m->a23, &m->a33, &m->a43,
			&m->a14, &m->a24, &m->a34, &m->a44);
}

void
process(config_t &config)
{
	exr_t exr;
	FILE *csv = fopen(config.csvname, "r");

	int fn;
	mat4x4 m0, m1;
	mat3x3 r0, r1, rot, r0_transp;
	vec3 t0, t1, trans, eul;

	scan_line(csv, &fn, &m0);
	for (int i = 1; i < config.n; i++) {
		fprintf(stderr, "processing %4d/%4d\r", i, config.n - 1); fflush(stderr);

		// read the data
		scan_line(csv, &fn, &m1);
		exr_read_z(config.exrfiles[i-1], exr);

		// extract the rotational information of the matrix and
		// transform it to camera coordinates
		r0 = extract_rot(&m0);
		r1 = extract_rot(&m1);
		r0_transp = matrix_transpose(&r0);
		rot = mul(&r1, &r0_transp);
		eul = mat3x3_to_euler(&rot);
		eul = mul(&r0_transp, &eul);
		//printf("rotation\n");
		//print_vec3(&eul);

		// extract the translationl information and transform it to
		// camera coordinates
		t0 = extract_trans(&m0);
		t1 = extract_trans(&m1);
		trans = sub(&t1, &t0);
		trans = mul(&r0_transp, &trans);
		//printf("tran_slation\n");
		//print_vec3(&trans);

		// generate file name and compute the flow
		char fbuf[256];
		snprintf(fbuf, 256, "%05d.flo", i);
		compute_flow(fbuf, &exr, trans, eul, config);

		//printf("\n");
		std::swap(m0, m1);
	}
	fclose(csv);
}

int
main(int argc, char **argv)
{
	config_t config;
	if (parse_args(config, argc, argv)) {
		fprintf(stderr, "%s", usage);
		return EXIT_FAILURE;
	}

	process(config);
	return EXIT_SUCCESS;
}
