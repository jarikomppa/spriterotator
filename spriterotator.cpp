#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define __STDC_LIB_EXT1__ // sprintf -> sprintf_n
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "gif.h"

int main(int parc, char** pars)
{
	if (parc < 3)
	{
		printf("Usage:\n%s inputfile steps\nexample:\nship.png 32\n(generates 32 steps of rotating ship.png)\n", pars[0]);
		return 0;
	}
	int x, y, comp;
	stbi_uc * src = stbi_load(pars[1], &x, &y, &comp, 4);
	if (src == 0)
	{
		printf("Failed to load \"%s\"\n", pars[1]);
		return 1;
	}
	int steps = atoi(pars[2]);
	printf("Loaded %s (%d x %d), %d steps\n", pars[1], x, y, steps);
	int px = x;
	int py = y;
	if (x != y)
	{
		if (px > py) py = px;
		if (py > px) px = py;
		printf("Note: input non-square, output will be %d x %d\n", px, py);
	}
	unsigned int bg = *((unsigned int*)src);
	printf("Note: using top-left corner pixel as \"background\" (0x%04x)\n", bg);

	int ox = px * 2;
	int oy = py * 2;

	unsigned int* base = new unsigned int[ox * oy];
	unsigned int* flip = new unsigned int[ox * oy];
	unsigned int* fb1 = new unsigned int[ox * oy];
	unsigned int* fb2 = new unsigned int[ox * oy];
	unsigned int* fb3 = new unsigned int[ox * oy];
	unsigned int* fb = new unsigned int[px * py];

	for (int i = 0; i < ox * oy; i++)
	{
		base[i] = bg;
		flip[i] = bg;
	}

	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			base[(i + (oy - y) / 2) * ox + j + (ox - x) / 2] = *((unsigned int*)src + i * x + j);
			flip[(i + (oy - y) / 2) * ox + j + (ox - x) / 2] = *((unsigned int*)src + (y - 1 - i) * x + (x - 1 - j));
		}
	}

	stbi_image_free(src);

	char temp[256];
	snprintf(temp, 256, "%s_anim.gif", pars[1]);
	printf("Writing animation preview to %s\n", temp);

	GifWriter gif;
	GifBegin(&gif, temp, px, py, 1);

	for (int step = 0; step < steps; step++)
	{
		int fl = 0;
		double angle = step * (3.142 * 2 / steps);
		while (angle >= 3.142 / 2) { fl = !fl; angle -= 3.142; }
		int xshear = (int)floor( - tan(angle / 2) * ox);
		int yshear = (int)floor(sin(angle) * oy);
		printf("Step %d: xshear %d, yshear %d, flip %d. ", step, xshear, yshear, fl);
		for (int i = 0; i < ox * oy; i++)
		{
			fb1[i] = bg;
			fb2[i] = bg;
			fb3[i] = bg;
		}
		unsigned int* s = base;
		if (fl) s = flip;
		for (int i = 0; i < oy; i++)
			for (int j = 0; j < ox; j++)
			{
				int p = j + xshear * i / oy - xshear / 2;
				if (p >= 0 && p < ox)
				{
					fb1[i * ox + p] = s[i * ox + j];
				}
			}
		
		for (int j = 0; j < ox; j++)
			for (int i = 0; i < oy; i++)
			{
				int p = i + yshear * j / ox - yshear / 2;
				if (p >= 0 && p < oy)
				{
					fb2[p * ox + j] = fb1[i * ox + j];
				}
			}

		for (int i = 0; i < oy; i++)
			for (int j = 0; j < ox; j++)
			{
				int p = j + xshear * i / oy - xshear / 2;
				if (p >= 0 && p < ox)
				{
					fb3[i * ox + p] = fb2[i * ox + j];
				}
			}

		for (int i = 0; i < py; i++)
			for (int j = 0; j < px; j++)
			{
				fb[i * px + j] = fb3[(i + py / 2) * ox + j + px / 2];
			}
		snprintf(temp, 256, "%s_%03d.png", pars[1], step);
		stbi_write_png(temp, px, py, 4, fb, px * 4);
		printf("Wrote %s.\n", temp);
		GifWriteFrame(&gif, (uint8_t*)fb, px, py, 1);
		/*
		snprintf(temp, 256, "1%s_%03d.png", pars[1], step);
		stbi_write_png(temp, ox, oy, 4, fb1, ox * 4);
		snprintf(temp, 256, "2%s_%03d.png", pars[1], step);
		stbi_write_png(temp, ox, oy, 4, fb2, ox * 4);
		*/
	}
	GifEnd(&gif);

	return 0;
}
