/**
 * LVGL v8.x Configuration for ESP32 + GC9A01 240x240 Display
 * 1990s Toyota MR2 SW20 Oil Pressure Gauge
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_CONF_INCLUDE_SIMPLE

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
#define LV_COLOR_16_SWAP 0

/* 1: Enable screen transparency. */
#define LV_COLOR_SCREEN_TRANSP 0

/* Images pixels with this color will not be drawn if they are chroma keyed)*/
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)         /*pure green*/

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB)*/
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
/* 1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()` */
#define LV_MEM_SIZE (48U * 1024U)          /* 48KB for ESP32 */
#define LV_MEM_ADR 0                        /*0: unused*/
#define LV_MEM_AUTO_DEFRAG 1
#else       /*LV_MEM_CUSTOM*/
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC malloc
#define LV_MEM_CUSTOM_FREE free
#define LV_MEM_CUSTOM_REALLOC realloc
#endif     /*LV_MEM_CUSTOM*/

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period. LVG will redraw changed areas with this period time*/
#define LV_DISP_DEF_REFR_PERIOD 30      /*[ms]*/

/* Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings. */
#define LV_DPI_DEF 130     /* For 1.28" 240x240 display */

/*=================
   OPERATING SYSTEM
 *=================*/

#define LV_TICK_CUSTOM 0
#if LV_TICK_CUSTOM
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"         /*Header for the system time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())    /*Expression evaluating to current system time in ms*/
#endif   /*LV_TICK_CUSTOM*/

/*================
 * LOG SETTINGS
 *================*/

/*1: Enable the log module*/
#define LV_USE_LOG 1
#if LV_USE_LOG

/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 * LV_LOG_LEVEL_USER        Only logs added by the user
 * LV_LOG_LEVEL_NONE        Do not log anything */
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/* 1: Print the log with 'printf'; 0: User need to register a callback with `lv_log_register_print_cb()` */
#define LV_LOG_PRINTF 1

#endif  /*LV_USE_LOG*/

/*=================
   DRAWING
 *=================*/

/* 1: Update the rendered image directly to the display. Requires less memory but slower*/
#define LV_DISP_DEF_REFR_DIRECT_MODE 0

/*==================
 * FONT USAGE
 *==================*/

/* Montserrat fonts with ASCII range and some symbols */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 1

/* Demonstrate special features */
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0
#define LV_FONT_SIMSUN_16_CJK            0

/* Pixel perfect monospace fonts */
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0

/* Set the default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable handling large font and/or fonts with a lot of characters.
 * The limit depends on the font size, font face and bpp. */
#define LV_FONT_FMT_TXT_LARGE 0

/*===================
 *  LV_OBJ SETTINGS
 *==================*/

#define LV_USE_OBJ_ID_BUILTIN 1

/*==================
 *  WIDGETS
 *==================*/

#define LV_USE_ARC          1
#define LV_USE_ANIMIMG      0
#define LV_USE_BAR          0
#define LV_USE_BTN          0
#define LV_USE_BTNMATRIX    0
#define LV_USE_CANVAS       0
#define LV_USE_CHECKBOX     0
#define LV_USE_DROPDOWN     0
#define LV_USE_IMG          1
#define LV_USE_LABEL        1
#define LV_USE_LINE         1
#define LV_USE_ROLLER       0
#define LV_USE_SLIDER       0
#define LV_USE_SWITCH       0
#define LV_USE_TEXTAREA     0
#define LV_USE_TABLE        0

/*==================
 * EXTRA COMPONENTS
 *==================*/

/* 1: Enable the extra widgets */
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      1  /* Gauge widget - REQUIRED */
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*==================
 * THEMES
 *==================*/

/* A simple, impressive and very complete theme */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT

/* 0: Light mode; 1: Dark mode */
#define LV_THEME_DEFAULT_DARK 1

/* 1: Enable grow on press */
#define LV_THEME_DEFAULT_GROW 1

/* Default transition time in [ms] */
#define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/* A very simple theme that is a good starting point for a custom theme */
#define LV_USE_THEME_BASIC 0

/* A theme designed for monochrome displays */
#define LV_USE_THEME_MONO 0

/*==================
 * LAYOUTS
 *==================*/

/* A layout similar to Flexbox in CSS. */
#define LV_USE_FLEX 1

/* A layout similar to Grid in CSS. */
#define LV_USE_GRID 1

/*==================
 * OTHERS
 *==================*/

/* 1: Enable API to take snapshot for object */
#define LV_USE_SNAPSHOT 0

/* 1: Enable Monkey test */
#define LV_USE_MONKEY 0

/* 1: Enable grid navigation */
#define LV_USE_GRIDNAV 0

/* 1: Enable lv_obj fragment */
#define LV_USE_FRAGMENT 0

/* 1: Support using images as font in label or span widgets */
#define LV_USE_IMGFONT 0

/* 1: Enable a published subscriber based messaging system */
#define LV_USE_MSG 0

/* 1: Enable Pinyin input method */
#define LV_USE_IME_PINYIN 0

/* 1: Enable file explorer */
#define LV_USE_FILE_EXPLORER 0

/*==================
 * EXAMPLES
 *==================*/

/* Enable the examples to be built with the library */
#define LV_BUILD_EXAMPLES 0

/*==================
 * DEMO USAGE
 *==================*/

/* Show some demos */
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/
