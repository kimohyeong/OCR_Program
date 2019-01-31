
#include "stdafx.h"
#include "OCRFunc.h"
#include "afxdialogex.h"
#include <direct.h>


OCRFunc::OCRFunc()
{
	//데이터 초기화
	all.count = 0;
	for (int i = 0; i < MAX_DATA_COUNT; i++)
	{
		all.data[i].isFixed = false;
		all.data[i].nextIsSpace = false;
	}
	standard.count = STANDARD_NUM;
}


OCRFunc::~OCRFunc()
{
}
void OCRFunc::initOCR()
{
	//데이터 초기화
	all.count = 0;
	for (int i = 0; i < MAX_DATA_COUNT; i++)
	{
		all.data[i].isFixed = false;
		all.data[i].nextIsSpace = false;
	}

}

void OCRFunc::setImg(String p)
{
	path = p;
	img = imread(p, IMREAD_GRAYSCALE);
}
void OCRFunc::imgThresholding() {
	unsigned char** inputImage = MemAlloc2D(img.rows, img.cols, 0);
	unsigned char** temp = MemAlloc2D(img.rows, img.cols, 0);
	unsigned char** result = MemAlloc2D(img.rows, img.cols, 0);
	unsigned char** last = MemAlloc2D(img.rows, img.cols, 0);
	for (int h = 0; h < img.rows; h++) {
		for (int w = 0; w < img.cols; w++) {
			inputImage[h][w] = img.at<unsigned char>(h, w);
		}
	}
	bottomHat(inputImage, temp, img.rows, img.cols, 15, DILATION);
	bottomHat(temp, result, img.rows, img.cols, 15, EROSION);

	//Thresholding Value
	int Hist[256] = { 0, };
	int sum = 0;
	for (int h = 0; h < img.rows; h++) {
		for (int w = 0; w < img.cols; w++)
		{
			result[h][w] = temp[h][w] - img.at<unsigned char>(h, w);
			Hist[result[h][w]]++;
			sum += result[h][w];
		}
	}

	int threshold = 0;
	int wB = 0;	//background의 weight
	int wF = 0;
	float sumB = 0;
	float varMax = 0;
	float mG = sum / (img.rows*img.cols);
	float subSumB = 0;
	float subSumF = 0;
	cout << threshold << endl;

	//threshold값을 변경시키면서 가장 분산을 크게 만드는 값을 찾는다.
	for (int t = 0; t < 256; t++) {
		wB += Hist[t];
		if (wB == 0)
			continue;
		wF = (img.rows*img.cols) - wB;
		sumB += (t*Hist[t]);

		//Background와 Foreground의 각각의 평균
		float mB = sumB / wB;
		float mF = (sum - sumB) / wF;

		subSumB += pow(mB - mG, 2);
		subSumF += pow(mF - mG, 2);

		//분산 계산
		float varBetween = (float)wB*wF*(mB - mF)*(mB - mF);
		if (varBetween > varMax) {
			varMax = varBetween;
			threshold = t;
		}
	}
	cout << threshold << endl;

	for (int h = 0; h < img.rows; h++) {
		for (int w = 0; w < img.cols; w++)
		{
			if (temp[h][w] - img.at<unsigned char>(h, w) < threshold)
				result[h][w] = 0;
			else
				result[h][w] = 255;
		}
	}
	bottomHat(result, last, img.rows, img.cols, 2, DILATION);
	bottomHat(last, result, img.rows, img.cols, 1, EROSION);
	for (int h = 0; h < img.rows; h++) {
		for (int w = 0; w < img.cols; w++)
		{
			img.at<unsigned char>(h, w) = 255 - result[h][w];
		}
	}
	MemFree2D(inputImage, img.rows);
	MemFree2D(last, img.rows);
	MemFree2D(temp, img.rows);
	MemFree2D(result, img.rows);
}
/*Step1. 전체 이미지에 대하여 라인 구분(세로선 찾기)*/
void OCRFunc::ParsingStepFirst()
{
	all.count = 0;

	int nWidth = img.cols;
	int nHeight = img.rows;
	int top, bottom;

	unsigned char pixelVal;

	bool isLetter;
	bool prevLineState = false;

	//이미지의 가로 전체를 탐색하지 않고 중간만 탐색해서 대략적인 높이를 알아낸다.
	int xStart = 0;
	int xEnd = nWidth;

	for (int y = 0; y < nHeight; y++)
	{
		isLetter = false;
		for (int x = xStart; x < xEnd; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);

			if (pixelVal < COLOR_CHECK_RANGE)
			{//글자가 있는 라인 발견
				isLetter = true;
				break;
			}
		}
		if (isLetter)
		{//글자라인일 때
			if (!prevLineState)
			{//prevLineState는 이전 라인이 배경일 때 false이다.
			 //따라서 이전라인이 배경이고, 글자라인을 찾았다면, 이 라인은 글자 상단에 해당하는 라인이 된다.
				top = y;
			}
		}
		else
		{
			if (prevLineState)
			{//이전 라인 상태가 글자라인이었고, 현재 배경이면 이 라인의 전 라인이 글자 하단에 해당하는 라인이 된다.
				bottom = y - 1;

				//한 줄의 top과 bottom을 찾았으므로 가로경계를 찾아내는 단계 진행
				ParsingStepSecond(top, bottom);
			}
		}
		prevLineState = isLetter;
	}
}

