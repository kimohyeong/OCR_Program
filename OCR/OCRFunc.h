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
	//글자 데이터 하나를 나타내는 구조체. 우리는 32*48의 크기로 글자하나를 저장한다.
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
	bool nextIsSpace;	//이 글자 다음에 띄어쓰기를 추가해야하는가?
};
struct AllData {
	//이미지 내의 모든 글자 데이터를 저장하는 구조체
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