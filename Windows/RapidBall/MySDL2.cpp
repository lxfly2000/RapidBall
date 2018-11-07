﻿#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include"MySDL2.h"

#define GetVarBit(x,n) (((x)>>(n))&1)

namespace MySDL2
{

#ifdef _SDL_IMAGE_H

	/*
	//加载图像，返回图像指针，失败时返回 NULL
	Image *LoadImage(SDL_Renderer *pRenderer, const char *file)
	{
		Image *texture = IMG_LoadTexture(pRenderer, file);
		if (!texture)SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "LoadImage", IMG_GetError(), NULL);
		return texture;
	}
	*/

#endif


	/*
	//释放图像
	void DeleteImage(Image *img)
	{
		SDL_DestroyTexture(img);
	}
	*/

	//绘制图像
	//srcRect: 图像区域，为空表示使用整个图像
	//x, y, w, h: 以图像左上角为原点的绘制区域，图像区域会缩放至绘制区域大小；w, h 至少一个为 0 时绘制区域大小为图像大小
	//rot: 以 cpoint 为中心的顺时针旋转的角度
	//cpoint: 指定旋转中心，默认为图片的中心
	//flip: 指定翻转，默认为不翻转
	//成功返回 0, 出错返回 -1
	int DrawImage(SDL_Renderer *renderer, Image *img, int x, int y, int w, int h, double rot, const SDL_Rect *srcRect, const SDL_Point *cpoint, const SDL_RendererFlip flip)
	{
		SDL_Rect pos = { x,y,w,h };
		if (w == 0 || h == 0)
			if (SDL_QueryTexture(img, NULL, NULL, &pos.w, &pos.h) == -1)return -1;//取得宽度和高度
		if (SDL_RenderCopyEx(renderer, img, srcRect, &pos, rot, cpoint, flip) == -1)return -1;
		return 0;
	}


#ifdef _SDL_TTF_H

	//加载文字，返回图像指针，失败时返回 NULL
	//* 不支持多行文字
	Image *LoadTextToImage(SDL_Renderer *pRenderer, const char *text, TTF_Font *font, const SDL_Color color)
	{
		SoftImage *surface = TTF_RenderUTF8_Blended(font, text, color);
		Image *img = SDL_CreateTextureFromSurface(pRenderer, surface);
		SDL_FreeSurface(surface);
		return img;
	}

	//绘制文字，成功返回 0，否则为 -1
	//最好使用 LoadTextToImage, 然后用 DrawImage 绘制文字
	//* 不支持多行文字
	int DrawText(SDL_Renderer *pRenderer, const char *text, int x, int y, TTF_Font *font, const SDL_Color color)
	{
		Image *img = LoadTextToImage(pRenderer, text, font, color);
		DrawImage(pRenderer, img, x, y);
		SDLDeleteImage(img);
		return 0;
	}

	/*
	//加载字体，返回字体指针，失败时返回 NULL
	TTF_Font *LoadFont(const char *fontfilepath, int fontsize)
	{
		TTF_Font *font = TTF_OpenFont(fontfilepath, fontsize);
		if (!font)SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "LoadFont", TTF_GetError(), NULL);
		return font;
	}

	//释放字体
	void DeleteFont(TTF_Font *font)
	{
		TTF_CloseFont(font);
	}
	*/

#endif

