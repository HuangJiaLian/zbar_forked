/*------------------------------------------------------------------------
 *  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <config.h>
#include <stdlib.h>     /* malloc, free, abs */
#include <string.h>     /* memset */

#include <zbar.h>
#include "svg.h"

#ifdef DEBUG_SCANNER
# define DEBUG_LEVEL (DEBUG_SCANNER)
#endif
#include "debug.h"

#ifndef ZBAR_FIXED
# define ZBAR_FIXED 5
#endif
#define ROUND (1 << (ZBAR_FIXED - 1))

/* FIXME add runtime config API for these */
#ifndef ZBAR_SCANNER_THRESH_MIN
# define ZBAR_SCANNER_THRESH_MIN  4
#endif

#ifndef ZBAR_SCANNER_THRESH_INIT_WEIGHT
# define ZBAR_SCANNER_THRESH_INIT_WEIGHT .44
#endif
#define THRESH_INIT ((unsigned)((ZBAR_SCANNER_THRESH_INIT_WEIGHT       \
                                 * (1 << (ZBAR_FIXED + 1)) + 1) / 2))

#ifndef ZBAR_SCANNER_THRESH_FADE
# define ZBAR_SCANNER_THRESH_FADE 8
#endif

#ifndef ZBAR_SCANNER_EWMA_WEIGHT
# define ZBAR_SCANNER_EWMA_WEIGHT .78
#endif
#define EWMA_WEIGHT ((unsigned)((ZBAR_SCANNER_EWMA_WEIGHT              \
                                 * (1 << (ZBAR_FIXED + 1)) + 1) / 2))

/* scanner state */
struct zbar_scanner_s {
    zbar_decoder_t *decoder; /* associated bar width decoder */
    unsigned y1_min_thresh; /* minimum threshold */

    unsigned x;             /* relative scan position of next sample */
    int y0[4];              /* short circular buffer of average intensities */

    int y1_sign;            /* slope at last crossing */
    unsigned y1_thresh;     /* current slope threshold */

    unsigned cur_edge;      /* interpolated position of tracking edge */
    unsigned last_edge;     /* interpolated position of last located edge */
    unsigned width;         /* last element width */
};

zbar_scanner_t *zbar_scanner_create (zbar_decoder_t *dcode)
{
    zbar_scanner_t *scn = malloc(sizeof(zbar_scanner_t));
    scn->decoder = dcode;
    scn->y1_min_thresh = ZBAR_SCANNER_THRESH_MIN;
    zbar_scanner_reset(scn);
    return(scn);
}

void zbar_scanner_destroy (zbar_scanner_t *scn)
{
    free(scn);
}

zbar_symbol_type_t zbar_scanner_reset (zbar_scanner_t *scn)
{
    memset(&scn->x, 0, sizeof(zbar_scanner_t) + (void*)scn - (void*)&scn->x);
    scn->y1_thresh = scn->y1_min_thresh;
    if(scn->decoder)
        zbar_decoder_reset(scn->decoder);
    return(ZBAR_NONE);
}

unsigned zbar_scanner_get_width (const zbar_scanner_t *scn)
{
    return(scn->width);
}

unsigned zbar_scanner_get_edge (const zbar_scanner_t *scn,
                                unsigned offset,
                                int prec)
{
    unsigned edge = scn->last_edge - offset - (1 << ZBAR_FIXED) - ROUND;
    prec = ZBAR_FIXED - prec;
    if(prec > 0)
        return(edge >> prec);
    else if(!prec)
        return(edge);
    else
        return(edge << -prec);
}

zbar_color_t zbar_scanner_get_color (const zbar_scanner_t *scn)
{
    return((scn->y1_sign <= 0) ? ZBAR_SPACE : ZBAR_BAR);
}

// 计算阈值
static inline unsigned calc_thresh (zbar_scanner_t *scn)
{
    /* threshold 1st to improve noise rejection */
    unsigned thresh = scn->y1_thresh;
    if((thresh <= scn->y1_min_thresh) || !scn->width) {
        dprintf(1, " tmin=%d", scn->y1_min_thresh);
        return(scn->y1_min_thresh);
    }
    /* slowly return threshold to min */
    unsigned dx = (scn->x << ZBAR_FIXED) - scn->last_edge;
    unsigned long t = thresh * dx;
    t /= scn->width;
    t /= ZBAR_SCANNER_THRESH_FADE;
    dprintf(1, " thr=%d t=%ld x=%d last=%d.%d (%d)",
            thresh, t, scn->x, scn->last_edge >> ZBAR_FIXED,
            scn->last_edge & ((1 << ZBAR_FIXED) - 1), dx);
    if(thresh > t) {
        thresh -= t;
        if(thresh > scn->y1_min_thresh)
            return(thresh);
    }
    scn->y1_thresh = scn->y1_min_thresh;
    return(scn->y1_min_thresh);
}