/*Step2. 하나의 라인 내에서 각 글자의 가로선 구분하기.
가로선 구분 후 마지막에는 줄바꿈을 추가해준다.(라인이 바뀌는 것을 표시하기 위해)
각 글자 사이의 간격들 중 가장 큰 간격을 구한 후 그 길이의 0.7만큼을 띄어쓰기 영역 기준으로 두자.*/
void OCRFunc::ParsingStepSecond(int top, int bottom)
{
	int nWidth = img.cols;

	unsigned char pixelVal;
	bool isLetter;
	bool prevLineState = false;
	int prevX = -1;

	int startCount = all.count;
	gapSpace = 0;
	for (int x = 0; x < nWidth; x++)
	{
		isLetter = false;
		for (int y = top; y <= bottom; y++)
		{
			pixelVal = img.at<unsigned char>(y, x);

			if (pixelVal <COLOR_CHECK_RANGE)
			{//글자영역 찾음
				isLetter = true;
				break;
			}
		}
		if (isLetter)
		{
			if (!prevLineState)
			{
				all.data[all.count].rect.start.x = x;
				all.data[all.count].rect.start.y = top;
			}
		}
		else
		{
			if (prevLineState)
			{
				//한 글자의 가로영역 찾기 완료.
				all.data[all.count].rect.end.x = x - 1;
				all.data[all.count].rect.end.y = bottom;

				if (prevX > 0)
				{//라인의 첫 글자를 배제하기 위해서!
					if (gapSpace < (all.data[all.count].rect.start.x) - prevX)
						gapSpace = (all.data[all.count].rect.start.x) - prevX;
				}
				else {
					gapSpace = (all.data[all.count].rect.end.x) - (all.data[all.count].rect.start.x);
				}
				prevX = x - 1;

				//경계선 보정작업
				ParsingStepThird(&all.data[all.count].rect);
				ShowResultImage(all.data[all.count].rect.end.y - all.data[all.count].rect.start.y, all.data[all.count].rect.end.x - all.data[all.count].rect.start.x, all.data[all.count].rect.start.y, all.data[all.count].rect.start.x);
				all.count += 1;
			}
		}
		prevLineState = isLetter;
	}
	//띄어쓰기 추가하기
	cout << all.count << " " << gapSpace << endl;
	AddSpaces(startCount, all.count - 1, gapSpace*0.5);
	//줄바꿈 문자 추가하기
	all.data[all.count].isFixed = true;
	all.data[all.count++].letter.value = '\n';
}

