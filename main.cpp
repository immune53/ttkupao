/*
天天酷跑开发日志
1.创建项目
2.导入素材
3.创建游戏界面
	实际的开发流程
	对于初学者建议从用户界面入手
	选择图形库或者其他引擎
	天天酷跑，是基于“easyx"图形库
	1)创建游戏窗口
	2）设计游戏背景
		a.3重背景不同速度同时移动
		b.循环滚动背景的实现
	3）实现游戏背景
		a.加载背景资源
		b.渲染（背景知识：坐标）
		遇到问题：1.背景图片的png格式图片出现黑色 使用开源的新的接口载入透明背景
				  2.图片滚动出现闪的情况 使用批量绘制代码 BeginBatchDraw()  EndBatchDraw();
 4.实现玩家的奔跑
 5.实现玩家的跳跃
 6.实现随机小乌龟
 7.创建障碍物结构体数据类型
 8.使用障碍物结构体后重新初始化
 9.封装后多个障碍物的显示
 10.实现玩家的下蹲技能
 11.实现柱子障碍物
 12.实现碰撞检测
	判断两个矩形是否相交
 13.分数检测
 14.显示分数
 15.判断结束条件
*/

#define win_width 1012
#define win_height 396
#include<stdio.h>
#include <graphics.h>
#include "tools.h"
#include <conio.h>
#include<vector>
#define OBSTACLE_COUNT 100
#define WIN_SCORE 50

using namespace std;//声明命名空间
IMAGE imgBgs[3];//背景图片
int bgX[3];//背景图片的x坐标
int bgSpeed[3] = { 1,2,4 };
IMAGE imgHeros[12];
int heroX;//玩家的X坐标
int heroY;//玩家的Y坐标
int heroIndex;//玩家奔跑时的图片帧序号
bool heroJump;//表示玩家正在跳跃
int jumpHeight_max;//跳跃的最高高度
int heroJumpOff;
bool update;//是否是要马上刷新
int heroBlood;
int score;//分数
IMAGE imgSZ[10];
typedef enum {
	TORTOISE,//乌龟 0
	LION,//狮子 1
	HOOK1,
	HOOK2,
	HOOK3,
	HOOK4,
	OBSTACLE_TYPE_COUNT//2
}obstacle_type;

vector<vector<IMAGE>> obstacleImgs;//存放所有障碍物的图片
IMAGE imgHeroDown[2];
bool heroDown;//玩家是否正在下蹲
typedef struct obstacle {
	int type;//障碍物的类型
	int imgIndex;
	int x, y;//障碍物的坐标
	int speed;
	int power;
	bool exist;
	bool hited;
	bool passed;

}obstacle_t;

