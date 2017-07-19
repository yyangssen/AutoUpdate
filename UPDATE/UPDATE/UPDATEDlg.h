
// UPDATEDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CUPDATEDlg 对话框
class CUPDATEDlg : public CDialogEx
{
// 构造
public:
	CUPDATEDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BOOL DownloadFromFTP(CString remotepath, CString localpath, BOOL isDir, CString ftpaddr, CString username, CString password, int port);
	int GetFileCountFromFTP(CString remotepath, CString localpath, CString ftpaddr, CString username, CString password, int port);

protected:
	
public:
	CString GetVersion(CString ver_iniPath);
	void myDeleteDirectory(CString directory_path, BOOL deleteDir);
	// 提示条
	CStatic m_tip;
	CButton m_ctrl_update;
	CString m_ver_getFromServer;
	CString m_ver_getFromLocal;
	CString m_serverFilePath;
	CString m_ParentWindowName;
	CWnd *w_parebtWindow;
	afx_msg void OnBnClickedUpdate();
	// 升级进度条
	CProgressCtrl m_ctrl_progress;
	BOOL ResetProgress();
	int m_serverFileCount;

	//服务器参数
	CString m_serverIP;
	CString m_serverUserName;
	CString m_serverPassword;
	int m_serverPort;
};
