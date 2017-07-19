 
// UPDATEDlg.cpp : 实现文件
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
// CUPDATEDlg 对话框
CWinThread* pThread;//accept线程
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


// CUPDATEDlg 消息处理程序

BOOL CUPDATEDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_ctrl_update.EnableWindow(FALSE);
	//MessageBox(GetCommandLine());
	//MessageBox(AfxGetApp()->m_lpCmdLine);
	//获取ini中的服务器参数

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
	//获取主程序传来的参数
	m_ctrl_progress.SetRange(0, 100);
	ResetProgress();
	m_serverFileCount = 0;
#if !IS_DEBUG

	if (""== (temp = AfxGetApp()->m_lpCmdLine))
	{
		MessageBoxEx(0,"主程序参数异常，无法更新","UpdateError",0,0);
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
	}
	else//解析参数
	{
		//研发部//Y杨森;MFCApplication1
		int pos=temp.Find(";");//获取；位置
		m_serverFilePath = temp.Left(pos);
		//MessageBox(m_serverFilePath);
		m_ParentWindowName = temp.Right(temp.GetLength()-(pos+1));
		//MessageBox(m_ParentWindowName);
	}
#else
	m_serverFilePath = "//研发部//Y杨森//Debug";
	m_ParentWindowName = "MFCApplication1";
#endif
	w_parebtWindow = FindWindow(NULL, m_ParentWindowName);
	
	//创建临时目录,
	CreateDirectory("C:\\FATS_update_temp",NULL);
	//启动版本号检查线程
	pThread = AfxBeginThread(VersionCheckThread, (LPVOID*)this);
	
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUPDATEDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUPDATEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
参数说明:
remotefile   要下载的ftp目录或文件,如""//研发部//Y杨森//FATSSettings.ini""
localpath    保存到本地的目录,如"C:\\FATS_update_temp"
isDir        指明要下载的是文件还是文件夹
ftpaddr      ftp服务器地址
username　　　登陆ftp服务器的用户名
ftpPass       登陆f密码
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
		if (pFtpCon == NULL)    //如果连接失败
		{
			return FALSE;
		}
		CFtpFileFind ftpFinder(pFtpCon);
		BOOL bWorking = ftpFinder.FindFile(remotepath);
		if (bWorking != TRUE)   //如果在FTP服务器上没有找到该文件
		{
			return FALSE;
		}
		if (isDir)   //如果要下载的是文件夹，创建目录
		{
			CreateDirectory(localpath + '\\' + remotefile, NULL);
		}
		else        //如果要下载的是settings.ini文件，直接下载到C:\\FATS_update_temp\\FATSSettings.ini
		{
			
			//pFtpCon->GetFile(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + ftpFinder.GetFileName(), TRUE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 1);
			pFtpCon->GetFile(remotepath ,"C:\\FATS_update_temp\\FATSSettings.ini", FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 1);
		}
		
		//下载文件
		while (bWorking&&isDir)
		{

			bWorking = ftpFinder.FindNextFile();
			if (ftpFinder.IsDots())     //如果找到的是"."（表示返回顶级目录）或者".."（表示返回上一级目录）
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				continue;
			}
			else if (ftpFinder.IsDirectory())    //如果找到的是文件夹，递归调用DownloadFromFTP()
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				DownloadFromFTP(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + remotefile, TRUE, ftpaddr, username, password, port);
			}
			else if(ftpFinder.GetFileName()!="UPDATE.exe")   //如果找到的是文件,直接下载
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				CString temp;
				temp.Format("正在下载更新...%d/%d", ++nowFilePos,m_serverFileCount);
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
		
		MessageBoxEx(0, "服务器连接异常!检查FATSSettings.ini【FTP】配置", "ConnectError", 0, 0);
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

//获取服务器文件数
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
		if (pFtpCon == NULL)    //如果连接失败
		{
			return 0;
		}
		CFtpFileFind ftpFinder(pFtpCon);
		BOOL bWorking = ftpFinder.FindFile(remotepath);
		if (bWorking != TRUE)   //如果在FTP服务器上没有找到该文件
		{
			return 0;
		}
		//获取文件数
		while (bWorking)
		{
			bWorking = ftpFinder.FindNextFile();
			if (ftpFinder.IsDots())     //如果找到的是"."（表示返回顶级目录）或者".."（表示返回上一级目录）
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				continue;
			}
			else if (ftpFinder.IsDirectory())    //如果找到的是文件夹，递归调用DownloadFromFTP()
			{
				TRACE("%s\n", ftpFinder.GetFileName());
				GetFileCountFromFTP(remotepath + '/' + ftpFinder.GetFileName(), localpath + '\\' + remotefile, ftpaddr, username, password, port);
			}
			else  //如果找到的是文件,
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



