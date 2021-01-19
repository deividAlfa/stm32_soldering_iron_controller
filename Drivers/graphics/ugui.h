/* -------------------------------------------------------------------------------- */
/* -- µGUI - Generic GUI module (C)Achim Döbler, 2015                            -- */
/* -------------------------------------------------------------------------------- */
// µGUI is a generic GUI module for embedded systems.
// This is a free software that is open for education, research and commercial
// developments under license policy of following terms.
//
//  Copyright (C) 2015, Achim Döbler, all rights reserved.
//  URL: http://www.embeddedlightning.com/
//
// * The µGUI module is a free software and there is NO WARRANTY.
// * No restriction on use. You can use, modify and redistribute it for
//   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
// * Redistributions of source code must retain the above copyright notice.
//
/* -------------------------------------------------------------------------------- */
#ifndef __UGUI_H
#define __UGUI_H

#include "ugui_config.h"


/* -------------------------------------------------------------------------------- */
/* -- µGUI FONTS                                                                 -- */
/* -- Source: http://www.mikrocontroller.net/user/show/benedikt                  -- */
/* -------------------------------------------------------------------------------- */
typedef enum
{
	FONT_TYPE_1BPP,
	FONT_TYPE_8BPP
} FONT_TYPE;

typedef struct
{
   unsigned char* p;
   FONT_TYPE font_type;
   UG_U16 char_width;
   UG_U16 char_height;
   UG_U16 start_char;
   UG_U16 end_char;
   UG_U8  *widths;
} UG_FONT;

#ifdef USE_FONT_IRON_TEMP
	extern const UG_FONT font_iron_temp;
#endif
#ifdef USE_FONT_NO_IRON
	extern const UG_FONT font_no_iron;
#endif

#ifdef USE_FONT_6X8_reduced
   extern const UG_FONT FONT_6X8_reduced;
#endif
#ifdef USE_FONT_8X14_reduced
   extern const UG_FONT FONT_8X14_reduced;
#endif
#ifdef USE_FONT_10X16_reduced
   extern const UG_FONT FONT_10X16_reduced;
#endif
#ifdef USE_FONT_16X26_reduced
   extern const UG_FONT FONT_16X26_reduced;
#endif

/* -------------------------------------------------------------------------------- */
/* -- TYPEDEFS                                                                   -- */
/* -------------------------------------------------------------------------------- */
typedef struct S_OBJECT                               UG_OBJECT;
typedef struct S_WINDOW                               UG_WINDOW;
typedef UG_S8                                         UG_RESULT;
#ifdef USE_MONOCHROME
typedef bool                                        	UG_COLOR;
#endif
#ifdef USE_COLOR_RGB888
typedef UG_U32                                        UG_COLOR;
#endif
#ifdef USE_COLOR_RGB565
typedef UG_U16                                        UG_COLOR;
#endif
/* -------------------------------------------------------------------------------- */
/* -- DEFINES                                                                    -- */
/* -------------------------------------------------------------------------------- */
#ifndef NULL
   #define NULL ((void*) 0)
#endif

/* Alignments */
#define ALIGN_H_LEFT                                  (1<<0)
#define ALIGN_H_CENTER                                (1<<1)
#define ALIGN_H_RIGHT                                 (1<<2)
#define ALIGN_V_TOP                                   (1<<3)
#define ALIGN_V_CENTER                                (1<<4)
#define ALIGN_V_BOTTOM                                (1<<5)
#define ALIGN_BOTTOM_RIGHT                            (ALIGN_V_BOTTOM|ALIGN_H_RIGHT)
#define ALIGN_BOTTOM_CENTER                           (ALIGN_V_BOTTOM|ALIGN_H_CENTER)
#define ALIGN_BOTTOM_LEFT                             (ALIGN_V_BOTTOM|ALIGN_H_LEFT)
#define ALIGN_CENTER_RIGHT                            (ALIGN_V_CENTER|ALIGN_H_RIGHT)
#define ALIGN_CENTER                                  (ALIGN_V_CENTER|ALIGN_H_CENTER)
#define ALIGN_CENTER_LEFT                             (ALIGN_V_CENTER|ALIGN_H_LEFT)
#define ALIGN_TOP_RIGHT                               (ALIGN_V_TOP|ALIGN_H_RIGHT)
#define ALIGN_TOP_CENTER                              (ALIGN_V_TOP|ALIGN_H_CENTER)
#define ALIGN_TOP_LEFT                                (ALIGN_V_TOP|ALIGN_H_LEFT)

/* Default IDs */
#define OBJ_ID_0                                      0
#define OBJ_ID_1                                      1
#define OBJ_ID_2                                      2
#define OBJ_ID_3                                      3
#define OBJ_ID_4                                      4
#define OBJ_ID_5                                      5
#define OBJ_ID_6                                      6
#define OBJ_ID_7                                      7
#define OBJ_ID_8                                      8
#define OBJ_ID_9                                      9
#define OBJ_ID_10                                     10
#define OBJ_ID_11                                     11
#define OBJ_ID_12                                     12
#define OBJ_ID_13                                     13
#define OBJ_ID_14                                     14
#define OBJ_ID_15                                     15
#define OBJ_ID_16                                     16
#define OBJ_ID_17                                     17
#define OBJ_ID_18                                     18
#define OBJ_ID_19                                     19

