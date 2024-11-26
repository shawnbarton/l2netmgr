/*
 * $Id: wingui.cpp,v 1.8 2006/06/02 16:24:59 int19 Exp $
 *
 * Copyright (c) 2006, Eric Enright
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Eric Enright nor the names of his contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * A simple Windows GUI for l2netmgr.  It provides an output window and
 * a system tray icon to which the window can be minimized to.
 */

#include "stdafx.h"
#include <commdlg.h>
#include "l2netmgr.h"
#include <shellapi.h>
#define MAX_LOADSTRING 100

#include <fstream>
#include <ios>
#include "log.hpp"

#define WM_USER_SHELLICON  (WM_USER + 1)
#define ID_EDIT 1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

NOTIFYICONDATA g_nid;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Settings(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_L2NETMGR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_L2NETMGR);

	DWORD threadId;
	HANDLE netThread = CreateThread(NULL, 0, network_thread, NULL, 0,
		&threadId);

	// Main message loop:
	while (!g_shutdown && GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WaitForSingleObject(netThread, INFINITE);
	Shell_NotifyIcon(NIM_DELETE, &g_nid);

	g_config.saveFile("l2netmgr.conf");

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_L2NETMGR);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_L2NETMGR;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

	// System tray stuff
	HICON hMainIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_L2NETMGR));
	HICON hmainIcon1 = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_L2NETMGR));

	g_nid.cbSize = sizeof(g_nid);
	g_nid.hWnd = hWnd;
	g_nid.uID = IDC_MYICON;
	g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	strcpy(g_nid.szTip, "l2netmgr");
	g_nid.hIcon = hMainIcon;
	g_nid.uCallbackMessage = WM_USER_SHELLICON;

	Shell_NotifyIcon(NIM_ADD, &g_nid);

   return TRUE;
}

