/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qdata.h"

byte *soundtrack;
char base[32];

/*
===============================================================================

WAV loading

===============================================================================
*/

typedef struct
{
  int rate;
  int width;
  int channels;
  int loopstart;
  int samples;
  int dataofs; // chunk starts this many bytes from file start
} wavinfo_t;

byte *data_p;
byte *iff_end;
byte *last_chunk;
byte *iff_data;
int iff_chunk_len;

int samplecounts[0x10000];

wavinfo_t wavinfo;

short GetLittleShort(void)
{
  short val = 0;
  val = *data_p;
  val = val + (*(data_p + 1) << 8);
  data_p += 2;
  return val;
}

int GetLittleLong(void)
{
  int val = 0;
  val = *data_p;
  val = val + (*(data_p + 1) << 8);
  val = val + (*(data_p + 2) << 16);
  val = val + (*(data_p + 3) << 24);
  data_p += 4;
  return val;
}

void FindNextChunk(char *name)
{
  while (1) {
    data_p = last_chunk;

    if (data_p >= iff_end) { // didn't find the chunk
      data_p = NULL;
      return;
    }

    data_p += 4;
    iff_chunk_len = GetLittleLong();
    if (iff_chunk_len < 0) {
      data_p = NULL;
      return;
    }
    //		if (iff_chunk_len > 1024*1024)
    //			Sys_Error ("FindNextChunk: %i length is past the 1 meg sanity
    // limit", iff_chunk_len);
    data_p -= 8;
    last_chunk = data_p + 8 + ((iff_chunk_len + 1) & ~1);
    if (!strncmp(data_p, name, 4))
      return;
  }
}

void FindChunk(char *name)
{
  last_chunk = iff_data;
  FindNextChunk(name);
}

void DumpChunks(void)
{
  char str[5];

  str[4] = 0;
  data_p = iff_data;
  do {
    memcpy(str, data_p, 4);
    data_p += 4;
    iff_chunk_len = GetLittleLong();
    printf("0x%x : %s (%d)\n", (int) (data_p - 4), str, iff_chunk_len);
    data_p += (iff_chunk_len + 1) & ~1;
  } while (data_p < iff_end);
}

/*
============
GetWavinfo
============
*/
wavinfo_t GetWavinfo(char *name, byte *wav, int wavlength)
{
  wavinfo_t info;
  int i;
  int format;
  int samples;

  memset(&info, 0, sizeof(info));

  if (!wav)
    return info;

  iff_data = wav;
  iff_end = wav + wavlength;

  // find "RIFF" chunk
  FindChunk("RIFF");
  if (!(data_p && !strncmp(data_p + 8, "WAVE", 4))) {
    printf("Missing RIFF/WAVE chunks\n");
    return info;
  }

  // get "fmt " chunk
  iff_data = data_p + 12;
  // DumpChunks ();

  FindChunk("fmt ");
  if (!data_p) {
    printf("Missing fmt chunk\n");
    return info;
  }
  data_p += 8;
  format = GetLittleShort();
  if (format != 1) {
    printf("Microsoft PCM format only\n");
    return info;
  }

  info.channels = GetLittleShort();
  info.rate = GetLittleLong();
  data_p += 4 + 2;
  info.width = GetLittleShort() / 8;

  // get cue chunk
  FindChunk("cue ");
  if (data_p) {
    data_p += 32;
    info.loopstart = GetLittleLong();
    //		Com_Printf("loopstart=%d\n", sfx->loopstart);

    // if the next chunk is a LIST chunk, look for a cue length marker
    FindNextChunk("LIST");
    if (data_p) {
      if (!strncmp(data_p + 28, "mark",
                   4)) { // this is not a proper parse, but it works with cooledit...
        data_p += 24;
        i = GetLittleLong(); // samples in loop
        info.samples = info.loopstart + i;
      }
    }
  } else
    info.loopstart = -1;

  // find data chunk
  FindChunk("data");
  if (!data_p) {
    printf("Missing data chunk\n");
    return info;
  }

  data_p += 4;
  samples = GetLittleLong();

  if (info.samples) {
    if (samples < info.samples)
      Error("Sound %s has a bad loop length", name);
  } else
    info.samples = samples;

  info.dataofs = data_p - wav;

  return info;
}

//=====================================================================

