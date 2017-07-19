 
// UPDATEDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UPDATE.h"
#include "UPDATEDlg.h"
#include "afxdialogex.h"
#include "afxinet.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IS_DEBUG 0
/*
#define SERVERIP "172.16.9.20"
#define USER "publlic"
#define PASSWORD "33343666"
#define PORT 21
*/
// CUPDATEDlg �Ի���
CWinThread* pThread;//accept�߳�
UINT VersionCheckThread(LPVOID lpParam);
UINT FileDownlaodThread(LPVOID lpParam);

CUPDATEDlg::CUPDATEDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPDATE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUPDATEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIP, m_tip);
	DDX_Control(pDX, ID_UPDATE, m_ctrl_update);
	DDX_Control(pDX, IDC_UPDATEPROGRESS, m_ctrl_progress);
}

BEGIN_MESSAGE_MAP(CUPDATEDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_UPDATE, &CUPDATEDlg::OnBnClickedUpdate)
END_MESSAGE_MAP()


// CUPDATEDlg ��Ϣ�������

BOOL CUPDATEDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_ctrl_update.EnableWindow(FALSE);
	//MessageBox(GetCommandLine());
	//MessageBox(AfxGetApp()->m_lpCmdLine);
	//��ȡini�еķ���������

	char buf[100];
	CString temp;
	::GetPrivateProfileString("FTP", "IP", "", buf, MAX_PATH, ".\\FATSSettings.ini");
	m_serverIP.Format("%s", buf);
	
	::GetPrivateProfileString("FTP", "User", "", buf, MAX_PATH, ".\\FATSSettings.ini");
	m_serverUserName.Format("%s", buf);

	::GetPrivateProfileString("FTP", "Password", "", buf, MAX_PATH, ".\\FATSSettings.ini");
	m_serverPassword.Format("%s", buf);

	::GetPrivateProfileString("FTP", "Port", "", buf, MAX_PATH, ".\\FATSSettings.ini");
	temp.Format("%s", buf);
	m_serverPort = atoi(temp);
	//��ȡ���������Ĳ���
	m_ctrl_progress.SetRange(0, 100);
	ResetProgress();
	m_serverFileCount = 0;
#if !IS_DEBUG

	if (""== (temp = AfxGetApp()->m_lpCmdLine))
	{
		MessageBoxEx(0,"����������쳣���޷�����","UpdateError",0,0);
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
	}
	else//��������
	{
		//�з���//Y��ɭ;MFCApplication1
		int pos=temp.Find(";");//��ȡ��λ��
		m_serverFilePath = temp.Left(pos);
		//MessageBox(m_serverFilePath);
		m_ParentWindowName = temp.Right(temp.GetLength()-(pos+1));
		//MessageBox(m_ParentWindowName);
	}
#else
	m_serverFilePath = "//�з���//Y��ɭ//Debug";
	m_ParentWindowName = "MFCApplication1";
