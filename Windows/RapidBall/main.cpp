//SDL 2.0 中文教程：
//http://adolfans.github.io/sdltutorialcn/
//https://github.com/Twinklebear/TwinklebearDev-Lessons

#ifdef _MSC_VER
#include"UseUTF-8.h"
#include"UseVisualStyle.h"
#endif

#include<SDL.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<stdlib.h>
#include<time.h>
#include"MySDL2.h"
#include"repinput.h"
#include"CRC32.h"
#include"lcg_random.h"

using namespace MySDL2;

#ifdef _DEBUG
char _DrawPrintfBuf[256];
void DrawPrintf(SDL_Renderer *r, int x, int y, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_vsnprintf(_DrawPrintfBuf, SDL_arraysize(_DrawPrintfBuf), fmt, ap);
	va_end(ap);
	ASCII_px_6x8::DrawPxString(r, _DrawPrintfBuf, SDL_arraysize(_DrawPrintfBuf), x, y, 4, { 255,255,255,255 });
}
#define DEBUG_ON
#endif
#define FONT_PATH_CH "font.ttf"
#define FONT_PATH_EN FONT_PATH_CH
#define LIFEUP_SCORE 1000
#define REPLAY_FILE_NAME "replay.bin"
static const unsigned CRC32_font_ttf = 0xD67079EC;
char szReplayFileFullPath[256];

enum RepInputu32Keys
{
	REPINPUT_ID_SRAND,
	REPINPUT_ID_TITLE_ROLL
};

enum RepInputU8Keys
{
	REPINPUT_ID_LEFT,
	REPINPUT_ID_RIGHT
};

#define LOAD_PTR_SDL(ptr,action,geterrorfunc,window,errorreturn)\
ptr=action;\
if(!ptr)\
{\
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,__FUNCTION__,geterrorfunc(),window);\
	return errorreturn;\
}

int screenWidth = 960;
int screenHeight = 600;

SDL_Window *g_pWindow = NULL;
SDL_Renderer *g_pRenderer = NULL;

RepInput repinput;
unsigned titleRollFrames;

struct GameInfo
{
	const int maxLife = 6;
	int score;
	int level;
	int life;
	void Reset()
	{
		score = 0;
		level = 1;
		life = 3;
	}
	GameInfo()
	{
		Reset();
	}
}gameinfo;

enum eImgs
{
	eImgTextTitle,
	eImgTextAuthor,
	eImgTextMenuStart,
	eImgTextBall,
	eImgTextSpike,
	eImgTextLeft,
	eImgTextRight,
	eImgTextEmptyBall,
	eImgTextLevel,
	eImgTextScore,
	eImgTextYourScore,
	eImgTextMenuRetry,
	eImgTextMenuReturn,
	eImgBackground,
	eImgTextMenuReplay,
	eImgTextMenuSaveReplay,
	eImgTextMenuReplaySaved,
	eImgTextMenuReplaySaveFailed
};
enum eWavs
{
	eWavPing,
	eWav1up,
	eWavSpike
};
TTF_Font *font = NULL, *fontItem = NULL;
Mix_Music *waves[3] = { NULL };

struct Ball
{
	int centerX, centerY, previousCenterY;
	int width, height;
	int radius;
	float crrate;
	const int acc = 1;
	int speed;
	Image *img;
	Ball(Image *_img, int r, float cr=1.0f)
	{
		img = _img;
		radius = r;
		SDL_QueryTexture(img, NULL, NULL, &width, &height);
		crrate = cr;
		width = (int)(width*cr);
		height = (int)(height*cr);
	}
	void SetBall(int x, int y)
	{
		centerX = x;
		centerY = y;
		speed = 0;
	}
	//正右负左
	void Move(int hspeed)
	{
		centerX += hspeed;
		if (centerX < 0)
			centerX = 0;
		if (centerX > screenWidth)
			centerX = screenWidth;
	}
	void MoveUp(int pixels)
	{
		centerY -= pixels;
	}
	bool IsOut()
	{
		return centerY < 0;
	}
	int Fall()
	{
		previousCenterY = centerY;
		centerY += speed;
		return speed;
	}
	//-1表示使球立即停止
	void TakeAcc(int outerUpwardAcc = 0)
	{
		if (outerUpwardAcc == -1)
			speed = 0;
		else if (speed > 16)
			speed = 16;
		else if (speed >= 0)
			speed = speed + acc - outerUpwardAcc;
		else
			speed = 0;
	}
	void Draw()
	{
		DrawImage(g_pRenderer, img, centerX - width / 2, centerY - height / 2, width, height);
	}
};

Ball *ball;

