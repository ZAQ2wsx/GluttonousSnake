// GluttonousSnake.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEBUGMODE							// 调试模式开关

#define	WIDTH				36				// 地图宽度
#define	HEIGHT				28				// 地图高度
#define	UNIT				20				// 每个地图瓦片的实际像素
#define AMOUNT				WIDTH*HEIGHT	// 瓦片总数

#define SNAKE_MOVE_SPEED	200				// 蛇的移动速度
#define SPEED_STEP			25				// 移动速度变化步长
#define SPEED_FAST			100				// 移动速度阀值

#define SNAKE_HEAD_COLOR	0x00FFFF		// 蛇的头部颜色
#define SNAKE_BODY_COLOR	0x0000FF		// 蛇的身体颜色
#define SNAKE_TAIL_COLOR	0xFFCC00		// 蛇的尾巴颜色
#define SNAKE_FOOD_COLOR	0x00FF00		// 食物颜色
#define UNIT_CLEAR			0x000000		// 瓦片还原
#define DEBUG_UNIT_COLOR	0x5C5C5C		// 调试模式瓦片边框色

// 按键命令
enum CMD
{
	CMD_LEFT,
	CMD_UP,
	CMD_RIGHT,
	CMD_DOWN,
	CMD_PAUSE,
	CMD_RESUME,
	CMD_QUIT,
};

// 瓦片绘制类型
enum DRAW
{
	HEAD,	// 蛇的头部
	BODY,	// 蛇的身体
	TAIL,	// 蛇的尾部
	CLEAR,  // 蛇身移除
	FOOD,	// 食物
	DEBUG,	// 绘制调试瓦片
};

typedef struct postion
{
	int x:8;
	int y:8;
}postion;

typedef struct SNAKEBODY
{
	CMD		dir:4;
	postion pos; 
}SNAKEBODY;


typedef struct FOODBODY
{
	int		score:4;
	postion pos;
}FOODBODY;

SNAKEBODY	g_SnakeBody[AMOUNT] = {0};
DWORD		g_SnakeMoveSpeed	= SNAKE_MOVE_SPEED;
FOODBODY	g_SnakeFood			= {0};
int			g_SnakeLength		= 3;
CMD			g_Cmd				= CMD_LEFT;

void GetCmd();
void DebugView();
void DispatchCmd(CMD _cmd);
void OnUp();
void OnDown();
void OnLeft();
void OnRight();
void DrawUnit(int x, int y, COLORREF c, DRAW _draw);
void DifficultyChange();
void GenerateFood();
void Init();
void Quit();
void SnakeMoveLogic(void (*)());
void CollisionDetection();
void DrawSnake();
void NewGame();

int main(int argc, char* argv[])
{

	Init();
	while(true)
	{
		GetCmd();// 从键盘获取移动方向指令
	}
	return 0;
}

void Init()
{
	initgraph(WIDTH*UNIT, HEIGHT*UNIT);
	
	#ifdef DEBUGMODE
		DebugView();
	#endif
	NewGame();
}

// 游戏开始（重新开始）
void NewGame()
{
	g_SnakeMoveSpeed = SNAKE_MOVE_SPEED;
	g_SnakeLength = 3;
	memset(g_SnakeBody,0,sizeof(SNAKEBODY)*AMOUNT);
	g_Cmd = CMD_LEFT;
	int i = 0;
	while(i<g_SnakeLength)
	{
		g_SnakeBody[i].dir	 = CMD_LEFT;
		g_SnakeBody[i].pos.x = 16+i;
		g_SnakeBody[i].pos.y = 15;
		i++;
	}
	DrawUnit(g_SnakeFood.pos.x,g_SnakeFood.pos.y,DEBUG_UNIT_COLOR,CLEAR);
	GenerateFood();
	DrawSnake();
}

// 创建食物
void GenerateFood()
{
	srand(time(NULL));
	FOODBODY f={1, rand()%WIDTH, rand()%HEIGHT};
	
	int i = g_SnakeLength-1;
	while (i>=0)
	{
		while(g_SnakeBody[i].pos.x==f.pos.x && g_SnakeBody[i].pos.y==f.pos.y)
		{
			f.pos.x = rand()%WIDTH;
			f.pos.y = rand()%HEIGHT;
		}
		--i;
	}
	g_SnakeFood = f;
	DrawUnit(f.pos.x, f.pos.y, SNAKE_FOOD_COLOR, FOOD);
}

// 游戏退出
void Quit()
{
	closegraph(); 
	exit(0);
}

// 瓦片地图调试视图
void DebugView()
{
	int i;
	for(i=0; i<WIDTH*HEIGHT; ++i)
	{
		DrawUnit(i/HEIGHT,i%HEIGHT,DEBUG_UNIT_COLOR,DEBUG);
	}
}

