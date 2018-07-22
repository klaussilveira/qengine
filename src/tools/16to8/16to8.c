#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

unsigned char *table;
unsigned char pal[768];

int findcol(int r, int g, int b)
{
  int minc = 0, maxc = 256;
  int c1, c = 0, e, ee = 768;
  long r1, g1, b1;

  for (c1 = minc; c1 < maxc; c1++) {
    r1 = pal[c1 + c1 + c1] - r;
    g1 = pal[c1 + c1 + c1 + 1] - g;
    b1 = pal[c1 + c1 + c1 + 2] - b;
    e = sqrt(r1 * r1 + g1 * g1 + b1 * b1);
    if (e < ee) {
      c = c1;
      ee = e;
    }
  }
  return c;
}

int pcxpalload(unsigned char *name, unsigned char *buf)
{
  unsigned long p;
  unsigned long w, h;

  FILE *pcx = fopen(name, "rb");
  if (!pcx)
    return (1);
  if (buf != NULL) {
    fseek(pcx, -768, SEEK_END); /* Load palette */
    fread(buf, 1, 768, pcx);
  }
  fclose(pcx);
  return 0;
}

int main(void)
{
  long x, y, x1, y1, x0, y0;
  FILE *datfile;

  table = malloc(65536L);
  if (pcxpalload("colormap.pcx", pal)) {
    printf("You need colormap.pcx to run 16to8\n");
    exit(1);
  }
  printf("32 to go...\n");
  for (x1 = 0; x1 < 32; x1++) {
    printf("%d\n", 31 - x1);
    for (x = 0; x < 32; x++)
      for (y = 0; y < 64; y++) {
        table[(x1 << 11) + (y << 5) + x] = findcol(x * 8, y * 4, x1 * 8);
      }
  }

  printf("Saving 16to8.dat\n");

  datfile = fopen("16to8.dat", "wb");
  for (x = 0; x < 65536; x++)
    fputc(table[x], datfile);
  fclose(datfile);
  return 0;
}
