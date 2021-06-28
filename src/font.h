#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_WIDTH 5

static const uint8_t font_error_symbol[FONT_WIDTH] = {0xfe, 0xaa, 0x92, 0xaa, 0xfe};

//
// Generated values below (use font_convert.py)
//

static const uint8_t font_data[FONT_WIDTH * 99] = {
  // 0x00000020 - 0x0000005f
  0x00, 0x00, 0x00, 0x00, 0x00, //   (0x00000020) #0
  0x00, 0x00, 0xbe, 0x00, 0x00, // ! (0x00000021) #1
  0x00, 0x0c, 0x00, 0x0c, 0x00, // " (0x00000022) #2
  0x28, 0xfe, 0x28, 0xfe, 0x28, // # (0x00000023) #3
  0x4c, 0x92, 0xfe, 0x92, 0x64, // $ (0x00000024) #4
  0x06, 0xe6, 0x10, 0xce, 0xc0, // % (0x00000025) #5
  0x40, 0xac, 0x92, 0xac, 0x60, // & (0x00000026) #6
  0x00, 0x00, 0x0c, 0x00, 0x00, // ' (0x00000027) #7
  0x00, 0x7c, 0x82, 0x00, 0x00, // ( (0x00000028) #8
  0x00, 0x00, 0x82, 0x7c, 0x00, // ) (0x00000029) #9
  0x00, 0x08, 0x14, 0x08, 0x00, // * (0x0000002a) #10
  0x00, 0x20, 0x70, 0x20, 0x00, // + (0x0000002b) #11
  0x00, 0x00, 0x80, 0x40, 0x00, // , (0x0000002c) #12
  0x00, 0x20, 0x20, 0x20, 0x00, // - (0x0000002d) #13
  0x00, 0x00, 0x80, 0x00, 0x00, // . (0x0000002e) #14
  0x00, 0xc0, 0x38, 0x06, 0x00, // / (0x0000002f) #15
  0x7c, 0xc2, 0xba, 0x86, 0x7c, // 0 (0x00000030) #16
  0x00, 0x8c, 0xfe, 0x80, 0x00, // 1 (0x00000031) #17
  0x84, 0xc2, 0xa2, 0x92, 0x8c, // 2 (0x00000032) #18
  0x44, 0x82, 0x92, 0xaa, 0x44, // 3 (0x00000033) #19
  0x1e, 0x10, 0x10, 0x10, 0xfe, // 4 (0x00000034) #20
  0x4e, 0x92, 0x92, 0x92, 0x62, // 5 (0x00000035) #21
  0x7c, 0x92, 0x92, 0x92, 0x64, // 6 (0x00000036) #22
  0x02, 0x02, 0xe2, 0x12, 0x0e, // 7 (0x00000037) #23
  0x6c, 0x92, 0x92, 0x92, 0x6c, // 8 (0x00000038) #24
  0x4c, 0x92, 0x92, 0x92, 0x7c, // 9 (0x00000039) #25
  0x00, 0x00, 0x48, 0x00, 0x00, // : (0x0000003a) #26
  0x00, 0x80, 0x48, 0x00, 0x00, // ; (0x0000003b) #27
  0x00, 0x20, 0x50, 0x88, 0x00, // < (0x0000003c) #28
  0x00, 0x50, 0x50, 0x50, 0x00, // = (0x0000003d) #29
  0x00, 0x88, 0x50, 0x20, 0x00, // > (0x0000003e) #30
  0x0c, 0x02, 0xa2, 0x12, 0x0c, // ? (0x0000003f) #31
  0x7c, 0x82, 0x9a, 0xa2, 0xbc, // @ (0x00000040) #32
  0xfc, 0x12, 0x12, 0x12, 0xfc, // A (0x00000041) #33
  0xfe, 0x92, 0x92, 0x92, 0x7c, // B (0x00000042) #34
  0x7c, 0x82, 0x82, 0x82, 0x44, // C (0x00000043) #35
  0xfe, 0x82, 0x82, 0x82, 0x7c, // D (0x00000044) #36
  0xfe, 0x92, 0x92, 0x82, 0x82, // E (0x00000045) #37
  0xfe, 0x12, 0x12, 0x12, 0x02, // F (0x00000046) #38
  0x7c, 0x82, 0x92, 0x92, 0x74, // G (0x00000047) #39
  0xfe, 0x10, 0x10, 0x10, 0xfe, // H (0x00000048) #40
  0x82, 0x82, 0xfe, 0x82, 0x82, // I (0x00000049) #41
  0x40, 0x80, 0x82, 0x7e, 0x02, // J (0x0000004a) #42
  0xfe, 0x10, 0x10, 0x28, 0xc6, // K (0x0000004b) #43
  0xfe, 0x80, 0x80, 0x80, 0x80, // L (0x0000004c) #44
  0xfe, 0x08, 0x30, 0x08, 0xfe, // M (0x0000004d) #45
  0xfe, 0x08, 0x10, 0x20, 0xfe, // N (0x0000004e) #46
  0x7c, 0x82, 0x82, 0x82, 0x7c, // O (0x0000004f) #47
  0xfe, 0x12, 0x12, 0x12, 0x0c, // P (0x00000050) #48
  0x7c, 0x82, 0x82, 0x42, 0xbc, // Q (0x00000051) #49
  0xfe, 0x12, 0x12, 0x12, 0xec, // R (0x00000052) #50
  0x4c, 0x92, 0x92, 0x92, 0x64, // S (0x00000053) #51
  0x02, 0x02, 0xfe, 0x02, 0x02, // T (0x00000054) #52
  0x7e, 0x80, 0x80, 0x80, 0x7e, // U (0x00000055) #53
  0x1e, 0x60, 0x80, 0x60, 0x1e, // V (0x00000056) #54
  0x7e, 0x80, 0x7e, 0x80, 0x7e, // W (0x00000057) #55
  0xc6, 0x28, 0x10, 0x28, 0xc6, // X (0x00000058) #56
  0x06, 0x08, 0xf0, 0x08, 0x06, // Y (0x00000059) #57
  0xc2, 0xa2, 0x92, 0x8a, 0x86, // Z (0x0000005a) #58
  0x00, 0xfe, 0x82, 0x00, 0x00, // [ (0x0000005b) #59
  0x00, 0x06, 0x38, 0xc0, 0x00, // \ (0x0000005c) #60
  0x00, 0x00, 0x82, 0xfe, 0x00, // ] (0x0000005d) #61
  0x00, 0x08, 0x04, 0x08, 0x00, // ^ (0x0000005e) #62
  0x00, 0x80, 0x80, 0x80, 0x00, // _ (0x0000005f) #63
  // 0x0000d081 - 0x0000d081
  0xfc, 0x95, 0x94, 0x85, 0x84, // Ё (0x0000d081) #64
  // 0x0000d090 - 0x0000d0af
  0xfc, 0x12, 0x12, 0x12, 0xfc, // А (0x0000d090) #65
  0xfe, 0x92, 0x92, 0x92, 0xe2, // Б (0x0000d091) #66
  0xfe, 0x92, 0x92, 0x92, 0x6c, // В (0x0000d092) #67
  0xfe, 0x02, 0x02, 0x02, 0x02, // Г (0x0000d093) #68
  0xc0, 0x7c, 0x42, 0x42, 0xfe, // Д (0x0000d094) #69
  0xfe, 0x92, 0x92, 0x92, 0x82, // Е (0x0000d095) #70
  0xc6, 0x28, 0xfe, 0x28, 0xc6, // Ж (0x0000d096) #71
  0x44, 0x82, 0x82, 0x92, 0x6c, // З (0x0000d097) #72
  0xfe, 0x40, 0x20, 0x10, 0xfe, // И (0x0000d098) #73
  0xfe, 0x40, 0x23, 0x10, 0xfe, // Й (0x0000d099) #74
  0xfe, 0x10, 0x10, 0x28, 0xc6, // К (0x0000d09a) #75
  0xf8, 0x04, 0x02, 0x02, 0xfe, // Л (0x0000d09b) #76
  0xfe, 0x08, 0x10, 0x08, 0xfe, // М (0x0000d09c) #77
  0xfe, 0x10, 0x10, 0x10, 0xfe, // Н (0x0000d09d) #78
  0x7c, 0x82, 0x82, 0x82, 0x7c, // О (0x0000d09e) #79
  0xfe, 0x02, 0x02, 0x02, 0xfe, // П (0x0000d09f) #80
  0xfe, 0x22, 0x22, 0x22, 0x1c, // Р (0x0000d0a0) #81
  0x7c, 0x82, 0x82, 0x82, 0x82, // С (0x0000d0a1) #82
  0x02, 0x02, 0xfe, 0x02, 0x02, // Т (0x0000d0a2) #83
  0x4e, 0x90, 0x90, 0x90, 0x7e, // У (0x0000d0a3) #84
  0x1c, 0x22, 0xfe, 0x22, 0x1c, // Ф (0x0000d0a4) #85
  0xc6, 0x28, 0x10, 0x28, 0xc6, // Х (0x0000d0a5) #86
  0xfe, 0x80, 0x80, 0xfe, 0x80, // Ц (0x0000d0a6) #87
  0x0e, 0x10, 0x10, 0x10, 0xfe, // Ч (0x0000d0a7) #88
  0xfe, 0x80, 0xfc, 0x80, 0xfe, // Ш (0x0000d0a8) #89
  0x7e, 0x40, 0x7c, 0x40, 0xfe, // Щ (0x0000d0a9) #90
  0x02, 0xfe, 0x90, 0x90, 0x60, // Ъ (0x0000d0aa) #91
  0xfe, 0x90, 0x60, 0x00, 0xfe, // Ы (0x0000d0ab) #92
  0xfe, 0x90, 0x90, 0x90, 0x60, // Ь (0x0000d0ac) #93
  0x44, 0x82, 0x92, 0x92, 0x7c, // Э (0x0000d0ad) #94
  0xfe, 0x10, 0x7c, 0x82, 0x7c, // Ю (0x0000d0ae) #95
  0xec, 0x12, 0x12, 0x12, 0xfe, // Я (0x0000d0af) #96
  // 0x00e296aa - 0x00e296aa
  0x00, 0x00, 0x10, 0x00, 0x00, // ▪ (0x00e296aa) #97
  // 0x00e29c94 - 0x00e29c94
  0x10, 0x20, 0x10, 0x08, 0x04, // ✔ (0x00e29c94) #98
};


#endif /* FONT_H */
