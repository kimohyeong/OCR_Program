
#include "stdafx.h"
#include "OCRFunc.h"
#include "afxdialogex.h"
#include <direct.h>


OCRFunc::OCRFunc()
{
	//������ �ʱ�ȭ
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
	//������ �ʱ�ȭ
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
	int wB = 0;	//background�� weight
	int wF = 0;
	float sumB = 0;
	float varMax = 0;
	float mG = sum / (img.rows*img.cols);
	float subSumB = 0;
	float subSumF = 0;
	cout << threshold << endl;

	//threshold���� �����Ű�鼭 ���� �л��� ũ�� ����� ���� ã�´�.
	for (int t = 0; t < 256; t++) {
		wB += Hist[t];
		if (wB == 0)
			continue;
		wF = (img.rows*img.cols) - wB;
		sumB += (t*Hist[t]);

		//Background�� Foreground�� ������ ���
		float mB = sumB / wB;
		float mF = (sum - sumB) / wF;

		subSumB += pow(mB - mG, 2);
		subSumF += pow(mF - mG, 2);

		//�л� ���
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
/*Step1. ��ü �̹����� ���Ͽ� ���� ����(���μ� ã��)*/
void OCRFunc::ParsingStepFirst()
{
	all.count = 0;

	int nWidth = img.cols;
	int nHeight = img.rows;
	int top, bottom;

	unsigned char pixelVal;

	bool isLetter;
	bool prevLineState = false;

	//�̹����� ���� ��ü�� Ž������ �ʰ� �߰��� Ž���ؼ� �뷫���� ���̸� �˾Ƴ���.
	int xStart = 0;
	int xEnd = nWidth;

	for (int y = 0; y < nHeight; y++)
	{
		isLetter = false;
		for (int x = xStart; x < xEnd; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);

			if (pixelVal < COLOR_CHECK_RANGE)
			{//���ڰ� �ִ� ���� �߰�
				isLetter = true;
				break;
			}
		}
		if (isLetter)
		{//���ڶ����� ��
			if (!prevLineState)
			{//prevLineState�� ���� ������ ����� �� false�̴�.
			 //���� ���������� ����̰�, ���ڶ����� ã�Ҵٸ�, �� ������ ���� ��ܿ� �ش��ϴ� ������ �ȴ�.
				top = y;
			}
		}
		else
		{
			if (prevLineState)
			{//���� ���� ���°� ���ڶ����̾���, ���� ����̸� �� ������ �� ������ ���� �ϴܿ� �ش��ϴ� ������ �ȴ�.
				bottom = y - 1;

				//�� ���� top�� bottom�� ã�����Ƿ� ���ΰ�踦 ã�Ƴ��� �ܰ� ����
				ParsingStepSecond(top, bottom);
			}
		}
		prevLineState = isLetter;
	}
}

/*Step2. �ϳ��� ���� ������ �� ������ ���μ� �����ϱ�.
���μ� ���� �� ���������� �ٹٲ��� �߰����ش�.(������ �ٲ�� ���� ǥ���ϱ� ����)
�� ���� ������ ���ݵ� �� ���� ū ������ ���� �� �� ������ 0.7��ŭ�� ���� ���� �������� ����.*/
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
			{//���ڿ��� ã��
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
				//�� ������ ���ο��� ã�� �Ϸ�.
				all.data[all.count].rect.end.x = x - 1;
				all.data[all.count].rect.end.y = bottom;

				if (prevX > 0)
				{//������ ù ���ڸ� �����ϱ� ���ؼ�!
					if (gapSpace < (all.data[all.count].rect.start.x) - prevX)
						gapSpace = (all.data[all.count].rect.start.x) - prevX;
				}
				else {
					gapSpace = (all.data[all.count].rect.end.x) - (all.data[all.count].rect.start.x);
				}
				prevX = x - 1;

				//��輱 �����۾�
				ParsingStepThird(&all.data[all.count].rect);
				ShowResultImage(all.data[all.count].rect.end.y - all.data[all.count].rect.start.y, all.data[all.count].rect.end.x - all.data[all.count].rect.start.x, all.data[all.count].rect.start.y, all.data[all.count].rect.start.x);
				all.count += 1;
			}
		}
		prevLineState = isLetter;
	}
	//���� �߰��ϱ�
	cout << all.count << " " << gapSpace << endl;
	AddSpaces(startCount, all.count - 1, gapSpace*0.5);
	//�ٹٲ� ���� �߰��ϱ�
	all.data[all.count].isFixed = true;
	all.data[all.count++].letter.value = '\n';
}