/*
==============
LoadSoundtrack
==============
*/
void LoadSoundtrack(void)
{
  char name[1024];
  FILE *f;
  int len;
  int i, val, j;

  soundtrack = NULL;
  sprintf(name, "%svideo/%s/%s.wav", gamedir, base, base);
  printf("%s\n", name);
  f = fopen(name, "rb");
  if (!f) {
    printf("no soundtrack for %s\n", base);
    return;
  }
  len = Q_filelength(f);
  soundtrack = malloc(len);
  fread(soundtrack, 1, len, f);
  fclose(f);

  wavinfo = GetWavinfo(name, soundtrack, len);

  // count samples for compression
  memset(samplecounts, 0, sizeof(samplecounts));

  j = wavinfo.samples / 2;
  for (i = 0; i < j; i++) {
    val = ((unsigned short *) (soundtrack + wavinfo.dataofs))[i];
    samplecounts[val]++;
  }
  val = 0;
  for (i = 0; i < 0x10000; i++)
    if (samplecounts[i])
      val++;

  printf("%i unique sample values\n", val);
}

/*
==================
WriteSound
==================
*/
void WriteSound(FILE *output, int frame)
{
  int start, end;
  int count;
  int empty = 0;
  int i;
  int sample;
  int width;

  width = wavinfo.width * wavinfo.channels;

  start = frame * wavinfo.rate / 14;
  end = (frame + 1) * wavinfo.rate / 14;
  count = end - start;

  for (i = 0; i < count; i++) {
    sample = start + i;
    if (sample > wavinfo.samples || !soundtrack)
      fwrite(&empty, 1, width, output);
    else
      fwrite(soundtrack + wavinfo.dataofs + sample * width, 1, width, output);
  }
}

//==========================================================================

/*
==================
MTF
==================
*/
cblock_t MTF(cblock_t in)
{
  int i, j, b, code;
  byte *out_p;
  int index[256];
  cblock_t out;

  out_p = out.data = malloc(in.count + 4);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  for (i = 0; i < 256; i++)
    index[i] = i;

  for (i = 0; i < in.count; i++) {
    b = in.data[i];
    code = index[b];
    *out_p++ = code;

    // shuffle b indexes to 0
    for (j = 0; j < 256; j++)
      if (index[j] < code)
        index[j]++;
    index[b] = 0;
  }

  out.count = out_p - out.data;

  return out;
}

//==========================================================================

int bwt_size;
byte *bwt_data;

int bwtCompare(const void *elem1, const void *elem2)
{
  int i;
  int i1, i2;
  int b1, b2;

  i1 = *(int *) elem1;
  i2 = *(int *) elem2;

  for (i = 0; i < bwt_size; i++) {
    b1 = bwt_data[i1];
    b2 = bwt_data[i2];
    if (b1 < b2)
      return -1;
    if (b1 > b2)
      return 1;
    if (++i1 == bwt_size)
      i1 = 0;
    if (++i2 == bwt_size)
      i2 = 0;
  }

  return 0;
}

/*
==================
BWT
==================
*/
cblock_t BWT(cblock_t in)
{
  int *sorted;
  int i;
  byte *out_p;
  cblock_t out;

  bwt_size = in.count;
  bwt_data = in.data;

  sorted = malloc(in.count * sizeof(*sorted));
  for (i = 0; i < in.count; i++)
    sorted[i] = i;
  qsort(sorted, in.count, sizeof(*sorted), bwtCompare);

  out_p = out.data = malloc(in.count + 8);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  // write head index
  for (i = 0; i < in.count; i++)
    if (sorted[i] == 0)
      break;
  *out_p++ = i & 255;
  *out_p++ = (i >> 8) & 255;
  *out_p++ = (i >> 16) & 255;
  *out_p++ = (i >> 24) & 255;

  // write the L column
  for (i = 0; i < in.count; i++)
    *out_p++ = in.data[(sorted[i] + in.count - 1) % in.count];

  free(sorted);

  out.count = out_p - out.data;

  return out;
}

//==========================================================================

typedef struct hnode_s
{
  int count;
  qboolean used;
  int children[2];
} hnode_t;

int numhnodes;
hnode_t hnodes[512];
unsigned charbits[256];
int charbitscount[256];

int SmallestNode(void)
{
  int i;
  int best, bestnode;

  best = 99999999;
  bestnode = -1;
  for (i = 0; i < numhnodes; i++) {
    if (hnodes[i].used)
      continue;
    if (!hnodes[i].count)
      continue;
    if (hnodes[i].count < best) {
      best = hnodes[i].count;
      bestnode = i;
    }
  }

  if (bestnode == -1)
    return -1;

  hnodes[bestnode].used = true;
  return bestnode;
}