	namespace ASCII_px_6x8 {
		const unsigned ch_count = 256;
		const int px_count_horizonal = 6, px_count_vertical = 8;
		//采用 BE 顺序
		const unsigned char asciipx6x8[ch_count][px_count_vertical] = {
			0x70,0x50,0x00,0x28,0x38,0x00,0x10,0x1C,// NUL
			0x00,0x44,0x00,0x00,0x44,0x38,0x00,0x00,// SOH
			0x00,0x44,0x00,0x00,0x38,0x44,0x00,0x00,// STX
			0x28,0x7C,0x7C,0x7C,0x38,0x10,0x00,0x00,// ETX
			0x10,0x38,0x38,0x7C,0x38,0x38,0x10,0x00,// EOT
			0x38,0x38,0x10,0x7C,0x7C,0x10,0x38,0x00,// ENQ
			0x10,0x38,0x38,0x7C,0x7C,0x10,0x38,0x00,// ACK
			0x00,0x38,0x7C,0x7C,0x7C,0x38,0x00,0x00,// BEL
			0x7C,0x7C,0x6C,0x44,0x6C,0x7C,0x7C,0x00,// BS
			0x00,0x38,0x44,0x44,0x44,0x38,0x00,0x00,// HT
			0x7C,0x6C,0x54,0x38,0x54,0x6C,0x7C,0x00,// LF
			0x10,0x38,0x54,0x10,0x38,0x44,0x44,0x38,// VT
			0x38,0x44,0x44,0x38,0x10,0x7C,0x10,0x00,// FF
			0x10,0x18,0x14,0x14,0x10,0x70,0xF0,0x60,// CR
			0x0C,0x34,0x24,0x24,0x2C,0x6C,0x60,0x00,// SO
			0x10,0x54,0x38,0x6C,0x38,0x54,0x10,0x00,// SI
			0x40,0x60,0x70,0x78,0x70,0x60,0x40,0x00,// DLE
			0x04,0x0C,0x1C,0x3C,0x1C,0x0C,0x04,0x00,// DC1
			0x10,0x38,0x54,0x10,0x54,0x38,0x10,0x00,// DC2
			0x28,0x28,0x28,0x28,0x28,0x00,0x28,0x00,// DC3
			0x34,0x74,0x74,0x34,0x14,0x14,0x14,0x00,// DC4
			0x38,0x44,0x30,0x48,0x24,0x18,0x44,0x38,// NAK
			0x00,0x00,0x00,0x00,0x7C,0x7C,0x00,0x00,// SYN
			0x10,0x38,0x54,0x10,0x54,0x38,0x10,0x7C,// ETB
			0x10,0x38,0x54,0x10,0x10,0x10,0x10,0x00,// CAN
			0x10,0x10,0x10,0x10,0x54,0x38,0x10,0x00,// EM
			0x00,0x10,0x08,0x7C,0x08,0x10,0x00,0x00,// SUB
			0x00,0x10,0x20,0x7C,0x20,0x10,0x00,0x00,// ESC
			0x00,0x40,0x40,0x40,0x40,0x7C,0x00,0x00,// FS
			0x10,0x20,0x7C,0x20,0x10,0xF8,0x10,0x20,// GS
			0x00,0x00,0x10,0x38,0x7C,0x00,0x00,0x00,// RS
			0x00,0x00,0x7C,0x38,0x10,0x00,0x00,0x00,// US
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// 空格
			0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x00,// !
			0x24,0x24,0x48,0x00,0x00,0x00,0x00,0x00,// "
			0x28,0x28,0x7C,0x28,0x7C,0x28,0x28,0x00,// #
			0x10,0x3C,0x50,0x38,0x14,0x78,0x10,0x00,// $
			0x60,0x64,0x08,0x10,0x20,0x4C,0x0C,0x00,// %
			0x30,0x48,0x50,0x20,0x54,0x48,0x34,0x00,// &
			0x10,0x10,0x20,0x00,0x00,0x00,0x00,0x00,// '
			0x08,0x10,0x20,0x20,0x20,0x10,0x08,0x00,// (
			0x20,0x10,0x08,0x08,0x08,0x10,0x20,0x00,// )
			0x10,0x54,0x38,0x10,0x38,0x54,0x10,0x00,// *
			0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00,// +
			0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00,// ,
			0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,// -
			0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,// .
			0x04,0x08,0x08,0x10,0x20,0x20,0x40,0x00,// /
			0x38,0x44,0x4C,0x54,0x64,0x44,0x38,0x00,// 0
			0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,// 1
			0x38,0x44,0x04,0x18,0x20,0x40,0x7C,0x00,// 2
			0x38,0x44,0x04,0x18,0x04,0x44,0x38,0x00,// 3
			0x08,0x18,0x28,0x48,0x7C,0x08,0x08,0x00,// 4
			0x7C,0x40,0x78,0x44,0x04,0x44,0x38,0x00,// 5
			0x38,0x44,0x40,0x78,0x44,0x44,0x38,0x00,// 6
			0x7C,0x04,0x08,0x08,0x10,0x10,0x10,0x00,// 7
			0x38,0x44,0x44,0x38,0x44,0x44,0x38,0x00,// 8
			0x38,0x44,0x44,0x3C,0x04,0x44,0x38,0x00,// 9
			0x00,0x10,0x00,0x00,0x00,0x10,0x00,0x00,// :
			0x00,0x10,0x00,0x00,0x10,0x10,0x20,0x00,// ;
			0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x00,// <
			0x00,0x00,0x7C,0x00,0x7C,0x00,0x00,0x00,// =
			0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,// >
			0x38,0x44,0x04,0x08,0x10,0x00,0x10,0x00,// ?
			0x38,0x44,0x4C,0x54,0x5C,0x40,0x3C,0x00,// @
			0x10,0x28,0x44,0x44,0x7C,0x44,0x44,0x00,// A
			0x78,0x44,0x44,0x78,0x44,0x44,0x78,0x00,// B
			0x38,0x44,0x40,0x40,0x40,0x44,0x38,0x00,// C
			0x78,0x44,0x44,0x44,0x44,0x44,0x78,0x00,// D
			0x7C,0x40,0x40,0x78,0x40,0x40,0x7C,0x00,// E
			0x7C,0x40,0x40,0x78,0x40,0x40,0x40,0x00,// F
			0x38,0x44,0x40,0x4C,0x44,0x44,0x38,0x00,// G
			0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x00,// H
			0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00,// I
			0x04,0x04,0x04,0x04,0x04,0x44,0x38,0x00,// J
			0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00,// K
			0x40,0x40,0x40,0x40,0x40,0x40,0x7C,0x00,// L
			0x44,0x6C,0x54,0x44,0x44,0x44,0x44,0x00,// M
			0x44,0x64,0x54,0x4C,0x44,0x44,0x44,0x00,// N
			0x38,0x44,0x44,0x44,0x44,0x44,0x38,0x00,// O
			0x78,0x44,0x44,0x78,0x40,0x40,0x40,0x00,// P
			0x38,0x44,0x44,0x44,0x54,0x48,0x34,0x00,// Q
			0x78,0x44,0x44,0x78,0x50,0x48,0x44,0x00,// R
			0x38,0x44,0x40,0x38,0x04,0x44,0x38,0x00,// S
			0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00,// T
			0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x00,// U
			0x44,0x44,0x44,0x28,0x28,0x10,0x10,0x00,// V
			0x44,0x44,0x44,0x44,0x54,0x6C,0x44,0x00,// W
			0x44,0x28,0x28,0x10,0x28,0x28,0x44,0x00,// X
			0x44,0x28,0x28,0x10,0x10,0x10,0x10,0x00,// Y
			0x7C,0x04,0x08,0x10,0x20,0x40,0x7C,0x00,// Z
			0x38,0x20,0x20,0x20,0x20,0x20,0x38,0x00,// [
			0x40,0x20,0x20,0x10,0x08,0x08,0x04,0x00,// 反斜杠
			0x38,0x08,0x08,0x08,0x08,0x08,0x38,0x00,// ]
			0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00,// ^
			0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,// _
			0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,// `
			0x00,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,// a
			0x40,0x40,0x78,0x44,0x44,0x44,0x78,0x00,// b
			0x00,0x00,0x38,0x44,0x40,0x44,0x38,0x00,// c
			0x04,0x04,0x3C,0x44,0x44,0x44,0x3C,0x00,// d
			0x00,0x00,0x38,0x44,0x7C,0x40,0x38,0x00,// e
			0x0C,0x10,0x7C,0x10,0x10,0x10,0x10,0x00,// f
			0x00,0x00,0x3C,0x44,0x44,0x3C,0x04,0x38,// g
			0x40,0x40,0x78,0x44,0x44,0x44,0x44,0x00,// h
			0x10,0x00,0x10,0x10,0x10,0x10,0x10,0x00,// i
			0x08,0x00,0x08,0x08,0x08,0x08,0x48,0x30,// j
			0x40,0x40,0x48,0x50,0x60,0x50,0x4C,0x00,// k
			0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x00,// l
			0x00,0x00,0x68,0x54,0x54,0x54,0x54,0x00,// m
			0x00,0x00,0x58,0x64,0x44,0x44,0x44,0x00,// n
			0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,// o
			0x00,0x00,0x58,0x64,0x44,0x78,0x40,0x40,// p
			0x00,0x00,0x34,0x4C,0x44,0x3C,0x04,0x04,// q
			0x00,0x00,0x58,0x64,0x40,0x40,0x40,0x00,// r
			0x00,0x00,0x3C,0x40,0x38,0x04,0x78,0x00,// s
			0x20,0x20,0x78,0x20,0x20,0x24,0x18,0x00,// t
			0x00,0x00,0x44,0x44,0x44,0x4C,0x34,0x00,// u
			0x00,0x00,0x44,0x44,0x28,0x28,0x10,0x00,// v
			0x00,0x00,0x44,0x54,0x54,0x28,0x28,0x00,// w
			0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00,// x
			0x00,0x00,0x44,0x44,0x44,0x3C,0x04,0x38,// y
			0x00,0x00,0x7C,0x08,0x10,0x20,0x7C,0x00,// z
			0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,// {
			0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,// |
			0x20,0x10,0x10,0x08,0x10,0x10,0x20,0x00,// }
			0x20,0x54,0x08,0x00,0x00,0x00,0x00,0x00,// ~
			0x00,0x10,0x28,0x44,0x44,0x7C,0x00,0x00,// DEL
			0x38,0x44,0x40,0x40,0x44,0x38,0x10,0x30,// 以下为扩展 ASCII 码字符，此处采用 OEM 字符集
			0x28,0x00,0x44,0x44,0x44,0x4C,0x34,0x00,
			0x08,0x10,0x00,0x38,0x44,0x7C,0x40,0x38,
			0x10,0x28,0x00,0x38,0x04,0x3C,0x44,0x3C,
			0x28,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
			0x20,0x10,0x00,0x38,0x04,0x3C,0x44,0x3C,
			0x10,0x28,0x10,0x38,0x04,0x3C,0x44,0x3C,
			0x00,0x38,0x44,0x40,0x44,0x38,0x10,0x30,
			0x10,0x28,0x00,0x38,0x44,0x7C,0x40,0x38,
			0x28,0x00,0x38,0x44,0x7C,0x40,0x38,0x00,
			0x20,0x10,0x00,0x38,0x44,0x7C,0x40,0x38,
			0x28,0x00,0x10,0x10,0x10,0x10,0x10,0x00,
			0x10,0x28,0x00,0x10,0x10,0x10,0x10,0x00,
			0x20,0x10,0x00,0x10,0x10,0x10,0x10,0x00,
			0x28,0x00,0x10,0x28,0x44,0x7C,0x44,0x00,
			0x10,0x28,0x10,0x10,0x28,0x44,0x7C,0x44,
			0x08,0x10,0x00,0x7C,0x40,0x78,0x40,0x7C,
			0x00,0x00,0x68,0x14,0x3C,0x50,0x3C,0x00,
			0x3C,0x50,0x90,0xFC,0x90,0x90,0x9C,0x00,
			0x10,0x28,0x00,0x38,0x44,0x44,0x38,0x00,
			0x28,0x00,0x38,0x44,0x44,0x44,0x38,0x00,
			0x20,0x10,0x00,0x38,0x44,0x44,0x38,0x00,
			0x10,0x28,0x00,0x44,0x44,0x4C,0x34,0x00,
			0x20,0x10,0x44,0x44,0x44,0x4C,0x34,0x00,
			0x28,0x00,0x44,0x44,0x44,0x3C,0x04,0x38,
			0x28,0x00,0x38,0x44,0x44,0x44,0x44,0x38,
			0x28,0x00,0x44,0x44,0x44,0x44,0x44,0x38,
			0x10,0x38,0x54,0x50,0x50,0x54,0x38,0x10,
			0x10,0x28,0x20,0x70,0x20,0x24,0x78,0x00,
			0x44,0x28,0x10,0x7C,0x10,0x7C,0x10,0x00,
			0xC0,0xA0,0xC0,0x80,0x4C,0xE8,0x44,0x6C,
			0x08,0x14,0x10,0x7C,0x10,0x10,0x50,0x20,
			0x08,0x10,0x00,0x38,0x04,0x3C,0x44,0x3C,
			0x08,0x10,0x00,0x10,0x10,0x10,0x10,0x00,
			0x08,0x10,0x00,0x38,0x44,0x44,0x38,0x00,
			0x08,0x10,0x44,0x44,0x44,0x4C,0x34,0x00,
			0x28,0x50,0x00,0x58,0x64,0x44,0x44,0x00,
			0x28,0x50,0x00,0x44,0x64,0x54,0x4C,0x44,
			0x18,0x04,0x1C,0x24,0x1C,0x00,0x00,0x00,
			0x18,0x24,0x24,0x24,0x18,0x00,0x00,0x00,
			0x10,0x00,0x10,0x20,0x40,0x44,0x38,0x00,
			0x00,0x00,0x00,0x7C,0x40,0x40,0x00,0x00,
			0x00,0x00,0x00,0x7C,0x04,0x04,0x00,0x00,
			0x80,0x88,0x90,0x20,0x58,0x84,0x08,0x1C,
			0x80,0x88,0x90,0x20,0x54,0x94,0x1C,0x04,
			0x00,0x10,0x00,0x10,0x10,0x10,0x10,0x10,
			0x00,0x24,0x48,0x90,0x48,0x24,0x00,0x00,
			0x00,0x90,0x48,0x24,0x48,0x90,0x00,0x00,
			0x54,0x00,0xA8,0x00,0x54,0x00,0xA8,0x00,
			0xA8,0x54,0xA8,0x54,0xA8,0x54,0xA8,0x54,
			0xA8,0xFC,0x54,0xFC,0xA8,0xFC,0x54,0xFC,
			0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
			0x10,0x10,0x10,0xF0,0x10,0x10,0x10,0x10,
			0x10,0x10,0xF0,0x10,0xF0,0x10,0x10,0x10,
			0x28,0x28,0x28,0xE8,0x28,0x28,0x28,0x28,
			0x00,0x00,0x00,0xF8,0x28,0x28,0x28,0x28,
			0x00,0x00,0xF0,0x10,0xF0,0x10,0x10,0x10,
			0x28,0x28,0xE8,0x08,0xE8,0x28,0x28,0x28,
			0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
			0x00,0x00,0xF8,0x08,0xE8,0x28,0x28,0x28,
			0x28,0x28,0xE8,0x08,0xF8,0x00,0x00,0x00,
			0x28,0x28,0x28,0xF8,0x00,0x00,0x00,0x00,
			0x10,0x10,0xF0,0x10,0xF0,0x00,0x00,0x00,
			0x00,0x00,0x00,0xF0,0x10,0x10,0x10,0x10,
			0x10,0x10,0x10,0x1C,0x00,0x00,0x00,0x00,
			0x10,0x10,0x10,0xFC,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0xFC,0x10,0x10,0x10,0x10,
			0x10,0x10,0x10,0x1C,0x10,0x10,0x10,0x10,
			0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,
			0x10,0x10,0x10,0xFC,0x10,0x10,0x10,0x10,
			0x10,0x10,0x1C,0x10,0x1C,0x10,0x10,0x10,
			0x28,0x28,0x28,0x2C,0x28,0x28,0x28,0x28,
			0x28,0x28,0x2C,0x20,0x3C,0x00,0x00,0x00,
			0x00,0x00,0x3C,0x20,0x2C,0x28,0x28,0x28,
			0x28,0x28,0xEC,0x00,0xFC,0x00,0x00,0x00,
			0x00,0x00,0xFC,0x00,0xEC,0x28,0x28,0x28,
			0x28,0x28,0x2C,0x20,0x2C,0x28,0x28,0x28,
			0x00,0x00,0xFC,0x00,0xFC,0x00,0x00,0x00,
			0x28,0x28,0xEC,0x00,0xEC,0x28,0x28,0x28,
			0x10,0x10,0xFC,0x00,0xFC,0x00,0x00,0x00,
			0x28,0x28,0x28,0xFC,0x00,0x00,0x00,0x00,
			0x00,0x00,0xFC,0x00,0xFC,0x10,0x10,0x10,
			0x00,0x00,0x00,0xFC,0x28,0x28,0x28,0x28,
			0x28,0x28,0x28,0x3C,0x00,0x00,0x00,0x00,
			0x10,0x10,0x1C,0x10,0x1C,0x00,0x00,0x00,
			0x00,0x00,0x1C,0x10,0x1C,0x10,0x10,0x10,
			0x00,0x00,0x00,0x3C,0x28,0x28,0x28,0x28,
			0x28,0x28,0x28,0xFC,0x28,0x28,0x28,0x28,
			0x10,0x10,0xFC,0x10,0xFC,0x10,0x10,0x10,
			0x10,0x10,0x10,0xF0,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x1C,0x10,0x10,0x10,0x10,
			0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
			0x00,0x00,0x00,0x00,0xFC,0xFC,0xFC,0xFC,
			0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
			0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
			0xFC,0xFC,0xFC,0xFC,0x00,0x00,0x00,0x00,
			0x00,0x00,0x34,0x4C,0x48,0x48,0x34,0x00,
			0x00,0x18,0x24,0x28,0x24,0x44,0x58,0x40,
			0x7C,0x40,0x40,0x40,0x40,0x40,0x40,0x00,
			0x00,0x00,0x7C,0x28,0x28,0x28,0x4C,0x00,
			0x7C,0x40,0x20,0x10,0x20,0x40,0x7C,0x00,
			0x00,0x00,0x3C,0x48,0x48,0x48,0x30,0x00,
			0x00,0x00,0x44,0x44,0x44,0x4C,0x74,0x40,
			0x00,0x00,0x7C,0x10,0x10,0x10,0x08,0x00,
			0x10,0x38,0x54,0x54,0x54,0x38,0x10,0x00,
			0x38,0x44,0x44,0x7C,0x44,0x44,0x38,0x00,
			0x38,0x44,0x44,0x44,0x44,0x28,0x6C,0x00,
			0x3C,0x20,0x10,0x28,0x48,0x48,0x30,0x00,
			0x00,0x00,0x28,0x54,0x28,0x00,0x00,0x00,
			0x00,0x00,0x48,0x54,0x54,0x38,0x10,0x10,
			0x00,0x00,0x38,0x44,0x30,0x44,0x38,0x00,
			0x38,0x44,0x44,0x44,0x44,0x44,0x44,0x00,
			0x00,0x7C,0x00,0x7C,0x00,0x7C,0x00,0x00,
			0x10,0x10,0x7C,0x10,0x10,0x00,0x7C,0x00,
			0xC0,0x30,0x0C,0x30,0xC0,0x0C,0x30,0xC0,
			0x0C,0x30,0xC0,0x30,0x0C,0xC0,0x30,0x0C,
			0x08,0x14,0x10,0x10,0x10,0x10,0x10,0x10,
			0x10,0x10,0x10,0x10,0x10,0x10,0x50,0x20,
			0x00,0x10,0x00,0x7C,0x00,0x10,0x00,0x00,
			0x00,0x20,0x54,0x08,0x20,0x54,0x08,0x00,
			0x10,0x28,0x10,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x10,0x38,0x10,0x00,0x00,0x00,
			0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
			0x1C,0x10,0x10,0x20,0xA0,0x60,0x20,0x00,
			0x30,0x28,0x28,0x00,0x00,0x00,0x00,0x00,
			0x30,0x48,0x10,0x20,0x78,0x00,0x00,0x00,
			0x00,0x7C,0x7C,0x7C,0x7C,0x7C,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x44,0x7C,0x00
		};
		const unsigned char asciipx6x8_notdefined[px_count_vertical] = {
			0x7C,0x44,0x44,0x44,0x44,0x44,0x7C,0x00 // 未定义的字符
		};

