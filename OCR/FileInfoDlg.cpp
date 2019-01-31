// FileInfoDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "OCR.h"
#include "FileInfoDlg.h"
#include "afxdialogex.h"


// CFileInfoDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CFileInfoDlg, CDialogEx)

CFileInfoDlg::CFileInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FILEINFODIG, pParent)
	, m_radio_fileInfo(0)
{
	nFormat = 0;
}

CFileInfoDlg::~CFileInfoDlg()
{
}

void CFileInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_PDF, m_radio_fileInfo);
}


BEGIN_MESSAGE_MAP(CFileInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_OKBUTTON_F, &CFileInfoDlg::OnBnClickedOkbuttonF)
	ON_BN_CLICKED(IDC_CANCELBUTTON_F, &CFileInfoDlg::OnBnClickedCancelbuttonF)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_PDF, IDC_RADIO_PIC, OnClickedRadio)
END_MESSAGE_MAP()


// CFileInfoDlg �޽��� ó�����Դϴ�.


void CFileInfoDlg::OnBnClickedOkbuttonF()
{
	CDialogEx::OnOK();
}


void CFileInfoDlg::OnBnClickedCancelbuttonF()
{
	CDialogEx::OnCancel();
}

void CFileInfoDlg::OnClickedRadio(UINT msg)
{
	UpdateData(TRUE);
	switch (m_radio_fileInfo) 
	{
	case 0://pdf
		nFormat = 0;
		break;
	case 1:	//picture
		nFormat = 1;
		break;
	}
}

int CFileInfoDlg::GetFileFormat()
{
	return nFormat;
}