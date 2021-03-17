#include <afxdlgs.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <ctype.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <atlbase.h>
#include <atlstr.h>
#include <shobjidl.h>
#include <ShlObj.h>
#include <WinUser.h>
#include <structuredquery.h>
#include <winioctl.h>
#include <vector>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

//콘솔 출력용 코드
//#ifdef UNICODE
//#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
//#else
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
//#endif

using namespace std;

#define MAX_LOADSTRING 100

HWND hButtonOpenFileDialog;           // 파일열기 대화상자를 실행하기 위한 버튼의 핸들
HWND hEditFileToBeOpened;             // 파일의 경로와 이름을 가져오는 에디트 컨트롤의 핸들
OPENFILENAME OFN;                     // 파일열기 대화상자를 초기화하기 위한 변수
const UINT nFileNameMaxLen = 512;     // 다음 줄에 정의하는 szFileName 문자열의 최대 길이
WCHAR szFileName[nFileNameMaxLen];    // 파일의 경로 및 이름을 복사하기 위한 문자열

BOOL CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

wstring diskList[26];                //물리디스크 리스트를 저장할 배열 선언
ULONGLONG diskSize[26];              //물리디스크 크기 저장 배열 선언          

HWND hDlg;
HWND hTabCtrl;
HWND imagePath;
HWND diskListBox;
HWND backUpName;
HWND backUpNameStatic;
HWND runBtn;

int tabIndex = 0;   //tab 인덱스 

string TCHARToString(TCHAR* ptsz) {
	int len = wcslen((wchar_t*)ptsz);

	char* psz = new char[2 * len + 1];

	wcstombs(psz, (wchar_t*)ptsz, 2 * len + 1);
	string s = psz;

	delete[] psz;

	return s;
}

//physical drive 리스트 가져오는 함수
void GetDiskGeometry()
{
	HANDLE	hDevice;			// handle to the drive to be examined
	BOOL	bResult = FALSE;		// results flag
	DWORD	dwjunk = 0;		// discard results

	UINT64	uLBASize = 0L;

	DISK_GEOMETRY_EX	stDiskGeometryEx;		// disk geometry structure
	TCHAR tchDriveName[100];                    // 디스크 이름 저장 배열

	//반복문을 돌면서 물리 디스크 존재하는 개수만큼 이름 저장
	for (int i = 0; i < 26; i++)
	{
		wsprintf(tchDriveName, _T("\\\\.\\PhysicalDrive%d"), i);

		hDevice = CreateFile(tchDriveName,	// drive to open
			0,			// no access to the drive
			FILE_SHARE_READ |		// share mode
			FILE_SHARE_WRITE,
			NULL,			// default security attributes
			OPEN_EXISTING,		// disposition
			0,			// file attributes
			NULL);			// do not copy file attributes

		if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
		{
			break;
		}

		diskList[i] = tchDriveName;    // diskList에 추가

		bResult = DeviceIoControl(hDevice,		// device to be queried
			IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,	// operation to perform
			NULL, 0, // no input buffer
			&stDiskGeometryEx, sizeof(DISK_GEOMETRY_EX),	// output buffer
			&dwjunk,							// # bytes returned
			(LPOVERLAPPED)NULL);				// synchronous I/O

		CloseHandle(hDevice);


		ULONGLONG DiskSize = stDiskGeometryEx.Geometry.Cylinders.QuadPart * (ULONG)stDiskGeometryEx.Geometry.TracksPerCylinder *
			(ULONG)stDiskGeometryEx.Geometry.SectorsPerTrack * (ULONG)stDiskGeometryEx.Geometry.BytesPerSector;
		
		diskSize[i] = (double)DiskSize / (1024 * 1024 * 1024);

	}

	return;
}