#endif
	w_parebtWindow = FindWindow(NULL, m_ParentWindowName);
	
	//������ʱĿ¼,
	CreateDirectory("C:\\FATS_update_temp",NULL);
	//�����汾�ż���߳�
	pThread = AfxBeginThread(VersionCheckThread, (LPVOID*)this);
	
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUPDATEDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CUPDATEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
����˵��:
remotefile   Ҫ���ص�ftpĿ¼���ļ�,��""//�з���//Y��ɭ//FATSSettings.ini""
localpath    ���浽���ص�Ŀ¼,��"C:\\FATS_update_temp"
isDir        ָ��Ҫ���ص����ļ������ļ���
ftpaddr      ftp��������ַ
username��������½ftp���������û���
ftpPass       ��½f����
*/
BOOL CUPDATEDlg::DownloadFromFTP(CString remotepath, CString localpath, BOOL isDir, CString ftpaddr, CString username, CString password, int port)
{	
	m_serverFileCount = GetFileCountFromFTP(remotepath, localpath, ftpaddr, username, password, port);
	static int nowFilePos = 0;
	int index = remotepath.ReverseFind('/');
	if (index == -1)
	{
		return FALSE;
	}
	CString remotefile = remotepath.Mid(index + 1, remotepath.GetLength());
	CInternetSession sess(_T("Download Files Session"));
	CFtpConnection* pFtpCon = NULL;
	try
	{
		pFtpCon = sess.GetFtpConnection(ftpaddr, username, password, port);
		if (pFtpCon == NULL)    //�������ʧ��
		{
			return FALSE;
		}
		CFtpFileFind ftpFinder(pFtpCon);
		BOOL bWorking = ftpFinder.FindFile(remotepath);
		if (bWorking != TRUE)   //�����FTP��������û���ҵ����ļ�
		{
			return FALSE;
		}
		if (isDir)   //���Ҫ���ص����ļ��У�����Ŀ¼
		{
			CreateDirectory(localpath + '\\' + remotefile, NULL);
		}
		else        //���Ҫ���ص���settings.ini�ļ���ֱ�����ص�C:\\FATS_update_temp\\FATSSettings.ini
		{
			
			//pFtpCon->GetFile(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + ftpFinder.GetFileName(), TRUE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 1);
			pFtpCon->GetFile(remotepath ,"C:\\FATS_update_temp\\FATSSettings.ini", FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 1);
		}
		
		//�����ļ�
		while (bWorking&&isDir)
		{

			bWorking = ftpFinder.FindNextFile();
			if (ftpFinder.IsDots())     //����ҵ�����"."����ʾ���ض���Ŀ¼������".."����ʾ������һ��Ŀ¼��
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				continue;
			}
			else if (ftpFinder.IsDirectory())    //����ҵ������ļ��У��ݹ����DownloadFromFTP()
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				DownloadFromFTP(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + remotefile, TRUE, ftpaddr, username, password, port);
			}
			else if(ftpFinder.GetFileName()!="UPDATE.exe")   //����ҵ������ļ�,ֱ������
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				CString temp;
				temp.Format("�������ظ���...%d/%d", ++nowFilePos,m_serverFileCount);
				m_tip.SetWindowText(temp);
				pFtpCon->GetFile(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' /*+ remotefile + '\\' */+ ftpFinder.GetFileName(), FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 1);
				m_ctrl_progress.SetStep(120 / m_serverFileCount);
				m_ctrl_progress.StepIt();
			}
		}
		m_ctrl_progress.SetPos(100);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		//printf("%s", sz);
		
		MessageBoxEx(0, "�����������쳣!���FATSSettings.ini��FTP������", "ConnectError", 0, 0);
		pEx->Delete();
	}
	if (pFtpCon != NULL)
	{
		pFtpCon->Close();
		delete pFtpCon;
		pFtpCon = NULL;
	}
	return TRUE;
}

//��ȡ�������ļ���
int CUPDATEDlg::GetFileCountFromFTP(CString remotepath, CString localpath, CString ftpaddr, CString username, CString password, int port)
{
	int fileCount = 0;
	int index = remotepath.ReverseFind('/');
	if (index == -1)
	{
		return 0;
	}
	CString remotefile = remotepath.Mid(index + 1, remotepath.GetLength());
	CInternetSession sess(_T("Download Files Session"));
	CFtpConnection* pFtpCon = NULL;
	try
	{
		pFtpCon = sess.GetFtpConnection(ftpaddr, username, password, port);
		if (pFtpCon == NULL)    //�������ʧ��
		{
			return 0;
		}
		CFtpFileFind ftpFinder(pFtpCon);
		BOOL bWorking = ftpFinder.FindFile(remotepath);
		if (bWorking != TRUE)   //�����FTP��������û���ҵ����ļ�
		{
			return 0;
		}
		//��ȡ�ļ���
		while (bWorking)
		{
			bWorking = ftpFinder.FindNextFile();
			if (ftpFinder.IsDots())     //����ҵ�����"."����ʾ���ض���Ŀ¼������".."����ʾ������һ��Ŀ¼��
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				continue;
			}
			else if (ftpFinder.IsDirectory())    //����ҵ������ļ��У��ݹ����DownloadFromFTP()
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				GetFileCountFromFTP(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + remotefile, ftpaddr, username, password, port);
			}
			else  //����ҵ������ļ�,
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				fileCount++;
			}
		}

	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		printf("%s", sz);
		pEx->Delete();
	}
	if (pFtpCon != NULL)
	{
		pFtpCon->Close();
		delete pFtpCon;
		pFtpCon = NULL;
	}
	return fileCount;
}



//return �������İ汾��
//���� ver�����ļ�·��
//����·����������������汾�����ذ汾
CString CUPDATEDlg::GetVersion(CString ver_iniPath)
{
	char buf[100];
	CString temp;
	::GetPrivateProfileString("VERSION","ver","",buf,MAX_PATH, ver_iniPath);
	temp.Format("%s", buf);
	return temp;
}