//return 服务器的版本号
//参数 ver配置文件路径
//根据路径参数区别服务器版本？本地版本
CString CUPDATEDlg::GetVersion(CString ver_iniPath)
{
	char buf[100];
	CString temp;
	::GetPrivateProfileString("VERSION","ver","",buf,MAX_PATH, ver_iniPath);
	temp.Format("%s", buf);
	return temp;
}


//版本检查线程
UINT VersionCheckThread(LPVOID lpParam)
{
	CUPDATEDlg *tDlg = (CUPDATEDlg*)lpParam;
	CString temp;
	//CDApp  *papp = (CDApp*)AfxGetApp();
	tDlg->m_tip.SetWindowText("检查更新中...");
	//下载setting.INI
	if (tDlg->DownloadFromFTP(tDlg->m_serverFilePath + "//FATSSettings.ini", "C:\\FATS_update_temp", FALSE, tDlg->m_serverIP, tDlg->m_serverUserName, tDlg->m_serverPassword, tDlg->m_serverPort))
	{
		Sleep(500);
		//读取ini
		tDlg->m_ver_getFromServer = tDlg->GetVersion("C:\\FATS_update_temp\\FATSSettings.ini");
		tDlg->m_ver_getFromLocal = tDlg->GetVersion(".\\FATSSettings.ini");
		if (tDlg->m_ver_getFromServer != tDlg->m_ver_getFromLocal)
		{
			//关闭需要更新的程序的进程
#if !IS_DEBUG
			if (tDlg->w_parebtWindow == NULL)
				MessageBoxEx(0, "主程序参数异常，无法更新", "UpdateError", 0, 0);
			else
				tDlg->w_parebtWindow->SendMessage(WM_CLOSE);
#endif	
			temp.Format("有新版本：%s,当前版本：%s.", tDlg->m_ver_getFromServer, tDlg->m_ver_getFromLocal);
			tDlg->m_tip.SetWindowText(temp);
			tDlg->m_ctrl_update.EnableWindow(TRUE);
			tDlg->m_ctrl_progress.SetPos(0);
		}
		else
		{
			temp.Format("最新版本：%s,当前版本：%s.无更新.", tDlg->m_ver_getFromServer, tDlg->m_ver_getFromLocal);
			tDlg->m_tip.SetWindowText(temp);
			Sleep(1000);
			AfxGetMainWnd()->SendMessage(WM_CLOSE);
		}
		return 1;
	}
	else
	{
		tDlg->MessageBox("服务器无FATSSettings.ini文件，请检查！");
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
		return 0;
	}
}

//文件夹下载线程
UINT FileDownlaodThread(LPVOID lpParam)
{
	CUPDATEDlg *tDlg = (CUPDATEDlg*)lpParam;
	//下载文件夹到temp
	tDlg->m_tip.SetWindowText("正在下载更新...");
	if (tDlg->DownloadFromFTP(tDlg->m_serverFilePath, ".\\", TRUE, tDlg->m_serverIP, tDlg->m_serverUserName, tDlg->m_serverPassword, tDlg->m_serverPort))
		tDlg->m_tip.SetWindowText("下载完成.");
	

	//删除temp文件夹
	tDlg->m_tip.SetWindowText("删除缓存文件夹...");
	Sleep(500);
	tDlg->myDeleteDirectory("C:\\FATS_update_temp",TRUE);
	//重启程序???
	tDlg->m_tip.SetWindowText("更新完成.");
	tDlg->MessageBox("更新完成！");
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
	return 1;
}

void CUPDATEDlg::OnBnClickedUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//删除INI,此文件存在将无法覆盖
	//DeleteFile(".\\FATSSettings.ini");
	//删除所有文件，重新下载
	//myDeleteDirectory(".\\",FALSE);
	//启动升级线程
	pThread = AfxBeginThread(FileDownlaodThread, (LPVOID*)this);
	
	
}

void CUPDATEDlg::myDeleteDirectory(CString directory_path,BOOL deleteDir)   //删除一个文件夹下的所有内容
{
	CFileFind finder;
	CString path;
	path.Format("%s/*.*", directory_path);
	BOOL bWorking = finder.FindFile(path);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots()) //处理文件夹
		{
			myDeleteDirectory(finder.GetFilePath(),FALSE); //递归删除文件夹
			RemoveDirectory(finder.GetFilePath());
		}
		else //处理文件
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
