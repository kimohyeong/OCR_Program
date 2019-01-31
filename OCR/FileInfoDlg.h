#pragma once


// CFileInfoDlg ��ȭ �����Դϴ�.

class CFileInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileInfoDlg)

public:
	CFileInfoDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CFileInfoDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILEINFODIG };
#endif
public:
	int nFormat;

	int GetFileFormat();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	int m_radio_fileInfo;
	afx_msg void OnBnClickedOkbuttonF();
	afx_msg void OnBnClickedCancelbuttonF();
	afx_msg void OnClickedRadio(UINT msg);
};