void OCRFunc::ParsingStepThird(my_Rect * rect)
{
	bool isLetter;
	unsigned char pixelVal;

	//데이터의 윗쪽영역 탐색 필요하다면 축소
	for (int y = rect->start.y;; y++)
	{
		isLetter = false;
		for (int x = rect->start.x; x <= rect->end.x; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);
			if (pixelVal < COLOR_CHECK_RANGE)
			{
				//문자라인 발견시 top을 변경해준다.
				rect->start.y = y;
				isLetter = true;
				break;
			}
		}
		if (isLetter)	//이미 글자라인이라면 탐색할 필요 없음
			break;
	}

	//데이터의 아래쪽 영역 탐색 및 축소
	for (int y = rect->end.y;; y--)
	{
		isLetter = false;
		for (int x = rect->start.x; x <= rect->end.x; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);
			if (pixelVal < COLOR_CHECK_RANGE)
			{
				//문자라인 발견시 bottom을 변경해준다.
				rect->end.y = y;
				isLetter = true;
				break;
			}
		}
		if (isLetter)	//글자라인이 아니라면 더이상 탐색할 필요가 없다.
			break;
	}
}

/*인자: 라인의 시작 카운트, 끝 카운트, 공백 기준
다음 문자와의 거리가 기준거리보다 크면 띄어쓰기로 인식, 해당 데이터의 nextIsSpace를 true로 바꿔준다.*/
void OCRFunc::AddSpaces(int startCount, int endCount, int stdGap)
{
	int gap;

	for (int i = startCount; i < endCount; i++)
	{
		gap = (all.data[i + 1].rect.start.x) - (all.data[i].rect.end.x);
		if (gap > stdGap)
			all.data[i].nextIsSpace = true;
	}
}

void OCRFunc::ShowResultImage(int height, int width, int top, int startX)
{
	Mat output = Mat(height, width, CV_8UC1);

	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			output.at<unsigned char>(h, w) = img.at<unsigned char>(top + h, startX + w);
		}
	}

	char path[1024];
	GetModuleFileName(NULL, path, 1024);
	PathRemoveFileSpec(path);

	string fileName = path;
	fileName.append("\\data\\" + to_string(all.count) + ".jpg");
	imwrite(fileName, output);
	//waitKey(0);
}


void OCRFunc::MakeImageData()
{
	//string filename = "Imagedata.txt";	//파일이름
	//ofstream outFile(filename.data());

	for (int i = 0; i < all.count - 1; i++)
	{
		Data *data = &all.data[i];
		Letter *letter = &data->letter;
		my_Rect *rect = &data->rect;

		double xSize, ySize;
		unsigned char pixelVal;
		char path[1024];
		GetModuleFileName(NULL, path, 1024);
		PathRemoveFileSpec(path);

		string fileName = path;
		fileName.append("\\data\\" + to_string(i) + ".jpg");

		Mat Letterimage = imread(fileName.data(), IMREAD_GRAYSCALE);

		//만약 해당 글자가 공백이면 무시
		if (data->isFixed)
		{
			//cout << "a" << endl;
			;
		}
		else
		{

			unsigned int buffer;

			for (int j = 0; j < 48; j++)	//배열 초기화
				letter->img[j] = 0x00000000;

			xSize = (double)(rect->end.x - rect->start.x) / 31;		//이미지의 가로 32칸으로 나눴을때 가로 간격
			ySize = (double)(rect->end.y - rect->start.y) / 47;		//이미지의 세로를 48칸으로 나눴을때 세로간격

			for (int y = 0; y < 48; y++)
			{
				for (int x = 0; x < 32; x++)
				{

					if (x == 31 && y != 47) {
						pixelVal = Letterimage.at<unsigned char>(y*ySize, x*xSize - 1);
					}
					else if (x != 31 && y == 47) {
						pixelVal = Letterimage.at<unsigned char>(y*ySize - 1, x*xSize);
					}
					else if (x == 31 && y == 47) {
						pixelVal = Letterimage.at<unsigned char>(y*ySize - 1, x*xSize - 1);
					}
					else {
						pixelVal = Letterimage.at<unsigned char>(y*ySize, x*xSize);
					}
					if (pixelVal < COLOR_CHECK_RANGE) //글자면
					{
						buffer = 0x80000000;	//가장 왼쪽을 1로 설정
						buffer >>= x;	//해당하는 위치(x)만큼 밀어준다.
						letter->img[y] |= buffer; //OR연산자랑 비교해서 넣어준다.
					}

				}
			}
		}
		/*
		unsigned int buffer2;

		//파일에 데이터 쓰기
		for (int y = 0; y < 48; y++)
		{
		buffer2 = letter->img[y];		//가로 한줄씩 가져온다.

		for (int x = 0; x < 32; x++)		//32번을 돌면서
		{
		if (buffer2 & 0x80000000)		//만약에 가장 왼쪽의 bit가 1이면 (글씨)
		outFile << 1;	//파일에 1을 쓴다.
		else
		outFile << 0;
		buffer2 <<= 1;	//한칸 밀어줌
		}
		outFile << endl;	//한줄 다 쓰고, 개행문자쓰기
		}

		outFile << endl;	//한줄 다 쓰고, 개행문자쓰기

		*/
	}
}



