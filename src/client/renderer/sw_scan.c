/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

#include "header/local.h"

#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#define QENGINE_SCAN_AVX2 1
#else
#define QENGINE_SCAN_AVX2 0
#endif

static qboolean scan_avx2_available;

void D_InitSIMD(void)
{
#if QENGINE_SCAN_AVX2
  __builtin_cpu_init();
  scan_avx2_available = __builtin_cpu_supports("avx2") != 0;
#else
  scan_avx2_available = false;
#endif
}

pixel_t *r_turb_pbase, *r_turb_pdest;
int r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int *r_turb_turb;
int r_turb_spancount;

void D_DrawTurbulent8Span(void);

byte **warp_rowptr;
int *warp_column;
espan_t *edge_basespans;

#define SPANSTEP_SHIFT 4
#define SPANSTEP_VALUE (1 << SPANSTEP_SHIFT)

/*
=============
D_WarpScreen

this performs a slight compression of the screen at the same time as
the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen(void)
{
  int w, h;
  int u, v;
  pixel_t *dest;
  int *turb;
  byte **row;

  static int cached_width, cached_height;

  //
  // these are constant over resolutions, and can be saved
  //
  w = r_newrefdef.width;
  h = r_newrefdef.height;
  if (w != cached_width || h != cached_height) {
    cached_width = w;
    cached_height = h;
    for (v = 0; v < h + AMP2 * 2; v++) {
      int v2;

      v2 = (int) ((float) v / (h + AMP2 * 2) * r_refdef.vrect.height);
      warp_rowptr[v] = r_warpbuffer + (WARP_WIDTH * v2);
    }

    for (u = 0; u < w + AMP2 * 2; u++) {
      int u2;

      u2 = (int) ((float) u / (w + AMP2 * 2) * r_refdef.vrect.width);
      warp_column[u] = u2;
    }
  }

  turb = intsintable + ((int) (r_newrefdef.time * SPEED) & (CYCLE - 1));
  dest = vid_buffer + r_newrefdef.y * vid.width + r_newrefdef.x;

  for (v = 0; v < h; v++, dest += vid.width) {
    int *col;

    col = warp_column + turb[v];
    row = warp_rowptr + v;
    for (u = 0; u < w; u++) {
      dest[u] = row[turb[u]][col[u]];
    }
  }
}

/*
=============
D_DrawTurbulent8Span
=============
*/
void D_DrawTurbulent8Span(void)
{
  do {
    int sturb, tturb;
    sturb = ((r_turb_s + r_turb_turb[(r_turb_t >> 16) & (CYCLE - 1)]) >> 16) & 63;
    tturb = ((r_turb_t + r_turb_turb[(r_turb_s >> 16) & (CYCLE - 1)]) >> 16) & 63;
    *r_turb_pdest++ = *(r_turb_pbase + (tturb << 6) + sturb);
    r_turb_s += r_turb_sstep;
    r_turb_t += r_turb_tstep;
  } while (--r_turb_spancount > 0);
}