obstacle_t obstacles[OBSTACLE_COUNT];
int lastObsIndex;
//IMAGE imgTortoise;//小乌龟
//int torToiseX;//小乌龟的水平坐标
//int torToiseY;//小乌龟的Y坐标
//bool tortoiseExist;//当前窗口是否有小乌龟
void restart() {
	heroBlood = 100;
	score = 0;
	mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
}
void creatObstacle()
{
	int i;
	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist == false)
		{
			break;
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;
	}
	obstacles[i].exist = true;
	obstacles[i].imgIndex = 0;
	obstacles[i].hited = false;
	obstacles[i].passed = false;
	//obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].type = (obstacle_type)(rand() % 3);
	if (obstacles[i].type == HOOK1) {
		obstacles[i].type += rand() % 4;
	}
	if (lastObsIndex >= 0 &&
		obstacles[lastObsIndex].type >= HOOK1 &&
		obstacles[lastObsIndex].type <= HOOK4 &&
		obstacles[i].type == LION && obstacles[lastObsIndex].x > (win_width - 500))
	{
		obstacles[i].type = TORTOISE;
	}
	lastObsIndex = i;
	obstacles[i].x = win_width;
	obstacles[i].y = 345 + 5 - obstacleImgs[obstacles[i].type][0].getheight();
	if (obstacles[i].type == TORTOISE)
	{
		obstacles[i].speed = 0;
		obstacles[i].power = 5;//伤害值
	}
	else if (obstacles[i].type == LION)
	{
		obstacles[i].speed = 4;
		obstacles[i].power = 20;
	}
	else if (obstacles[i].type >=HOOK1 && obstacles[i].type<=HOOK4)
	{
		obstacles[i].speed = 0;
		obstacles[i].power = 20;
		obstacles[i].y = 0;
	}
}
void checkHit()
{
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist && obstacles[i].hited == false )
		{
			int a1x, a1y, a2x, a2y;
			int off = 30;
			if (!heroDown)//非下蹲（就是奔跑，跳跃）
			{
				a1x = heroX + off;
				a1y = heroY + off;
				a2x = heroX + imgHeros[heroIndex].getwidth() - off;
				a2y = heroY + imgHeros[heroIndex].getheight();

			}
			else {
				a1x = heroX + off;
				a1y = 345 - imgHeroDown[heroIndex].getheight();
				a2x = heroX + imgHeroDown[heroIndex].getwidth() - off;
				a2y = 345;
			}
			IMAGE img = obstacleImgs[obstacles[i].type][obstacles[i].imgIndex];
			int b1x = obstacles[i].x + off;
			int b1y = obstacles[i].y + off;
			int b2x = obstacles[i].x + img.getwidth() - off;
			int b2y = obstacles[i].y + img.getheight() - 10;

			//判断相交
			if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b2y, b2x, b2y))
			{
				heroBlood -= obstacles[i].power;
				playSound("res/hit.mp3");
				obstacles[i].hited = true;
			}
		}
	}
}
//循环滚动效果
void fly()
{
	for (int i = 0; i < 3; i++)
	{
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -win_width) {
			bgX[i] = 0;
		}
	}
	//实现跳跃
	if (heroJump) {
		if (heroY < jumpHeight_max) {
			heroJumpOff = 4;
		}
		heroY += heroJumpOff;
		if (heroY > 345 - imgHeros[0].getheight()) {
			heroJump = false;
			heroJumpOff = -4;
		}
	}
	else if (heroDown)
	{
		static int count = 0;
		int delay[2] = { 8,30};
		count++;
		if (count >= delay[heroIndex]) {
			count = 0;
			heroIndex++;
			if (heroIndex >= 2)
			{
				heroIndex = 0;
				heroDown = false;
			}
		}
	}
	else
	{  //不跳跃
		heroIndex = (heroIndex + 1) % 12;
	}
	//创建小乌龟
	static int framecount = 0;
	static int enemyFre = 50;//障碍物刷新频率
	framecount++;
	if (framecount > enemyFre)
	{
		framecount = 0;
		enemyFre = 50 + rand() % 50;//50-99
		creatObstacle();
		/*if (!tortoiseExist) {
			tortoiseExist = true;
			torToiseX = win_width;
			enemyFre = 100 + rand() % 300;
		}*/
	}
	//if (tortoiseExist)
	//{
	//	torToiseX -= bgSpeed[2];
	//	if (torToiseX < -imgTortoise.getwidth())
	//	{
	//		tortoiseExist = false;
	//	}
	//}
	//更新所有障碍物坐标
	for(int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist)
		{
			obstacles[i].x -= obstacles[i].speed + bgSpeed[2];
			if (obstacles[i].x < -obstacleImgs[obstacles[i].type][0].getwidth() * 2)
			{
				obstacles[i].exist = false;
			}
			int len = obstacleImgs[obstacles[i].type].size();
			obstacles[i].imgIndex = (obstacles[i].imgIndex + 1) % len;
		}
	}
	//玩家和障碍物的“碰撞检测”
	checkHit();

}