struct Img
{
	SDL_Rect rect;
	Image *img;
	Img(Image*_img, int cx, int cy, float cr = 1.0f)
	{
		img = _img;
		SDL_QueryTexture(_img, NULL, NULL, &rect.w, &rect.h);
		rect.x = (int)(cx - rect.w / 2 * cr);
		rect.y = (int)(cy - rect.h / 2 * cr);
		rect.w =(int)(rect.w* cr);
		rect.h =(int)(rect.h* cr);
	}
	bool HitTest(int x, int y)
	{
		return x > rect.x&&x<rect.x + rect.w&&y>rect.y&&y < rect.y + rect.h;
	}
	void Draw(float cr = 1.0f, int fpoint = 4)
	{
		switch (fpoint)
		{
		case 0:DrawImage(g_pRenderer, img, rect.x, rect.y, (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 1:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr) / 2), rect.y, (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 2:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr)), rect.y, (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 3:DrawImage(g_pRenderer, img, rect.x, rect.y + (int)(rect.h*(1.0f - cr) / 2), (int)(rect.w*cr, rect.h*cr)); break;
		case 4:default:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr) / 2), (int)(rect.y + rect.h*(1.0f - cr) / 2), (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 5:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr)), (int)(rect.y + rect.h*(1.0f - cr) / 2), (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 6:DrawImage(g_pRenderer, img, rect.x, (int)(rect.y + rect.h*(1.0f - cr)), (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 7:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr) / 2), (int)(rect.y + rect.h*(1.0f - cr)), (int)(rect.w*cr), (int)(rect.h*cr)); break;
		case 8:DrawImage(g_pRenderer, img, (int)(rect.x + rect.w*(1.0f - cr)), (int)(rect.y + rect.h*(1.0f - cr)), (int)(rect.w*cr), (int)(rect.h*cr)); break;
		}
	}
	void SetPos(int x, int y)
	{
		rect.x = x;
		rect.y = y;
	}
};

Img *imgs[18], *textDigitImgs[10];

struct Board
{
	bool use;
	int centerX, centerY;
	int length;
	int type;//0=普通，1=有刺，2=向左，3=向右
	bool touched;
	SDL_Color color;
	static int rollspeed;
	Board() :type(0), animpx(0), use(false)
	{
		color.a = 255;
		Board::rollspeed = 2;// screenWidth / 400;
	}
	void MoveUp(int pixels)
	{
		centerY -= pixels;
		rect.y -= pixels;
	}
	bool IsOut()
	{
		return centerY < 0;
	}
	void Draw()
	{
		switch (type)
		{
		case 0:DrawNormal(); break;
		case 1:DrawSpike(); break;
		case 2:DrawLeft(); break;
		case 3:DrawRight(); break;
		}
	}
	void NewBoard(int y,int t)
	{
		centerX = lcg_rand() % screenWidth;
		centerY = y;
		rect.h = 20;
		rect.w = screenWidth / 4 + lcg_rand() % (screenWidth / 12);
		rect.x = centerX - rect.w / 2;
		rect.y = centerY - rect.h / 2;
		color.r = lcg_rand() % 256;
		color.g = lcg_rand() % 256;
		color.b = lcg_rand() % 256;
		touched = false;
		type = t;
		if (type == 1)
		{
			spikeLeft = rect.w / imgs[eImgTextSpike]->rect.w;
			spikeLeft = centerX - spikeLeft*imgs[eImgTextSpike]->rect.w / 2;
		}
	}
	SDL_Rect rect;
private:
	void DrawNormal()
	{
		SDL_SetRenderDrawColor(g_pRenderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(g_pRenderer, &rect);
	}
	void DrawSpike()
	{
		DrawNormal();
		for (int i = spikeLeft; i + imgs[eImgTextSpike]->rect.w < rect.x + rect.w; i += imgs[eImgTextSpike]->rect.w)
			DrawImage(g_pRenderer, imgs[eImgTextSpike]->img, i, rect.y - imgs[eImgTextSpike]->rect.h);
	}
	void DrawLeft()
	{
		DrawNormal();
		for (int i = 0; i < rect.w; i += imgs[eImgTextLeft]->rect.w)
		{
			if (i == 0)
			{
				srcRect.x = animpx;
				srcRect.y = 0;
				srcRect.w = imgs[eImgTextLeft]->rect.w - animpx;
				srcRect.h = imgs[eImgTextLeft]->rect.h;
				DrawImage(g_pRenderer, imgs[eImgTextLeft]->img, rect.x, rect.y, srcRect.w, rect.h, 0.0, &srcRect);
			}
			else if (i + imgs[eImgTextLeft]->rect.w > rect.w)
			{
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.h = imgs[eImgTextLeft]->rect.h;
				if (i - animpx < rect.w - imgs[eImgTextLeft]->rect.w)
				{
					srcRect.w = rect.w - (i - animpx + imgs[eImgTextLeft]->rect.w);
					DrawImage(g_pRenderer, imgs[eImgTextLeft]->img, rect.x + i - animpx, rect.y,
						imgs[eImgTextLeft]->rect.w, rect.h);
					DrawImage(g_pRenderer, imgs[eImgTextLeft]->img, rect.x + i - animpx + imgs[eImgTextLeft]->rect.w, rect.y,
						srcRect.w, rect.h, 0.0, &srcRect);
				}
				else
				{
					srcRect.w = rect.w - (i - animpx);
					DrawImage(g_pRenderer, imgs[eImgTextLeft]->img, rect.x + i - animpx, rect.y, srcRect.w, rect.h, 0.0, &srcRect);
				}
			}
			else
				DrawImage(g_pRenderer, imgs[eImgTextLeft]->img, rect.x + i - animpx, rect.y,
					imgs[eImgTextLeft]->rect.w, rect.h);
		}
		animpx = (animpx + 1) % imgs[eImgTextLeft]->rect.w;
	}
	void DrawRight()
	{
		DrawNormal();
		for (int i = -imgs[eImgTextLeft]->rect.w; i < rect.w; i += imgs[eImgTextLeft]->rect.w)
		{
			if (i < 0)
			{
				srcRect.x = imgs[eImgTextRight]->rect.w - animpx;
				srcRect.y = 0;
				srcRect.w = animpx;
				srcRect.h = imgs[eImgTextRight]->rect.h;
				DrawImage(g_pRenderer, imgs[eImgTextRight]->img, rect.x, rect.y, srcRect.w, rect.h, 0.0, &srcRect);
			}
			else if (i + 2 * imgs[eImgTextRight]->rect.w > rect.w)
			{
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.h = imgs[eImgTextRight]->rect.h;
				if (i + animpx < rect.w - imgs[eImgTextRight]->rect.w)
				{
					srcRect.w = rect.w - (i + animpx + imgs[eImgTextRight]->rect.w);
					DrawImage(g_pRenderer, imgs[eImgTextRight]->img, rect.x + i + animpx, rect.y,
						imgs[eImgTextRight]->rect.w, rect.h);
					DrawImage(g_pRenderer, imgs[eImgTextRight]->img, rect.x + i + animpx + imgs[eImgTextRight]->rect.w, rect.y,
						srcRect.w, rect.h, 0.0, &srcRect);
				}
				else
				{
					srcRect.w = rect.w - (i + animpx);
					DrawImage(g_pRenderer, imgs[eImgTextRight]->img, rect.x + i + animpx, rect.y, srcRect.w, rect.h, 0.0, &srcRect);
				}
				break;
			}
			else
				DrawImage(g_pRenderer, imgs[eImgTextRight]->img, rect.x + i + animpx, rect.y,
					imgs[eImgTextRight]->rect.w, rect.h);
		}
		animpx = (animpx + 1) % imgs[eImgTextLeft]->rect.w;
	}
	SDL_Rect srcRect;
	int animpx;
	int spikeLeft;
};

int Board::rollspeed = 0;

Board boards[30];
int iBoard = 0;

struct LifeUp
{
	int attachedtoboard;
	bool available = false;
	float crrate;
	Img *img;
	SDL_Rect rect;
	LifeUp() :crrate(1.0f) {}
	void Draw()
	{
		DrawImage(g_pRenderer, imgs[eImgTextEmptyBall]->img, rect.x, rect.y, rect.w, rect.h);
	}
	void NewLifeUp(int boardnum)
	{
		img = imgs[eImgTextEmptyBall];
		attachedtoboard = boardnum;
		available = true;
		rect.x = (int)(boards[attachedtoboard].centerX - img->rect.w / 2 * crrate);
		rect.y = (int)(boards[attachedtoboard].centerY - img->rect.h * crrate);
		rect.w = (int)(img->rect.w*crrate);
		rect.h = (int)(img->rect.h*crrate);
	}
	bool Coll(Ball *b)
	{
		return b->centerX > rect.x - b->radius&&b->centerX<rect.x + rect.w + b->radius&&
			b->centerY>rect.y - b->radius&&b->centerY < rect.y + rect.h + b->radius;
	}
	void MoveUp(int speed)
	{
		rect.y -= speed;
	}
}lifeups[5];
int iLifeup = 0;
unsigned g_srand = 0;

bool Load()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	g_pWindow = SDL_CreateWindow("Rapid Ball 1.2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	g_pRenderer = SDL_CreateRenderer(g_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_SetRenderDrawBlendMode(g_pRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1");
	SDL_RenderSetLogicalSize(g_pRenderer, screenWidth, screenHeight);