static inline zbar_symbol_type_t process_edge (zbar_scanner_t *scn,
                                               int y1)
{
    if(!scn->y1_sign)
        scn->last_edge = scn->cur_edge = (1 << ZBAR_FIXED) + ROUND;
    else if(!scn->last_edge)
        scn->last_edge = scn->cur_edge;

    scn->width = scn->cur_edge - scn->last_edge;
    dprintf(1, " sgn=%d cur=%d.%d w=%d (%s)\n",
            scn->y1_sign, scn->cur_edge >> ZBAR_FIXED,
            scn->cur_edge & ((1 << ZBAR_FIXED) - 1), scn->width,
            ((y1 > 0) ? "SPACE" : "BAR"));
    scn->last_edge = scn->cur_edge;

#if DEBUG_SVG > 1
    svg_path_moveto(SVG_ABS, scn->last_edge - (1 << ZBAR_FIXED) - ROUND, 0);
#endif

    /* pass to decoder */
    if(scn->decoder)
        return(zbar_decode_width(scn->decoder, scn->width));
    return(ZBAR_PARTIAL);
}

inline zbar_symbol_type_t zbar_scanner_flush (zbar_scanner_t *scn)
{
    if(!scn->y1_sign)
        return(ZBAR_NONE);

    unsigned x = (scn->x << ZBAR_FIXED) + ROUND;

    if(scn->cur_edge != x || scn->y1_sign > 0) {
        dprintf(1, "flush0:");
        zbar_symbol_type_t edge = process_edge(scn, -scn->y1_sign);
        scn->cur_edge = x;
        scn->y1_sign = -scn->y1_sign;
        return(edge);
    }

    scn->y1_sign = scn->width = 0;
    if(scn->decoder)
        return(zbar_decode_width(scn->decoder, 0));
    return(ZBAR_PARTIAL);
}
// 通过这个函数　要得到条码的类型 　后续再看如何确定是何种二维码
zbar_symbol_type_t zbar_scanner_new_scan (zbar_scanner_t *scn)
{
    zbar_symbol_type_t edge = ZBAR_NONE;
    while(scn->y1_sign) {
        zbar_symbol_type_t tmp = zbar_scanner_flush(scn);
        if(tmp < 0 || tmp > edge)
            edge = tmp;
    }

    /* reset scanner and associated decoder */
    memset(&scn->x, 0, sizeof(zbar_scanner_t) + (void*)scn - (void*)&scn->x);
    scn->y1_thresh = scn->y1_min_thresh;
    if(scn->decoder)
        zbar_decoder_new_scan(scn->decoder);
    return(edge);
}
// 这个函数究竟做了什么? 
// 参考:
// http://blog.csdn.net/sunflower_boy/article/details/50783179
// http://blog.csdn.net/u013738531/article/details/54574262
// 来完成滤波，阈值，确定边缘，转化成宽度流 
zbar_symbol_type_t zbar_scan_y (zbar_scanner_t *scn,
                                int y)
{
    /* FIXME calc and clip to max y range... */
    // register 代表这个变量是申请放在寄存器中的，为了提高速度
    /* retrieve short value history */
    // 这里的x不代表坐标: 数值先属于(0,width-1)范围 后属于(0,height-1)范围
    register int x = scn->x; // x: relative scan position of next sample 
    #ifdef ENABLE_DEBUG
        printf("DEBUG: scn->x = %d\n", x);
    #endif
    // scn->y0[4]这个小数组在之前已经准备好了?
    register int y0_1 = scn->y0[(x - 1) & 3];
    register int y0_0 = y0_1;
    if(x) {
        /* update weighted moving average */
        // EWMA: exponentially weighted moving average 指数加权平均
        // 参考: 
        // １．http://www.sohu.com/a/197998432_206784
        // ２．http://www.360doc.com/content/11/1116/10/7242176_164724098.shtml
        // 下面这个表达式的运算顺序是: 
        // 1. ((int)((y - y0_1) * EWMA_WEIGHT)) >> ZBAR_FIXE
        // 2. 将上式的结果加y0_0 再赋值给y0_0
        // 它在做什么?
        y0_0 += ((int)((y - y0_1) * EWMA_WEIGHT)) >> ZBAR_FIXED;
        scn->y0[x & 3] = y0_0;
    }
    // 如果x==0, 也就是第一列(行)时
    else
        y0_0 = y0_1 = scn->y0[0] = scn->y0[1] = scn->y0[2] = scn->y0[3] = y;

    register int y0_2 = scn->y0[(x - 2) & 3];
    register int y0_3 = scn->y0[(x - 3) & 3];


    // 在一个很小的范围内求差分
    // 求差分是为了判断是否是边界
    /* 1st differential @ x-1 */
    // 求一阶差分
    register int y1_1 = y0_1 - y0_2;

    // 下面这个括号加得莫名奇妙，我把它注释掉了
    // { 
    // 这个没有理清，感觉这个假设永远不会成立
    // 但事实却有成立的
    register int y1_2 = y0_2 - y0_3;
    if((abs(y1_1) < abs(y1_2)) && 
        ((y1_1 >= 0) == (y1_2 >= 0)))
        {
            #ifdef ENABLE_DEBUG
            printf("DEBUG: I'm IN This IF (y1_1, y1_2) = ( %d, %d ) \n",y1_1, y1_2);
            #endif

            y1_1 = y1_2;
        }

    // }

    /* 2nd differentials @ x-1 & x-2 */
    // 求二阶差分
    register int y2_1 = y0_0 - (y0_1 * 2) + y0_2;
    register int y2_2 = y0_1 - (y0_2 * 2) + y0_3;

    dprintf(1, "scan: x=%d y=%d y0=%d y1=%d y2=%d",
            x, y, y0_1, y1_1, y2_1);

    zbar_symbol_type_t edge = ZBAR_NONE;
    // ???
    /* 2nd zero-crossing is 1st local min/max - could be edge */
    // 如果判断出是边界，那么应该就要将边界信息保存下来

    if((!y2_1 ||
        ((y2_1 > 0) ? y2_2 < 0 : y2_2 > 0)) &&
       (calc_thresh(scn) <= abs(y1_1)))
    {
        /* check for 1st sign change */
        char y1_rev = (scn->y1_sign > 0) ? y1_1 < 0 : y1_1 > 0;
        if(y1_rev)
            /* intensity change reversal - finalize previous edge */
            edge = process_edge(scn, y1_1);

        if(y1_rev || (abs(scn->y1_sign) < abs(y1_1))) {
            scn->y1_sign = y1_1;

            /* adaptive thresholding */
            /* start at multiple of new min/max */
            scn->y1_thresh = (abs(y1_1) * THRESH_INIT + ROUND) >> ZBAR_FIXED;
            dprintf(1, "\tthr=%d", scn->y1_thresh);
            if(scn->y1_thresh < scn->y1_min_thresh)
                scn->y1_thresh = scn->y1_min_thresh;

            /* update current edge */
            int d = y2_1 - y2_2;
            scn->cur_edge = 1 << ZBAR_FIXED;
            if(!d)
                scn->cur_edge >>= 1;
            else if(y2_1)
                /* interpolate zero crossing */
                scn->cur_edge -= ((y2_1 << ZBAR_FIXED) + 1) / d;
            scn->cur_edge += x << ZBAR_FIXED;
            dprintf(1, "\n");
        }
    }
    else
        dprintf(1, "\n");
    /* FIXME add fall-thru pass to decoder after heuristic "idle" period
       (eg, 6-8 * last width) */
    scn->x = x + 1;
    return(edge);
}

/* undocumented API for drawing cutesy debug graphics */
void zbar_scanner_get_state (const zbar_scanner_t *scn,
                             unsigned *x,
                             unsigned *cur_edge,
                             unsigned *last_edge,
                             int *y0,
                             int *y1,
                             int *y2,
                             int *y1_thresh)
{
    register int y0_0 = scn->y0[(scn->x - 1) & 3];
    register int y0_1 = scn->y0[(scn->x - 2) & 3];
    register int y0_2 = scn->y0[(scn->x - 3) & 3];
    if(x) *x = scn->x - 1;
    if(cur_edge) *cur_edge = scn->cur_edge;
    if(last_edge) *last_edge = scn->last_edge;
    if(y0) *y0 = y0_1;
    if(y1) *y1 = y0_1 - y0_2;
    if(y2) *y2 = y0_0 - (y0_1 * 2) + y0_2;
    /* NB not quite accurate (uses updated x) */
    zbar_scanner_t *mut_scn = (zbar_scanner_t*)scn;
    if(y1_thresh) *y1_thresh = calc_thresh(mut_scn);
    dprintf(1, "\n");
}
