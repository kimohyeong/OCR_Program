
// OCRDlg.h : 헤더 파일
//
#pragma once
#include <opencv2\opencv.hpp>
#include "FileInfoDlg.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "OCRFunc.h"
#include <iostream>

using namespace cv;
using namespace std;

enum ImageFormat 
{
	FORMAT_PDF,
	FORMAT_PIC
};

// COCRDlg 대화 상자
class COCRDlg : public CDialogEx
{
// 생성입니다.
public:
	COCRDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OCR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileopen();
	afx_msg void OnBnClickedOcrbutton();

	Mat InpImg;
	
	CStatic mFormat;
	CStatic mFilename;
	CStatic m_ResultText;

	int nFormat;
	string nFilename;
	string nReultFilename;

	CProgressCtrl m_Progress;

	OCRFunc* ocr;

	afx_msg void OnBnClickedOpen();
	CButton m_OpenButton;
};