void BuildChars(int nodenum, unsigned bits, int bitcount)
{
  hnode_t *node;

  if (nodenum < 256) {
    if (bitcount > 32)
      Error("bitcount > 32");
    charbits[nodenum] = bits;
    charbitscount[nodenum] = bitcount;
    return;
  }

  node = &hnodes[nodenum];
  bits <<= 1;
  BuildChars(node->children[0], bits, bitcount + 1);
  bits |= 1;
  BuildChars(node->children[1], bits, bitcount + 1);
}

/*
==================
Huffman
==================
*/
cblock_t Huffman(cblock_t in)
{
  int i;
  hnode_t *node;
  int outbits, c;
  unsigned bits;
  byte *out_p;
  cblock_t out;
  int max, maxchar;

  // count
  memset(hnodes, 0, sizeof(hnodes));
  for (i = 0; i < in.count; i++)
    hnodes[in.data[i]].count++;

  // normalize counts
  max = 0;
  maxchar = 0;
  for (i = 0; i < 256; i++) {
    if (hnodes[i].count > max) {
      max = hnodes[i].count;
      maxchar = i;
    }
  }
  if (max == 0)
    Error("Huffman: max == 0");

  for (i = 0; i < 256; i++) {
    hnodes[i].count = (hnodes[i].count * 255 + max - 1) / max;
  }

  // build the nodes
  numhnodes = 256;
  while (numhnodes != 511) {
    node = &hnodes[numhnodes];

    // pick two lowest counts
    node->children[0] = SmallestNode();
    if (node->children[0] == -1)
      break; // no more

    node->children[1] = SmallestNode();
    if (node->children[1] == -1) {
      if (node->children[0] != numhnodes - 1)
        Error("Bad smallestnode");
      break;
    }
    node->count = hnodes[node->children[0]].count + hnodes[node->children[1]].count;
    numhnodes++;
  }

  BuildChars(numhnodes - 1, 0, 0);

  out_p = out.data = malloc(in.count * 2 + 1024);
  memset(out_p, 0, in.count * 2 + 1024);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  // save out the 256 normalized counts so the tree can be recreated
  for (i = 0; i < 256; i++)
    *out_p++ = hnodes[i].count;

  // write bits
  outbits = 0;
  for (i = 0; i < in.count; i++) {
    c = charbitscount[in.data[i]];
    bits = charbits[in.data[i]];
    while (c) {
      c--;
      if (bits & (1 << c))
        out_p[outbits >> 3] |= 1 << (outbits & 7);
      outbits++;
    }
  }

  out_p += (outbits + 7) >> 3;

  out.count = out_p - out.data;

  return out;
}

//==========================================================================

/*
==================
RLE
==================
*/
#define RLE_CODE 0xe8
#define RLE_TRIPPLE 0xe9

int rle_counts[256];
int rle_bytes[256];

cblock_t RLE(cblock_t in)
{
  int i;
  byte *out_p;
  int val;
  int repeat;
  cblock_t out;

  out_p = out.data = malloc(in.count * 2);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  for (i = 0; i < in.count;) {
    val = in.data[i];
    rle_bytes[val]++;
    repeat = 1;
    i++;
    while (i < in.count && repeat < 255 && in.data[i] == val) {
      repeat++;
      i++;
    }
    if (repeat < 256)
      rle_counts[repeat]++;
    if (repeat > 3 || val == RLE_CODE) {
      *out_p++ = RLE_CODE;
      *out_p++ = val;
      *out_p++ = repeat;
    } else {
      while (repeat--)
        *out_p++ = val;
    }
  }

  out.count = out_p - out.data;
  return out;
}

//==========================================================================

unsigned lzss_head[256];
unsigned lzss_next[0x20000];

/*
==================
LZSS
==================
*/
#define BACK_WINDOW 0x10000
#define BACK_BITS 16
#define FRONT_WINDOW 16
#define FRONT_BITS 4