/*
=============
Turbulent8
=============
*/
void Turbulent8(espan_t *pspan)
{
  int snext, tnext;
  float spancountminus1;
  float sdivz16stepu, tdivz16stepu, zi16stepu;

  r_turb_turb = sintable + ((int) (r_newrefdef.time * SPEED) & (CYCLE - 1));

  r_turb_sstep = 0; // keep compiler happy
  r_turb_tstep = 0; // ditto

  r_turb_pbase = (unsigned char *) cacheblock;

  sdivz16stepu = d_sdivzstepu * 16;
  tdivz16stepu = d_tdivzstepu * 16;
  zi16stepu = d_zistepu * 16;

  do {
    int count;
    float sdivz, tdivz, zi, z, du, dv;

    r_turb_pdest = d_viewbuffer + (r_screenwidth * pspan->v) + pspan->u;

    count = pspan->count;

    // calculate the initial s/z, t/z, 1/z, s, and t and clamp
    du = (float) pspan->u;
    dv = (float) pspan->v;

    sdivz = d_sdivzorigin + dv * d_sdivzstepv + du * d_sdivzstepu;
    tdivz = d_tdivzorigin + dv * d_tdivzstepv + du * d_tdivzstepu;
    zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
    z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

    r_turb_s = (int) (sdivz * z) + sadjust;
    if (r_turb_s > bbextents)
      r_turb_s = bbextents;
    else if (r_turb_s < 0)
      r_turb_s = 0;

    r_turb_t = (int) (tdivz * z) + tadjust;
    if (r_turb_t > bbextentt)
      r_turb_t = bbextentt;
    else if (r_turb_t < 0)
      r_turb_t = 0;

    do {
      // calculate s and t at the far end of the span
      if (count >= 16)
        r_turb_spancount = 16;
      else
        r_turb_spancount = count;

      count -= r_turb_spancount;

      if (count) {
        // calculate s/z, t/z, zi->fixed s and t at far end of span,
        // calculate s and t steps across span by shifting
        sdivz += sdivz16stepu;
        tdivz += tdivz16stepu;
        zi += zi16stepu;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

        snext = (int) (sdivz * z) + sadjust;
        if (snext > bbextents)
          snext = bbextents;
        else if (snext < 16)
          snext = 16; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + tadjust;
        if (tnext > bbextentt)
          tnext = bbextentt;
        else if (tnext < 16)
          tnext = 16; // guard against round-off error on <0 steps

        r_turb_sstep = (snext - r_turb_s) >> 4;
        r_turb_tstep = (tnext - r_turb_t) >> 4;
      } else {
        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
        // can't step off polygon), clamp, calculate s and t steps across
        // span by division, biasing steps low so we don't run off the
        // texture
        spancountminus1 = (float) (r_turb_spancount - 1);
        sdivz += d_sdivzstepu * spancountminus1;
        tdivz += d_tdivzstepu * spancountminus1;
        zi += d_zistepu * spancountminus1;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point
        snext = (int) (sdivz * z) + sadjust;
        if (snext > bbextents)
          snext = bbextents;
        else if (snext < 16)
          snext = 16; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + tadjust;
        if (tnext > bbextentt)
          tnext = bbextentt;
        else if (tnext < 16)
          tnext = 16; // guard against round-off error on <0 steps

        if (r_turb_spancount > 1) {
          r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
          r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
        }
      }

      r_turb_s = r_turb_s & ((CYCLE << 16) - 1);
      r_turb_t = r_turb_t & ((CYCLE << 16) - 1);

      D_DrawTurbulent8Span();

      r_turb_s = snext;
      r_turb_t = tnext;

    } while (count > 0);

  } while ((pspan = pspan->pnext) != NULL);
}

//====================
// PGM
/*
=============
NonTurbulent8 - this is for drawing scrolling textures. they're warping water
textures but the turbulence is automatically 0.
=============
*/
void NonTurbulent8(espan_t *pspan)
{
  int snext, tnext;
  float spancountminus1;
  float sdivz16stepu, tdivz16stepu, zi16stepu;

  r_turb_turb = blanktable;

  r_turb_sstep = 0; // keep compiler happy
  r_turb_tstep = 0; // ditto

  r_turb_pbase = (unsigned char *) cacheblock;

  sdivz16stepu = d_sdivzstepu * 16;
  tdivz16stepu = d_tdivzstepu * 16;
  zi16stepu = d_zistepu * 16;

  do {
    int count;
    float sdivz, tdivz, zi, z, dv, du;

    r_turb_pdest = d_viewbuffer + (r_screenwidth * pspan->v) + pspan->u;

    count = pspan->count;

    // calculate the initial s/z, t/z, 1/z, s, and t and clamp
    du = (float) pspan->u;
    dv = (float) pspan->v;

    sdivz = d_sdivzorigin + dv * d_sdivzstepv + du * d_sdivzstepu;
    tdivz = d_tdivzorigin + dv * d_tdivzstepv + du * d_tdivzstepu;
    zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
    z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

    r_turb_s = (int) (sdivz * z) + sadjust;
    if (r_turb_s > bbextents)
      r_turb_s = bbextents;
    else if (r_turb_s < 0)
      r_turb_s = 0;

    r_turb_t = (int) (tdivz * z) + tadjust;
    if (r_turb_t > bbextentt)
      r_turb_t = bbextentt;
    else if (r_turb_t < 0)
      r_turb_t = 0;

    do {
      // calculate s and t at the far end of the span
      if (count >= 16)
        r_turb_spancount = 16;
      else
        r_turb_spancount = count;

      count -= r_turb_spancount;

      if (count) {
        // calculate s/z, t/z, zi->fixed s and t at far end of span,
        // calculate s and t steps across span by shifting
        sdivz += sdivz16stepu;
        tdivz += tdivz16stepu;
        zi += zi16stepu;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

        snext = (int) (sdivz * z) + sadjust;
        if (snext > bbextents)
          snext = bbextents;
        else if (snext < 16)
          snext = 16; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + tadjust;
        if (tnext > bbextentt)
          tnext = bbextentt;
        else if (tnext < 16)
          tnext = 16; // guard against round-off error on <0 steps

        r_turb_sstep = (snext - r_turb_s) >> 4;
        r_turb_tstep = (tnext - r_turb_t) >> 4;
      } else {
        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
        // can't step off polygon), clamp, calculate s and t steps across
        // span by division, biasing steps low so we don't run off the
        // texture
        spancountminus1 = (float) (r_turb_spancount - 1);
        sdivz += d_sdivzstepu * spancountminus1;
        tdivz += d_tdivzstepu * spancountminus1;
        zi += d_zistepu * spancountminus1;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point
        snext = (int) (sdivz * z) + sadjust;
        if (snext > bbextents)
          snext = bbextents;
        else if (snext < 16)
          snext = 16; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + tadjust;
        if (tnext > bbextentt)
          tnext = bbextentt;
        else if (tnext < 16)
          tnext = 16; // guard against round-off error on <0 steps

        if (r_turb_spancount > 1) {
          r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
          r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
        }
      }

      r_turb_s = r_turb_s & ((CYCLE << 16) - 1);
      r_turb_t = r_turb_t & ((CYCLE << 16) - 1);

      D_DrawTurbulent8Span();

      r_turb_s = snext;
      r_turb_t = tnext;

    } while (count > 0);

  } while ((pspan = pspan->pnext) != NULL);
}
// PGM
//====================

