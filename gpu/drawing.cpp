#include <array>
#include <utility>

#include "math.h"
#include "limits"

#include "cxdxxxxx.h"
#include "rasterize.hh"

extern bool g_emulationPaused;
extern int g_mousePossitionX;
extern int g_mousePossitionY;
extern bool g_logPixel;

//TO DO 
//Define option for old and new GPU 

class Slope
{
	float begin, step;
public:
	Slope() {}
	Slope(float from, float to, float num_steps)
	{
		float inv_step = 1.f / num_steps;
		begin = from;                   // Begin here
		step = (to - from) * inv_step; // Stepsize = (end-begin) / num_steps
	}
	float get() const { return begin; }
	void advance(float n = 1) { begin += step * n; }
};

template<typename PlotFunc, typename T, std::size_t Size = std::tuple_size_v<T>>
void DrawPolygonInterpolated(
	T p0,
	T p1,
	T p2,
	PlotFunc&& Plot)
{
	using SlopeData = std::array<Slope, Size - 1>; // All input units except y

	RasterizeTriangle(
		&p0, &p1, &p2,
		// GetXY: Retrieve std::tuple<int,int> or std::array<int,2> from a PointType
		[&](const auto& p) { return std::tuple{ int(std::get<0>(p)), int(std::get<1>(p)) }; },
		// Slope generator
		[&](const T* from, const T* to, int num_steps)
		{
			SlopeData result;
			// Number of steps = number of scanlines
			// Retrieve X coordinates for begin and end
			result[0] = Slope(std::get<0>(*from), std::get<0>(*to), num_steps);
			// Take the rest of the props
			[&] <std::size_t...p>(std::index_sequence<p...>)
			{
				// Retrieve begin to end for each prop. The +2 skips x and y.
				((result[p + 1] = Slope(std::get<p + 2>(*from), std::get<p + 2>(*to), num_steps)), ...);
			}
			(std::make_index_sequence<Size - 2>{});
			return result;
		},
		// Scanline function
		[&](int y, SlopeData& left, SlopeData& right)
		{
			float leftx = left[0].get(), endx = right[0].get();
			auto intleftx = std::floor(leftx + .5f), rounding_error = intleftx - leftx + .5f;

			// Number of steps = number of pixels on this scanline = endx-x
			std::array<Slope, Size - 2> props;
			for (unsigned p = 0; p < Size - 2; ++p)
			{
				props[p] = Slope(left[p + 1].get(), right[p + 1].get(), endx - leftx);
				props[p].advance(rounding_error);
			}

			for (int x = intleftx, e = endx + .5f; x < e; ++x) // Render each pixel
			{
				std::apply([&](auto... args) { Plot(x, y, args.get()...); }, props);

				// After each pixel, update the props by their step-sizes
				for (auto& prop : props) prop.advance();
			}

			// After the scanline is drawn, update the X coordinate and props on both sides
			for (auto& slope : left) slope.advance();
			for (auto& slope : right) slope.advance();
		});
}

template<typename PlotFunc, typename T, std::size_t Size = std::tuple_size_v<T>>
void DrawTinyTriangleInterpolated(
	T p0,
	T p1,
	T p2,
	PlotFunc&& Plot)
{
	using SlopeData = std::array<Slope, Size - 1>; // All input units except y

	RasterizeTriangle(
		&p0, &p1, &p2,
		// GetXY: Retrieve std::tuple<int,int> or std::array<int,2> from a PointType
		[&](const auto& p) { return std::tuple{ int(std::get<0>(p)), int(std::get<1>(p)) }; },
		// Slope generator
		[&](const T* from, const T* to, int num_steps)
		{
			SlopeData result;
			// Number of steps = number of scanlines
			// Retrieve X coordinates for begin and end
			result[0] = Slope(std::get<0>(*from), std::get<0>(*to), num_steps);
			// Take the rest of the props
			[&] <std::size_t...p>(std::index_sequence<p...>)
			{
				// Retrieve begin to end for each prop. The +2 skips x and y.
				((result[p + 1] = Slope(std::get<p + 2>(*from), std::get<p + 2>(*to), num_steps)), ...);
			}
			(std::make_index_sequence<Size - 2>{});
			return result;
		},
		// Scanline function
		[&](int y, SlopeData& left, SlopeData& right)
		{
			float leftx = left[0].get(), endx = right[0].get();
			auto intleftx = std::floor(leftx + .5f), rounding_error = intleftx - leftx + .5f;

			// Number of steps = number of pixels on this scanline = endx-x
			std::array<Slope, Size - 2> props;
			for (unsigned p = 0; p < Size - 2; ++p)
			{
				props[p] = Slope(left[p + 1].get(), right[p + 1].get(), endx - leftx);
				props[p].advance(rounding_error);
			}

			for (int x = intleftx, e = endx + .5f; x <= e; ++x) // Render each pixel
			{
				std::apply([&](auto... args) { Plot(x, y, args.get()...); }, props);

				// After each pixel, update the props by their step-sizes
				for (auto& prop : props) prop.advance();
			}

			// After the scanline is drawn, update the X coordinate and props on both sides
			for (auto& slope : left) slope.advance();
			for (auto& slope : right) slope.advance();
		});
}

