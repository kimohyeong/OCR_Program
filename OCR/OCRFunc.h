#pragma once
#include "opencv2\opencv.hpp"
#include <iostream>
#include <fstream>
#define MAX_DATA_COUNT 2000
#define COLOR_CHECK_RANGE 100
#define STANDARD_NUM 52
#define EROSION 1
#define DILATION 2

using namespace cv;
using namespace std;
struct  Letter
{
	//���� ������ �ϳ��� ��Ÿ���� ����ü. �츮�� 32*48�� ũ��� �����ϳ��� �����Ѵ�.
	char value;
	unsigned int img[48];
};
struct my_Rect
{
	Point start;
	Point end;
};
struct Data
{
	Letter letter;
	my_Rect rect;
	bool isFixed;
	bool nextIsSpace;	//�� ���� ������ ���⸦ �߰��ؾ��ϴ°�?
};
struct AllData {
	//�̹��� ���� ��� ���� �����͸� �����ϴ� ����ü
	int count;
	Data data[MAX_DATA_COUNT];
};
struct Standard {
	Letter letter[STANDARD_NUM];
	int count;
};

class OCRFunc
{
private:
	Mat img;
	AllData all;
	Standard standard;
	String path;
	int gapSpace = 0;
public:
	OCRFunc();
	~OCRFunc();

	void initOCR();
	void setImg(String path);
	void imgThresholding();
	void ParsingStepFirst();
	void ParsingStepSecond(int top, int bottom);
	void ParsingStepThird(my_Rect* rect);
	void AddSpaces(int startCount, int endCount, int stdGap);
	void ShowResultImage(int height, int width, int top, int startX);

	void MakeImageData();
	void CreateStandard();
	void FindLetterValue();
	void StoreResult();

	void CorrectBigsmall();
	void ChangeBigSmall(char *, int);
};

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nlnitVal);
void MemFree2D(unsigned char** Mem, int nHeight);
unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize);
unsigned char getMax(unsigned char* arr, int arrLength);
unsigned char getMin(unsigned char* arr, int arrLength);
void bottomHat(unsigned char **Img_in, unsigned char** Out, int nHeight, int nWidth, int nFilterSize, int type);