		//绘制单个字符的点阵
		//字符不存在返回 -1, 否则为 0
		int DrawPxChar(SDL_Renderer *renderer, unsigned ch, int x, int y, int pxsize)
		{
			SDL_Rect rect = { x,y,pxsize,pxsize };
			for (int j = 0; j < px_count_vertical; j++)
			{
				for (int i = 0; i < px_count_horizonal; i++)
				{
					if (ch < ch_count)
					{
						if (GetVarBit(asciipx6x8[ch][j], 8 * sizeof(asciipx6x8[ch][j]) - i - 1))
							SDL_RenderFillRect(renderer, &rect);
					}
					else
					{
						if (GetVarBit(asciipx6x8_notdefined[j], 8 * sizeof(asciipx6x8_notdefined[j]) - i - 1))
							SDL_RenderFillRect(renderer, &rect);
					}
					rect.x += pxsize;
				}
				rect.y += pxsize;
				rect.x = x;
			}
			return ch < ch_count ? 0 : -1;
		}

		//绘制点阵字符串
		//str: 要显示的字符串
		//strsize: 字符串长度（通过 SDL_arraysize 获取）
		//x, y: 文字左上角的座标
		//pxsize: 单个像素的边长
		//color: 填充颜色
		//multiline: 是否允许换行（默认允许）
		//字符不存在返回 -1, 否则为 0
		int DrawPxString(SDL_Renderer *renderer, const char *str, int strsize, int x, int y, int pxsize, const SDL_Color color, bool multiline)
		{
			const int ox = x;
			int ret = 0;
			SDL_Color prevcolor;
			SDL_GetRenderDrawColor(renderer, &prevcolor.r, &prevcolor.g, &prevcolor.b, &prevcolor.a);
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			for (int i = 0; i < strsize; i++)
			{
				if (str[i] == '\0')
				{
					break;
				}
				else if (str[i] == '\n'&&multiline)
				{
					x = ox;
					y += px_count_vertical*pxsize;
				}
				else
				{
					ret |= DrawPxChar(renderer, str[i] & 0xFF, x, y, pxsize);
					x += px_count_horizonal*pxsize;
				}
			}
			SDL_SetRenderDrawColor(renderer, prevcolor.r, prevcolor.g, prevcolor.b, prevcolor.a);
			return ret;
		}
	}

}