cblock_t LZSS(cblock_t in)
{
  int i;
  byte *out_p;
  cblock_t out;
  int val;
  int j, start, max;
  int bestlength, beststart;
  int outbits;

  if (in.count >= sizeof(lzss_next) / 4)
    Error("LZSS: too big");

  memset(lzss_head, -1, sizeof(lzss_head));

  out_p = out.data = malloc(in.count * 2);
  memset(out.data, 0, in.count * 2);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  outbits = 0;
  for (i = 0; i < in.count;) {
    val = in.data[i];
#if 1
    // chained search
    bestlength = 0;
    beststart = 0;

    max = FRONT_WINDOW;
    if (i + max > in.count)
      max = in.count - i;

    start = lzss_head[val];
    while (start != -1 && start >= i - BACK_WINDOW) {
      // count match length
      for (j = 0; j < max; j++)
        if (in.data[start + j] != in.data[i + j])
          break;
      if (j > bestlength) {
        bestlength = j;
        beststart = start;
      }
      start = lzss_next[start];
    }

#else
    // slow simple search
    // search for a match
    max = FRONT_WINDOW;
    if (i + max > in.count)
      max = in.count - i;

    start = i - BACK_WINDOW;
    if (start < 0)
      start = 0;
    bestlength = 0;
    beststart = 0;
    for (; start < i; start++) {
      if (in.data[start] != val)
        continue;
      // count match length
      for (j = 0; j < max; j++)
        if (in.data[start + j] != in.data[i + j])
          break;
      if (j > bestlength) {
        bestlength = j;
        beststart = start;
      }
    }
#endif
    beststart = BACK_WINDOW - (i - beststart);

    if (bestlength < 3) { // output a single char
      bestlength = 1;

      out_p[outbits >> 3] |= 1 << (outbits & 7); // set bit to mark char
      outbits++;
      for (j = 0; j < 8; j++, outbits++)
        if (val & (1 << j))
          out_p[outbits >> 3] |= 1 << (outbits & 7);
    } else {     // output a phrase
      outbits++; // leave a 0 bit to mark phrase
      for (j = 0; j < BACK_BITS; j++, outbits++)
        if (beststart & (1 << j))
          out_p[outbits >> 3] |= 1 << (outbits & 7);
      for (j = 0; j < FRONT_BITS; j++, outbits++)
        if (bestlength & (1 << j))
          out_p[outbits >> 3] |= 1 << (outbits & 7);
    }

    while (bestlength--) {
      val = in.data[i];
      lzss_next[i] = lzss_head[val];
      lzss_head[val] = i;
      i++;
    }
  }

  out_p += (outbits + 7) >> 3;
  out.count = out_p - out.data;
  return out;
}

//==========================================================================

#define MIN_REPT 15
#define MAX_REPT 0
#define HUF_TOKENS (256 + MAX_REPT)

unsigned charbits1[256][HUF_TOKENS];
int charbitscount1[256][HUF_TOKENS];

hnode_t hnodes1[256][HUF_TOKENS * 2];
int numhnodes1[256];

int order0counts[256];

/*
==================
SmallestNode1
==================
*/
int SmallestNode1(hnode_t *hnodes, int numhnodes)
{
  int i;
  int best, bestnode;

  best = 99999999;
  bestnode = -1;
  for (i = 0; i < numhnodes; i++) {
    if (hnodes[i].used)
      continue;
    if (!hnodes[i].count)
      continue;
    if (hnodes[i].count < best) {
      best = hnodes[i].count;
      bestnode = i;
    }
  }

  if (bestnode == -1)
    return -1;

  hnodes[bestnode].used = true;
  return bestnode;
}

/*
==================
BuildChars1
==================
*/
void BuildChars1(int prev, int nodenum, unsigned bits, int bitcount)
{
  hnode_t *node;

  if (nodenum < HUF_TOKENS) {
    if (bitcount > 32)
      Error("bitcount > 32");
    charbits1[prev][nodenum] = bits;
    charbitscount1[prev][nodenum] = bitcount;
    return;
  }

  node = &hnodes1[prev][nodenum];
  bits <<= 1;
  BuildChars1(prev, node->children[0], bits, bitcount + 1);
  bits |= 1;
  BuildChars1(prev, node->children[1], bits, bitcount + 1);
}

/*
==================
BuildTree1
==================
*/
void BuildTree1(int prev)
{
  hnode_t *node, *nodebase;
  int numhnodes;

  // build the nodes
  numhnodes = HUF_TOKENS;
  nodebase = hnodes1[prev];
  while (1) {
    node = &nodebase[numhnodes];

    // pick two lowest counts
    node->children[0] = SmallestNode1(nodebase, numhnodes);
    if (node->children[0] == -1)
      break; // no more

    node->children[1] = SmallestNode1(nodebase, numhnodes);
    if (node->children[1] == -1)
      break;

    node->count = nodebase[node->children[0]].count + nodebase[node->children[1]].count;
    numhnodes++;
  }
  numhnodes1[prev] = numhnodes - 1;
  BuildChars1(prev, numhnodes - 1, 0, 0);
}