	SDL_snprintf(szReplayFileFullPath, sizeof(szReplayFileFullPath), "%s/%s", SDL_GetPrefPath("lxfly2000", "RapidBall"), REPLAY_FILE_NAME);
	LOAD_PTR_SDL(font, TTFLoadFont(FONT_PATH_CH, 72), TTF_GetError, g_pWindow, false);
	LOAD_PTR_SDL(fontItem, TTFLoadFont(FONT_PATH_CH, 36), TTF_GetError, g_pWindow, false);
	size_t fontmemsize;
	void *pfontmem=SDL_LoadFile(FONT_PATH_CH,&fontmemsize);
	if (pfontmem==NULL||CRC32Calc(pfontmem,fontmemsize) != CRC32_font_ttf)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, NULL, "文件被修改或不完整，请重新下载。", g_pWindow);
		return false;
	}
	SDL_free(pfontmem);
	char str[32];
	for (int i = 0; i < SDL_arraysize(textDigitImgs); i++)
	{
		SDL_snprintf(str, SDL_arraysize(str), "%d", i);
		textDigitImgs[i] = new Img(LoadTextToImage(g_pRenderer, str, font, { 42,253,5 }), 0, 0);
	}
	imgs[eImgTextTitle] = new Img(LoadTextToImage(g_pRenderer, "彩球滑梯", font, { 255,255,255 }), screenWidth / 2, screenHeight / 4);
	Img *p=imgs[eImgTextAuthor] = new Img(LoadTextToImage(g_pRenderer, "By lxfly2000", fontItem, { 23,42,87 }), screenWidth / 2, 0);
	p->rect.y = screenHeight - p->rect.h;
	imgs[eImgTextMenuStart] = new Img(LoadTextToImage(g_pRenderer, "开始", font, { 0,255,0,255 }), screenWidth / 2, screenHeight * 2 / 3);
	imgs[eImgTextMenuReplay] = new Img(LoadTextToImage(g_pRenderer, "回放", fontItem, { 255,255,255 }), screenWidth / 2, imgs[eImgTextMenuStart]->rect.h + screenHeight * 2 / 3);
	imgs[eImgTextBall] = new Img(LoadTextToImage(g_pRenderer, "●", font, { 255,255,255 }),0,0);
	imgs[eImgTextSpike] = new Img(LoadTextToImage(g_pRenderer, "△", fontItem, { 251,102,70,230 }),0,0);
	imgs[eImgTextLeft] = new Img(LoadTextToImage(g_pRenderer, "＜", fontItem, { 255,255,0,255 }), 0, 0);
	imgs[eImgTextRight] = new Img(LoadTextToImage(g_pRenderer, "＞", fontItem, { 255,255,0,255 }), 0, 0);
	imgs[eImgTextEmptyBall] = new Img(LoadTextToImage(g_pRenderer, "○", font, { 255,255,255 }),0,0);
	p = imgs[eImgTextLevel] = new Img(LoadTextToImage(g_pRenderer, "等级", fontItem, { 255,255,255 }), 0, 0);
	p->rect.x = (screenWidth - p->rect.w) / 2;
	p->rect.y = 0;
	p = imgs[eImgTextScore] = new Img(LoadTextToImage(g_pRenderer, "分数", fontItem, { 255,255,255 }), 0, 0);
	p->rect.x = screenWidth - p->rect.w - textDigitImgs[0]->rect.w;
	p->rect.y = 0;
	imgs[eImgTextYourScore] = new Img(LoadTextToImage(g_pRenderer, "你的得分", font, { 255,255,255 }), screenWidth / 2, screenHeight / 3);
	imgs[eImgTextMenuRetry] = new Img(LoadTextToImage(g_pRenderer, "重新开始", fontItem, { 255,255,255 }), screenWidth / 2, screenHeight * 2 / 3);
	imgs[eImgTextMenuSaveReplay] = new Img(LoadTextToImage(g_pRenderer, "保存录像", fontItem, { 255,255,255 }), screenWidth / 2, imgs[eImgTextMenuRetry]->rect.h + screenHeight * 2 / 3);
	imgs[eImgTextMenuReplaySaved] = new Img(LoadTextToImage(g_pRenderer, "已保存", fontItem, { 0,255,0,255 }), screenWidth / 2, imgs[eImgTextMenuRetry]->rect.h + screenHeight * 2 / 3);
	imgs[eImgTextMenuReplaySaveFailed] = new Img(LoadTextToImage(g_pRenderer, "保存失败", fontItem, { 255,0,0,255 }), screenWidth / 2, imgs[eImgTextMenuRetry]->rect.h + screenHeight * 2 / 3);
	imgs[eImgTextMenuReturn] = new Img(LoadTextToImage(g_pRenderer, "返回主界面", fontItem, { 255,255,255 }), screenWidth / 2, imgs[eImgTextMenuReplay]->rect.h + imgs[eImgTextMenuRetry]->rect.h + screenHeight * 2 / 3);
	imgs[eImgBackground] = new Img(SDL_CreateTexture(g_pRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, screenWidth, screenHeight), screenWidth / 2, screenHeight / 2);
	SDL_SetRenderTarget(g_pRenderer, imgs[eImgBackground]->img);
	SDL_Rect r = { 0,screenHeight,screenWidth,1 };
	while (--r.y)
	{
		SDL_SetRenderDrawColor(g_pRenderer, 0, 0, (Uint8)(192 * pow(0.992, screenHeight - r.y)), 255);
		SDL_RenderDrawLine(g_pRenderer, r.x, r.y, r.w, r.y);
	}
	ball = new Ball(imgs[eImgTextBall]->img, 0);
	ball->radius = imgs[eImgTextBall]->rect.w / 2 + 1;

	LOAD_PTR_SDL(waves[eWavPing], Mix_LoadMUS("Ping.ogg"), Mix_GetError, g_pWindow, false);
	LOAD_PTR_SDL(waves[eWav1up], Mix_LoadMUS("Ball.ogg"), Mix_GetError, g_pWindow, false);
	LOAD_PTR_SDL(waves[eWavSpike], Mix_LoadMUS("Spike.ogg"), Mix_GetError, g_pWindow, false);

	SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
	SDL_SetRenderTarget(g_pRenderer, NULL);

	return true;
}

