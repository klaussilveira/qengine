#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

unsigned char *cmap, *pal;

pcxsave(unsigned char *fname)
{
  short tavu1, tavu2, c, oc, tmp;
  long x, y;
  unsigned long w = 256, h = 320;
  unsigned long pos, size, po2;
  FILE *out;
  unsigned char hdr[128];

  if (w == 0 || h == 0)
    return (1);

  for (c = 0; c < 128; c++) /* clean up */
    hdr[c] = 0;

  hdr[0] = 10;
  hdr[1] = 5;
  hdr[2] = 1;
  hdr[3] = 8; /* standard PCX header start */

  hdr[8] = w - 1 & 0xff;
  hdr[9] = w - 1 >> 8; /* "Window size" */

  hdr[10] = h - 1 & 0xff;
  hdr[11] = h - 1 >> 8;
  hdr[12] = 1;
  hdr[14] = 1; /* Just for fun.. 256DPI. ;) */

  hdr[65] = 1; /* "Nplanes" */

  hdr[66] = w & 0xff;
  hdr[67] = w >> 8; /* Bytes per line.. */

  hdr[70] = w & 0xff;
  hdr[71] = w >> 8; /* screen size */

  hdr[72] = h & 0xff;
  hdr[73] = h >> 8;

  out = fopen(fname, "wb");
  if (!out) {
    printf("Can't write file %s\n", fname);
    return (1);
  }
  printf("Saving as %s...\n", fname);

  fwrite(hdr, 1, 128, out);

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      c = cmap[y * w + x];
      if (c > 190) {
        fputc(193, out);
        fputc(c, out);
      } else
        fputc(c, out);
    }
  }

  fputc(12, out);
  for (c = 0; c < 768; c++)
    fputc(pal[c], out);
  fclose(out);

  return 0;
}

short findcol(int r, int g, int b, int beg, int end)
{ /* FIND RGB COLOR */
  int c1, c = 0, e, ee = 768;

  for (c1 = beg; c1 < end; c1++) {
    e = abs(pal[c1 + c1 + c1] - r) + abs(pal[c1 + c1 + c1 + 1] - g) + abs(pal[c1 + c1 + c1 + 2] - b);
    if (e < ee) {
      c = c1;
      ee = e;
    }
  }
  return c;
}

void do_color_map(unsigned char *fname)
{
  long x, y, r, g, b;
  short c;

  for (y = 0; y < 64; y++)
    for (x = 0; x < 256; x++) {
      if (x == 255)
        c = x;
      else {
        r = pal[x * 3] * (63 - y) >> 5;
        g = pal[x * 3 + 1] * (63 - y) >> 5;
        b = pal[x * 3 + 2] * (63 - y) >> 5;
        c = findcol(r, g, b, 0, 255);
      }
      cmap[(y << 8) + x] = c;
    }
  for (y = 0; y < 256; y++)
    for (x = 0; x < 256; x++) {
      if (x == 255 || y == 255)
        c = 255;
      else {
        r = (pal[x * 3] * 2 + pal[y * 3]) / 3;
        g = (pal[x * 3 + 1] * 2 + pal[y * 3 + 1]) / 3;
        b = (pal[x * 3 + 2] * 2 + pal[y * 3 + 2]) / 3;
        c = findcol(r, g, b, 0, 255);
      }
      cmap[(y + 64 << 8) + x] = c;
    }
  pcxsave(fname);
}

int main(int argc, char *argv[])
{
  FILE *fil;

  printf("Colormap.exe by Iikka Keränen 1998\n\n");
  if (argc < 3) {
    printf("Usage: colormap <palette>.pal <colormap>.pcx\n");
    return 0;
  }
  cmap = malloc(256 * 320);
  pal = malloc(768);
  fil = fopen(argv[1], "rb");
  fread(pal, 1, 768, fil);
  fclose(fil);
  do_color_map(argv[2]);

  return 0;
}