/*
=============
D_DrawSpans16
=============
*/
void D_DrawSpans16(espan_t *pspan)
{
  int spancount;
  int ditherkernel[2][2][2];
  const int (*ditherrow)[2];
  int screenx;
  unsigned char *pbase;
  int snext, tnext, sstep, tstep;
  float spancountminus1;
  float sdivzstepstepu, tdivzstepstepu, zistepstepu;
  const int lcachewidth = cachewidth;
  const int lbbextents = bbextents;
  const int lbbextentt = bbextentt;
  const int lsadjust = sadjust;
  const int ltadjust = tadjust;
  const int lscreenwidth = r_screenwidth;
  pixel_t *const lviewbuffer = d_viewbuffer;
  const int ludither = r_udither->value != 0;

  sstep = 0; // keep compiler happy
  tstep = 0; // ditto

  {
    int x, y;
    int ditherdiv = 1 << d_miplevel;

    for (y = 0; y < 2; y++) {
      for (x = 0; x < 2; x++) {
        ditherkernel[y][x][0] = r_ditherkernel[y][x][0] / ditherdiv;
        ditherkernel[y][x][1] = r_ditherkernel[y][x][1] / ditherdiv;
      }
    }
  }

  pbase = (unsigned char *) cacheblock;

  sdivzstepstepu = d_sdivzstepu * SPANSTEP_VALUE;
  tdivzstepstepu = d_tdivzstepu * SPANSTEP_VALUE;
  zistepstepu = d_zistepu * SPANSTEP_VALUE;

  do {
    pixel_t *pdest;
    int count, s, t;
    float sdivz, tdivz, zi, z, du, dv;

    pdest = lviewbuffer + (lscreenwidth * pspan->v) + pspan->u;

    count = pspan->count;
    screenx = pspan->u;
    ditherrow = ditherkernel[pspan->v & 1];

    // calculate the initial s/z, t/z, 1/z, s, and t and clamp
    du = (float) pspan->u;
    dv = (float) pspan->v;

    sdivz = d_sdivzorigin + dv * d_sdivzstepv + du * d_sdivzstepu;
    tdivz = d_tdivzorigin + dv * d_tdivzstepv + du * d_tdivzstepu;
    zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
    z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

    s = (int) (sdivz * z) + lsadjust;
    if (s > lbbextents)
      s = lbbextents;
    else if (s < 0)
      s = 0;

    t = (int) (tdivz * z) + ltadjust;
    if (t > lbbextentt)
      t = lbbextentt;
    else if (t < 0)
      t = 0;

    do {
      // calculate s and t at the far end of the span
      if (count >= SPANSTEP_VALUE)
        spancount = SPANSTEP_VALUE;
      else
        spancount = count;

      count -= spancount;

      if (count) {
        // calculate s/z, t/z, zi->fixed s and t at far end of span,
        // calculate s and t steps across span by shifting
        sdivz += sdivzstepstepu;
        tdivz += tdivzstepstepu;
        zi += zistepstepu;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point

        snext = (int) (sdivz * z) + lsadjust;
        if (snext > lbbextents)
          snext = lbbextents;
        else if (snext < SPANSTEP_VALUE)
          snext = SPANSTEP_VALUE; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + ltadjust;
        if (tnext > lbbextentt)
          tnext = lbbextentt;
        else if (tnext < SPANSTEP_VALUE)
          tnext = SPANSTEP_VALUE; // guard against round-off error on <0 steps

        sstep = (snext - s) >> SPANSTEP_SHIFT;
        tstep = (tnext - t) >> SPANSTEP_SHIFT;
      } else {
        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
        // can't step off polygon), clamp, calculate s and t steps across
        // span by division, biasing steps low so we don't run off the
        // texture
        spancountminus1 = (float) (spancount - 1);
        sdivz += d_sdivzstepu * spancountminus1;
        tdivz += d_tdivzstepu * spancountminus1;
        zi += d_zistepu * spancountminus1;
        z = (float) 0x10000 / zi; // prescale to 16.16 fixed-point
        snext = (int) (sdivz * z) + lsadjust;
        if (snext > lbbextents)
          snext = lbbextents;
        else if (snext < SPANSTEP_VALUE)
          snext = SPANSTEP_VALUE; // prevent round-off error on <0 steps from
        //  from causing overstepping & running off the
        //  edge of the texture

        tnext = (int) (tdivz * z) + ltadjust;
        if (tnext > lbbextentt)
          tnext = lbbextentt;
        else if (tnext < SPANSTEP_VALUE)
          tnext = SPANSTEP_VALUE; // guard against round-off error on <0 steps

        if (spancount > 1) {
          sstep = (snext - s) / (spancount - 1);
          tstep = (tnext - t) / (spancount - 1);
        }
      }

      if (!ludither) {
        int remaining = spancount;

        while (remaining >= 4) {
          pdest[0] = pbase[(s >> 16) + (t >> 16) * lcachewidth];
          pdest[1] = pbase[((s + sstep) >> 16) + ((t + tstep) >> 16) * lcachewidth];
          pdest[2] = pbase[((s + sstep * 2) >> 16) + ((t + tstep * 2) >> 16) * lcachewidth];
          pdest[3] = pbase[((s + sstep * 3) >> 16) + ((t + tstep * 3) >> 16) * lcachewidth];
          pdest += 4;
          s += sstep * 4;
          t += tstep * 4;
          remaining -= 4;
        }

        while (remaining > 0) {
          *pdest++ = *(pbase + (s >> 16) + (t >> 16) * lcachewidth);
          s += sstep;
          t += tstep;
          remaining--;
        }
      } else {
        do {
          const int *kernel = ditherrow[screenx & 1];
          int idiths = s + kernel[0];
          int iditht = t + kernel[1];

          /* Clamp to texture bounds to prevent artifacts at edges */
          if (idiths < 0)
            idiths = 0;
          else if (idiths > lbbextents)
            idiths = lbbextents;

          if (iditht < 0)
            iditht = 0;
          else if (iditht > lbbextentt)
            iditht = lbbextentt;

          *pdest++ = *(pbase + (idiths >> 16) + (iditht >> 16) * lcachewidth);
          s += sstep;
          t += tstep;
          screenx++;
        } while (--spancount > 0);
      }

      s = snext;
      t = tnext;

    } while (count > 0);

  } while ((pspan = pspan->pnext) != NULL);
}