void End()
{
	Mix_CloseAudio();
	for (int i = 0; i < SDL_arraysize(waves); i++)
		Mix_FreeMusic(waves[i]);
	delete ball;
	for (int i = 0; i < SDL_arraysize(textDigitImgs); i++)
	{
		if (textDigitImgs[i])SDLDeleteImage(textDigitImgs[i]->img);
		delete textDigitImgs[i];
	}
	for (int i = 0; i < SDL_arraysize(imgs); i++)
	{
		if (imgs[i])SDLDeleteImage(imgs[i]->img);
		delete imgs[i];
	}
	TTFDeleteFont(font);

	SDL_DestroyRenderer(g_pRenderer);
	SDL_DestroyWindow(g_pWindow);

	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}

int scene = 11;
struct Keys
{
	bool pressedLeft = false,
		pressedRight = false,
		pressedUp = false,
		pressedDown = false,
		pressedOK = false,
		pressedBack = false,
		isonpress = false;
	int x = 0, y = 0;
}keys;

void Game();
void TitleScene();
void PlayScene();
void ScoreScene();
void InitScoreScene();
void ResetGame(unsigned _set_rseed = 0);
void ResetBall();

void Loop()
{
	SDL_Event e;
	while (scene)
	{
		while(SDL_PollEvent(&e));
		switch (e.type)
		{
		case SDL_QUIT:scene = 0; break;
		case SDL_FINGERDOWN:
			if (e.tfinger.x < 0.5f)
			{
				keys.isonpress = !keys.pressedLeft;
				keys.pressedLeft = true;
			}
			else
			{
				keys.isonpress = !keys.pressedRight;
				keys.pressedRight = true;
			}
			if (e.tfinger.y < 0.5f)
			{
				keys.isonpress = !keys.pressedUp;
				keys.pressedUp = true;
			}
			else
			{
				keys.isonpress = !keys.pressedDown;
				keys.pressedDown = true;
			}
			keys.x = (int)(e.tfinger.x*screenWidth);
			keys.y = (int)(e.tfinger.y*screenHeight);
			break;
		case SDL_FINGERUP:
			keys.isonpress = false;
			if (e.tfinger.x < 0.5f)
				keys.pressedLeft = false;
			else
				keys.pressedRight = false;
			if (e.tfinger.y < 0.5f)
				keys.pressedUp = false;
			else
				keys.pressedDown = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button != SDL_BUTTON_LEFT)break;
			if (e.button.x < screenWidth / 2)
			{
				keys.isonpress = !keys.pressedLeft;
				keys.pressedLeft = true;
			}
			else
			{
				keys.isonpress = !keys.pressedRight;
				keys.pressedRight = true;
			}
			if (e.button.y < screenHeight / 2)
			{
				keys.isonpress = !keys.pressedUp;
				keys.pressedUp = true;
			}
			else
			{
				keys.isonpress = !keys.pressedDown;
				keys.pressedDown = true;
			}
			keys.x = e.button.x;
			keys.y = e.button.y;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button != SDL_BUTTON_LEFT)break;
			keys.isonpress = false;
			if (e.button.x < screenWidth / 2)
				keys.pressedLeft = false;
			else
				keys.pressedRight = false;
			if (e.button.y < screenHeight / 2)
				keys.pressedUp = false;
			else
				keys.pressedDown = false;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_LEFT:keys.isonpress = !keys.pressedLeft; keys.pressedLeft = true; break;
			case SDLK_RIGHT:keys.isonpress = !keys.pressedRight; keys.pressedRight = true; break;
			case SDLK_UP:keys.isonpress = !keys.pressedUp; keys.pressedUp = true; break;
			case SDLK_DOWN:keys.isonpress = !keys.pressedDown; keys.pressedDown = true; break;
			case SDLK_MENU:case SDLK_RETURN:keys.isonpress = !keys.pressedOK; keys.pressedOK = true; break;
			case SDLK_ESCAPE:case SDLK_AC_BACK:keys.isonpress = !keys.pressedBack; keys.pressedBack = true; break;
			}
			break;
		case SDL_KEYUP:
			keys.isonpress = false;
			switch (e.key.keysym.sym)
			{
			case SDLK_LEFT:keys.pressedLeft = false; break;
			case SDLK_RIGHT:keys.pressedRight = false; break;
			case SDLK_UP:keys.pressedUp = false; break;
			case SDLK_DOWN:keys.pressedDown = false; break;
			case SDLK_MENU:case SDLK_RETURN:keys.pressedOK = false; break;
			case SDLK_ESCAPE:case SDLK_AC_BACK:keys.pressedBack = false; break;
			}
			break;
		}

		SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 0);
		SDL_RenderClear(g_pRenderer);

		Game();
		
		SDL_RenderPresent(g_pRenderer);
	}
}