DWORD m_oldtime;
// 获取控制命令
void GetCmd()
{
	// 获取控制值
	while(true)
	{	

		DWORD newtime = GetTickCount();
		if (newtime - m_oldtime >= g_SnakeMoveSpeed)
		{
			m_oldtime = newtime;
			DispatchCmd(g_Cmd);
		}
		// 如果有按键，返回按键对应的功能
		if (kbhit())
		{
			switch(getch())
			{
				// 相反方向移动命令抵消
				case 'w':
				case 'W':	g_Cmd = g_Cmd^CMD_DOWN	? CMD_UP	: CMD_DOWN;		break;
				case 'a':
				case 'A':	g_Cmd = g_Cmd^CMD_RIGHT ? CMD_LEFT	: CMD_RIGHT;	break;
				case 'd':
				case 'D':	g_Cmd = g_Cmd^CMD_LEFT	? CMD_RIGHT	: CMD_LEFT;		break;
				case 's':
				case 'S':	g_Cmd = g_Cmd^CMD_UP	? CMD_DOWN	: CMD_UP;		break;
				case ' ':	g_Cmd = CMD_PAUSE;									break;
				case 27:	g_Cmd = CMD_QUIT;									break;
				case 0:
				case 0xE0:
					switch(getch())
					{
						case 72:	g_Cmd = g_Cmd^CMD_DOWN	? CMD_UP	: CMD_DOWN;		break;
						case 75:	g_Cmd = g_Cmd^CMD_RIGHT ? CMD_LEFT	: CMD_RIGHT;	break;
						case 77:	g_Cmd = g_Cmd^CMD_LEFT	? CMD_RIGHT	: CMD_LEFT;		break;
						case 80:	g_Cmd = g_Cmd^CMD_UP	? CMD_DOWN	: CMD_UP;		break;
					}
			}
		}
		// 延时 (降低 CPU 占用率)
		Sleep(20);
	}
}

// 蛇的移动速度变化
void DifficultyChange()
{
	if (g_SnakeMoveSpeed > SPEED_FAST)
	{
		g_SnakeMoveSpeed -= SPEED_STEP;
	}

	//增加蛇身长度
	CMD LastUnitDir =  g_SnakeBody[g_SnakeLength-1].dir;
	g_SnakeBody[g_SnakeLength] = g_SnakeBody[g_SnakeLength-1];
	if (LastUnitDir == CMD_LEFT)
	{
		g_SnakeBody[g_SnakeLength].pos.x += 1;
		g_SnakeBody[g_SnakeLength].dir = CMD_LEFT;
	}
	else if (LastUnitDir == CMD_UP)
	{
		g_SnakeBody[g_SnakeLength].pos.y -= 1;
		g_SnakeBody[g_SnakeLength].dir = CMD_UP;
	}
	else if (LastUnitDir == CMD_RIGHT)
	{
		g_SnakeBody[g_SnakeLength].pos.x -= 1;
		g_SnakeBody[g_SnakeLength].dir = CMD_RIGHT;
	}
	else if (LastUnitDir == CMD_DOWN)
	{
		g_SnakeBody[g_SnakeLength].pos.y += 1;
		g_SnakeBody[g_SnakeLength].dir = CMD_DOWN;
	}
	++g_SnakeLength;
}

// 分发控制命令
void DispatchCmd(CMD _cmd)
{
	switch(_cmd)
	{
		case CMD_UP:	SnakeMoveLogic(OnUp);		break;
		case CMD_LEFT:	SnakeMoveLogic(OnLeft);		break;
		case CMD_RIGHT:	SnakeMoveLogic(OnRight);	break;
		case CMD_DOWN:	SnakeMoveLogic(OnDown);		break;
		case CMD_QUIT:	Quit();						break;
	}
}

// 向左移动命令
void OnLeft()
{
	g_SnakeBody[0].dir = CMD_LEFT;
	g_SnakeBody[0].pos.x -= 1;
}

//向上移动命令
void OnUp()
{
	g_SnakeBody[0].dir = CMD_UP;
	g_SnakeBody[0].pos.y += 1;
}

// 向右移动命令
void OnRight()
{
	g_SnakeBody[0].dir = CMD_RIGHT;
	g_SnakeBody[0].pos.x += 1;
}

// 向下移动命令
void OnDown()
{
	g_SnakeBody[0].dir = CMD_DOWN;
	g_SnakeBody[0].pos.y -= 1;
}

