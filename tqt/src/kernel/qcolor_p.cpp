/****************************************************************************
**
** Named color support for non-X platforms.
** The color names have been borrowed from X.
**
** Created : 000228
**
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "ntqglobal.h"
#if defined(Q_CC_BOR)
// needed for qsort() because of a std namespace problem on Borland
#include "qplatformdefs.h"
#endif

#include "ntqcolor.h"

#ifndef TQT_NO_COLORNAMES

#include <stdlib.h>

#undef TQRGB
#define TQRGB(r,g,b) (r*65536 + g*256 + b)

const int rgbTblSize = 657;

static const struct RGBData {
    uint  value;
    const char *name;
} rgbTbl[] = {
  { TQRGB(240,248,255),	"aliceblue" },
  { TQRGB(250,235,215),	"antiquewhite" },
  { TQRGB(255,239,219),	"antiquewhite1" },
  { TQRGB(238,223,204),	"antiquewhite2" },
  { TQRGB(205,192,176),	"antiquewhite3" },
  { TQRGB(139,131,120),	"antiquewhite4" },
  { TQRGB(127,255,212),	"aquamarine" },
  { TQRGB(127,255,212),	"aquamarine1" },
  { TQRGB(118,238,198),	"aquamarine2" },
  { TQRGB(102,205,170),	"aquamarine3" },
  { TQRGB( 69,139,116),	"aquamarine4" },
  { TQRGB(240,255,255),	"azure" },
  { TQRGB(240,255,255),	"azure1" },
  { TQRGB(224,238,238),	"azure2" },
  { TQRGB(193,205,205),	"azure3" },
  { TQRGB(131,139,139),	"azure4" },
  { TQRGB(245,245,220),	"beige" },
  { TQRGB(255,228,196),	"bisque" },
  { TQRGB(255,228,196),	"bisque1" },
  { TQRGB(238,213,183),	"bisque2" },
  { TQRGB(205,183,158),	"bisque3" },
  { TQRGB(139,125,107),	"bisque4" },
  { TQRGB(  0,  0,  0),	"black" },
  { TQRGB(255,235,205),	"blanchedalmond" },
  { TQRGB(  0,  0,255),	"blue" },
  { TQRGB(  0,  0,255),	"blue1" },
  { TQRGB(  0,  0,238),	"blue2" },
  { TQRGB(  0,  0,205),	"blue3" },
  { TQRGB(  0,  0,139),	"blue4" },
  { TQRGB(138, 43,226),	"blueviolet" },
  { TQRGB(165, 42, 42),	"brown" },
  { TQRGB(255, 64, 64),	"brown1" },
  { TQRGB(238, 59, 59),	"brown2" },
  { TQRGB(205, 51, 51),	"brown3" },
  { TQRGB(139, 35, 35),	"brown4" },
  { TQRGB(222,184,135),	"burlywood" },
  { TQRGB(255,211,155),	"burlywood1" },
  { TQRGB(238,197,145),	"burlywood2" },
  { TQRGB(205,170,125),	"burlywood3" },
  { TQRGB(139,115, 85),	"burlywood4" },
  { TQRGB( 95,158,160),	"cadetblue" },
  { TQRGB(152,245,255),	"cadetblue1" },
  { TQRGB(142,229,238),	"cadetblue2" },
  { TQRGB(122,197,205),	"cadetblue3" },
  { TQRGB( 83,134,139),	"cadetblue4" },
  { TQRGB(127,255,  0),	"chartreuse" },
  { TQRGB(127,255,  0),	"chartreuse1" },
  { TQRGB(118,238,  0),	"chartreuse2" },
  { TQRGB(102,205,  0),	"chartreuse3" },
  { TQRGB( 69,139,  0),	"chartreuse4" },
  { TQRGB(210,105, 30),	"chocolate" },
  { TQRGB(255,127, 36),	"chocolate1" },
  { TQRGB(238,118, 33),	"chocolate2" },
  { TQRGB(205,102, 29),	"chocolate3" },
  { TQRGB(139, 69, 19),	"chocolate4" },
  { TQRGB(255,127, 80),	"coral" },
  { TQRGB(255,114, 86),	"coral1" },
  { TQRGB(238,106, 80),	"coral2" },
  { TQRGB(205, 91, 69),	"coral3" },
  { TQRGB(139, 62, 47),	"coral4" },
  { TQRGB(100,149,237),	"cornflowerblue" },
  { TQRGB(255,248,220),	"cornsilk" },
  { TQRGB(255,248,220),	"cornsilk1" },
  { TQRGB(238,232,205),	"cornsilk2" },
  { TQRGB(205,200,177),	"cornsilk3" },
  { TQRGB(139,136,120),	"cornsilk4" },
  { TQRGB(  0,255,255),	"cyan" },
  { TQRGB(  0,255,255),	"cyan1" },
  { TQRGB(  0,238,238),	"cyan2" },
  { TQRGB(  0,205,205),	"cyan3" },
  { TQRGB(  0,139,139),	"cyan4" },
  { TQRGB(  0,  0,139),	"darkblue" },
  { TQRGB(  0,139,139),	"darkcyan" },
  { TQRGB(184,134, 11),	"darkgoldenrod" },
  { TQRGB(255,185, 15),	"darkgoldenrod1" },
  { TQRGB(238,173, 14),	"darkgoldenrod2" },
  { TQRGB(205,149, 12),	"darkgoldenrod3" },
  { TQRGB(139,101,  8),	"darkgoldenrod4" },
  { TQRGB(169,169,169),	"darkgray" },
  { TQRGB(  0,100,  0),	"darkgreen" },
  { TQRGB(169,169,169),	"darkgrey" },
  { TQRGB(189,183,107),	"darkkhaki" },
  { TQRGB(139,  0,139),	"darkmagenta" },
  { TQRGB( 85,107, 47),	"darkolivegreen" },
  { TQRGB(202,255,112),	"darkolivegreen1" },
  { TQRGB(188,238,104),	"darkolivegreen2" },
  { TQRGB(162,205, 90),	"darkolivegreen3" },
  { TQRGB(110,139, 61),	"darkolivegreen4" },
  { TQRGB(255,140,  0),	"darkorange" },
  { TQRGB(255,127,  0),	"darkorange1" },
  { TQRGB(238,118,  0),	"darkorange2" },
  { TQRGB(205,102,  0),	"darkorange3" },
  { TQRGB(139, 69,  0),	"darkorange4" },
  { TQRGB(153, 50,204),	"darkorchid" },
  { TQRGB(191, 62,255),	"darkorchid1" },
  { TQRGB(178, 58,238),	"darkorchid2" },
  { TQRGB(154, 50,205),	"darkorchid3" },
  { TQRGB(104, 34,139),	"darkorchid4" },
  { TQRGB(139,  0,  0),	"darkred" },
  { TQRGB(233,150,122),	"darksalmon" },
  { TQRGB(143,188,143),	"darkseagreen" },
  { TQRGB(193,255,193),	"darkseagreen1" },
  { TQRGB(180,238,180),	"darkseagreen2" },
  { TQRGB(155,205,155),	"darkseagreen3" },
  { TQRGB(105,139,105),	"darkseagreen4" },
  { TQRGB( 72, 61,139),	"darkslateblue" },
  { TQRGB( 47, 79, 79),	"darkslategray" },
  { TQRGB(151,255,255),	"darkslategray1" },
  { TQRGB(141,238,238),	"darkslategray2" },
  { TQRGB(121,205,205),	"darkslategray3" },
  { TQRGB( 82,139,139),	"darkslategray4" },
  { TQRGB( 47, 79, 79),	"darkslategrey" },
  { TQRGB(  0,206,209),	"darkturquoise" },
  { TQRGB(148,  0,211),	"darkviolet" },
  { TQRGB(255, 20,147),	"deeppink" },
  { TQRGB(255, 20,147),	"deeppink1" },
  { TQRGB(238, 18,137),	"deeppink2" },
  { TQRGB(205, 16,118),	"deeppink3" },
  { TQRGB(139, 10, 80),	"deeppink4" },
  { TQRGB(  0,191,255),	"deepskyblue" },
  { TQRGB(  0,191,255),	"deepskyblue1" },
  { TQRGB(  0,178,238),	"deepskyblue2" },
  { TQRGB(  0,154,205),	"deepskyblue3" },
  { TQRGB(  0,104,139),	"deepskyblue4" },
  { TQRGB(105,105,105),	"dimgray" },
  { TQRGB(105,105,105),	"dimgrey" },
  { TQRGB( 30,144,255),	"dodgerblue" },
  { TQRGB( 30,144,255),	"dodgerblue1" },
  { TQRGB( 28,134,238),	"dodgerblue2" },
  { TQRGB( 24,116,205),	"dodgerblue3" },
  { TQRGB( 16, 78,139),	"dodgerblue4" },
  { TQRGB(178, 34, 34),	"firebrick" },
  { TQRGB(255, 48, 48),	"firebrick1" },
  { TQRGB(238, 44, 44),	"firebrick2" },
  { TQRGB(205, 38, 38),	"firebrick3" },
  { TQRGB(139, 26, 26),	"firebrick4" },
  { TQRGB(255,250,240),	"floralwhite" },
  { TQRGB( 34,139, 34),	"forestgreen" },
  { TQRGB(220,220,220),	"gainsboro" },
  { TQRGB(248,248,255),	"ghostwhite" },
  { TQRGB(255,215,  0),	"gold" },
  { TQRGB(255,215,  0),	"gold1" },
  { TQRGB(238,201,  0),	"gold2" },
  { TQRGB(205,173,  0),	"gold3" },
  { TQRGB(139,117,  0),	"gold4" },
  { TQRGB(218,165, 32),	"goldenrod" },
  { TQRGB(255,193, 37),	"goldenrod1" },
  { TQRGB(238,180, 34),	"goldenrod2" },
  { TQRGB(205,155, 29),	"goldenrod3" },
  { TQRGB(139,105, 20),	"goldenrod4" },
  { TQRGB(190,190,190),	"gray" },
  { TQRGB(  0,  0,  0),	"gray0" },
  { TQRGB(  3,  3,  3),	"gray1" },
  { TQRGB( 26, 26, 26),	"gray10" },
  { TQRGB(255,255,255),	"gray100" },
  { TQRGB( 28, 28, 28),	"gray11" },
  { TQRGB( 31, 31, 31),	"gray12" },
  { TQRGB( 33, 33, 33),	"gray13" },
  { TQRGB( 36, 36, 36),	"gray14" },
  { TQRGB( 38, 38, 38),	"gray15" },
  { TQRGB( 41, 41, 41),	"gray16" },
  { TQRGB( 43, 43, 43),	"gray17" },
  { TQRGB( 46, 46, 46),	"gray18" },
  { TQRGB( 48, 48, 48),	"gray19" },
  { TQRGB(  5,  5,  5),	"gray2" },
  { TQRGB( 51, 51, 51),	"gray20" },
  { TQRGB( 54, 54, 54),	"gray21" },
  { TQRGB( 56, 56, 56),	"gray22" },
  { TQRGB( 59, 59, 59),	"gray23" },
  { TQRGB( 61, 61, 61),	"gray24" },
  { TQRGB( 64, 64, 64),	"gray25" },
  { TQRGB( 66, 66, 66),	"gray26" },
  { TQRGB( 69, 69, 69),	"gray27" },
  { TQRGB( 71, 71, 71),	"gray28" },
  { TQRGB( 74, 74, 74),	"gray29" },
  { TQRGB(  8,  8,  8),	"gray3" },
  { TQRGB( 77, 77, 77),	"gray30" },
  { TQRGB( 79, 79, 79),	"gray31" },
  { TQRGB( 82, 82, 82),	"gray32" },
  { TQRGB( 84, 84, 84),	"gray33" },
  { TQRGB( 87, 87, 87),	"gray34" },
  { TQRGB( 89, 89, 89),	"gray35" },
  { TQRGB( 92, 92, 92),	"gray36" },
  { TQRGB( 94, 94, 94),	"gray37" },
  { TQRGB( 97, 97, 97),	"gray38" },
  { TQRGB( 99, 99, 99),	"gray39" },
  { TQRGB( 10, 10, 10),	"gray4" },
  { TQRGB(102,102,102),	"gray40" },
  { TQRGB(105,105,105),	"gray41" },
  { TQRGB(107,107,107),	"gray42" },
  { TQRGB(110,110,110),	"gray43" },
  { TQRGB(112,112,112),	"gray44" },
  { TQRGB(115,115,115),	"gray45" },
  { TQRGB(117,117,117),	"gray46" },
  { TQRGB(120,120,120),	"gray47" },
  { TQRGB(122,122,122),	"gray48" },
  { TQRGB(125,125,125),	"gray49" },
  { TQRGB( 13, 13, 13),	"gray5" },
  { TQRGB(127,127,127),	"gray50" },
  { TQRGB(130,130,130),	"gray51" },
  { TQRGB(133,133,133),	"gray52" },
  { TQRGB(135,135,135),	"gray53" },
  { TQRGB(138,138,138),	"gray54" },
  { TQRGB(140,140,140),	"gray55" },
  { TQRGB(143,143,143),	"gray56" },
  { TQRGB(145,145,145),	"gray57" },
  { TQRGB(148,148,148),	"gray58" },
  { TQRGB(150,150,150),	"gray59" },
  { TQRGB( 15, 15, 15),	"gray6" },
  { TQRGB(153,153,153),	"gray60" },
  { TQRGB(156,156,156),	"gray61" },
  { TQRGB(158,158,158),	"gray62" },
  { TQRGB(161,161,161),	"gray63" },
  { TQRGB(163,163,163),	"gray64" },
  { TQRGB(166,166,166),	"gray65" },
  { TQRGB(168,168,168),	"gray66" },
  { TQRGB(171,171,171),	"gray67" },
  { TQRGB(173,173,173),	"gray68" },
  { TQRGB(176,176,176),	"gray69" },
  { TQRGB( 18, 18, 18),	"gray7" },
  { TQRGB(179,179,179),	"gray70" },
  { TQRGB(181,181,181),	"gray71" },
  { TQRGB(184,184,184),	"gray72" },
  { TQRGB(186,186,186),	"gray73" },
  { TQRGB(189,189,189),	"gray74" },
  { TQRGB(191,191,191),	"gray75" },
  { TQRGB(194,194,194),	"gray76" },
  { TQRGB(196,196,196),	"gray77" },
  { TQRGB(199,199,199),	"gray78" },
  { TQRGB(201,201,201),	"gray79" },
  { TQRGB( 20, 20, 20),	"gray8" },
  { TQRGB(204,204,204),	"gray80" },
  { TQRGB(207,207,207),	"gray81" },
  { TQRGB(209,209,209),	"gray82" },
  { TQRGB(212,212,212),	"gray83" },
  { TQRGB(214,214,214),	"gray84" },
  { TQRGB(217,217,217),	"gray85" },
  { TQRGB(219,219,219),	"gray86" },
  { TQRGB(222,222,222),	"gray87" },
  { TQRGB(224,224,224),	"gray88" },
  { TQRGB(227,227,227),	"gray89" },
  { TQRGB( 23, 23, 23),	"gray9" },
  { TQRGB(229,229,229),	"gray90" },
  { TQRGB(232,232,232),	"gray91" },
  { TQRGB(235,235,235),	"gray92" },
  { TQRGB(237,237,237),	"gray93" },
  { TQRGB(240,240,240),	"gray94" },
  { TQRGB(242,242,242),	"gray95" },
  { TQRGB(245,245,245),	"gray96" },
  { TQRGB(247,247,247),	"gray97" },
  { TQRGB(250,250,250),	"gray98" },
  { TQRGB(252,252,252),	"gray99" },
  { TQRGB(  0,255,  0),	"green" },
  { TQRGB(  0,255,  0),	"green1" },
  { TQRGB(  0,238,  0),	"green2" },
  { TQRGB(  0,205,  0),	"green3" },
  { TQRGB(  0,139,  0),	"green4" },
  { TQRGB(173,255, 47),	"greenyellow" },
  { TQRGB(190,190,190),	"grey" },
  { TQRGB(  0,  0,  0),	"grey0" },
  { TQRGB(  3,  3,  3),	"grey1" },
  { TQRGB( 26, 26, 26),	"grey10" },
  { TQRGB(255,255,255),	"grey100" },
  { TQRGB( 28, 28, 28),	"grey11" },
  { TQRGB( 31, 31, 31),	"grey12" },
  { TQRGB( 33, 33, 33),	"grey13" },
  { TQRGB( 36, 36, 36),	"grey14" },
  { TQRGB( 38, 38, 38),	"grey15" },
  { TQRGB( 41, 41, 41),	"grey16" },
  { TQRGB( 43, 43, 43),	"grey17" },
  { TQRGB( 46, 46, 46),	"grey18" },
  { TQRGB( 48, 48, 48),	"grey19" },
  { TQRGB(  5,  5,  5),	"grey2" },
  { TQRGB( 51, 51, 51),	"grey20" },
  { TQRGB( 54, 54, 54),	"grey21" },
  { TQRGB( 56, 56, 56),	"grey22" },
  { TQRGB( 59, 59, 59),	"grey23" },
  { TQRGB( 61, 61, 61),	"grey24" },
  { TQRGB( 64, 64, 64),	"grey25" },
  { TQRGB( 66, 66, 66),	"grey26" },
  { TQRGB( 69, 69, 69),	"grey27" },
  { TQRGB( 71, 71, 71),	"grey28" },
  { TQRGB( 74, 74, 74),	"grey29" },
  { TQRGB(  8,  8,  8),	"grey3" },
  { TQRGB( 77, 77, 77),	"grey30" },
  { TQRGB( 79, 79, 79),	"grey31" },
  { TQRGB( 82, 82, 82),	"grey32" },
  { TQRGB( 84, 84, 84),	"grey33" },
  { TQRGB( 87, 87, 87),	"grey34" },
  { TQRGB( 89, 89, 89),	"grey35" },
  { TQRGB( 92, 92, 92),	"grey36" },
  { TQRGB( 94, 94, 94),	"grey37" },
  { TQRGB( 97, 97, 97),	"grey38" },
  { TQRGB( 99, 99, 99),	"grey39" },
  { TQRGB( 10, 10, 10),	"grey4" },
  { TQRGB(102,102,102),	"grey40" },
  { TQRGB(105,105,105),	"grey41" },
  { TQRGB(107,107,107),	"grey42" },
  { TQRGB(110,110,110),	"grey43" },
  { TQRGB(112,112,112),	"grey44" },
  { TQRGB(115,115,115),	"grey45" },
  { TQRGB(117,117,117),	"grey46" },
  { TQRGB(120,120,120),	"grey47" },
  { TQRGB(122,122,122),	"grey48" },
  { TQRGB(125,125,125),	"grey49" },
  { TQRGB( 13, 13, 13),	"grey5" },
  { TQRGB(127,127,127),	"grey50" },
  { TQRGB(130,130,130),	"grey51" },
  { TQRGB(133,133,133),	"grey52" },
  { TQRGB(135,135,135),	"grey53" },
  { TQRGB(138,138,138),	"grey54" },
  { TQRGB(140,140,140),	"grey55" },
  { TQRGB(143,143,143),	"grey56" },
  { TQRGB(145,145,145),	"grey57" },
  { TQRGB(148,148,148),	"grey58" },
  { TQRGB(150,150,150),	"grey59" },
  { TQRGB( 15, 15, 15),	"grey6" },
  { TQRGB(153,153,153),	"grey60" },
  { TQRGB(156,156,156),	"grey61" },
  { TQRGB(158,158,158),	"grey62" },
  { TQRGB(161,161,161),	"grey63" },
  { TQRGB(163,163,163),	"grey64" },
  { TQRGB(166,166,166),	"grey65" },
  { TQRGB(168,168,168),	"grey66" },
  { TQRGB(171,171,171),	"grey67" },
  { TQRGB(173,173,173),	"grey68" },
  { TQRGB(176,176,176),	"grey69" },
  { TQRGB( 18, 18, 18),	"grey7" },
  { TQRGB(179,179,179),	"grey70" },
  { TQRGB(181,181,181),	"grey71" },
  { TQRGB(184,184,184),	"grey72" },
  { TQRGB(186,186,186),	"grey73" },
  { TQRGB(189,189,189),	"grey74" },
  { TQRGB(191,191,191),	"grey75" },
  { TQRGB(194,194,194),	"grey76" },
  { TQRGB(196,196,196),	"grey77" },
  { TQRGB(199,199,199),	"grey78" },
  { TQRGB(201,201,201),	"grey79" },
  { TQRGB( 20, 20, 20),	"grey8" },
  { TQRGB(204,204,204),	"grey80" },
  { TQRGB(207,207,207),	"grey81" },
  { TQRGB(209,209,209),	"grey82" },
  { TQRGB(212,212,212),	"grey83" },
  { TQRGB(214,214,214),	"grey84" },
  { TQRGB(217,217,217),	"grey85" },
  { TQRGB(219,219,219),	"grey86" },
  { TQRGB(222,222,222),	"grey87" },
  { TQRGB(224,224,224),	"grey88" },
  { TQRGB(227,227,227),	"grey89" },
  { TQRGB( 23, 23, 23),	"grey9" },
  { TQRGB(229,229,229),	"grey90" },
  { TQRGB(232,232,232),	"grey91" },
  { TQRGB(235,235,235),	"grey92" },
  { TQRGB(237,237,237),	"grey93" },
  { TQRGB(240,240,240),	"grey94" },
  { TQRGB(242,242,242),	"grey95" },
  { TQRGB(245,245,245),	"grey96" },
  { TQRGB(247,247,247),	"grey97" },
  { TQRGB(250,250,250),	"grey98" },
  { TQRGB(252,252,252),	"grey99" },
  { TQRGB(240,255,240),	"honeydew" },
  { TQRGB(240,255,240),	"honeydew1" },
  { TQRGB(224,238,224),	"honeydew2" },
  { TQRGB(193,205,193),	"honeydew3" },
  { TQRGB(131,139,131),	"honeydew4" },
  { TQRGB(255,105,180),	"hotpink" },
  { TQRGB(255,110,180),	"hotpink1" },
  { TQRGB(238,106,167),	"hotpink2" },
  { TQRGB(205, 96,144),	"hotpink3" },
  { TQRGB(139, 58, 98),	"hotpink4" },
  { TQRGB(205, 92, 92),	"indianred" },
  { TQRGB(255,106,106),	"indianred1" },
  { TQRGB(238, 99, 99),	"indianred2" },
  { TQRGB(205, 85, 85),	"indianred3" },
  { TQRGB(139, 58, 58),	"indianred4" },
  { TQRGB(255,255,240),	"ivory" },
  { TQRGB(255,255,240),	"ivory1" },
  { TQRGB(238,238,224),	"ivory2" },
  { TQRGB(205,205,193),	"ivory3" },
  { TQRGB(139,139,131),	"ivory4" },
  { TQRGB(240,230,140),	"khaki" },
  { TQRGB(255,246,143),	"khaki1" },
  { TQRGB(238,230,133),	"khaki2" },
  { TQRGB(205,198,115),	"khaki3" },
  { TQRGB(139,134, 78),	"khaki4" },
  { TQRGB(230,230,250),	"lavender" },
  { TQRGB(255,240,245),	"lavenderblush" },
  { TQRGB(255,240,245),	"lavenderblush1" },
  { TQRGB(238,224,229),	"lavenderblush2" },
  { TQRGB(205,193,197),	"lavenderblush3" },
  { TQRGB(139,131,134),	"lavenderblush4" },
  { TQRGB(124,252,  0),	"lawngreen" },
  { TQRGB(255,250,205),	"lemonchiffon" },
  { TQRGB(255,250,205),	"lemonchiffon1" },
  { TQRGB(238,233,191),	"lemonchiffon2" },
  { TQRGB(205,201,165),	"lemonchiffon3" },
  { TQRGB(139,137,112),	"lemonchiffon4" },
  { TQRGB(173,216,230),	"lightblue" },
  { TQRGB(191,239,255),	"lightblue1" },
  { TQRGB(178,223,238),	"lightblue2" },
  { TQRGB(154,192,205),	"lightblue3" },
  { TQRGB(104,131,139),	"lightblue4" },
  { TQRGB(240,128,128),	"lightcoral" },
  { TQRGB(224,255,255),	"lightcyan" },
  { TQRGB(224,255,255),	"lightcyan1" },
  { TQRGB(209,238,238),	"lightcyan2" },
  { TQRGB(180,205,205),	"lightcyan3" },
  { TQRGB(122,139,139),	"lightcyan4" },
  { TQRGB(238,221,130),	"lightgoldenrod" },
  { TQRGB(255,236,139),	"lightgoldenrod1" },
  { TQRGB(238,220,130),	"lightgoldenrod2" },
  { TQRGB(205,190,112),	"lightgoldenrod3" },
  { TQRGB(139,129, 76),	"lightgoldenrod4" },
  { TQRGB(250,250,210),	"lightgoldenrodyellow" },
  { TQRGB(211,211,211),	"lightgray" },
  { TQRGB(144,238,144),	"lightgreen" },
  { TQRGB(211,211,211),	"lightgrey" },
  { TQRGB(255,182,193),	"lightpink" },
  { TQRGB(255,174,185),	"lightpink1" },
  { TQRGB(238,162,173),	"lightpink2" },
  { TQRGB(205,140,149),	"lightpink3" },
  { TQRGB(139, 95,101),	"lightpink4" },
  { TQRGB(255,160,122),	"lightsalmon" },
  { TQRGB(255,160,122),	"lightsalmon1" },
  { TQRGB(238,149,114),	"lightsalmon2" },
  { TQRGB(205,129, 98),	"lightsalmon3" },
  { TQRGB(139, 87, 66),	"lightsalmon4" },
  { TQRGB( 32,178,170),	"lightseagreen" },
  { TQRGB(135,206,250),	"lightskyblue" },
  { TQRGB(176,226,255),	"lightskyblue1" },
  { TQRGB(164,211,238),	"lightskyblue2" },
  { TQRGB(141,182,205),	"lightskyblue3" },
  { TQRGB( 96,123,139),	"lightskyblue4" },
  { TQRGB(132,112,255),	"lightslateblue" },
  { TQRGB(119,136,153),	"lightslategray" },
  { TQRGB(119,136,153),	"lightslategrey" },
  { TQRGB(176,196,222),	"lightsteelblue" },
  { TQRGB(202,225,255),	"lightsteelblue1" },
  { TQRGB(188,210,238),	"lightsteelblue2" },
  { TQRGB(162,181,205),	"lightsteelblue3" },
  { TQRGB(110,123,139),	"lightsteelblue4" },
  { TQRGB(255,255,224),	"lightyellow" },
  { TQRGB(255,255,224),	"lightyellow1" },
  { TQRGB(238,238,209),	"lightyellow2" },
  { TQRGB(205,205,180),	"lightyellow3" },
  { TQRGB(139,139,122),	"lightyellow4" },
  { TQRGB( 50,205, 50),	"limegreen" },
  { TQRGB(250,240,230),	"linen" },
  { TQRGB(255,  0,255),	"magenta" },
  { TQRGB(255,  0,255),	"magenta1" },
  { TQRGB(238,  0,238),	"magenta2" },
  { TQRGB(205,  0,205),	"magenta3" },
  { TQRGB(139,  0,139),	"magenta4" },
  { TQRGB(176, 48, 96),	"maroon" },
  { TQRGB(255, 52,179),	"maroon1" },
  { TQRGB(238, 48,167),	"maroon2" },
  { TQRGB(205, 41,144),	"maroon3" },
  { TQRGB(139, 28, 98),	"maroon4" },
  { TQRGB(102,205,170),	"mediumaquamarine" },
  { TQRGB(  0,  0,205),	"mediumblue" },
  { TQRGB(186, 85,211),	"mediumorchid" },
  { TQRGB(224,102,255),	"mediumorchid1" },
  { TQRGB(209, 95,238),	"mediumorchid2" },
  { TQRGB(180, 82,205),	"mediumorchid3" },
  { TQRGB(122, 55,139),	"mediumorchid4" },
  { TQRGB(147,112,219),	"mediumpurple" },
  { TQRGB(171,130,255),	"mediumpurple1" },
  { TQRGB(159,121,238),	"mediumpurple2" },
  { TQRGB(137,104,205),	"mediumpurple3" },
  { TQRGB( 93, 71,139),	"mediumpurple4" },
  { TQRGB( 60,179,113),	"mediumseagreen" },
  { TQRGB(123,104,238),	"mediumslateblue" },
  { TQRGB(  0,250,154),	"mediumspringgreen" },
  { TQRGB( 72,209,204),	"mediumturquoise" },
  { TQRGB(199, 21,133),	"mediumvioletred" },
  { TQRGB( 25, 25,112),	"midnightblue" },
  { TQRGB(245,255,250),	"mintcream" },
  { TQRGB(255,228,225),	"mistyrose" },
  { TQRGB(255,228,225),	"mistyrose1" },
  { TQRGB(238,213,210),	"mistyrose2" },
  { TQRGB(205,183,181),	"mistyrose3" },
  { TQRGB(139,125,123),	"mistyrose4" },
  { TQRGB(255,228,181),	"moccasin" },
  { TQRGB(255,222,173),	"navajowhite" },
  { TQRGB(255,222,173),	"navajowhite1" },
  { TQRGB(238,207,161),	"navajowhite2" },
  { TQRGB(205,179,139),	"navajowhite3" },
  { TQRGB(139,121, 94),	"navajowhite4" },
  { TQRGB(  0,  0,128),	"navy" },
  { TQRGB(  0,  0,128),	"navyblue" },
  { TQRGB(253,245,230),	"oldlace" },
  { TQRGB(107,142, 35),	"olivedrab" },
  { TQRGB(192,255, 62),	"olivedrab1" },
  { TQRGB(179,238, 58),	"olivedrab2" },
  { TQRGB(154,205, 50),	"olivedrab3" },
  { TQRGB(105,139, 34),	"olivedrab4" },
  { TQRGB(255,165,  0),	"orange" },
  { TQRGB(255,165,  0),	"orange1" },
  { TQRGB(238,154,  0),	"orange2" },
  { TQRGB(205,133,  0),	"orange3" },
  { TQRGB(139, 90,  0),	"orange4" },
  { TQRGB(255, 69,  0),	"orangered" },
  { TQRGB(255, 69,  0),	"orangered1" },
  { TQRGB(238, 64,  0),	"orangered2" },
  { TQRGB(205, 55,  0),	"orangered3" },
  { TQRGB(139, 37,  0),	"orangered4" },
  { TQRGB(218,112,214),	"orchid" },
  { TQRGB(255,131,250),	"orchid1" },
  { TQRGB(238,122,233),	"orchid2" },
  { TQRGB(205,105,201),	"orchid3" },
  { TQRGB(139, 71,137),	"orchid4" },
  { TQRGB(238,232,170),	"palegoldenrod" },
  { TQRGB(152,251,152),	"palegreen" },
  { TQRGB(154,255,154),	"palegreen1" },
  { TQRGB(144,238,144),	"palegreen2" },
  { TQRGB(124,205,124),	"palegreen3" },
  { TQRGB( 84,139, 84),	"palegreen4" },
  { TQRGB(175,238,238),	"paleturquoise" },
  { TQRGB(187,255,255),	"paleturquoise1" },
  { TQRGB(174,238,238),	"paleturquoise2" },
  { TQRGB(150,205,205),	"paleturquoise3" },
  { TQRGB(102,139,139),	"paleturquoise4" },
  { TQRGB(219,112,147),	"palevioletred" },
  { TQRGB(255,130,171),	"palevioletred1" },
  { TQRGB(238,121,159),	"palevioletred2" },
  { TQRGB(205,104,137),	"palevioletred3" },
  { TQRGB(139, 71, 93),	"palevioletred4" },
  { TQRGB(255,239,213),	"papayawhip" },
  { TQRGB(255,218,185),	"peachpuff" },
  { TQRGB(255,218,185),	"peachpuff1" },
  { TQRGB(238,203,173),	"peachpuff2" },
  { TQRGB(205,175,149),	"peachpuff3" },
  { TQRGB(139,119,101),	"peachpuff4" },
  { TQRGB(205,133, 63),	"peru" },
  { TQRGB(255,192,203),	"pink" },
  { TQRGB(255,181,197),	"pink1" },
  { TQRGB(238,169,184),	"pink2" },
  { TQRGB(205,145,158),	"pink3" },
  { TQRGB(139, 99,108),	"pink4" },
  { TQRGB(221,160,221),	"plum" },
  { TQRGB(255,187,255),	"plum1" },
  { TQRGB(238,174,238),	"plum2" },
  { TQRGB(205,150,205),	"plum3" },
  { TQRGB(139,102,139),	"plum4" },
  { TQRGB(176,224,230),	"powderblue" },
  { TQRGB(160, 32,240),	"purple" },
  { TQRGB(155, 48,255),	"purple1" },
  { TQRGB(145, 44,238),	"purple2" },
  { TQRGB(125, 38,205),	"purple3" },
  { TQRGB( 85, 26,139),	"purple4" },
  { TQRGB(255,  0,  0),	"red" },
  { TQRGB(255,  0,  0),	"red1" },
  { TQRGB(238,  0,  0),	"red2" },
  { TQRGB(205,  0,  0),	"red3" },
  { TQRGB(139,  0,  0),	"red4" },
  { TQRGB(188,143,143),	"rosybrown" },
  { TQRGB(255,193,193),	"rosybrown1" },
  { TQRGB(238,180,180),	"rosybrown2" },
  { TQRGB(205,155,155),	"rosybrown3" },
  { TQRGB(139,105,105),	"rosybrown4" },
  { TQRGB( 65,105,225),	"royalblue" },
  { TQRGB( 72,118,255),	"royalblue1" },
  { TQRGB( 67,110,238),	"royalblue2" },
  { TQRGB( 58, 95,205),	"royalblue3" },
  { TQRGB( 39, 64,139),	"royalblue4" },
  { TQRGB(139, 69, 19),	"saddlebrown" },
  { TQRGB(250,128,114),	"salmon" },
  { TQRGB(255,140,105),	"salmon1" },
  { TQRGB(238,130, 98),	"salmon2" },
  { TQRGB(205,112, 84),	"salmon3" },
  { TQRGB(139, 76, 57),	"salmon4" },
  { TQRGB(244,164, 96),	"sandybrown" },
  { TQRGB( 46,139, 87),	"seagreen" },
  { TQRGB( 84,255,159),	"seagreen1" },
  { TQRGB( 78,238,148),	"seagreen2" },
  { TQRGB( 67,205,128),	"seagreen3" },
  { TQRGB( 46,139, 87),	"seagreen4" },
  { TQRGB(255,245,238),	"seashell" },
  { TQRGB(255,245,238),	"seashell1" },
  { TQRGB(238,229,222),	"seashell2" },
  { TQRGB(205,197,191),	"seashell3" },
  { TQRGB(139,134,130),	"seashell4" },
  { TQRGB(160, 82, 45),	"sienna" },
  { TQRGB(255,130, 71),	"sienna1" },
  { TQRGB(238,121, 66),	"sienna2" },
  { TQRGB(205,104, 57),	"sienna3" },
  { TQRGB(139, 71, 38),	"sienna4" },
  { TQRGB(135,206,235),	"skyblue" },
  { TQRGB(135,206,255),	"skyblue1" },
  { TQRGB(126,192,238),	"skyblue2" },
  { TQRGB(108,166,205),	"skyblue3" },
  { TQRGB( 74,112,139),	"skyblue4" },
  { TQRGB(106, 90,205),	"slateblue" },
  { TQRGB(131,111,255),	"slateblue1" },
  { TQRGB(122,103,238),	"slateblue2" },
  { TQRGB(105, 89,205),	"slateblue3" },
  { TQRGB( 71, 60,139),	"slateblue4" },
  { TQRGB(112,128,144),	"slategray" },
  { TQRGB(198,226,255),	"slategray1" },
  { TQRGB(185,211,238),	"slategray2" },
  { TQRGB(159,182,205),	"slategray3" },
  { TQRGB(108,123,139),	"slategray4" },
  { TQRGB(112,128,144),	"slategrey" },
  { TQRGB(255,250,250),	"snow" },
  { TQRGB(255,250,250),	"snow1" },
  { TQRGB(238,233,233),	"snow2" },
  { TQRGB(205,201,201),	"snow3" },
  { TQRGB(139,137,137),	"snow4" },
  { TQRGB(  0,255,127),	"springgreen" },
  { TQRGB(  0,255,127),	"springgreen1" },
  { TQRGB(  0,238,118),	"springgreen2" },
  { TQRGB(  0,205,102),	"springgreen3" },
  { TQRGB(  0,139, 69),	"springgreen4" },
  { TQRGB( 70,130,180),	"steelblue" },
  { TQRGB( 99,184,255),	"steelblue1" },
  { TQRGB( 92,172,238),	"steelblue2" },
  { TQRGB( 79,148,205),	"steelblue3" },
  { TQRGB( 54,100,139),	"steelblue4" },
  { TQRGB(210,180,140),	"tan" },
  { TQRGB(255,165, 79),	"tan1" },
  { TQRGB(238,154, 73),	"tan2" },
  { TQRGB(205,133, 63),	"tan3" },
  { TQRGB(139, 90, 43),	"tan4" },
  { TQRGB(216,191,216),	"thistle" },
  { TQRGB(255,225,255),	"thistle1" },
  { TQRGB(238,210,238),	"thistle2" },
  { TQRGB(205,181,205),	"thistle3" },
  { TQRGB(139,123,139),	"thistle4" },
  { TQRGB(255, 99, 71),	"tomato" },
  { TQRGB(255, 99, 71),	"tomato1" },
  { TQRGB(238, 92, 66),	"tomato2" },
  { TQRGB(205, 79, 57),	"tomato3" },
  { TQRGB(139, 54, 38),	"tomato4" },
  { TQRGB( 64,224,208),	"turquoise" },
  { TQRGB(  0,245,255),	"turquoise1" },
  { TQRGB(  0,229,238),	"turquoise2" },
  { TQRGB(  0,197,205),	"turquoise3" },
  { TQRGB(  0,134,139),	"turquoise4" },
  { TQRGB(238,130,238),	"violet" },
  { TQRGB(208, 32,144),	"violetred" },
  { TQRGB(255, 62,150),	"violetred1" },
  { TQRGB(238, 58,140),	"violetred2" },
  { TQRGB(205, 50,120),	"violetred3" },
  { TQRGB(139, 34, 82),	"violetred4" },
  { TQRGB(245,222,179),	"wheat" },
  { TQRGB(255,231,186),	"wheat1" },
  { TQRGB(238,216,174),	"wheat2" },
  { TQRGB(205,186,150),	"wheat3" },
  { TQRGB(139,126,102),	"wheat4" },
  { TQRGB(255,255,255),	"white" },
  { TQRGB(245,245,245),	"whitesmoke" },
  { TQRGB(255,255,  0),	"yellow" },
  { TQRGB(255,255,  0),	"yellow1" },
  { TQRGB(238,238,  0),	"yellow2" },
  { TQRGB(205,205,  0),	"yellow3" },
  { TQRGB(139,139,  0),	"yellow4" },
  { TQRGB(154,205, 50),	"yellowgreen" } };

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int __cdecl rgb_cmp( const void *d1, const void *d2 )
#else
static int rgb_cmp( const void *d1, const void *d2 )
#endif
{
    return tqstricmp( ((RGBData *)d1)->name, ((RGBData *)d2)->name );
}

#if defined(Q_C_CALLBACKS)
}
#endif

bool qt_get_named_rgb( const char *name, TQRgb* rgb )
{
    TQ_LONG len = strlen(name)+1;
    char *name_no_space = (char *)malloc(len);
    for(TQ_LONG o=0,i=0; i < len; i++) {
	if(name[i] != '\t' && name[i] != ' ')
	    name_no_space[o++] = name[i];
    }

    RGBData x;
    x.name = name_no_space;
    // Funtion bsearch() is supposed to be
    // void *bsearch(const void *key, const void *base, ...
    // So why (char*)? Are there broken bsearch() declarations out there?
    RGBData *r = (RGBData*)bsearch((char*)&x, (char*)rgbTbl, rgbTblSize,
				   sizeof(RGBData), rgb_cmp);
    free(name_no_space);
    if ( r ) {
	*rgb = r->value;
	return TRUE;
    } else {
	return FALSE;
    }
}

uint qt_get_rgb_val( const char *name )
{
    TQRgb r = 0;
    qt_get_named_rgb(name,&r);
    return r;
}
#ifndef TQT_NO_STRINGLIST
TQStringList TQColor::colorNames()
{
    int i = 0;
    TQStringList lst;
    for ( i = 0; i < rgbTblSize; i++ )
	lst << rgbTbl[i].name;

    return lst;
}
#endif
#else

bool qt_get_named_rgb( const char *, TQRgb* )
{
    return FALSE;
}

uint qt_get_rgb_val( const char * )
{
    return 0;
}
#ifndef TQT_NO_STRINGLIST
TQStringList TQColor::colorNames()
{
    return TQStringList();
}
#endif
#endif // TQT_NO_COLORNAMES
