#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

int main(int argc, char *argv[])
{
  unsigned char pb[768];
  FILE *pcx, *pal;

  if (argc != 3) {
    printf("Usage: PCX2PAL <input>.PCX <output>.PAL\n");
    return (0);
  }
  pcx = fopen(argv[1], "rb");
  pal = fopen(argv[2], "wb");
  if (!pcx) {
    fprintf(stderr, "Can't find file %s\n", argv[1]);
    exit(1);
  }
  if (!pal) {
    fprintf(stderr, "Can't open %s for writing! Disk full?\n", argv[2]);
    exit(1);
  }
  fseek(pcx, -768, SEEK_END);
  fread(pb, 1, 768, pcx);
  fclose(pcx);
  fwrite(pb, 1, 768, pal);
  fclose(pal);

  return 0;
}
