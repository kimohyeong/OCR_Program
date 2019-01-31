
// OCRDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "OCR.h"
#include "OCRDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// COCRDlg 대화 상자



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


// COCRDlg 메시지 처리기

BOOL COCRDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void COCRDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
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
				MessageBox("JPG파일이 아닙니다.");
				return;
			}
			InpImg = imread((const char*)dlg.GetPathName());

			nFormat = FileInfoDlg.GetFileFormat();
			nFilename = dlg.GetFileName();

			if (nFormat==0)
			{
				mFormat.SetWindowTextA("PDF캡쳐");
			}
			else
			{
				mFormat.SetWindowTextA("사진 영상");
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
		MessageBox("파일을 불러오지 않았습니다.");
		return;
	}

	//프로그래스바 초기화
	m_Progress.SetRange(0, 7);
	m_Progress.SetPos(0);
	

	ocr->setImg(nFilename);	//이미지 세팅
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

	message = nReultFilename + ".txt 파일이 저장되었습니다.";
	CString result(message.c_str());

	m_ResultText.SetWindowTextA(result);
	m_OpenButton.ShowWindow(TRUE);

}


void COCRDlg::OnBnClickedOpen()
{
	ShellExecute(NULL, "open", "notepad", (nReultFilename+".txt").c_str(), NULL, SW_SHOW);
}