// 蛇身移动逻辑
void SnakeMoveLogic(void (*SnakeHeadLogic)())
{
	int	i=0;
	while(i<g_SnakeLength)
	{
		DrawUnit(g_SnakeBody[i].pos.x, g_SnakeBody[i].pos.y, DEBUG_UNIT_COLOR, CLEAR);
		++i;
	}
	
	i=g_SnakeLength-1;
	while (i>=1)
	{
		g_SnakeBody[i] = g_SnakeBody[i-1];
		--i;
	}
	(*SnakeHeadLogic)();
	CollisionDetection();
	DrawSnake();
}

//蛇身绘制
void  DrawSnake()
{
	int i=0;
	while(i<g_SnakeLength)
	{
		if(i==0)
		{
			DrawUnit(g_SnakeBody[i].pos.x, g_SnakeBody[i].pos.y, SNAKE_HEAD_COLOR, HEAD);
		}
		else if (i == g_SnakeLength-1)
		{
			DrawUnit(g_SnakeBody[i].pos.x, g_SnakeBody[i].pos.y, SNAKE_TAIL_COLOR, TAIL);
		}
		else
		{
			DrawUnit(g_SnakeBody[i].pos.x, g_SnakeBody[i].pos.y, SNAKE_BODY_COLOR, BODY);
		}
		++i;
	}
}

// 碰撞检测
void CollisionDetection()
{
	// 吃到食物
	if(g_SnakeBody[0].pos.x==g_SnakeFood.pos.x && g_SnakeBody[0].pos.y==g_SnakeFood.pos.y)
	{
		DifficultyChange();
		GenerateFood();
	}
	
	// 碰到墙壁
	if(g_SnakeBody[0].pos.x<0 || g_SnakeBody[0].pos.x>=WIDTH || g_SnakeBody[0].pos.y<0 || g_SnakeBody[0].pos.y>=HEIGHT)
	{
		HWND wnd = GetHWnd();
		char gameLoseInfo[] = "是否继续游戏?\n点击\"是\"重新开始\n点击\"否\"结束游戏";
		if (MessageBox(wnd, _T(gameLoseInfo), _T("游戏结束"),  MB_YESNO | MB_ICONQUESTION) == IDYES)
			NewGame();
		else
			Quit();
	}
	// 碰到自己身体
	int i = 1;
	while(i!=g_SnakeLength)
	{
		if(g_SnakeBody[i].pos.x == g_SnakeBody[0].pos.x && g_SnakeBody[i].pos.y == g_SnakeBody[0].pos.y)
		{
			HWND wnd = GetHWnd();
			char gameLoseInfo[] = "是否继续游戏?\n点击\"是\"重新开始\n点击\"否\"结束游戏";
			if (MessageBox(wnd, _T(gameLoseInfo), _T("游戏结束"),  MB_YESNO | MB_ICONQUESTION) == IDYES)
				NewGame();
			else
				Quit();
		}
		++i;
	}

}

//瓦片绘制
void DrawUnit(int x, int y, COLORREF c, DRAW _draw)
{
	// 瓦片坐标转地图坐标
	int left	= x * UNIT;
	int top		= (HEIGHT - y - 1) * UNIT;
	int right	= (x + 1) * UNIT - 1;
	int bottom	= (HEIGHT - y) * UNIT - 1;
	
	// 瓦片绘制
	switch(_draw)
	{
		case HEAD:
			setlinecolor(c);
 			roundrect(left, top, right, bottom, 5, 5);
			setfillcolor(c);
			fillrectangle(left+1, top+1, right-1, bottom-1);
			break;
		case BODY:
			setlinecolor(c);
 			roundrect(left, top, right, bottom, 5, 5);
			setfillcolor(c);
			fillrectangle(left+1, top+1, right-1, bottom-1);
			break;
		case TAIL:
			setlinecolor(c);
 			roundrect(left, top, right, bottom, 5, 5);
			setfillcolor(c);
			fillrectangle(left+1, top+1, right-1, bottom-1);
			break;
		case CLEAR:
#ifdef DEBUGMODE
				setlinecolor(c);
 				roundrect(left, top, right, bottom, 5, 5);
				setfillcolor(UNIT_CLEAR);
 				fillrectangle(left, top, right, bottom);
#else
				setlinecolor(UNIT_CLEAR);
 				roundrect(left, top, right, bottom, 5, 5);
				setfillcolor(UNIT_CLEAR);
				fillrectangle(left+1, top+1, right-1, bottom-1);
#endif
			break;
		case FOOD:
				setlinecolor(c);
 				roundrect(left, top, right, bottom, 5, 5);
				setfillcolor(c);
				fillrectangle(left+1, top+1, right-1, bottom-1);
			break;
		case DEBUG:// 绘制调试瓦片
			setlinecolor(c);
 			roundrect(left, top, right, bottom, 5, 5);
			break;
	}
}