//渲染游戏背景
void updateBg()
{
	putimagePNG2(bgX[0],0,&imgBgs[0]);
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}
void init()
{
	//创建游戏窗口
	initgraph(win_width, win_height);
	//加载游戏资源
	char name[64];
	for (int i = 0; i < 3; i++)
	{
		//"res/bg001.png"  "res/bg002.png"  "res/bg003.png" 
		sprintf(name, "res/bg00%d.png", i + 1);//sprintf  把路径输入到name字符串中
		loadimage(&imgBgs[i], name);
		bgX[i] = 0;
	}

	//加载Hero的图片帧素材
	for (int i = 0; i < 12; i++)
	{
		//"res/hero1.png".... "res/hero2.png"
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHeros[i], name);
	}
	//设置玩家的初始位置
	heroX = win_width * 0.5 - imgHeros[0].getwidth() * 0.5;
	heroY = 345 - imgHeros[0].getheight();
	heroIndex = 0;
	heroJump = false;
	jumpHeight_max= 345 - imgHeros[0].getheight()-120;
	heroJumpOff = -4;
	update = true;
	//加载小乌龟素材
	/*loadimage(&imgTortoise, "res/t1.png");
	tortoiseExist = false;
	torToiseY = 350 - imgTortoise.getheight();*/
	IMAGE imgtort;
	loadimage(&imgtort, "res/t1.png");
	vector<IMAGE>imgTortArray;
	imgTortArray.push_back(imgtort);
	obstacleImgs.push_back(imgTortArray);

	IMAGE imgLion;
	vector<IMAGE>imgLionArray;
	for (int i = 0; i < 6; i++)
	{
		sprintf(name, "res/p%0d.png", i + 1);
		loadimage(&imgLion,name);
		imgLionArray.push_back(imgLion);
	}
	obstacleImgs.push_back(imgLionArray);
	//初始化障碍物
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		obstacles[i].exist = false;
	}
	//加载下蹲素材
	loadimage(&imgHeroDown[0], "res/d1.png");
	loadimage(&imgHeroDown[1], "res/d2.png");
	heroDown = false;
	//加载柱子障碍物
	IMAGE imgH;
	for (int i = 0; i < 4; i++)
	{	
		vector<IMAGE> imgHookArray;
		sprintf(name, "res/h%d.png", i + 1);
		loadimage(&imgH, name,63,260,true);
		imgHookArray.push_back(imgH);
		obstacleImgs.push_back(imgHookArray);
	}
	//英雄血量
	heroBlood = 100;
	//预加载音效
	preLoadSound("res/hit.mp3");

	mciSendString("play res/bg.mp3 repeat", 0, 0, 0);//播放背景音乐
	lastObsIndex = -1;
	score = 0;
	//加载数字图片
	for (int i = 0; i < 10; i++)
	{
		sprintf(name, "res/sz/%d.png",i);
		loadimage(&imgSZ[i], name);
	}
}
void jump()
{
	heroJump = true;
	update = true;
}
void down() {
	heroDown = true;
	update = true;
	heroIndex = 0;
}
void keyEvent()
{	
	char ch;
	if (kbhit())//如果有按键，kbhit()返回true
	{
		ch=getch();//getch()不需要按下回车即可直接读取
		if (ch == ' ')
		{
			jump();
		}
		else if (ch == 'a') {
			down();
		}

	}
}
void updateEnemy()
{
	////渲染小乌龟
	//if (tortoiseExist)
	//{
	//	putimagePNG2(torToiseX, torToiseY, win_width, &imgTortoise);
	//}
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist)
		{
			putimagePNG2(obstacles[i].x, obstacles[i].y, win_width, &obstacleImgs[obstacles[i].type][obstacles[i].imgIndex]);
		}
	}
}

void updateHero() {
	if (!heroDown)
	{
		putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
	}else
	{
		int y = 345 - imgHeroDown[heroIndex].getheight();
		putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
	}
}
void updateBloodbar()
{
	drawBloodBar(10, 10, 200, 10, 2, BLUE,DARKGRAY, RED, heroBlood / 100.0);
}
void checkOver()
{
	if (heroBlood <= 0)
	{
		loadimage(0, "res/over.png");
		FlushBatchDraw();
		mciSendString("stop res/bg/mp3", 0, 0, 0);
		system("pause");
		//暂停之后，充币复活，或者直接开始下一局
		restart();
	}
}
void checkScore() {
	for (int i = 0;i< OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist && obstacles[i].passed == false && obstacles[i].hited==false
			&& obstacles[i].x + obstacleImgs[obstacles[i].type][0].getwidth() < heroX) {
			score++;
			obstacles[i].passed = true;
		}
	}
}
void updateScore()
{
	char str[8];
	int x = 20;
	int y = 25;
	sprintf(str, "%d", score);
	for (int i = 0; str[i]; i++)
	{
		int sz = str[i] - '0';
		putimagePNG(x, y, &imgSZ[sz]);
		x += imgSZ[sz].getwidth() + 5;
	}
}

void checkWin() {
	if (score >= WIN_SCORE)
	{
		FlushBatchDraw();
		mciSendString("play res/win.mp3", 0, 0, 0);
		Sleep(2000);
		loadimage(0, "res/win.png");
		FlushBatchDraw();
		mciSendString("stop res/bg.mp3", 0, 0, 0);
		system("pause");
		restart();//重新初始化

	}
}
int main()
{
	init();
	//游戏界面初始化
	loadimage(0, "res/over.png");
	system("pause");
	int timer=0;
	while (1)
	{	
		keyEvent();
		timer += getDelay();
		if (timer > 30) 
		{
			timer = 0;
			update = true;
		}
		if (update)
		{	
			update = false;
			BeginBatchDraw();
			updateBg();
			//putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
			updateHero();
			updateEnemy();
			updateBloodbar();
			updateScore();
			checkWin();
			EndBatchDraw();

			checkOver();
			checkScore();
			fly();
		}

	}
	system("pause");
	return 0;
}