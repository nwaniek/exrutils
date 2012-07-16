#ifndef __IMFUTIL_HPP__A90FE2F0_8ADE_4816_BB6F_3A1164BC160B
#define __IMFUTIL_HPP__A90FE2F0_8ADE_4816_BB6F_3A1164BC160B

#include <OpenEXR/half.h>
#include <OpenEXR/ImfPixelType.h>

template <typename T>
struct imf_ttrait
{
	typedef T type;
	static constexpr Imf::PixelType PIXEL_TYPE = Imf::HALF;
};

template <>
struct imf_ttrait<half>
{
	typedef half type;
	static constexpr Imf::PixelType PIXEL_TYPE = Imf::HALF;
};

template <>
struct imf_ttrait<float>
{
	typedef float type;
	static constexpr Imf::PixelType PIXEL_TYPE = Imf::FLOAT;
};


template <typename T>
T lin2srgb(T x)
{
	if (x < 0.0) return 0.0;
	if (x > 1.0) return 1.0;

	if (x < 0.0031308)
		return x * 12.92;
	return 1.055 * powf(x, 1.0 / 2.4) - 0.055;
}

#endif /* __IMFUTIL_HPP__A90FE2F0_8ADE_4816_BB6F_3A1164BC160B */

