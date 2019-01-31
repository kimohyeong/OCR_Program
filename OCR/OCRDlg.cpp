
// OCRDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "OCR.h"
#include "OCRDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COCRDlg ��ȭ ����



COCRDlg::COCRDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_OCR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	ocr = new OCRFunc();

	ocr->CreateStandard();
}

void COCRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INPUTFORMAT, mFormat);
	DDX_Control(pDX, IDC_INPUTNAME, mFilename);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_RESULT, m_ResultText);
	DDX_Control(pDX, IDC_OPEN, m_OpenButton);
}

BEGIN_MESSAGE_MAP(COCRDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FIILE_FILEOPEN, &COCRDlg::OnFileopen)
	ON_BN_CLICKED(IDC_OCRBUTTON, &COCRDlg::OnBnClickedOcrbutton)
	ON_BN_CLICKED(IDC_OPEN, &COCRDlg::OnBnClickedOpen)
END_MESSAGE_MAP()


// COCRDlg �޽��� ó����

BOOL COCRDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void COCRDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void COCRDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR COCRDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COCRDlg::OnFileopen()
{
	ocr->initOCR();
	CFileInfoDlg FileInfoDlg;
	if (FileInfoDlg.DoModal() == IDOK)
	{

		CFileDialog dlg(TRUE);
		if (dlg.DoModal() == IDOK)
		{
			if (dlg.GetFileExt() != "jpg" && dlg.GetFileExt() != "JPG")
			{
				MessageBox("JPG������ �ƴմϴ�.");
				return;
			}
			InpImg = imread((const char*)dlg.GetPathName());

			nFormat = FileInfoDlg.GetFileFormat();
			nFilename = dlg.GetFileName();

			if (nFormat==0)
			{
				mFormat.SetWindowTextA("PDFĸ��");
			}
			else
			{
				mFormat.SetWindowTextA("���� ����");
			}
			mFilename.SetWindowTextA(dlg.GetFileName());

			m_ResultText.SetWindowTextA("");
			m_OpenButton.ShowWindow(FALSE);

			imshow("Inpimg", InpImg);

			waitKey(0);

		}
	}


}


void COCRDlg::OnBnClickedOcrbutton()
{	
	if (nFilename=="") {
		MessageBox("������ �ҷ����� �ʾҽ��ϴ�.");
		return;
	}

	//���α׷����� �ʱ�ȭ
	m_Progress.SetRange(0, 7);
	m_Progress.SetPos(0);
	

	ocr->setImg(nFilename);	//�̹��� ����
	m_Progress.SetPos(1);

	if (nFormat == FORMAT_PDF) {
		m_Progress.SetPos(2);
	}
	else {
		ocr->imgThresholding();
		m_Progress.SetPos(2);
	}

	ocr->ParsingStepFirst();
	m_Progress.SetPos(3);

	ocr->MakeImageData();
	m_Progress.SetPos(4);

	ocr->FindLetterValue();
	m_Progress.SetPos(5);

	ocr->CorrectBigsmall();
	m_Progress.SetPos(6);

	ocr->StoreResult();
	m_Progress.SetPos(7);


	string message = nFilename;
	nReultFilename = message.substr(0, message.length() - 4);

	message = nReultFilename + ".txt ������ ����Ǿ����ϴ�.";
	CString result(message.c_str());

	m_ResultText.SetWindowTextA(result);
	m_OpenButton.ShowWindow(TRUE);

}


void COCRDlg::OnBnClickedOpen()
{
	ShellExecute(NULL, "open", "notepad", (nReultFilename+".txt").c_str(), NULL, SW_SHOW);
}
