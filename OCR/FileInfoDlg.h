#pragma once


// CFileInfoDlg 대화 상자입니다.

class CFileInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileInfoDlg)

public:
	CFileInfoDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CFileInfoDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILEINFODIG };
#endif
public:
	int nFormat;

	int GetFileFormat();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_radio_fileInfo;
	afx_msg void OnBnClickedOkbuttonF();
	afx_msg void OnBnClickedCancelbuttonF();
	afx_msg void OnClickedRadio(UINT msg);
};