int main(int argc, char *argv[])
{
	if (Load())
		Loop();
	End();
	return 0;
}

void PlaySFX(int id)
{
	switch (id)
	{
	case 0:Mix_PlayMusic(waves[eWavPing], 0); break;
	case 1:Mix_PlayMusic(waves[eWav1up], 0); break;
	case 2:Mix_PlayMusic(waves[eWavSpike], 0); break;
	}
}

void Stage()
{
	imgs[eImgBackground]->Draw();
	for (int i = SDL_arraysize(boards) - 1; i >= 0; i--)
		if (boards[i].use)
		{
			boards[i].Draw();
#ifdef DEBUG_ON
			DrawPrintf(g_pRenderer, boards[i].centerX, boards[i].centerY, "%d,%d,%d", boards[i].centerX, boards[i].centerY,boards[i].rect.w);
#endif
		}
	for (int i = 0; i < SDL_arraysize(lifeups); i++)
		if (lifeups[i].available)
			lifeups[i].Draw();
	if (scene > 1)
	{
		ball->Draw();
	}
}

void Coll()
{
	for(int i=SDL_arraysize(boards)-1;i>=0;i--)
		if(boards[i].use&&ball->centerX>boards[i].rect.x&&ball->centerX<boards[i].rect.x+boards[i].rect.w)
			if (ball->centerY < boards[i].rect.y&&boards[i].rect.y - ball->centerY<ball->radius/* ||
				ball->centerY>boards[i].rect.y&&ball->previousCenterY < boards[i].rect.y*/)
			{
				ball->TakeAcc(-1);
				ball->SetBall(ball->centerX, boards[i].centerY - ball->radius);
				if (!boards[i].touched)
				{
					if (boards[i].type == 1)
					{
						gameinfo.life--;
						PlaySFX(2);
					}
					else
					{
						PlaySFX(0);
					}
					boards[i].touched = true;
				}
				switch (boards[i].type)
				{
				case 2:ball->Move(-(Board::rollspeed+3)); break;//比自己移动小一个速度
				case 3:ball->Move((Board::rollspeed+3)); break;
				}
				break;
			}
	gameinfo.score += ball->Fall();
	if (ball->centerY /*+ ball->radius - 1 */< 0 || ball->centerY - ball->radius > screenHeight)
	{
		gameinfo.life = -1;
		PlaySFX(2);
	}
	for(int i=0;i<SDL_arraysize(lifeups);i++)
		if (lifeups[i].available&&lifeups[i].Coll(ball))
		{
			if (gameinfo.life < gameinfo.maxLife)
				gameinfo.life++;
			PlaySFX(1);
			gameinfo.score += LIFEUP_SCORE;
			lifeups[i].available = false;
			if (boards[lifeups[i].attachedtoboard].type != 1)
				boards[lifeups[i].attachedtoboard].touched = true;
		}
}