void
save_file(HWND hWnd, LPSTR file)
{
	std::ofstream fout(file, std::ios::binary);

	if (!fout)
	{
		MessageBox(hWnd, TEXT("Unable to open file for writing"), TEXT("Uh oh.."),
			MB_OK | MB_ICONSTOP);
		return;
	}

	unsigned int count = (unsigned int)SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0);
	for (unsigned int i = 0; i < count; ++i)
	{
		char buf[1024];

		// Set up the length of the buffer...
		buf[0] = 0;
		buf[1] = 4;
		buf[2] = 0;
		buf[3] = 0;

		unsigned int n = (unsigned int)SendMessage(hwndEdit,
			EM_GETLINE, i, (LPARAM)buf);
		fout.write(buf, n);
		fout << std::endl;
	}

	fout.close();
}


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	OPENFILENAME ofn;
	char fname[1024];

	switch (message) 
	{
	case WM_CREATE:
		hwndEdit = CreateWindow(TEXT("edit"), NULL,
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER
				| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL /*| WS_DISABLED*/,
				0, 0, 0, 0, hWnd, (HMENU) ID_EDIT,
				((LPCREATESTRUCT)lParam)->hInstance, NULL);
		// Set a fixed-width font in the text box.
		SendMessage(hwndEdit, WM_SETFONT, (WPARAM)GetStockObject(OEM_FIXED_FONT), TRUE);
		SendMessage(hwndEdit, EM_SETREADONLY, TRUE, 0);
		break;
	case WM_SETFOCUS:
		SetFocus(hwndEdit);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 

		if (wmId == ID_EDIT
			&& (wmEvent == EN_ERRSPACE || wmEvent == EN_MAXTEXT))
		{
			MessageBox(hWnd, TEXT("Edit control out of space!"), 
				TEXT("Uh oh.."), MB_OK | MB_ICONSTOP);
			break;
		}


		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_MYABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_SETTINGS:
			DialogBox(hInst, (LPCTSTR)IDD_SETTINGS, hWnd, (DLGPROC)Settings);
			break;
		case IDM_EXIT:
			Shell_NotifyIcon(NIM_DELETE, &g_nid);
			DestroyWindow(hWnd);
			g_shutdown = true;
			break;
		case IDM_SAVELOG:
			memset(&ofn, '\0', sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			fname[0] = '\0';
			ofn.lpstrFilter = NULL;
			ofn.lpstrFile = fname;
			ofn.nMaxFile = sizeof(fname);
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
			ofn.lpstrTitle = NULL;
			if (GetSaveFileName(&ofn))
				save_file(hWnd, fname);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		g_shutdown = true;
		PostQuitMessage(0);
		break;
	case WM_USER_SHELLICON:
		switch (LOWORD(lParam))
		{
		case WM_LBUTTONDBLCLK:
			ShowWindow(hWnd, SW_NORMAL);
			break;
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_MINIMIZE)
		{
			ShowWindow(hWnd, SW_HIDE);
		}
		else
		{
			if (wParam == SC_CLOSE)
				Shell_NotifyIcon(NIM_DELETE, &g_nid);
			DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_SIZE:
		MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

// Message handler for the settings dialog
LRESULT CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int port;
	int keepalive;
	int debug;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT_CACHED_ADDR, g_config.getString("cached_addr"));
		g_config.getInt("cached_port", &port);
		SetDlgItemInt(hDlg, IDC_EDIT_CACHED_PORT, port, TRUE);

		SetDlgItemText(hDlg, IDC_EDIT_AUTHD_ADDR, g_config.getString("authd_addr"));
		g_config.getInt("authd_port", &port);
		SetDlgItemInt(hDlg, IDC_EDIT_AUTHD_PORT, port, TRUE);

		g_config.getInt("listen_port", &port);
		SetDlgItemInt(hDlg, IDC_EDIT_PORT, port, TRUE);

		g_config.getInt("keepalive", &keepalive);
		SetDlgItemInt(hDlg, IDC_EDIT_KEEPALIVE, keepalive, TRUE);
		g_config.getInt("debug", &debug);
		SendMessage(GetDlgItem(hDlg, IDC_DEBUG), BM_SETCHECK,
			debug > 0 ? BST_CHECKED : BST_UNCHECKED, 0);
		SetDlgItemText(hDlg, IDC_EDIT_PASSWD, g_config.getString("password"));
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			// Debug checkbox?
			int checked = (int)SendMessage(GetDlgItem(hDlg, IDC_DEBUG),
				BM_GETCHECK, 0, 0);

			if (checked)
			{
				g_config.setInt("debug", 1);
				am::l2netmgr::setLogLevel(L2LOG_DEBUG);
			}
			else
			{
				g_config.setInt("debug", 0);
				am::l2netmgr::setLogLevel(L2LOG_ERR);
			}

			// New listener port?
			BOOL translated;
			port = (int)GetDlgItemInt(hDlg, IDC_EDIT_PORT, &translated, FALSE);
			int oldport;
			g_config.getInt("listen_port", &oldport);

			if (port != oldport)
			{
				g_config.setInt("listen_port", port);
				MessageBox(hDlg, TEXT("New listener port will take effect"
					" the next time you start l2netmgr."), TEXT("Notice"),
					MB_OK | MB_ICONINFORMATION);
			}

			// New cached connection information?
			char newaddr[64];
			GetDlgItemText(hDlg, IDC_EDIT_CACHED_ADDR, newaddr, sizeof(newaddr));
			port = (int)GetDlgItemInt(hDlg, IDC_EDIT_CACHED_PORT, &translated, FALSE);
			g_config.getInt("cached_port", &oldport);
			if (port != oldport)
			{
				g_config.setInt("cached_port", port);
				g_reconnect = true;
			}
			if (strncmp(newaddr, g_config.getString("cached_addr"),
				sizeof(newaddr)))
			{
				g_config.setString("cached_addr", newaddr);
				g_reconnect = true;
			}

			// New authd connection information?
			GetDlgItemText(hDlg, IDC_EDIT_AUTHD_ADDR, newaddr, sizeof(newaddr));
			port = (int)GetDlgItemInt(hDlg, IDC_EDIT_AUTHD_PORT, &translated, FALSE);
			g_config.getInt("authd_port", &oldport);
			if (port != oldport)
			{
				g_config.setInt("authd_port", port);
				g_reconnect = true;
			}
			if (strncmp(newaddr, g_config.getString("authd_addr"),
				sizeof(newaddr)))
			{
				g_config.setString("authd_addr", newaddr);
				g_reconnect = true;
			}

			// Keepalive?
			int newka = (int)GetDlgItemInt(hDlg, IDC_EDIT_KEEPALIVE, &translated, FALSE);
			g_config.getInt("keepalive", &keepalive);
			if (newka != keepalive)
				g_config.setInt("keepalive", newka);

			// Password?
			char passwd[64];
			GetDlgItemText(hDlg, IDC_EDIT_PASSWD, passwd, sizeof(passwd));
			if (strncmp(passwd, g_config.getString("password"), sizeof(passwd)))
				g_config.setString("password", passwd);
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}

	return FALSE;
}