void Gpu::CalculateTransparency(const Vector2D& p, PsxColor16& frontPixelColor) {
	PsxColor16 backPixelColor;
	ReadVram(p, backPixelColor.data);
	switch (gpuStat.elem.semiTransparency) {
	case 0: //B/2+F/2
		frontPixelColor.component.red = (backPixelColor.component.red >> 1)
			+ (frontPixelColor.component.red >> 1);
		frontPixelColor.component.green = (backPixelColor.component.green >> 1)
			+ (frontPixelColor.component.green >> 1);
		frontPixelColor.component.blue = (backPixelColor.component.blue >> 1)
			+ (frontPixelColor.component.blue >> 1);
		break;
	case 1: //B+F
		frontPixelColor.component.red = (backPixelColor.component.red
			+ frontPixelColor.component.red) > 0x1f ?
			0x1f : backPixelColor.component.red + frontPixelColor.component.red;
		frontPixelColor.component.green = (backPixelColor.component.green
			+ frontPixelColor.component.green) > 0x1f ?
			0x1f : backPixelColor.component.green + frontPixelColor.component.green;
		frontPixelColor.component.blue = (backPixelColor.component.blue
			+ frontPixelColor.component.blue) > 0x1f ?
			0x1f : backPixelColor.component.blue + frontPixelColor.component.blue;
		break;
	case 2: //B-F
		frontPixelColor.component.red = (backPixelColor.component.red < frontPixelColor.component.red) ?
			0 : backPixelColor.component.red - frontPixelColor.component.red;
		frontPixelColor.component.green = (backPixelColor.component.green < frontPixelColor.component.green) ?
			0 : backPixelColor.component.green - frontPixelColor.component.green;
		frontPixelColor.component.blue = (backPixelColor.component.blue < frontPixelColor.component.blue) ?
			0 : backPixelColor.component.blue - frontPixelColor.component.blue;
		break;
	case 3: //B+F/4
		frontPixelColor.component.red = (backPixelColor.component.red
			+ (frontPixelColor.component.red >> 2)) > 0x1f ?
			0x1f : backPixelColor.component.red + (frontPixelColor.component.red >> 2);
		frontPixelColor.component.green = (backPixelColor.component.green
			+ (frontPixelColor.component.green >> 2)) > 0x1f ?
			0x1f : backPixelColor.component.green + (frontPixelColor.component.green >> 2);
		frontPixelColor.component.blue = (backPixelColor.component.blue
			+ (frontPixelColor.component.blue >> 2)) > 0x1f ?
			0x1f : backPixelColor.component.blue + (frontPixelColor.component.blue >> 2);
		break;
	}
}