//�汾����߳�
UINT VersionCheckThread(LPVOID lpParam)
{
	CUPDATEDlg *tDlg = (CUPDATEDlg*)lpParam;
	CString temp;
	//CDApp  *papp = (CDApp*)AfxGetApp();
	tDlg->m_tip.SetWindowText("��������...");
	//����setting.INI
	if (tDlg->DownloadFromFTP(tDlg->m_serverFilePath + "//FATSSettings.ini", "C:\\FATS_update_temp", FALSE, tDlg->m_serverIP, tDlg->m_serverUserName, tDlg->m_serverPassword, tDlg->m_serverPort))
	{
		Sleep(500);
		//��ȡini
		tDlg->m_ver_getFromServer = tDlg->GetVersion("C:\\FATS_update_temp\\FATSSettings.ini");
		tDlg->m_ver_getFromLocal = tDlg->GetVersion(".\\FATSSettings.ini");
		if (tDlg->m_ver_getFromServer != tDlg->m_ver_getFromLocal)
		{
			//�ر���Ҫ���µĳ���Ľ���
#if !IS_DEBUG
			if (tDlg->w_parebtWindow == NULL)
				MessageBoxEx(0, "����������쳣���޷�����", "UpdateError", 0, 0);
			else
				tDlg->w_parebtWindow->SendMessage(WM_CLOSE);
#endif	
			temp.Format("���°汾��%s,��ǰ�汾��%s.", tDlg->m_ver_getFromServer, tDlg->m_ver_getFromLocal);
			tDlg->m_tip.SetWindowText(temp);
			tDlg->m_ctrl_update.EnableWindow(TRUE);
			tDlg->m_ctrl_progress.SetPos(0);
		}
		else
		{
			temp.Format("���°汾��%s,��ǰ�汾��%s.�޸���.", tDlg->m_ver_getFromServer, tDlg->m_ver_getFromLocal);
			tDlg->m_tip.SetWindowText(temp);
			Sleep(1000);
			AfxGetMainWnd()->SendMessage(WM_CLOSE);
		}
		return 1;
	}
	else
	{
		tDlg->MessageBox("��������FATSSettings.ini�ļ������飡");
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
		return 0;
	}
}

//�ļ��������߳�
UINT FileDownlaodThread(LPVOID lpParam)
{
	CUPDATEDlg *tDlg = (CUPDATEDlg*)lpParam;
	//�����ļ��е�temp
	tDlg->m_tip.SetWindowText("�������ظ���...");
	if (tDlg->DownloadFromFTP(tDlg->m_serverFilePath, ".\\", TRUE, tDlg->m_serverIP, tDlg->m_serverUserName, tDlg->m_serverPassword, tDlg->m_serverPort))
		tDlg->m_tip.SetWindowText("�������.");
	

	//ɾ��temp�ļ���
	tDlg->m_tip.SetWindowText("ɾ�������ļ���...");
	Sleep(500);
	tDlg->myDeleteDirectory("C:\\FATS_update_temp",TRUE);
	//��������???
	tDlg->m_tip.SetWindowText("�������.");
	tDlg->MessageBox("������ɣ�");
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
	return 1;
}

void CUPDATEDlg::OnBnClickedUpdate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	//ɾ��INI,���ļ����ڽ��޷�����
	//DeleteFile(".\\FATSSettings.ini");
	//ɾ�������ļ�����������
	//myDeleteDirectory(".\\",FALSE);
	//���������߳�
	pThread = AfxBeginThread(FileDownlaodThread, (LPVOID*)this);
	
	
}

void CUPDATEDlg::myDeleteDirectory(CString directory_path,BOOL deleteDir)   //ɾ��һ���ļ����µ���������
{
	CFileFind finder;
	CString path;
	path.Format("%s/*.*", directory_path);
	BOOL bWorking = finder.FindFile(path);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots()) //�����ļ���
		{
			myDeleteDirectory(finder.GetFilePath(),FALSE); //�ݹ�ɾ���ļ���
			RemoveDirectory(finder.GetFilePath());
		}
		else //�����ļ�
		{
			if(finder.GetFileName()!="UPDATE.exe")
				DeleteFile(finder.GetFilePath());
		}
	}
	if(deleteDir)
		RemoveDirectory(directory_path);
}




BOOL CUPDATEDlg::ResetProgress()
{
	m_ctrl_progress.SetPos(0);
	return 1;
}