void OCRFunc::CreateStandard()
{
	int s = 0;
	char path[1024];
	GetModuleFileName(NULL, path, 1024);
	PathRemoveFileSpec(path);
	string fileName = path;
	string temp = path;
	fileName.append("\\data\\standard.txt");
	ofstream outFile(fileName);

	setImg(temp + "\\data\\data_img.jpg");

	ParsingStepFirst();
	cout << all.count << endl;


	for (int i = 0; i < all.count - 1; i++)
	{

		Letter *standardLetter = &standard.letter[s];	//원본 데이터을 담을 Standard구조체 letter변수

		Data *data = &all.data[i];
		Letter *letter = &data->letter;
		my_Rect *rect = &data->rect;


		double xSize, ySize;
		unsigned char pixelVal;
		unsigned int buffer;

		//만약 해당 글자가 공백이면 무시
		if (data->isFixed)
		{
			//cout << "a" << endl;
			;
		}
		else
		{
			for (int j = 0; j < 48; j++)	//배열 초기화
				standardLetter->img[j] = 0x00000000;

			xSize = (double)(rect->end.x - rect->start.x) / 31;		//이미지의 가로 32칸으로 나눴을때 가로 간격
			ySize = (double)(rect->end.y - rect->start.y) / 47;		//이미지의 세로를 48칸으로 나눴을때 세로간격

			for (int y = 0; y < 48; y++)
			{
				for (int x = 0; x < 32; x++)
				{
					pixelVal = img.at<unsigned char>(rect->start.y + (y*ySize), rect->start.x + (x*xSize));

					if (pixelVal < COLOR_CHECK_RANGE) //글자면
					{
						buffer = 0x80000000;	//가장 왼쪽을 1로 설정
						buffer >>= x;	//해당하는 위치(x)만큼 밀어준다.
						standardLetter->img[y] |= buffer; //OR연산자랑 비교해서 넣어준다.

					}

				}
			}
			//standardLetter = letter;
			s++;

		}

	}

	//value에 소문자와 대문자 지정해주기
	for (int i = 0; i < 26; i++)
	{
		standard.letter[i].value = 'a' + i;
	}
	for (int i = 0; i < 26; i++)
	{
		standard.letter[i + 26].value = 'A' + i;
	}



	for (int i = 0; i < standard.count; i++)
	{

		unsigned int buffer2;

		Letter *letter = &standard.letter[i];

		//만약 해당 글자가 공백이면 무시
		if (letter->value == '\n')
		{
			cout << "a" << endl;
		}
		else {
			outFile << letter->value << endl;
			//파일에 데이터 쓰기
			for (int y = 0; y < 48; y++)
			{
				buffer2 = letter->img[y];		//가로 한줄씩 가져온다.

				for (int x = 0; x < 32; x++)		//32번을 돌면서
				{
					if (buffer2 & 0x80000000)		//만약에 가장 왼쪽의 bit가 1이면 (글씨)
						outFile << 1;	//파일에 1을 쓴다.
					else
						outFile << 0;
					buffer2 <<= 1;	//한칸 밀어줌
				}
				outFile << endl;	//한줄 다 쓰고, 개행문자쓰기
			}

			outFile << endl;	//한줄 다 쓰고, 개행문자쓰기
		}

	}

	all.count = 0;
	for (int i = 0; i < MAX_DATA_COUNT; i++)
	{
		all.data[i].isFixed = false;
		all.data[i].nextIsSpace = false;
	}

	outFile.close();


}