/*
==================
Huffman1_Count
==================
*/
void Huffman1_Count(cblock_t in)
{
  int i;
  int prev;
  int v;
  int rept;

  prev = 0;
  for (i = 0; i < in.count; i++) {
    v = in.data[i];
    order0counts[v]++;
    hnodes1[prev][v].count++;
    prev = v;
#if 1
    for (rept = 1; i + rept < in.count && rept < MAX_REPT; rept++)
      if (in.data[i + rept] != v)
        break;
    if (rept > MIN_REPT) {
      hnodes1[prev][255 + rept].count++;
      i += rept - 1;
    }
#endif
  }
}

/*
==================
Huffman1_Build
==================
*/
byte scaled[256][HUF_TOKENS];

void Huffman1_Build(FILE *f)
{
  int i, j, v;
  int max;
  int total;

  for (i = 0; i < 256; i++) {
    // normalize and save the counts
    max = 0;
    for (j = 0; j < HUF_TOKENS; j++) {
      if (hnodes1[i][j].count > max)
        max = hnodes1[i][j].count;
    }
    if (max == 0)
      max = 1;
    total = 0;
    for (j = 0; j < HUF_TOKENS; j++) { // easy to overflow 32 bits here!
      v = (hnodes1[i][j].count * (double) 255 + max - 1) / max;
      if (v > 255)
        Error("v > 255");
      scaled[i][j] = hnodes1[i][j].count = v;
      if (v)
        total++;
    }
    if (total == 1) { // must have two tokens
      if (!scaled[i][0])
        scaled[i][0] = hnodes1[i][0].count = 1;
      else
        scaled[i][1] = hnodes1[i][1].count = 1;
    }

    BuildTree1(i);
  }

#if 0
    // count up the total bits
    total = 0;
    for (i=0 ; i<256 ; i++)
        for (j=0 ; j<256 ; j++)
            total += charbitscount1[i][j] * hnodes1[i][j].count;

    total = (total+7)/8;
    printf ("%i bytes huffman1 compressed\n", total);
#endif

  fwrite(scaled, 1, sizeof(scaled), f);
}

/*
==================
Huffman1

Order 1 compression with pre-built table
==================
*/
cblock_t Huffman1(cblock_t in)
{
  int i;
  int outbits, c;
  unsigned bits;
  byte *out_p;
  cblock_t out;
  int prev;
  int v;
  int rept;

  out_p = out.data = malloc(in.count * 2 + 1024);
  memset(out_p, 0, in.count * 2 + 1024);

  // write count
  *out_p++ = in.count & 255;
  *out_p++ = (in.count >> 8) & 255;
  *out_p++ = (in.count >> 16) & 255;
  *out_p++ = (in.count >> 24) & 255;

  // write bits
  outbits = 0;
  prev = 0;
  for (i = 0; i < in.count; i++) {
    v = in.data[i];

    c = charbitscount1[prev][v];
    bits = charbits1[prev][v];
    if (!c)
      Error("!bits");
    while (c) {
      c--;
      if (bits & (1 << c))
        out_p[outbits >> 3] |= 1 << (outbits & 7);
      outbits++;
    }

    prev = v;
#if 1
    // check for repeat encodes
    for (rept = 1; i + rept < in.count && rept < MAX_REPT; rept++)
      if (in.data[i + rept] != v)
        break;
    if (rept > MIN_REPT) {
      c = charbitscount1[prev][255 + rept];
      bits = charbits1[prev][255 + rept];
      if (!c)
        Error("!bits");
      while (c) {
        c--;
        if (bits & (1 << c))
          out_p[outbits >> 3] |= 1 << (outbits & 7);
        outbits++;
      }
      i += rept - 1;
    }
#endif
  }

  out_p += (outbits + 7) >> 3;

  out.count = out_p - out.data;

  return out;
}

//==========================================================================