void Gpu::DrawPolygon(const Param& param) {
	const uint8_t vertexCount = param.verticesNumber;

	Vector2D vert1 = param.vert1;
	Vector2D vert2 = param.vert2;
	Vector2D vert3 = param.vert3;
	Vector2D vert4 = param.vert4;

	PsxColor24 color1 = param.commandColor1;
	PsxColor24 color2 = param.color2;
	PsxColor24 color3 = param.color3;
	PsxColor24 color4 = param.color4;

	PsxColor16 frontPixelColor;
	PsxColor24 calculatedPixelColor;

	bool bDithering = (param.shadingType == GOURAUD_SHADING ||
		param.modulationType == TEXTURE_BLENDING) &&
		gpuStat.elem.dither24To15;


	auto plot = [&](int16_t x, int16_t y, uint8_t b, uint8_t g, uint8_t r) {
		Vector2D p(x, y);

		if (param.shadingType == GOURAUD_SHADING) {
			calculatedPixelColor.component.red = r;
			calculatedPixelColor.component.blue = b;
			calculatedPixelColor.component.green = g;
		}
		else
			calculatedPixelColor = param.commandColor1;

		frontPixelColor.data = format24to16Color(calculatedPixelColor.data);

		if (param.transpanencyType == SEMI_TRANSPARENT)
			CalculateTransparency(p, frontPixelColor);

		WriteVram(p, frontPixelColor.data, false, bDithering);
		};

	std::tuple p0{ vert1.x, vert1.y, color1.component.blue, color1.component.green, color1.component.red };
	std::tuple p1{ vert2.x, vert2.y, color2.component.blue, color2.component.green, color2.component.red };
	std::tuple p2{ vert3.x, vert3.y, color3.component.blue, color3.component.green, color3.component.red };

	DrawPolygonInterpolated(p0, p1, p2, plot);

	if (vertexCount == 4) {

		std::tuple p2{ vert2.x, vert2.y, color2.component.blue, color2.component.green, color2.component.red };
		std::tuple p3{ vert3.x, vert3.y, color3.component.blue, color3.component.green, color3.component.red };
		std::tuple p4{ vert4.x, vert4.y, color4.component.blue, color4.component.green, color4.component.red };

		DrawPolygonInterpolated(p2, p3, p4, plot);
	}
}

Gpu::TextureBitness Gpu::GetTextureBitness() const {
	switch (gpuStat.elem.texPageColors) {
	case 0: // 4-bit
		return { false, 2, 0x3, 2, 0x0f };

	case 1: // 8-bit
		return { false, 1, 0x1, 3, 0xff };

	case 2: // 15-bit
	case 3: // Reserved
		return { true, 0, 0, 0, 0 };
	}
}