void OCRFunc::FindLetterValue()
{
	//string filename = "aa.txt";	//파일이름
	//ofstream outFile(filename.data());

	int count = 0;
	int max = 0;

	unsigned int buffer, bit;

	for (int i = 0; i < all.count - 1; i++)
	{
		Data *data = &all.data[i];
		Letter *letter = &data->letter;
		my_Rect *rect = &data->rect;

		max = 0;
		for (int j = 0; j < standard.count; j++)	//원본 데이터의 갯수
		{
			count = 0;

			for (int y = 0; y < 48; y++)
			{
				/*buffer = letter->img[y] ^ standard.letter[j].img[y];	//가로줄 XOR연산자 결과를 buffer에 담음
			
				for (int x = 0; x < 32; x++)		//32번을 돌면서
				{
					if (buffer & 0x80000000)		//만약에 가장 왼쪽의 bit가 1이면 (글씨)
						outFile << 1;	//파일에 1을 쓴다.
					else
						outFile << 0;
					buffer <<= 1;	//한칸 밀어줌
				}
				outFile << endl;	//한줄 다 쓰고, 개행문자쓰기*/

				buffer = letter->img[y] ^ standard.letter[j].img[y];	//가로줄 XOR연산자 결과를 buffer에 담음

				//해당 가로줄에 얼마나많은 0이 있는지 count	. 0이 많을수록 이미지가 겹침
				for (int x = 0; x < 32; x++)
				{
					bit = 0x80000000;		//가장왼쪽비트를 1로 초기화
					bit >>= x;				//지금 돌고있는 위치로 1옮김
					bit = bit&buffer;	//&연산자로 bit확인

					if (!bit)	//만약 bit가 0이라면 0반환
						count++;
				}

			}

			if (count > max)		//만약 가장 높은 확률(max)보다 지금 count가 크면
			{
				letter->value = standard.letter[j].value;	//지금 가르키고있는 원본의 값을 넣어줌
				max = count;
			}
		}
	}

	//outFile.close();


}


void OCRFunc::StoreResult()	//결과를 text파일에 저장
{
	string filename;

	filename = path.substr(0, path.length() - 4);
	ofstream outFile(filename + ".txt");

	cout << all.count << endl;
	for (int i = 0; i < all.count; i++)
	{
		if (all.data[i].nextIsSpace)
			outFile << all.data[i].letter.value << " ";
		else if (all.data[i].isFixed)
			outFile << endl;
		else
			outFile << all.data[i].letter.value;
	}

	outFile.close();
}


void OCRFunc::CorrectBigsmall()
{
	char *c, *cNext;
	int SmalltoBig = 0;
	int BigtoSmall = 1;


	for (int i = 0; i < all.count - 1; i++)
	{
		c = &(all.data[i].letter.value);
		cNext = &(all.data[i + 1].letter.value);

		if (i == 0)	//만약에 첫번째 글자일때
		{
			if (*c >= 'a' && *c <= 'z')	//소문자일때
			{
				ChangeBigSmall(c, SmalltoBig);
			}
		}
		else  //첫번째 글자가 아니면
		{
			if (*c >= 'A' && *c <= 'Z')	//대문자일때
			{
				ChangeBigSmall(c, BigtoSmall);
			}
		}

	}
}


