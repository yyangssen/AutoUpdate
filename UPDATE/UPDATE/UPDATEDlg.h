
// UPDATEDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CUPDATEDlg �Ի���
class CUPDATEDlg : public CDialogEx
{
// ����
public:
	CUPDATEDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	// ��ʾ��
	CStatic m_tip;
	CButton m_ctrl_update;
	CString m_ver_getFromServer;
	CString m_ver_getFromLocal;
	CString m_serverFilePath;
	CString m_ParentWindowName;
	CWnd *w_parebtWindow;
	afx_msg void OnBnClickedUpdate();
	// ����������
	CProgressCtrl m_ctrl_progress;
	BOOL ResetProgress();
	int m_serverFileCount;

	//����������
	CString m_serverIP;
	CString m_serverUserName;
	CString m_serverPassword;
	int m_serverPort;
};
