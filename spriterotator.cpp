#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define __STDC_LIB_EXT1__ // sprintf -> sprintf_n
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "gif.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

bool optOutputGif = false;
bool optOutputIntermediaries = false;
bool optOutputSafeFrame = false;
bool optOutputFrames = false;
bool optRound = true;

void shear_x(double xshear, const unsigned int* src, unsigned int* dst, int xsize, int ysize)
{
	double rr = optRound ? 0.5 : 0;
	for (int i = 0; i < ysize; i++)
	{
		for (int j = 0; j < xsize; j++)
		{
			int p = (int)floor(rr + j + (i - (ysize / 2.0)) * xshear);
			if (p >= 0 && p < xsize)
			{
				dst[i * xsize + p] = src[i * xsize + j];
			}
		}
	}
}

void shear_y(double yshear, const unsigned int* src, unsigned int* dst, int xsize, int ysize)
{
	double rr = optRound ? 0.5 : 0;

	for (int j = 0; j < xsize; j++)
	{
		for (int i = 0; i < ysize; i++)
		{
			int p = (int)floor(rr + i + (j - (xsize / 2.0)) * yshear);
			if (p >= 0 && p < ysize)
			{
				dst[p * xsize + j] = src[i * xsize + j];
			}
		}
	}
}


int main(int parc, char** pars)
{
	if (parc < 3)
	{
		printf(
			"Usage:\n"
			"  %s [options] inputfile steps\n"
			"\n"
			"Options:\n"
			"-g output preview animation .gif\n"
			"-i output intermediaries (for debug)\n"
			"-s output safe frame template\n"
			"-f output frames\n"
			"-t truncate instead of rounding\n"
			"\n"
			"Example:\n"
			"  %s -g -f ship.png 32\n"
			"Generates 32 steps of rotating ship.png and preview gif\n", pars[0], pars[0]);
		return 0;
	}

	char* filename = 0;
	int steps = 0;

	for (int i = 1; i < parc; i++)
	{
		if (pars[i][0] != '-')
		{
			// assume first non-flag to be filename and the
			// next to be steps.
			if (!filename)
				filename = pars[i];
			else
				steps = atoi(pars[i]);
		}
		else
		{
			switch (pars[i][1])
			{
			case 'g': optOutputGif = true; break;
			case 'i': optOutputIntermediaries = true; break;
			case 's': optOutputSafeFrame = true; break;
			case 'f': optOutputFrames = true; break;
			case 't': optRound = false; break;
			default:
				printf("Error: unknown flag: %s\n", pars[i]);
				return 0;
			}
		}
	}

	if (!optOutputGif &&
		!optOutputIntermediaries &&
		!optOutputSafeFrame &&
		!optOutputFrames)
	{
		printf("Error: no output selected, use one or more of the following options: -g -i -f -s\n");
		return 0;
	}

	if (filename == 0)
	{
		printf("Error: no input filename provided\n");
		return 0;
	}

	if (steps <= 0)
	{
		printf("Error: Steps may not be 0 or under\n");
		return 0;
	}

	int x, y, comp;
	stbi_uc * img = stbi_load(filename, &x, &y, &comp, 4);
	if (img == 0)
	{
		printf("Failed to load \"%s\"\n", filename);
		return 1;
	}

	printf("Loaded %s (%d x %d), %d steps\n", filename, x, y, steps);
	
	int px = x;
	int py = y;
	if (x != y)
	{
		if (px > py) py = px;
		if (py > px) px = py;
		printf("Note: input non-square, output will be %d x %d\n", px, py);
	}

	int ox = px * 2;
	int oy = py * 2;

	unsigned int* base = new unsigned int[ox * oy] {};
	unsigned int* flip = new unsigned int[ox * oy] {};
	unsigned int* fb1 = new unsigned int[ox * oy] {};
	unsigned int* fb2 = new unsigned int[ox * oy] {};
	unsigned int* fb3 = new unsigned int[ox * oy] {};
	unsigned int* fb = new unsigned int[px * py] {};

	if (optOutputSafeFrame)
	{
		for (int i = 0; i < py; i++)
		{
			for (int j = 0; j < px; j++)
			{
				base[(i + py / 2) * ox + j + px / 2] = 0xffffffff;
			}
		}
		for (int step = 0; step < steps / 4 + 1; step++)
		{
			double angle = step * (M_PI * 2 / steps);
			while (angle >= M_PI / 2) { angle -= M_PI; }
			double xshear = -tan(angle / 2);
			double yshear = sin(angle);

			shear_x(xshear, base, fb1, ox, oy);
			shear_y(yshear, fb1, fb2, ox, oy);
			shear_x(xshear, fb2, fb3, ox, oy);

			// mask out pixels outside output region
			for (int i = 0; i < oy; i++)
			{
				for (int j = 0; j < ox; j++)
				{
					if (!(j >= px / 2 &&
						  j < px / 2 + px &&
						  i >= py / 2 &&
						  i < py / 2 + py))
					fb3[i * ox + j] = 0;
				}
			}
			// undo rotation
			shear_x(-xshear, fb3, fb2, ox, oy);
			shear_y(-yshear, fb2, fb1, ox, oy);
			shear_x(-xshear, fb1, base, ox, oy);
		}

		for (int i = 0; i < py; i++)
		{
			for (int j = 0; j < px; j++)
			{
				fb[i * px + j] = fb3[(i + py / 2) * ox + j + px / 2] | 0xff000000;
			}
		}

		char temp[256];
		snprintf(temp, 256, "%s_safe.png", filename);
		stbi_write_png(temp, px, py, 4, fb, px * 4);
		printf("Wrote %s\n", temp);

		if (!optOutputGif &&
			!optOutputIntermediaries &&
			!optOutputFrames)
		{
			// No other outputs, we're done
			return 0;
		}
	}

	unsigned int bg = *reinterpret_cast<unsigned int*>(img);
	printf("Using top-left corner pixel as \"background\" (0x%04x)\n", bg);

	for (int i = 0; i < ox * oy; i++)
	{
		base[i] = bg;
		flip[i] = bg;
	}

	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			base[(i + (oy - y) / 2) * ox + j + (ox - x) / 2] = *(reinterpret_cast<unsigned int*>(img) + i * x + j);
			flip[(i + (oy - y) / 2) * ox + j + (ox - x) / 2] = *(reinterpret_cast<unsigned int*>(img) + (y - 1 - i) * x + (x - 1 - j));
		}
	}

	stbi_image_free(img);

	GifWriter gif;

	if (optOutputGif)
	{
		char temp[256];
		snprintf(temp, 256, "%s_anim.gif", filename);
		printf("Writing animation preview to %s\n", temp);

		GifBegin(&gif, temp, px, py, 1);
	}


	for (int step = 0; step < steps; step++)
	{
		int fl = 0;
		double angle = step * (M_PI * 2 / steps);
		while (angle >= M_PI / 2) { fl = !fl; angle -= M_PI; }
		double xshear =-tan(angle / 2);
		double yshear =sin(angle);
		//printf("Step %d: xshear %d, yshear %d, flip %d. ", step, xshear, yshear, fl);
		for (int i = 0; i < ox * oy; i++)
		{
			fb1[i] = bg;
			fb2[i] = bg;
			fb3[i] = bg;
		}
		unsigned int* src = base;
		if (fl) src = flip;

		shear_x(xshear, src, fb1, ox, oy);
		shear_y(yshear, fb1, fb2, ox, oy);
		shear_x(xshear, fb2, fb3, ox, oy);

		for (int i = 0; i < py; i++)
		{
			for (int j = 0; j < px; j++)
			{
				fb[i * px + j] = fb3[(i + py / 2) * ox + j + px / 2];
			}
		}

		if (optOutputFrames)
		{
			char temp[256];
			snprintf(temp, 256, "%s_%03d.png", filename, step);
			stbi_write_png(temp, px, py, 4, fb, px * 4);
			printf("Wrote %s\n", temp);
		}

		if (optOutputGif)
		{
			GifWriteFrame(&gif, reinterpret_cast<uint8_t*>(fb), px, py, 1);
		}
		
		if (optOutputIntermediaries)
		{
			char temp[256];
			snprintf(temp, 256, "%s_1_%03d.png", filename, step);
			stbi_write_png(temp, ox, oy, 4, fb1, ox * 4);
			printf("Wrote %s\n", temp);
			snprintf(temp, 256, "%s_2_%03d.png", filename, step);
			stbi_write_png(temp, ox, oy, 4, fb2, ox * 4);
			printf("Wrote %s\n", temp);
		}
	}

	if (optOutputGif)
	{
		GifEnd(&gif);
	}

	return 0;
}