//메인
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR lpCmdLine, int nCmdShow) {

	GetDiskGeometry();   // 디스크 리스트 불러오기

	InitCommonControls();

	//Dialog 생성
	MSG  msg;
	BOOL ret;
	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DLGPROC)&DialogProc, 0);

	ShowWindow(hDlg, nCmdShow);

	// 사용자로부터의 메세지를 받아 처리
	while (GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//Tab 컨트롤
BOOL CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	TC_ITEM tItem;
	wstring strFolder;
	int iTab = TabCtrl_GetCurFocus(hTabCtrl);

	switch (iMessage)
	{
		// 탭 초기 설정
	case WM_INITDIALOG: {
		hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);
		tItem.mask = TCIF_TEXT;
		tItem.pszText = (LPWSTR)L"Restore";
		TabCtrl_InsertItem(hTabCtrl, 0, &tItem);

		tItem.pszText = (LPWSTR)L"BackUp";
		TabCtrl_InsertItem(hTabCtrl, 1, &tItem);

		// 필요한 ID 가져오기 
		imagePath = GetDlgItem(hDlg, IDC_PATH);
		diskListBox = GetDlgItem(hDlg, IDC_DISKLISTBOX);
		runBtn = GetDlgItem(hDlg, IDC_RUN);

		SendMessage(runBtn, WM_SETTEXT, NULL, (LPARAM)L"Recovery");

		// 백업에만 사용하므로 처음 복구에서는 감추기
		ShowWindow(backUpName, SW_HIDE);
		ShowWindow(backUpNameStatic, SW_HIDE);

		for (int i = 0; i < 26; ++i) {
			if (diskList[i].empty())
				break;

			string dSize = to_string(diskSize[i]);
			wstring wsize;
			wsize.assign(dSize.begin(), dSize.end());

			wstring result = diskList[i] + L"  |  " + wsize + L"GB";

			SendMessage(diskListBox, LB_ADDSTRING, 0, (LPARAM)((LPCTSTR)result.c_str()));
		}

		break;
	}

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		PostQuitMessage(0);
		break;

	case WM_NOTIFY:
		LPNMHDR nmhdr;
		if (wParam == IDC_TAB1) {
			nmhdr = (LPNMHDR)lParam;

			switch (nmhdr->code)
			{
				//탭 전환 이벤트 설정
			case TCN_SELCHANGE:
				iTab = TabCtrl_GetCurFocus(hTabCtrl);   //현재 탭 번호 가져오기

				// 복구 탭
				if (iTab == 0) {
					tabIndex = 0;

					SendMessage(imagePath, WM_SETTEXT, NULL, NULL);
					SendMessage(runBtn, WM_SETTEXT, NULL, (LPARAM)L"Recovery");
				}
				// 백업 탭
				else if (iTab == 1) {
					tabIndex = 1;

					SendMessage(imagePath, WM_SETTEXT, NULL, NULL);
					SendMessage(runBtn, WM_SETTEXT, NULL, (LPARAM)L"BackUp");
				}
			}
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BROWSER: {
			LPCTSTR fileFilter = _T("FFU files (*.FFU) | *.FFU");

			CFileDialog dlg(TRUE, _T("FFU"), NULL, 0, fileFilter, NULL, 0, 1);

			CString path;

			if (IDOK == dlg.DoModal()) {
				path = dlg.GetPathName();
			}

			TCHAR* folder = (TCHAR*)(LPCTSTR)path;

			SendMessage(imagePath, WM_SETTEXT, 0, (LPARAM)folder);       //경로를 imagePath에 전송

			return 0;
		}

		case IDC_RUN: {
			//복구 
			if (tabIndex == 0) {
				TCHAR recoverFilePath[100], captureDrive[100];

				int selection = SendMessage(diskListBox, LB_GETCURSEL, 0, 0);  //위치 얻어오기
				SendMessage(diskListBox, LB_GETTEXT, selection, (LPARAM)captureDrive); // 위치에 있는 문자열 얻어오기

				GetWindowText(imagePath, recoverFilePath, 100);

				if (TCHARToString(recoverFilePath) == "") {
					MessageBox(hDlg, L"Please Select FilePath!", L"Error", MB_OK);
				}
				else if (selection == -1) {
					MessageBox(hDlg, L"Please Select Disk!", L"Error", MB_OK);
				}
				else {
					string driveName = TCHARToString(captureDrive);
					int index = driveName.find("  |  ");

					string result = "/c DISM.exe /Apply-Ffu /ImageFile:" + TCHARToString(recoverFilePath) + " /ApplyDrive:" + driveName.substr(0, index) + " || pause";

					ShellExecuteA(NULL, "open", "cmd.exe", (LPCSTR)result.c_str(), NULL, SW_SHOW);  //실행
				}

				return 0;
			}

			//백업
			else if (tabIndex == 1) {
				TCHAR backupFilePath[100], captureDrive[100];

				int selection = SendMessage(diskListBox, LB_GETCURSEL, 0, 0);  //위치 얻어오기
				SendMessage(diskListBox, LB_GETTEXT, selection, (LPARAM)captureDrive); // 위치에 있는 문자열 얻어오기

				GetWindowText(imagePath, backupFilePath, 100);

				if (TCHARToString(backupFilePath) == "") {
					MessageBox(hDlg, L"Please Select FilePath!", L"Error", MB_OK);
				}
				else if (selection == -1) {
					MessageBox(hDlg, L"Please Select Disk!", L"Error", MB_OK);
				}
				else {
					string driveName = TCHARToString(captureDrive);
					int index = driveName.find("  |  ");

					string result = "/c DISM.exe /Capture-Ffu /ImageFile:" + TCHARToString(backupFilePath)
						+ " /CaptureDrive:" + driveName.substr(0, index) + " /Name:Drive0 || pause";

					ShellExecuteA(NULL, "open", "cmd.exe", (LPCSTR)result.c_str(), NULL, SW_SHOW);  //실행
				}
			}

			break;
		}

		case IDC_CANCLE:
			EndDialog(hDlg, FALSE);
			PostQuitMessage(0);
			break;

		}
		break;
	}

	return FALSE;
}