void OCRFunc::ParsingStepThird(my_Rect * rect)
{
	bool isLetter;
	unsigned char pixelVal;

	//�������� ���ʿ��� Ž�� �ʿ��ϴٸ� ���
	for (int y = rect->start.y;; y++)
	{
		isLetter = false;
		for (int x = rect->start.x; x <= rect->end.x; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);
			if (pixelVal < COLOR_CHECK_RANGE)
			{
				//���ڶ��� �߽߰� top�� �������ش�.
				rect->start.y = y;
				isLetter = true;
				break;
			}
		}
		if (isLetter)	//�̹� ���ڶ����̶�� Ž���� �ʿ� ����
			break;
	}

	//�������� �Ʒ��� ���� Ž�� �� ���
	for (int y = rect->end.y;; y--)
	{
		isLetter = false;
		for (int x = rect->start.x; x <= rect->end.x; x++)
		{
			pixelVal = img.at<unsigned char>(y, x);
			if (pixelVal < COLOR_CHECK_RANGE)
			{
				//���ڶ��� �߽߰� bottom�� �������ش�.
				rect->end.y = y;
				isLetter = true;
				break;
			}
		}
		if (isLetter)	//���ڶ����� �ƴ϶�� ���̻� Ž���� �ʿ䰡 ����.
			break;
	}
}