int waitingNewBoardTime = 0;
int levelUpTime = 0;

void MakeNewBoard(int h)
{
	int t = lcg_rand() % 10;
	iBoard = (iBoard + 1) % SDL_arraysize(boards);
	boards[iBoard].NewBoard(h, t > 3 ? 0 : t);
	boards[iBoard].use = true;
	waitingNewBoardTime = 60 + lcg_rand() % 120;
}

void MakeNewLifeup(int attachtoboard)
{
	lifeups[iLifeup].NewLifeUp(attachtoboard);
	iLifeup = (iLifeup + 1) % SDL_arraysize(lifeups);
}

void RollUp()
{
	for (int i = SDL_arraysize(boards) - 1; i >= 0; i--)
	{
		if (boards[i].use)
		{
			boards[i].MoveUp(Board::rollspeed+gameinfo.level-1);
			if (boards[i].centerY < 0)
				boards[i].use = false;
		}
	}
	for (int i = 0; i < SDL_arraysize(lifeups); i++)
		if (lifeups[i].available)
		{
			lifeups[i].MoveUp(Board::rollspeed+gameinfo.level-1);
			if (lifeups[i].rect.x + lifeups[i].rect.h < 0)
				lifeups[i].available = false;
		}

	if (waitingNewBoardTime <= 0)
	{
		MakeNewBoard(screenHeight);
		if (lcg_rand() % 20 == 0)
			MakeNewLifeup(iBoard);
	}

	waitingNewBoardTime -= (gameinfo.level + 1);
}

int curMenu = 0;
bool recordMode;
bool replayAvailable;

void InitTitleScene()
{
	curMenu = 0;
	repinput.Reset();
	if (repinput.LoadFromFile(szReplayFileFullPath))
		replayAvailable = true;
	else
		replayAvailable = false;
	ResetGame();
}

void StartRecordModeFromTitle()
{
	recordMode = true;
	repinput.Reset();
	repinput.BeginRecord();
	ResetBall();
	repinput.UpdateInputStateU32((u32)REPINPUT_ID_SRAND, g_srand);
	repinput.UpdateInputStateU32((u32)REPINPUT_ID_TITLE_ROLL, titleRollFrames);
}

void StartRecordModeFromRestart()
{
	recordMode = true;
	repinput.Reset();
	repinput.BeginRecord();
	ResetGame();
	repinput.UpdateInputStateU32((u32)REPINPUT_ID_SRAND, g_srand);
}

void StartReplayMode()
{
	recordMode = false;
	repinput.Reset();
	repinput.LoadFromFile(szReplayFileFullPath);
	repinput.BeginReplay();
	if (repinput.ReplayGetCurrentEntry()->tick == 0 && repinput.ReplayGetCurrentEntry()->values.size() == 4 &&
		repinput.ReplayGetCurrentEntry()->idInput == REPINPUT_ID_SRAND)
	{
		ResetGame(repinput.ReplayGetCurrentEntry()->ValueU32());
		repinput.MovePositionToNext();
	}
	else
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, NULL, "SRand出错。", g_pWindow);
		return;
	}
	if (repinput.ReplayGetCurrentEntry()->idInput == REPINPUT_ID_TITLE_ROLL && repinput.ReplayGetCurrentEntry()->values.size() == 4)
	{
		if (repinput.ReplayGetCurrentEntry()->tick == 0)
		{
			titleRollFrames = repinput.ReplayGetCurrentEntry()->ValueU32();
			repinput.MovePositionToNext();
			while (titleRollFrames)
			{
				RollUp();
				titleRollFrames--;
			}
			ResetBall();
		}
		else
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, NULL, "获取 titleRollFrams出错。", g_pWindow);
			return;
		}
	}
}