void OCRFunc::ChangeBigSmall(char *c, int num)
{
	switch (num)
	{
	case 0:   //Small to Big
		if (*c == 'l')   //소문자 ㅣ(L)
			*c = 'I';   //대문자 I (i)
		else if (*c == 'c')
			*c = 'C';
		else if (*c == 'o')
			*c = 'O';
		else if (*c == 's')
			*c = 'S';
		else if (*c == 'u')
			*c = 'U';
		else if (*c == 'v')
			*c = 'V';
		else if (*c == 'w')
			*c = 'W';
		else if (*c == 'x')
			*c = 'X';
		else if (*c == 'z')
			*c = 'Z';
		break;
	case 1:   //Big to Small
		if (*c == 'I')   //대문자 I(i)
			*c = 'l';   //소문자 l(L)
		else if (*c == 'C')
			*c = 'c';
		else if (*c == 'O')
			*c = 'o';
		else if (*c == 'S')
			*c = 's';
		else if (*c == 'U')
			*c = 'u';
		else if (*c == 'V')
			*c = 'v';
		else if (*c == 'W')
			*c = 'w';
		else if (*c == 'X')
			*c = 'x';
		else if (*c == 'Z')
			*c = 'z';
		break;
	default:
		break;
	}
}

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nlnitVal)
{
	unsigned char** rtn = new unsigned char*[nHeight];
	for (int n = 0; n < nHeight; n++)
	{
		rtn[n] = new unsigned char[nWidth];
		memset(rtn[n], nlnitVal, sizeof(unsigned char)*nWidth);
	}

	return rtn;
}

void MemFree2D(unsigned char** Mem, int nHeight)
{
	for (int n = 0; n < nHeight; n++)
		delete[] Mem[n];

	delete[] Mem;
}

unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize)
{
	int nPadSize = (int)(nFilterSize / 2);
	unsigned char** Pad = MemAlloc2D(nHeight + 2 * nPadSize, nWidth + 2 * nPadSize, 0);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
			Pad[h + nPadSize][w + nPadSize] = In[h][w];
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			Pad[h][w + nPadSize] = In[0][w];
			Pad[h + (nHeight - 1)][w + nPadSize] = In[nHeight - 1][w];
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h + nPadSize][w] = In[h][0];
			Pad[h + nPadSize][w + (nWidth - 1)] = In[h][nWidth - 1];
		}
	}
	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h][w] = In[0][0];
			Pad[h + (nHeight - 1)][w] = In[nHeight - 1][0];
			Pad[h][w + (nWidth - 1)] = In[0][nWidth - 1];
			Pad[h + (nHeight - 1)][w + (nWidth - 1)] = In[nHeight - 1][nWidth - 1];
		}
	}
	return Pad;
}

unsigned char getMax(unsigned char* arr, int arrLength)
{
	unsigned char max = arr[0];

	for (int i = 0; i < arrLength; i++)
		if (arr[i] > max)
			max = arr[i];
	return max;
}


unsigned char getMin(unsigned char* arr, int arrLength)
{
	unsigned char min = arr[0];

	for (int i = 0; i < arrLength; i++)
		if (arr[i] < min)
			min = arr[i];
	return min;
}


void bottomHat(unsigned char **Img_in, unsigned char** Out, int nHeight, int nWidth, int nFilterSize, int type)
{
	int nPadSize = (int)(nFilterSize / 2);
	unsigned char** Img_Pad = Padding(Img_in, nHeight, nWidth, nFilterSize);

	unsigned char* filterArr;
	filterArr = new unsigned char[sizeof(unsigned char)*nFilterSize*nFilterSize];

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			for (int m = 0; m < nFilterSize; m++)
			{
				for (int n = 0; n < nFilterSize; n++)
				{
					filterArr[m*nFilterSize + n] = Img_Pad[h + m][w + n];
				}
			}
			if (type == EROSION) {
				Out[h][w] = getMin(filterArr, nFilterSize*nFilterSize);
			}
			else if (type == DILATION) {
				Out[h][w] = getMax(filterArr, nFilterSize*nFilterSize);
			}

		}
	}
	delete[] filterArr;
	MemFree2D(Img_Pad, nHeight = 2 * nPadSize);
}