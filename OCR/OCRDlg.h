
// OCRDlg.h : ��� ����
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

// COCRDlg ��ȭ ����
class COCRDlg : public CDialogEx
{
// �����Դϴ�.
public:
	COCRDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OCR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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