void Game()
{
	switch (scene)
	{
	case 1:TitleScene(); break;
	case 2:PlayScene(); break;
	case 3:ScoreScene(); break;
	case 11:InitTitleScene(); scene = 1; break;
	case 12:InitScoreScene(); scene = 3; break;
	case 102:StartRecordModeFromTitle(); scene = 2; break;
	case 103:StartReplayMode(); scene = 2; break;
	case 104:StartRecordModeFromRestart(); scene = 2; break;
	}
}

void ResetGame(unsigned _set_rseed)
{
	gameinfo.Reset();
	levelUpTime = gameinfo.level * 1800;
	waitingNewBoardTime = 30;//忘了设置一个变量导致回放始终不能复原
	titleRollFrames = 0;
	iBoard = 0;
	g_srand = _set_rseed ? _set_rseed : (unsigned)time(NULL);
	lcg_srand(g_srand);
	for (int i = 0; i < SDL_arraysize(boards); i++)
		boards[i].use = false;
	for (int i = 0; i < screenHeight;)
	{
		i += waitingNewBoardTime;
		MakeNewBoard(i);
	}
	for (int i = 0; i < SDL_arraysize(lifeups); i++)
		lifeups[i].available = false;
	ResetBall();
}

void ResetBall()
{
	bool ballOk = false;
	for (int i = 0; i < SDL_arraysize(boards); i++)
		if (boards[i].use&&boards[i].type!=1)
			if (boards[i].centerY > screenHeight / 2 && boards[i].centerY < screenHeight)
			{
				ball->SetBall(boards[i].centerX, boards[i].centerY - Board::rollspeed * 50);
				ballOk = true;
				break;
			}
	if (!ballOk)
	{
		MakeNewBoard(screenHeight / 2);
		ball->SetBall(boards[iBoard].centerX, boards[iBoard].centerY - Board::rollspeed * 50);
	}
}

void TitleScene()
{
	if (keys.pressedBack&&keys.isonpress)
	{
		scene = 0;
		return;
	}
	RollUp();
	Stage();
	imgs[eImgTextTitle]->Draw();
	imgs[eImgTextMenuStart]->Draw();
	if (replayAvailable)
		imgs[eImgTextMenuReplay]->Draw();
	imgs[eImgTextAuthor]->Draw();
	if (keys.isonpress)
	{
		if (imgs[eImgTextMenuStart]->HitTest(keys.x, keys.y) || keys.pressedOK&&curMenu==0)
		{
			scene = 102;
		}
		else if (imgs[eImgTextMenuReplay]->HitTest(keys.x, keys.y) || keys.pressedOK&&curMenu == 1)
		{
			if (replayAvailable)
				scene = 103;
		}
		else if (keys.pressedUp || keys.pressedDown)
		{
			if (replayAvailable)
				curMenu = (curMenu + 1) % 2;
		}
		keys.isonpress = false;
	}
	if (replayAvailable)
	{
		SDL_SetRenderDrawColor(g_pRenderer, 255, 255, 255, 255);
		switch (curMenu)
		{
		case 0:SDL_RenderDrawRect(g_pRenderer, &imgs[eImgTextMenuStart]->rect); break;
		case 1:SDL_RenderDrawRect(g_pRenderer, &imgs[eImgTextMenuReplay]->rect); break;
		}
	}
	titleRollFrames++;
}

int tempScoreX;

void ShowInfo()
{
	if (gameinfo.score == 0)
	{
		textDigitImgs[0]->SetPos(screenWidth - textDigitImgs[0]->rect.w, 0);
		textDigitImgs[0]->Draw();
		imgs[eImgTextScore]->SetPos(textDigitImgs[0]->rect.x - imgs[eImgTextScore]->rect.w, 0);
		imgs[eImgTextScore]->Draw();
	}
	else
	{
		tempScoreX = screenWidth;
		for (int t = gameinfo.score; t > 0; t /= 10)
		{
			tempScoreX -= textDigitImgs[t % 10]->rect.w;
			textDigitImgs[t % 10]->SetPos(tempScoreX, 0);
			textDigitImgs[t % 10]->Draw();
		}
		imgs[eImgTextScore]->SetPos(tempScoreX - imgs[eImgTextScore]->rect.w, 0);
		imgs[eImgTextScore]->Draw();
	}
	imgs[eImgTextLevel]->Draw();
	if(gameinfo.level<10)
	{
		textDigitImgs[gameinfo.level]->SetPos(imgs[eImgTextLevel]->rect.w + imgs[eImgTextLevel]->rect.x, 0);
		textDigitImgs[gameinfo.level]->Draw();
	}
	else
	{
		textDigitImgs[gameinfo.level/10]->SetPos(imgs[eImgTextLevel]->rect.w + imgs[eImgTextLevel]->rect.x, 0);
		textDigitImgs[gameinfo.level/10]->Draw();
		textDigitImgs[gameinfo.level % 10]->SetPos(textDigitImgs[gameinfo.level / 10]->rect.w +
			textDigitImgs[gameinfo.level / 10]->rect.x, 0);
		textDigitImgs[gameinfo.level%10]->Draw();
	}
	for(int i=0;i<gameinfo.maxLife;i++)
	{
		if (i < gameinfo.life)
		{
			imgs[eImgTextBall]->SetPos(ball->radius*2*i, 0);
			imgs[eImgTextBall]->Draw();
		}
		else
		{
			imgs[eImgTextEmptyBall]->SetPos(ball->radius*2*i, 0);
			imgs[eImgTextEmptyBall]->Draw();
		}
	}
}