/* -------------------------------------------------------------------------------- */
/* -- FUNCTION RESULTS                                                           -- */
/* -------------------------------------------------------------------------------- */
#define UG_RESULT_FAIL                               -1
#define UG_RESULT_OK                                  0

/* -------------------------------------------------------------------------------- */
/* -- UNIVERSAL STRUCTURES                                                       -- */
/* -------------------------------------------------------------------------------- */
/* Area structure */
typedef struct
{
   UG_U16 xs;
   UG_U16 ys;
   UG_U16 xe;
   UG_U16 ye;
} UG_AREA;

/* Text structure */
typedef struct
{
   char* str;
   const UG_FONT* font;
   UG_AREA a;
   UG_COLOR fc;
   UG_COLOR bc;
   UG_U8 align;
   UG_U16 h_space;
   UG_U16 v_space;
} UG_TEXT;

/* -------------------------------------------------------------------------------- */
/* -- BITMAP                                                                     -- */
/* -------------------------------------------------------------------------------- */


typedef struct
{
   const uint8_t *p;
   const uint8_t width;
   const uint8_t height;
} UG_BMP_MONO;

#define BMP_BPP_1                                     (1<<0)
#define BMP_BPP_2                                     (1<<1)
#define BMP_BPP_4                                     (1<<2)
#define BMP_BPP_8                                     (1<<3)
#define BMP_BPP_16                                    (1<<4)
#define BMP_BPP_32                                    (1<<5)
#define BMP_RGB888                                    (1<<0)
#define BMP_RGB565                                    (1<<1)
#define BMP_RGB555                                    (1<<2)

/* -------------------------------------------------------------------------------- */
/* -- µGUI CORE STRUCTURE                                                        -- */
/* -------------------------------------------------------------------------------- */
typedef struct
{
   void (*pset)(UG_U16,UG_U16,UG_COLOR);
   UG_U16 x_dim;
   UG_U16 y_dim;
   UG_FONT font;
   UG_S8 char_h_space;
   UG_S8 char_v_space;
   UG_COLOR fore_color;
   UG_COLOR back_color;
   UG_U8 state;
} UG_GUI;

/* -------------------------------------------------------------------------------- */
/* -- µGUI COLORS                                                                -- */
/* -- Source: http://www.rapidtables.com/web/color/RGB_Color.htm                 -- */
/* -------------------------------------------------------------------------------- */
#ifdef USE_MONOCHROME
#define C_BLACK                 		(bool)0
#define C_WHITE                 		(bool)1
#endif
/* -------------------------------------------------------------------------------- */
/* -- PROTOTYPES                                                                 -- */
/* -------------------------------------------------------------------------------- */
/* Classic functions */
UG_U16 UG_Init( UG_GUI* g, void (*p)(UG_U16,UG_U16,UG_COLOR), UG_U16 x, UG_U16 y );
UG_U16 UG_SelectGUI( UG_GUI* g );
void UG_FontSelect( const UG_FONT* font );
void UG_FillFrame( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_COLOR c );
void UG_FillRoundFrame( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_U16 r, UG_COLOR c );
void UG_DrawMesh( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_COLOR c );
void UG_DrawFrame( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_COLOR c );
void UG_DrawRoundFrame( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_U16 r, UG_COLOR c );
void UG_DrawPixel( UG_U16 x0, UG_U16 y0, UG_COLOR c );
void UG_DrawCircle( UG_U16 x0, UG_U16 y0, UG_U16 r, UG_COLOR c );
void UG_FillCircle( UG_U16 x0, UG_U16 y0, UG_U16 r, UG_COLOR c );
void UG_DrawArc( UG_U16 x0, UG_U16 y0, UG_U16 r, UG_U8 s, UG_COLOR c );
void UG_DrawLine( UG_U16 x1, UG_U16 y1, UG_U16 x2, UG_U16 y2, UG_COLOR c );
uint8_t UG_GetStringWidth(char* str );
void UG_PutString( UG_U16 x, UG_U16 y, char* str );
void UG_PutChar( char chr, UG_U16 x, UG_U16 y, UG_COLOR fc, UG_COLOR bc );
void UG_SetForecolor( UG_COLOR c );
void UG_SetBackcolor( UG_COLOR c );
UG_U16 UG_GetXDim( void );
UG_U16 UG_GetYDim( void );
void UG_FontSetHSpace( UG_U16 s );
void UG_FontSetVSpace( UG_U16 s );

/* Miscellaneous functions */
void UG_DrawBMP_MONO( UG_U8 xp, UG_U8 yp, UG_BMP_MONO* bmp );

extern UG_GUI user_gui;
#endif
