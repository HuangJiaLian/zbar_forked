/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#ifndef _QRCODE_H_
#define _QRCODE_H_

#include <zbar.h>

typedef struct qr_reader qr_reader;

typedef int qr_point[2];
/*
问:
typedef int array[2]
表示声明一个array来表示int [2]
但typedef的用法不是第一个是原类型 第二个是新别名么？那为什么不是typedef int[2] array这样呢？
答:
这只能记住了！你的理解是完全正确的，按规则似乎应该是typedef int[2] array这样，可遗憾的是int[2]
却不是已有的类型，这不就出错了？只好写成int array[2]这样了，在typedef的配合下为int型二元素数组
起了个类型别名叫array！
参考:
https://zhidao.baidu.com/question/1607436659439157907.html

也就是是说在这里，qr_point　这种类型就是int[2] 一个两个元素的数组
*/
typedef struct qr_finder_line qr_finder_line;

/*
  The number of bits of subpel precision to store image coordinates in.
  This helps when estimating positions in low-resolution images, which may have
   a module pitch only a pixel or two wide, making rounding errors matter a
   great deal.
   用于存储图像坐标的子像素精度位数。这有助于在低分辨率图像中估计位置，
   这可能会导致模块间距只有一个像素或两个像素宽度，这使得舍入误差成为难题。
*/
#define QR_FINDER_SUBPREC (2)

/*A line crossing a finder pattern.
  Whether the line is horizontal or vertical is determined by context.
  The offsts to various parts of the finder pattern are as follows:

  跨过定位图案的线．线是水平的还是垂直的，取决于上下文。定位图案的各个部分的偏移如下：
    |*****|     |*****|*****|*****|     |*****|
    |*****|     |*****|*****|*****|     |*****|
       ^        ^                 ^        ^
       |        |                 |        |
       |        |                 |       pos[v]+len+eoffs
       |        |                pos[v]+len
       |       pos[v]
      pos[v]-boffs

  Here v is 0 for horizontal and 1 for vertical lines.*/
  // v为0表示经过定位符的水平线，v为1则表示经过定位符的垂直线
  // 计算机里面画线最容易画的便是这两种线了


struct qr_finder_line {
  /*The location of the upper/left endpoint of the line.
    The left/upper edge of the center section is used, since other lines must
    cross in this region.
    线的上/左端点的位置。
    使用中心部分的左/上边缘，因为其他线必须在此区域中交叉。 
     */
  qr_point pos;

  /*
    The length of the center section.
    This extends to the right/bottom of the center section, since other lines
    must cross in this region.
    中心部分的长度。
    中间部分右侧/底部的扩展，因为其他线路必须跨越此区域。 
     */
  int      len;

  /*The offset to the midpoint of the upper/left section (part of the outside
     ring), or 0 if we couldn't identify the edge of the beginning section.
    We use the midpoint instead of the edge because it can be located more
     reliably.*/
  int      boffs;
  /*The offset to the midpoint of the end section (part of the outside ring),
     or 0 if we couldn't identify the edge of the end section.
    We use the midpoint instead of the edge because it can be located more
     reliably.*/
  int      eoffs;
};

qr_reader *_zbar_qr_create(void);
void _zbar_qr_destroy(qr_reader *reader);
void _zbar_qr_reset(qr_reader *reader);

int _zbar_qr_found_line(qr_reader *reader,
                        int direction,
                        const qr_finder_line *line);
int _zbar_qr_decode(qr_reader *reader,
                    zbar_image_scanner_t *iscn,
                    zbar_image_t *img);
#endif