void UpdateKeys()
{
	if (recordMode)
	{
		repinput.UpdateInputStateU8(REPINPUT_ID_LEFT, keys.pressedLeft);
		repinput.UpdateInputStateU8(REPINPUT_ID_RIGHT, keys.pressedRight);
	}
	else
	{
		repinput.ReplayInputState();
	}
}

void PlayScene()
{
	RollUp();
	Stage();
	Coll();
	ShowInfo();
	ball->TakeAcc();
	UpdateKeys();
	if (levelUpTime < 0 && gameinfo.level < 99)
	{
		gameinfo.level++;
		levelUpTime = gameinfo.level * 1800;
	}
	levelUpTime--;
	if (gameinfo.life < 0)scene = 12;
	if (repinput.GetInputStates().u8Inputs[REPINPUT_ID_LEFT]&&ball->centerX > 0)
		ball->Move(-(Board::rollspeed * 4+gameinfo.level));
	if (repinput.GetInputStates().u8Inputs[REPINPUT_ID_RIGHT]&&ball->centerX < screenWidth)
		ball->Move(Board::rollspeed * 4+gameinfo.level);//水平速度随着等级提升
	if (keys.pressedBack)
	{
		scene = 11;
		keys.isonpress = false;
	}
	repinput.TickCounterAdd();
}

int digitsx[10];
SDL_Rect scoreRect;
int replaySaved;

void SaveReplay()
{
	if (replaySaved == 1)
		return;
	if (repinput.SaveToFile(szReplayFileFullPath)==0)
	{
		if(SDL_GetPrefPath("lxfly2000", "RapidBall")==NULL||repinput.SaveToFile(szReplayFileFullPath)==0)
		{
			replaySaved = -1;
			return;
		}
	}
	replaySaved = 1;
}

void ScoreScene()
{
	Stage();
	ShowInfo();
	SDL_SetRenderDrawColor(g_pRenderer, 29, 172, 236, 64);
	SDL_RenderFillRect(g_pRenderer, &scoreRect);
	Img *pImgSaveReplayOption;
	if (replaySaved == 1)
		pImgSaveReplayOption = imgs[eImgTextMenuReplaySaved];
	else if (replaySaved == -1)
		pImgSaveReplayOption = imgs[eImgTextMenuReplaySaveFailed];
	else
		pImgSaveReplayOption = imgs[eImgTextMenuSaveReplay];
	imgs[eImgTextYourScore]->Draw();
	imgs[eImgTextMenuRetry]->Draw();
	pImgSaveReplayOption->Draw();
	imgs[eImgTextMenuReturn]->Draw();
	int i = 0;
	for (int t = gameinfo.score; t > 0; t /= 10)
	{
		textDigitImgs[t % 10]->SetPos(digitsx[i], (screenHeight - textDigitImgs[0]->rect.h) / 2);
		textDigitImgs[t % 10]->Draw();
		i++;
	}
	SDL_SetRenderDrawColor(g_pRenderer, 255, 255, 255, 255);
	switch (curMenu)
	{
	case 0:SDL_RenderDrawRect(g_pRenderer, &imgs[eImgTextMenuRetry]->rect); break;
	case 1:SDL_RenderDrawRect(g_pRenderer, &pImgSaveReplayOption->rect); break;
	case 2:SDL_RenderDrawRect(g_pRenderer, &imgs[eImgTextMenuReturn]->rect); break;
	}
	if (keys.isonpress)
	{
		if (imgs[eImgTextMenuRetry]->HitTest(keys.x, keys.y) || keys.pressedOK&&curMenu == 0)
		{
			if (recordMode)
				scene = 104;
			else
				scene = 103;
		}
		else if (pImgSaveReplayOption->HitTest(keys.x, keys.y) || keys.pressedOK&&curMenu == 1)
		{
			SaveReplay();
			curMenu = 1;
		}
		else if (imgs[eImgTextMenuReturn]->HitTest(keys.x, keys.y) || keys.pressedOK&&curMenu == 2)
			scene = 11;
		else if (keys.pressedUp)
			curMenu = (curMenu + 2) % 3;
		else if (keys.pressedDown)
			curMenu = (curMenu + 1) % 3;
		keys.isonpress = false;
	}
}

void InitScoreScene()
{
	int n = 0;
	keys.x = keys.y = 0;
	curMenu = 0;
	if (recordMode)
	{
		replaySaved = 0;
		repinput.EndRecord();
	}
	else
	{
		replaySaved = 1;
		repinput.EndReplay();
	}
	scoreRect = { 0,screenHeight / 4,screenWidth,screenHeight / 2 + imgs[eImgTextMenuReturn]->rect.h / 2 + imgs[eImgTextMenuSaveReplay]->rect.h };
	for (int t = gameinfo.score; t > 0; t /= 10)
		n++;
	for (int i = n - 1; i >= 0; i--)
		digitsx[i] = screenWidth / 2 - n*textDigitImgs[0]->rect.w / 2 + (n - i - 1)*textDigitImgs[0]->rect.w;
}