/*
===================
LoadFrame
===================
*/
cblock_t LoadFrame(char *base, int frame, int digits, byte **palette)
{
  int ten3, ten2, ten1, ten0;
  cblock_t in;
  int width, height;
  char name[1024];
  FILE *f;

  in.data = NULL;
  in.count = -1;

  ten3 = frame / 1000;
  ten2 = (frame - ten3 * 1000) / 100;
  ten1 = (frame - ten3 * 1000 - ten2 * 100) / 10;
  ten0 = frame % 10;

  if (digits == 4)
    sprintf(name, "%svideo/%s/%s%i%i%i%i.pcx", gamedir, base, base, ten3, ten2, ten1, ten0);
  else
    sprintf(name, "%svideo/%s/%s%i%i%i.pcx", gamedir, base, base, ten2, ten1, ten0);

  f = fopen(name, "rb");
  if (!f) {
    in.data = NULL;
    return in;
  }
  fclose(f);

  printf("%s\n", name);
  Load256Image(name, &in.data, palette, &width, &height);
  in.count = width * height;
  // FIXME: map 0 and 255!

#if 0
    // rle compress
    rle = RLE(in);
    free (in.data);

    return rle;
#endif

  return in;
}

/*
===============
Cmd_Video

video <directory> <framedigits>
===============
*/
void Cmd_Video(void)
{
  char savename[1024];
  char name[1024];
  FILE *output;
  int startframe, frame;
  byte *palette;
  int width, height;
  byte current_palette[768];
  int command;
  int i;
  int digits;
  cblock_t in, huffman;
  int swap;

  GetToken(false);
  strcpy(base, token);
  if (g_release) {
    //		sprintf (savename, "video/%s.cin", token);
    //		ReleaseFile (savename);
    return;
  }

  GetToken(false);
  digits = atoi(token);

  // optionally skip frames
  if (TokenAvailable()) {
    GetToken(false);
    startframe = atoi(token);
  } else
    startframe = 0;

  sprintf(savename, "%svideo/%s.cin", gamedir, base);

  // clear stuff
  memset(charbits1, 0, sizeof(charbits1));
  memset(charbitscount1, 0, sizeof(charbitscount1));
  memset(hnodes1, 0, sizeof(hnodes1));
  memset(numhnodes1, 0, sizeof(numhnodes1));
  memset(order0counts, 0, sizeof(order0counts));

  // load the entire sound wav file if present
  LoadSoundtrack();

  if (digits == 4)
    sprintf(name, "%svideo/%s/%s0000.pcx", gamedir, base, base);
  else
    sprintf(name, "%svideo/%s/%s000.pcx", gamedir, base, base);

  printf("%s\n", name);
  Load256Image(name, NULL, &palette, &width, &height);

  output = fopen(savename, "wb");
  if (!output)
    Error("Can't open %s", savename);

  // write header info
  i = LittleLong(width);
  fwrite(&i, 4, 1, output);
  i = LittleLong(height);
  fwrite(&i, 4, 1, output);
  i = LittleLong(wavinfo.rate);
  fwrite(&i, 4, 1, output);
  i = LittleLong(wavinfo.width);
  fwrite(&i, 4, 1, output);
  i = LittleLong(wavinfo.channels);
  fwrite(&i, 4, 1, output);

  // build the dictionary
  for (frame = startframe;; frame++) {
    printf("counting ", frame);
    in = LoadFrame(base, frame, digits, &palette);
    if (!in.data)
      break;
    Huffman1_Count(in);
    free(in.data);
  }
  printf("\n");

  // build nodes and write counts
  Huffman1_Build(output);

  memset(current_palette, 0, sizeof(current_palette));

  // compress it with the dictionary
  for (frame = startframe;; frame++) {
    printf("packing ", frame);
    in = LoadFrame(base, frame, digits, &palette);
    if (!in.data)
      break;

    // see if the palette has changed
    for (i = 0; i < 768; i++)
      if (palette[i] != current_palette[i]) {
        // write a palette change
        memcpy(current_palette, palette, sizeof(current_palette));
        command = LittleLong(1);
        fwrite(&command, 1, 4, output);
        fwrite(current_palette, 1, sizeof(current_palette), output);
        break;
      }
    if (i == 768) {
      command = 0; // no palette change
      fwrite(&command, 1, 4, output);
    }

    // save the image
    huffman = Huffman1(in);
    printf("%5i bytes after huffman1\n", huffman.count);

    swap = LittleLong(huffman.count);
    fwrite(&swap, 1, sizeof(swap), output);

    fwrite(huffman.data, 1, huffman.count, output);

    // save some sound samples
    WriteSound(output, frame);

    free(palette);
    free(in.data);
    free(huffman.data);
  }
  printf("\n");

  // write end-of-file command
  command = 2;
  fwrite(&command, 1, 4, output);

  printf("Total size: %i\n", ftell(output));

  fclose(output);

  if (soundtrack)
    free(soundtrack);
}