void Gpu::DrawTexPolygon(const Param& param) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	TextureBitness textureBitness = GetTextureBitness();

	clutCache.RebuildCache(this, param.texCoord1Palette.elem.attribute);

	const uint8_t vertexCount = param.verticesNumber;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert2.x;
	int16_t y2 = param.vert2.y;
	int16_t x3 = param.vert3.x;
	int16_t y3 = param.vert3.y;
	int16_t x4 = param.vert4.x;
	int16_t y4 = param.vert4.y;

	PsxColor24 color1 = param.commandColor1;
	PsxColor24 color2 = param.color2;
	PsxColor24 color3 = param.color3;
	PsxColor24 color4 = param.color4;

	uint8_t u1 = param.texCoord1Palette.elem.u;
	uint8_t v1 = param.texCoord1Palette.elem.v;
	uint8_t u2 = param.texCoord2TexPage.elem.u;
	uint8_t v2 = param.texCoord2TexPage.elem.v;
	uint8_t u3 = param.texCoord3.elem.u;
	uint8_t v3 = param.texCoord3.elem.v;
	uint8_t u4 = param.texCoord4.elem.u;
	uint8_t v4 = param.texCoord4.elem.v;

	PsxColor24 blendingColor;
	PsxColor16 frontPixelColor;

	bool bDithering = (param.shadingType == GOURAUD_SHADING ||
		param.modulationType == TEXTURE_BLENDING) &&
		gpuStat.elem.dither24To15;

	auto plot = [&](int16_t x, int16_t y, uint8_t b, uint8_t g, uint8_t r, uint8_t u, uint8_t v) {

		Vector2D point(x, y);

		Vector2D pointTex;

		pointTex.y = gpuStat.elem.texPageYBase << 8;
		pointTex.y += (v & 255 & (~(texWinMask.y << 3)))
			| ((texWinOffset.y & texWinMask.y) << 3);

		pointTex.x = gpuStat.elem.texPageXBase << 6;
		pointTex.x += (((u & 255 & (~((texWinMask.x << 3))))
			| ((texWinOffset.x & texWinMask.x) << 3)))
			>> textureBitness.shiftRight;

		uint16_t data;
		ReadVram(pointTex, data, true);

		if (textureBitness.is15BitTexture)
			frontPixelColor.data = data;
		else
			frontPixelColor.data = clutCache.GetColor(
				(data >> ((u & textureBitness.firstAnd) << textureBitness.shiftLeft))
				& textureBitness.secondAnd
			);

		if (frontPixelColor.data) {
			if (param.shadingType == GOURAUD_SHADING) {
				blendingColor.component.red = r;
				blendingColor.component.blue = b;
				blendingColor.component.green = g;
			}
			else
				blendingColor = param.commandColor1;

			if (param.modulationType == TEXTURE_BLENDING) {
				//Old 160pin GPU v0
				/*uint32_t red = (blendingColor.component.red >> 3) * frontPixelColor.component.red >> 4;
				uint32_t green = (blendingColor.component.green >> 3) * frontPixelColor.component.green >> 4;
				uint32_t blue = (blendingColor.component.blue >> 3) * frontPixelColor.component.blue >> 4;*/

				//New 208pin GPU v2
				uint32_t red = blendingColor.component.red * frontPixelColor.component.red >> 7;
				uint32_t green = blendingColor.component.green * frontPixelColor.component.green >> 7;
				uint32_t blue = blendingColor.component.blue * frontPixelColor.component.blue >> 7;

				frontPixelColor.component.red = red > 0x1f ? 0x1f : red;
				frontPixelColor.component.green = green > 0x1f ? 0x1f : green;
				frontPixelColor.component.blue = blue > 0x1f ? 0x1f : blue;
			}

			if ((param.transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
				CalculateTransparency(point, frontPixelColor);

			WriteVram(point, frontPixelColor.data, false, bDithering);
		}
		};

	std::tuple p0{ x1, y1, color1.component.blue, color1.component.green, color1.component.red, u1, v1 };
	std::tuple p1{ x2, y2, color2.component.blue, color2.component.green, color2.component.red, u2, v2 };
	std::tuple p2{ x3, y3, color3.component.blue, color3.component.green, color3.component.red, u3, v3 };

	DrawPolygonInterpolated(p0, p1, p2, plot);

	//for Doom
	//DrawTinyTriangleInterpolated(p0, p1, p2, plot);


	if (vertexCount == 4) {

		std::tuple p2{ x2, y2, color2.component.blue, color2.component.green, color2.component.red, u2, v2 };
		std::tuple p3{ x3, y3, color3.component.blue, color3.component.green, color3.component.red, u3, v3 };
		std::tuple p4{ x4, y4, color4.component.blue, color4.component.green, color4.component.red, u4, v4 };

		DrawPolygonInterpolated(p2, p3, p4, plot);

		//for Doom
		//DrawTinyTriangleInterpolated(p2, p3, p4, plot);
	}
}

void Gpu::DrawMonochromeRect(const Param& param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.sizeYX & 0xffff);
	int16_t y2 = y1 + ((param.sizeYX >> 16) & 0xffff);

	for (int16_t y = y1; y < y2; y++)
		for (int16_t x = x1; x < x2; x++) {
			Vector2D p(x, y);

			PsxColor16 frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				CalculateTransparency(p, frontPixelColor);

			WriteVram(p, frontPixelColor.data);
		}
}

