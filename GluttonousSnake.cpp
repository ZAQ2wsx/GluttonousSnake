// GluttonousSnake.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEBUGMODE							// ����ģʽ����

#define	WIDTH				36				// ��ͼ���
#define	HEIGHT				28				// ��ͼ�߶�
#define	UNIT				20				// ÿ����ͼ��Ƭ��ʵ������
#define AMOUNT				WIDTH*HEIGHT	// ��Ƭ����

#define SNAKE_MOVE_SPEED	200				// �ߵ��ƶ��ٶ�
#define SPEED_STEP			25				// �ƶ��ٶȱ仯����
#define SPEED_FAST			100				// �ƶ��ٶȷ�ֵ

#define SNAKE_HEAD_COLOR	0x00FFFF		// �ߵ�ͷ����ɫ
#define SNAKE_BODY_COLOR	0x0000FF		// �ߵ�������ɫ
#define SNAKE_TAIL_COLOR	0xFFCC00		// �ߵ�β����ɫ
#define SNAKE_FOOD_COLOR	0x00FF00		// ʳ����ɫ
#define UNIT_CLEAR			0x000000		// ��Ƭ��ԭ
#define DEBUG_UNIT_COLOR	0x5C5C5C		// ����ģʽ��Ƭ�߿�ɫ

// ��������
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

// ��Ƭ��������
enum DRAW
{
	HEAD,	// �ߵ�ͷ��
	BODY,	// �ߵ�����
	TAIL,	// �ߵ�β��
	CLEAR,  // �����Ƴ�
	FOOD,	// ʳ��
	DEBUG,	// ���Ƶ�����Ƭ
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
		GetCmd();// �Ӽ��̻�ȡ�ƶ�����ָ��
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

// ��Ϸ��ʼ�����¿�ʼ��
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

// ����ʳ��
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

// ��Ϸ�˳�
void Quit()
{
	closegraph(); 
	exit(0);
}

// ��Ƭ��ͼ������ͼ
void DebugView()
{
	int i;
	for(i=0; i<WIDTH*HEIGHT; ++i)
	{
		DrawUnit(i/HEIGHT,i%HEIGHT,DEBUG_UNIT_COLOR,DEBUG);
	}
}

DWORD m_oldtime;
// ��ȡ��������
void GetCmd()
{
	// ��ȡ����ֵ
	while(true)
	{	

		DWORD newtime = GetTickCount();
		if (newtime - m_oldtime >= g_SnakeMoveSpeed)
		{
			m_oldtime = newtime;
			DispatchCmd(g_Cmd);
		}
		// ����а��������ذ�����Ӧ�Ĺ���
		if (kbhit())
		{
			switch(getch())
			{
				// �෴�����ƶ��������
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
		// ��ʱ (���� CPU ռ����)
		Sleep(20);
	}
}

// �ߵ��ƶ��ٶȱ仯
void DifficultyChange()
{
	if (g_SnakeMoveSpeed > SPEED_FAST)
	{
		g_SnakeMoveSpeed -= SPEED_STEP;
	}

	//����������
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

// �ַ���������
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

// �����ƶ�����
void OnLeft()
{
	g_SnakeBody[0].dir = CMD_LEFT;
	g_SnakeBody[0].pos.x -= 1;
}

//�����ƶ�����
void OnUp()
{
	g_SnakeBody[0].dir = CMD_UP;
	g_SnakeBody[0].pos.y += 1;
}

// �����ƶ�����
void OnRight()
{
	g_SnakeBody[0].dir = CMD_RIGHT;
	g_SnakeBody[0].pos.x += 1;
}

// �����ƶ�����
void OnDown()
{
	g_SnakeBody[0].dir = CMD_DOWN;
	g_SnakeBody[0].pos.y -= 1;
}

// �����ƶ��߼�
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

//�������
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

// ��ײ���
void CollisionDetection()
{
	// �Ե�ʳ��
	if(g_SnakeBody[0].pos.x==g_SnakeFood.pos.x && g_SnakeBody[0].pos.y==g_SnakeFood.pos.y)
	{
		DifficultyChange();
		GenerateFood();
	}
	
	// ����ǽ��
	if(g_SnakeBody[0].pos.x<0 || g_SnakeBody[0].pos.x>=WIDTH || g_SnakeBody[0].pos.y<0 || g_SnakeBody[0].pos.y>=HEIGHT)
	{
		HWND wnd = GetHWnd();
		char gameLoseInfo[] = "�Ƿ������Ϸ?\n���\"��\"���¿�ʼ\n���\"��\"������Ϸ";
		if (MessageBox(wnd, _T(gameLoseInfo), _T("��Ϸ����"),  MB_YESNO | MB_ICONQUESTION) == IDYES)
			NewGame();
		else
			Quit();
	}
	// �����Լ�����
	int i = 1;
	while(i!=g_SnakeLength)
	{
		if(g_SnakeBody[i].pos.x == g_SnakeBody[0].pos.x && g_SnakeBody[i].pos.y == g_SnakeBody[0].pos.y)
		{
			HWND wnd = GetHWnd();
			char gameLoseInfo[] = "�Ƿ������Ϸ?\n���\"��\"���¿�ʼ\n���\"��\"������Ϸ";
			if (MessageBox(wnd, _T(gameLoseInfo), _T("��Ϸ����"),  MB_YESNO | MB_ICONQUESTION) == IDYES)
				NewGame();
			else
				Quit();
		}
		++i;
	}

}

//��Ƭ����
void DrawUnit(int x, int y, COLORREF c, DRAW _draw)
{
	// ��Ƭ����ת��ͼ����
	int left	= x * UNIT;
	int top		= (HEIGHT - y - 1) * UNIT;
	int right	= (x + 1) * UNIT - 1;
	int bottom	= (HEIGHT - y) * UNIT - 1;
	
	// ��Ƭ����
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
		case DEBUG:// ���Ƶ�����Ƭ
			setlinecolor(c);
 			roundrect(left, top, right, bottom, 5, 5);
			break;
	}
}