/*����: ������ ���� ī��Ʈ, �� ī��Ʈ, ���� ����
���� ���ڿ��� �Ÿ��� ���ذŸ����� ũ�� ����� �ν�, �ش� �������� nextIsSpace�� true�� �ٲ��ش�.*/
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
	//string filename = "Imagedata.txt";	//�����̸�
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

		//���� �ش� ���ڰ� �����̸� ����
		if (data->isFixed)
		{
			//cout << "a" << endl;
			;
		}
		else
		{

			unsigned int buffer;

			for (int j = 0; j < 48; j++)	//�迭 �ʱ�ȭ
				letter->img[j] = 0x00000000;

			xSize = (double)(rect->end.x - rect->start.x) / 31;		//�̹����� ���� 32ĭ���� �������� ���� ����
			ySize = (double)(rect->end.y - rect->start.y) / 47;		//�̹����� ���θ� 48ĭ���� �������� ���ΰ���

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
					if (pixelVal < COLOR_CHECK_RANGE) //���ڸ�
					{
						buffer = 0x80000000;	//���� ������ 1�� ����
						buffer >>= x;	//�ش��ϴ� ��ġ(x)��ŭ �о��ش�.
						letter->img[y] |= buffer; //OR�����ڶ� ���ؼ� �־��ش�.
					}

				}
			}
		}
		/*
		unsigned int buffer2;

		//���Ͽ� ������ ����
		for (int y = 0; y < 48; y++)
		{
		buffer2 = letter->img[y];		//���� ���پ� �����´�.

		for (int x = 0; x < 32; x++)		//32���� ���鼭
		{
		if (buffer2 & 0x80000000)		//���࿡ ���� ������ bit�� 1�̸� (�۾�)
		outFile << 1;	//���Ͽ� 1�� ����.
		else
		outFile << 0;
		buffer2 <<= 1;	//��ĭ �о���
		}
		outFile << endl;	//���� �� ����, ���๮�ھ���
		}

		outFile << endl;	//���� �� ����, ���๮�ھ���

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

		Letter *standardLetter = &standard.letter[s];	//���� �������� ���� Standard����ü letter����

		Data *data = &all.data[i];
		Letter *letter = &data->letter;
		my_Rect *rect = &data->rect;


		double xSize, ySize;
		unsigned char pixelVal;
		unsigned int buffer;

		//���� �ش� ���ڰ� �����̸� ����
		if (data->isFixed)
		{
			//cout << "a" << endl;
			;
		}
		else
		{
			for (int j = 0; j < 48; j++)	//�迭 �ʱ�ȭ
				standardLetter->img[j] = 0x00000000;

			xSize = (double)(rect->end.x - rect->start.x) / 31;		//�̹����� ���� 32ĭ���� �������� ���� ����
			ySize = (double)(rect->end.y - rect->start.y) / 47;		//�̹����� ���θ� 48ĭ���� �������� ���ΰ���

			for (int y = 0; y < 48; y++)
			{
				for (int x = 0; x < 32; x++)
				{
					pixelVal = img.at<unsigned char>(rect->start.y + (y*ySize), rect->start.x + (x*xSize));

					if (pixelVal < COLOR_CHECK_RANGE) //���ڸ�
					{
						buffer = 0x80000000;	//���� ������ 1�� ����
						buffer >>= x;	//�ش��ϴ� ��ġ(x)��ŭ �о��ش�.
						standardLetter->img[y] |= buffer; //OR�����ڶ� ���ؼ� �־��ش�.

					}

				}
			}
			//standardLetter = letter;
			s++;

		}

	}

	//value�� �ҹ��ڿ� �빮�� �������ֱ�
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

		//���� �ش� ���ڰ� �����̸� ����
		if (letter->value == '\n')
		{
			cout << "a" << endl;
		}
		else {
			outFile << letter->value << endl;
			//���Ͽ� ������ ����
			for (int y = 0; y < 48; y++)
			{
				buffer2 = letter->img[y];		//���� ���پ� �����´�.

				for (int x = 0; x < 32; x++)		//32���� ���鼭
				{
					if (buffer2 & 0x80000000)		//���࿡ ���� ������ bit�� 1�̸� (�۾�)
						outFile << 1;	//���Ͽ� 1�� ����.
					else
						outFile << 0;
					buffer2 <<= 1;	//��ĭ �о���
				}
				outFile << endl;	//���� �� ����, ���๮�ھ���
			}

			outFile << endl;	//���� �� ����, ���๮�ھ���
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
	//string filename = "aa.txt";	//�����̸�
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
		for (int j = 0; j < standard.count; j++)	//���� �������� ����
		{
			count = 0;

			for (int y = 0; y < 48; y++)
			{
				/*buffer = letter->img[y] ^ standard.letter[j].img[y];	//������ XOR������ ����� buffer�� ����
			
				for (int x = 0; x < 32; x++)		//32���� ���鼭
				{
					if (buffer & 0x80000000)		//���࿡ ���� ������ bit�� 1�̸� (�۾�)
						outFile << 1;	//���Ͽ� 1�� ����.
					else
						outFile << 0;
					buffer <<= 1;	//��ĭ �о���
				}
				outFile << endl;	//���� �� ����, ���๮�ھ���*/

				buffer = letter->img[y] ^ standard.letter[j].img[y];	//������ XOR������ ����� buffer�� ����

				//�ش� �����ٿ� �󸶳����� 0�� �ִ��� count	. 0�� �������� �̹����� ��ħ
				for (int x = 0; x < 32; x++)
				{
					bit = 0x80000000;		//������ʺ�Ʈ�� 1�� �ʱ�ȭ
					bit >>= x;				//���� �����ִ� ��ġ�� 1�ű�
					bit = bit&buffer;	//&�����ڷ� bitȮ��

					if (!bit)	//���� bit�� 0�̶�� 0��ȯ
						count++;
				}

			}

			if (count > max)		//���� ���� ���� Ȯ��(max)���� ���� count�� ũ��
			{
				letter->value = standard.letter[j].value;	//���� ����Ű���ִ� ������ ���� �־���
				max = count;
			}
		}
	}

	//outFile.close();


}


void OCRFunc::StoreResult()	//����� text���Ͽ� ����
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

		if (i == 0)	//���࿡ ù��° �����϶�
		{
			if (*c >= 'a' && *c <= 'z')	//�ҹ����϶�
			{
				ChangeBigSmall(c, SmalltoBig);
			}
		}
		else  //ù��° ���ڰ� �ƴϸ�
		{
			if (*c >= 'A' && *c <= 'Z')	//�빮���϶�
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
		if (*c == 'l')   //�ҹ��� ��(L)
			*c = 'I';   //�빮�� I (i)
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
		if (*c == 'I')   //�빮�� I(i)
			*c = 'l';   //�ҹ��� l(L)
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