void Gpu::DrawLine(const Param& param) { //could be optimized //too much math // TO DO Rewrite

	PsxColor24 color1 = param.commandColor1;
	int x1 = param.vert1.x;
	int y1 = param.vert1.y;

	PsxColor24 color2 = param.color2;
	int x2 = param.vert2.x;
	int y2 = param.vert2.y;

	PsxColor16 frontPixelColor;

	bool bDithering = gpuStat.elem.dither24To15;


	if (y2 < y1) {
		std::swap(y1, y2);
		std::swap(x1, x2);
		//std::swap(color1.data, color2.data);???
	}

	int dy1 = y2 - y1;
	int dx1 = x2 - x1;

	float k = 0;

	if (dy1) k = dx1 / (float)abs(dy1);
	else k = std::numeric_limits<float>::max();

	if ((k >= 1.0f) || (k <= -1.0f)) {
		int ax = x1;
		int bx = x2;

		if (bx < ax)
			std::swap(ax, bx);

		for (int i = ax; i <= bx; i++) {
			Vector2D p(i, round(y1 + (float)(i - x1) * 1.0f / k));

			if (param.shadingType == GOURAUD_SHADING) {
				float lamdaAbs = sqrt(pow((param.vert1.x - param.vert2.x), 2)
					+ pow((param.vert1.y - param.vert2.y), 2));
				float lamdaPointAbs = sqrt(pow((param.vert1.x - p.x), 2)
					+ pow((param.vert1.y - p.y), 2));
				float lamda1 = 1 - lamdaPointAbs / lamdaAbs;
				float lamda2 = 1 - lamda1;

				PsxColor24 calculatedColor;

				calculatedColor.component.red = (((float)color1.component.red * lamda1
					+ (float)color2.component.red * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.red * lamda1 + (float)color2.component.red * lamda2;
				calculatedColor.component.green = (((float)color1.component.green * lamda1
					+ (float)color2.component.green * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.green * lamda1 + (float)color2.component.green * lamda2;
				calculatedColor.component.blue = (((float)color1.component.blue * lamda1
					+ (float)color2.component.blue * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2;

				frontPixelColor = format24to16Color(calculatedColor.data);
			}

			if (param.shadingType == FLAT_SHADING)
				frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				CalculateTransparency(p, frontPixelColor);

			WriteVram(p, frontPixelColor.data, false, bDithering);
		}
	}
	else if ((k < 1.0f) && (k > -1.0f)) {
		for (int i = y1; i <= y2; i++) {
			Vector2D p(round(x1 + (float)(i - y1) * k), i);

			if (param.shadingType == GOURAUD_SHADING) {
				float lamdaAbs = sqrt(pow((param.vert1.x - param.vert2.x), 2)
					+ pow((param.vert1.y - param.vert2.y), 2));
				float lamdaPointAbs = sqrt(pow((param.vert1.x - p.x), 2)
					+ pow((param.vert1.y - p.y), 2));
				float lamda1 = 1 - lamdaPointAbs / lamdaAbs;
				float lamda2 = 1 - lamda1;

				PsxColor24 calculatedColor;

				calculatedColor.component.red = (((float)color1.component.red * lamda1
					+ (float)color2.component.red * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.red * lamda1 + (float)color2.component.red * lamda2;
				calculatedColor.component.green = (((float)color1.component.green * lamda1
					+ (float)color2.component.green * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.green * lamda1 + (float)color2.component.green * lamda2;
				calculatedColor.component.blue = (((float)color1.component.blue * lamda1
					+ (float)color2.component.blue * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2;

				frontPixelColor = format24to16Color(calculatedColor.data);
			}

			if (param.shadingType == FLAT_SHADING)
				frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				CalculateTransparency(p, frontPixelColor);

			WriteVram(p, frontPixelColor.data, false, bDithering);
		}
	}
}

void Gpu::DrawTexRect(const Param& param) {

	TextureBitness textureBitness = GetTextureBitness();

	clutCache.RebuildCache(this, param.texCoord1Palette.elem.attribute);

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.sizeYX & 0xffff);
	int16_t y2 = y1 + ((param.sizeYX >> 16) & 0xffff);

	PsxColor24 blendingColor = param.commandColor1;
	int16_t u1 = param.texCoord1Palette.elem.u;
	int16_t v1 = param.texCoord1Palette.elem.v;

	PsxColor16 frontPixelColor;

	Vector2D point;

	Vector2D pointTex;

	for (int16_t y = 0; y < y2 - y1; y++) {

		pointTex.y = gpuStat.elem.texPageYBase << 8;
		pointTex.y += ((v1 + y) & 255 & (~(texWinMask.y << 3)))
			| ((texWinOffset.y & texWinMask.y) << 3);

		point.y = y1 + y;

		for (int16_t x = 0; x < x2 - x1; x++) {

			pointTex.x = gpuStat.elem.texPageXBase << 6;
			//TO check
			pointTex.x += (((((u1 >> textureBitness.shiftRight)
				+ ((x + (u1 & textureBitness.firstAnd)) >> textureBitness.shiftRight))
				& (255 >> textureBitness.shiftRight)
				& (~((texWinMask.x << (3 - textureBitness.shiftRight)))))
				| ((texWinOffset.x & texWinMask.x) << (3 - textureBitness.shiftRight))));

			point.x = x1 + x;

			uint16_t texData;
			ReadVram(pointTex, texData, true);
			if (textureBitness.is15BitTexture)
				frontPixelColor.data = texData;
			else
				frontPixelColor.data = clutCache.GetColor(
					texData >> (((x + (u1 & textureBitness.firstAnd))
					& textureBitness.firstAnd) << textureBitness.shiftLeft)
					& textureBitness.secondAnd);

			if (frontPixelColor.data) {
				if (param.modulationType == TEXTURE_BLENDING) { //?? Same as for Tex Polygons
					//Old 160pin GPU v0
					/*uint32_t red = (blendingColor.component.red >> 3) * frontPixelColor.component.red >> 4;
					uint32_t green = (blendingColor.component.green >> 3) * frontPixelColor.component.green >> 4;
					uint32_t blue = (blendingColor.component.blue >> 3) * frontPixelColor.component.blue >> 4;*/

					//New 208pin GPU v2
					uint32_t red = blendingColor.component.red * frontPixelColor.component.red >> 7;
					uint32_t green = blendingColor.component.green * frontPixelColor.component.green >> 7;
					uint32_t blue = blendingColor.component.blue * frontPixelColor.component.blue >> 7;

					frontPixelColor.component.red = red > 0x1f ? 0x1f : red;
					frontPixelColor.component.green = green > 0x1f ? 0x1f : green;
					frontPixelColor.component.blue = blue > 0x1f ? 0x1f : blue;
				}

				if ((param.transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
					CalculateTransparency(point, frontPixelColor);

				WriteVram(point, frontPixelColor.data);
			}
		}
	}
}

void Gpu::CopyRectCpuToVram(const uint32_t& dest_coord, const uint32_t& width_height) {
	const uint32_t x1 = (dest_coord & 0xffff) & 0x3ff;
	uint32_t x2 = x1 + (width_height & 0xffff);
	const uint32_t y1 = (dest_coord >> 16) & 0x1ff;
	uint32_t y2 = y1 + (width_height >> 16);

	uint32_t i = 0;
	uint32_t data;
	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			if ((i & 0x1) == 0)
				data = fifo.GetNext();

			Vector2D position(x, y);
			WriteVram(position, ((i & 0x1) == 0) ? (uint16_t)data : uint16_t(data >> 16), true);

			i++;
		}
	}
}

void Gpu::CopyRectVramToCpu(const uint32_t& source_coord, const uint32_t& width_height) {
	const uint16_t x1 = source_coord & 0xffff;
	const uint16_t x2 = x1 + (width_height & 0xffff);
	const uint16_t y1 = source_coord >> 16;
	const uint16_t y2 = y1 + (width_height >> 16);

	uint16_t data16;
	uint32_t data32;
	uint32_t i = 0;
	for (uint16_t y = y1; y < y2; y++) {
		for (uint16_t x = x1; x < x2; x++) {
			Vector2D position(x, y);
			ReadVram(position, data16, true);

			if ((i & 0x1) == 0)
				data32 = 0x00000000 | (uint32_t)data16;
			else {
				data32 |= (uint32_t)data16 << 16;
				outputFifo.PushBack(data32);
			}
			i++;
		}
	}
}

void Gpu::CopyRectVramToVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight) {
	const uint16_t sourceX1 = sourceCoord & 0xffff;
	const uint16_t sourceY1 = sourceCoord >> 16;

	const uint16_t destX1 = destCoord & 0xffff;
	const uint16_t destY1 = destCoord >> 16;

	const uint16_t width = widthHeight & 0xffff;
	const uint16_t height = widthHeight >> 16;


	uint16_t data;

	uint16_t x;
	uint16_t y = 0;
	do { //Why is here do loop used insted of for loop? 
		x = 0;
		do {
			Vector2D sourcePosition(sourceX1 + x, sourceY1 + y);
			ReadVram(sourcePosition, data, true);
			Vector2D destPosition(destX1 + x, destY1 + y);
			WriteVram(destPosition, data, true);
			x++;
		} while (x < width);
		y++;
	} while (y < height);
}

void Gpu::FillRectVram(const Param& param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert4.x;
	int16_t y2 = param.vert4.y;

	uint16_t data = format24to16Color(param.commandColor1);

	uint32_t oldSetMaskBitWhileDrawing = gpuStat.elem.setMaskBitWhileDrawing;
	gpuStat.elem.setMaskBitWhileDrawing = 0;
	uint32_t oldDrawPixels = gpuStat.elem.drawPixels;
	gpuStat.elem.drawPixels = 0;

	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			Vector2D p(x, y);
			WriteVram(p, data, true);
		}
	}

	gpuStat.elem.setMaskBitWhileDrawing = oldSetMaskBitWhileDrawing;
	gpuStat.elem.drawPixels = oldDrawPixels;
}