#if QENGINE_SCAN_AVX2
__attribute__((target("avx2"))) static void D_DrawZSpanAVX2(zvalue_t *pdest, int count, int izi, int izistep)
{
  __m256i step8 = _mm256_set1_epi32(izistep * 8);
  __m256i value =
      _mm256_setr_epi32(0, izistep, izistep * 2, izistep * 3, izistep * 4, izistep * 5, izistep * 6, izistep * 7);

  value = _mm256_add_epi32(value, _mm256_set1_epi32(izi));

  while (count >= 8) {
    _mm256_storeu_si256((__m256i *) pdest, _mm256_srai_epi32(value, 16));
    value = _mm256_add_epi32(value, step8);
    pdest += 8;
    count -= 8;
    izi += izistep * 8;
  }

  while (count > 0) {
    *pdest++ = izi >> 16;
    izi += izistep;
    count--;
  }
}
#endif

/*
=============
D_DrawZSpans
=============
*/
void D_DrawZSpans(espan_t *pspan)
{
  int izistep;

  // FIXME: check for clamping/range problems
  // we count on FP exceptions being turned off to avoid range problems
  izistep = (int) (d_zistepu * 0x8000 * 0x10000);

  do {
    int count;
    int izi;
    zvalue_t *pdest;
    float zi;
    float du, dv;

    pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

    count = pspan->count;

    // calculate the initial 1/z
    du = (float) pspan->u;
    dv = (float) pspan->v;

    zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
    // we count on FP exceptions being turned off to avoid range problems
    izi = (int) (zi * 0x8000 * 0x10000);

#if QENGINE_SCAN_AVX2
    if (scan_avx2_available && count >= 8) {
      D_DrawZSpanAVX2(pdest, count, izi, izistep);
      continue;
    }
#endif

    while (count > 0) {
      *pdest++ = izi >> 16;
      izi += izistep;
      count--;
    }
  } while ((pspan = pspan->pnext) != NULL);
}
