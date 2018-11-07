#ifndef _MYSDL2_H
#define _MYSDL2_H

namespace MySDL2
{

	//仅用于个人理解
	using Image = SDL_Texture;//材质其实就是图像
	using SoftImage = SDL_Surface;//软件图像，存储在内存中


#ifdef _SDL_IMAGE_H

//加载图像，返回图像指针，失败时返回 NULL
#define IMGLoadImage IMG_LoadTexture

#endif


//释放图像
#define SDLDeleteImage SDL_DestroyTexture

//绘制图像
//srcRect: 图像区域，为空表示使用整个图像
//x, y, w, h: 以图像左上角为原点的绘制区域，图像区域会缩放至绘制区域大小；w, h 至少一个为 0 时绘制区域大小为图像大小
//rot: 以 cpoint 为中心的顺时针旋转的角度
//cpoint: 指定旋转中心，默认为图片的中心
//flip: 指定翻转，默认为不翻转
//成功返回 0, 出错返回 -1
	extern int DrawImage(SDL_Renderer *renderer, Image *img, int x, int y, int w = 0, int h = 0, double rot = 0.0, const SDL_Rect *srcRect = NULL, const SDL_Point *cpoint = NULL, const SDL_RendererFlip flip = SDL_FLIP_NONE);


#ifdef _SDL_TTF_H

	//加载文字，返回图像指针，失败时返回 NULL
	//* 不支持多行文字
	extern Image *LoadTextToImage(SDL_Renderer *pRenderer, const char *text, TTF_Font *font, const SDL_Color color);

	//绘制文字，成功返回 0，否则为 -1
	//最好使用 LoadTextToImage, 然后用 DrawImage 绘制文字
	//* 不支持多行文字
	extern int DrawText(SDL_Renderer *pRenderer, const char *text, int x, int y, TTF_Font *font, const SDL_Color color);

	//加载字体，返回字体指针，失败时返回 NULL
#define TTFLoadFont TTF_OpenFont

//释放字体
#define TTFDeleteFont TTF_CloseFont

#endif

	namespace ASCII_px_6x8 {
		extern const unsigned ch_count;
		extern const int px_count_horizonal, px_count_vertical;

		//绘制单个字符的点阵
		//字符不存在返回 -1, 否则为 0
		extern int DrawPxChar(SDL_Renderer *renderer, unsigned ch, int x, int y, int pxsize);

		//绘制点阵字符串
		//str: 要显示的字符串
		//strsize: 字符串长度（通过 SDL_arraysize 获取）
		//x, y: 文字左上角的座标
		//pxsize: 单个像素的边长
		//color: 填充颜色
		//multiline: 是否允许换行（默认允许）
		//字符不存在返回 -1, 否则为 0
		extern int DrawPxString(SDL_Renderer *renderer, const char *str, int strsize, int x, int y, int pxsize, const SDL_Color color, bool multiline = true);
	}

#endif

}

