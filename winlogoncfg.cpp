#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winstrct.h>
#include <objbase.h>
#include <shlobj.h>

#include "winlogoncfg.rc.h"

#define SUBKEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define WINDOW_TITLE L"Winlogon Configuration Tool"
#define HELP_FILE L"winlogoncfg.hlp"
#define IDH_INTRO 90000

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ntdllp.lib")

#ifdef _M_ARM64
#define startup() WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
#else
#pragma comment(linker, "/subsystem:windows")
#pragma comment(linker, "/entry:startup")
#endif

typedef struct _WCHAR_BUFFER
{
  LPWSTR Buffer;
  int MaxChars;
} WCHAR_BUFFER, *PWCHAR_BUFFER;

HINSTANCE hInstance = NULL;
bool bSettingsChanged = false;
HKEY hKey = INVALID_HKEY;

void
DoEvents(HWND hWnd)
{
  MSG msg;
  while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
    {
      if (!IsDialogMessage(hWnd, &msg))
	TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
}

bool
LoadSettings(HWND hWnd, LPWSTR lpComputerName)
{
  HKEY hKeyNew = INVALID_HKEY;
  LONG lErrNo;

  if (lpComputerName == NULL)
    lErrNo = RegOpenKey(HKEY_LOCAL_MACHINE, SUBKEY, &hKeyNew);
  else
    {
      SetClassLongPtr(hWnd, GCLP_HCURSOR,
		      (LONG_PTR) LoadCursor(NULL, IDC_WAIT));
      SetCursor(LoadCursor(NULL, IDC_WAIT));

      HKEY hKeyRoot;
      lErrNo = RegConnectRegistry(lpComputerName, HKEY_LOCAL_MACHINE,
				  &hKeyRoot);

      SetClassLongPtr(hWnd, GCLP_HCURSOR,
		      (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
      SetCursor(LoadCursor(NULL, IDC_ARROW));

      if (lErrNo != NO_ERROR)
	{
	  LPSTR errmsg = win_errmsgA(lErrNo);
	  MessageBoxA(hWnd, errmsg, "Connection error",
		      MB_ICONEXCLAMATION);
	  LocalFree(errmsg);
	  return false;
	}

      lErrNo = RegOpenKey(hKeyRoot, SUBKEY, &hKeyNew);
      RegCloseKey(hKeyRoot);
    }

  if (lErrNo != NO_ERROR)
    {
      LPSTR errmsg = win_errmsgA(lErrNo);
      MessageBoxA(hWnd, errmsg, "Registry error", MB_ICONEXCLAMATION);
      LocalFree(errmsg);
      return false;
    }

  SetClassLongPtr(hWnd, GCLP_HCURSOR,
		  (LONG_PTR) LoadCursor(NULL, IDC_WAIT));
  SetCursor(LoadCursor(NULL, IDC_WAIT));

  if (hKey != INVALID_HKEY)
    RegCloseKey(hKey);
  hKey = hKeyNew;

  WCHAR wcDataBuffer[MAX_PATH];
  DWORD dwDataType;

  DWORD dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"DefaultUserName", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    SetDlgItemText(hWnd, IDC_EDIT_USERNAME, L"");
  else
    SetDlgItemText(hWnd, IDC_EDIT_USERNAME, wcDataBuffer);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"DefaultPassword", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    SetDlgItemText(hWnd, IDC_EDIT_PASSWORD, L"");
  else
    SetDlgItemText(hWnd, IDC_EDIT_PASSWORD, wcDataBuffer);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"DefaultDomainName", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    SetDlgItemText(hWnd, IDC_EDIT_DOMAIN, L"");
  else
    SetDlgItemText(hWnd, IDC_EDIT_DOMAIN, wcDataBuffer);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"AutoAdminLogon", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_AUTOLOG, BST_UNCHECKED);
  else if (wcstoul(wcDataBuffer, NULL, 0) > 0)
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_AUTOLOG, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_AUTOLOG, BST_UNCHECKED);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"IgnoreShiftOverride", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHIFT, BST_CHECKED);
  else if (wcstoul(wcDataBuffer, NULL, 0) > 0)
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHIFT, BST_UNCHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHIFT, BST_CHECKED);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"ForceAutoLogon", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    CheckDlgButton(hWnd, IDC_CHK_FORCE_AUTOLOGON, BST_INDETERMINATE);
  else if (wcstoul(wcDataBuffer, NULL, 0) > 0)
    CheckDlgButton(hWnd, IDC_CHK_FORCE_AUTOLOGON, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_FORCE_AUTOLOGON, BST_UNCHECKED);

  DWORD dwData;
  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"SFCDisable", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    {
      EnableWindow(GetDlgItem(hWnd, IDC_CHK_DISABLE_WFP), FALSE);
      CheckDlgButton(hWnd, IDC_CHK_DISABLE_WFP, BST_INDETERMINATE);
    }
  else
    {
      EnableWindow(GetDlgItem(hWnd, IDC_CHK_DISABLE_WFP), TRUE);
      if (dwData > 0)
	CheckDlgButton(hWnd, IDC_CHK_DISABLE_WFP, BST_CHECKED);
      else
	CheckDlgButton(hWnd, IDC_CHK_DISABLE_WFP, BST_UNCHECKED);
    }

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"SfcQuota", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    {
      EnableWindow(GetDlgItem(hWnd, IDC_STAT_WFP_LIMIT), FALSE);
      EnableWindow(GetDlgItem(hWnd, IDC_EDIT_WFP_LIMIT), FALSE);
      SetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, L"");
    }
  else
    {
      EnableWindow(GetDlgItem(hWnd, IDC_STAT_WFP_LIMIT), TRUE);
      EnableWindow(GetDlgItem(hWnd, IDC_EDIT_WFP_LIMIT), TRUE);
      if (dwData == (DWORD) -1)
	SetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, L"");
      else
	{
	  _snwprintf(wcDataBuffer, _countof(wcDataBuffer) - 1,
              L"%u", dwData);
	  SetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, wcDataBuffer);
	}
    }

  if (IsWindowEnabled(GetDlgItem(hWnd, IDC_EDIT_WFP_LIMIT)) |
      IsWindowEnabled(GetDlgItem(hWnd, IDC_CHK_DISABLE_WFP)))
    EnableWindow(GetDlgItem(hWnd, IDC_GROUP_WFP), TRUE);
  else
    EnableWindow(GetDlgItem(hWnd, IDC_GROUP_WFP), FALSE);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"ShutdownWithoutLogon", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHUTDOWN, BST_INDETERMINATE);
  else if (wcstoul(wcDataBuffer, NULL, 0) > 0)
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHUTDOWN, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_ENABLE_SHUTDOWN, BST_UNCHECKED);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"DisableCAD", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_CAD_REQUIRED, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_CAD_REQUIRED, BST_UNCHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_CAD_REQUIRED, BST_CHECKED);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"ShowLogonOptions", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_SHOW_ADV, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_SHOW_ADV, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_SHOW_ADV, BST_UNCHECKED);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"DontDisplayLastUserName", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LAST_USER, BST_INDETERMINATE);
  else if (wcstoul(wcDataBuffer, NULL, 0) > 0)
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LAST_USER, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LAST_USER, BST_UNCHECKED);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"Userinit", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    SetDlgItemText(hWnd, IDC_EDIT_USERINIT, L"");
  else
    SetDlgItemText(hWnd, IDC_EDIT_USERINIT, wcDataBuffer);

  dwSize = sizeof wcDataBuffer;
  lErrNo = RegQueryValueEx(hKey, L"Shell", NULL, &dwDataType,
			   (LPBYTE) wcDataBuffer, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_SZ))
    SetDlgItemText(hWnd, IDC_EDIT_SHELL, L"");
  else
    SetDlgItemText(hWnd, IDC_EDIT_SHELL, wcDataBuffer);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"RunLogonScriptSync", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_RUN_SCRIPTS_SYNC, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_RUN_SCRIPTS_SYNC, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_RUN_SCRIPTS_SYNC, BST_UNCHECKED);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"DisableLockWorkstation", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LOCK_WKS, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LOCK_WKS, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_DISABLE_LOCK_WKS, BST_UNCHECKED);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"AllowMultipleTSSessions", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_ALLOW_MULTISESS, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_ALLOW_MULTISESS, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_ALLOW_MULTISESS, BST_UNCHECKED);

  dwSize = sizeof dwData;
  lErrNo = RegQueryValueEx(hKey, L"AutoRestartShell", NULL, &dwDataType,
			   (LPBYTE) &dwData, &dwSize);
  if ((lErrNo != NO_ERROR) | (dwDataType != REG_DWORD))
    CheckDlgButton(hWnd, IDC_CHK_AUTO_RESTART_SHELL, BST_INDETERMINATE);
  else if (dwData > 0)
    CheckDlgButton(hWnd, IDC_CHK_AUTO_RESTART_SHELL, BST_CHECKED);
  else
    CheckDlgButton(hWnd, IDC_CHK_AUTO_RESTART_SHELL, BST_UNCHECKED);

  if (lpComputerName == NULL)
    SetWindowText(hWnd, WINDOW_TITLE L" - Local machine");
  else
    {
      _snwprintf(wcDataBuffer, sizeof(wcDataBuffer)/sizeof(*wcDataBuffer),
		 WINDOW_TITLE L" - %s", lpComputerName);
      wcDataBuffer[(sizeof(wcDataBuffer)/sizeof(*wcDataBuffer))-1] = 0;
      SetWindowText(hWnd, wcDataBuffer);
    }

  EnableMenuItem(GetMenu(hWnd), CM_FILE_SAVE, MF_ENABLED);
  SetFocus(GetDlgItem(hWnd, IDC_EDIT_USERNAME));
  SendDlgItemMessage(hWnd, IDC_EDIT_USERNAME, EM_SETSEL, 0, (LPARAM) (INT) -1);
  bSettingsChanged = false;

  SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
  SetCursor(LoadCursor(NULL, IDC_ARROW));

  return true;
}

bool
SaveSettings(HWND hWnd)
{
  SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_WAIT));
  SetCursor(LoadCursor(NULL, IDC_WAIT));

  LONG lLastErrNo = NO_ERROR;

  const WCHAR wcEnable[] = L"1";
  const WCHAR wcDisable[] = L"0";
  const DWORD dwEnable = 1;
  const DWORD dwDisable = 0;

  LONG lErrNo;

  WCHAR wcDataBuffer[MAX_PATH] = L"";
  UINT uiSize = GetDlgItemText(hWnd, IDC_EDIT_USERNAME, wcDataBuffer,
			       sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
  if (uiSize == 0)
    lErrNo = RegDeleteValue(hKey, L"DefaultUserName");
  else
    lErrNo = RegSetValueEx(hKey, L"DefaultUserName", 0, REG_SZ,
			   (CONST BYTE *) wcDataBuffer,
			   (uiSize + 1) * sizeof(*wcDataBuffer));
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  uiSize = GetDlgItemText(hWnd, IDC_EDIT_PASSWORD, wcDataBuffer,
			  sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
  if (uiSize == 0)
    lErrNo = RegDeleteValue(hKey, L"DefaultPassword");
  else
    lErrNo = RegSetValueEx(hKey, L"DefaultPassword", 0, REG_SZ,
			   (CONST BYTE *) wcDataBuffer,
			   (uiSize + 1) * sizeof(*wcDataBuffer));
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  uiSize = GetDlgItemText(hWnd, IDC_EDIT_DOMAIN, wcDataBuffer,
			  sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
  if (uiSize == 0)
    lErrNo = RegDeleteValue(hKey, L"DefaultDomainName");
  else
    lErrNo = RegSetValueEx(hKey, L"DefaultDomainName", 0, REG_SZ,
			   (CONST BYTE *) wcDataBuffer,
			   (uiSize + 1) * sizeof(*wcDataBuffer));
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_ENABLE_AUTOLOG))
    {
    case BST_CHECKED:
      if (GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT_PASSWORD)) == 0)
	MessageBoxA(hWnd,
		    "You have activated auto logon with an account with no "
		    "password. On Windows NT 4.0 and earlier, this means that "
		    "auto logon will happen only once.", "No password warning",
		    MB_ICONEXCLAMATION);

      if (IsDlgButtonChecked(hWnd, IDC_CHK_DISABLE_LAST_USER) == BST_CHECKED)
	MessageBoxA(hWnd,
		    "You have activated booth auto logon and to not display "
		    "last logged on user. This is not a compatible "
		    "combination and auto-logon will not work.",
		    "Compatibility warning", MB_ICONEXCLAMATION);

      if (IsDlgButtonChecked(hWnd, IDC_CHK_FORCE_AUTOLOGON) != BST_CHECKED)
	MessageBoxA(hWnd,
		    "You have activated auto logon but not to force the "
		    "auto-logon settings to be saved across reboots. On "
		    "Windows 2000 and later, this will cause auto logon to "
		    "work only on next reboot.", "Compatibility warning",
		    MB_ICONEXCLAMATION);

      lErrNo = RegSetValueEx(hKey, L"AutoAdminLogon", 0, REG_SZ,
			     (CONST BYTE *) wcEnable, sizeof wcEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"AutoAdminLogon", 0, REG_SZ,
			     (CONST BYTE *) wcDisable, sizeof wcDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"AutoAdminLogon");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_ENABLE_SHIFT))
    {
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"IgnoreShiftOverride", 0, REG_SZ,
			     (CONST BYTE *) wcEnable, sizeof wcEnable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"IgnoreShiftOverride");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_FORCE_AUTOLOGON))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"ForceAutoLogon", 0, REG_SZ,
			     (CONST BYTE *) wcEnable, sizeof wcEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"ForceAutoLogon", 0, REG_SZ,
			     (CONST BYTE *) wcDisable, sizeof wcDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"IgnoreShiftOverride");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  if (IsWindowEnabled(GetDlgItem(hWnd, IDC_CHK_DISABLE_WFP)))
    {
      switch (IsDlgButtonChecked(hWnd, IDC_CHK_DISABLE_WFP))
	{
	case BST_CHECKED:
	  lErrNo = RegSetValueEx(hKey, L"SFCDisable", 0, REG_DWORD,
				 (CONST BYTE *) &dwEnable, sizeof dwEnable);
	  break;
	case BST_UNCHECKED:
	  lErrNo = RegSetValueEx(hKey, L"SFCDisable", 0, REG_DWORD,
				 (CONST BYTE *) &dwDisable, sizeof dwDisable);
	  break;
	}
      if (lErrNo != NO_ERROR)
	lLastErrNo = lErrNo;
    }

  if (IsWindowEnabled(GetDlgItem(hWnd, IDC_EDIT_WFP_LIMIT)))
    {
      uiSize = GetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, wcDataBuffer,
			      sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
      DWORD dwLimit;
      if (uiSize == 0)
	dwLimit = (DWORD) -1;
      else
	dwLimit = wcstoul(wcDataBuffer, NULL, 0);

      lErrNo = RegSetValueEx(hKey, L"SfcQuota", 0, REG_DWORD,
			     (CONST BYTE *) &dwLimit, sizeof dwLimit);
      if (lErrNo != NO_ERROR)
	lLastErrNo = lErrNo;
    }

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_ENABLE_SHUTDOWN))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"ShutdownWithoutLogon", 0, REG_SZ,
			     (CONST BYTE *) wcEnable, sizeof wcEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"ShutdownWithoutLogon", 0, REG_SZ,
			     (CONST BYTE *) wcDisable, sizeof wcDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"ShutdownWithoutLogon");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_SHOW_ADV))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"ShowLogonOptions", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"ShowLogonOptions", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"ShowLogonOptions");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_CAD_REQUIRED))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"DisableCAD", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"DisableCAD", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"DisableCAD");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_DISABLE_LAST_USER))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"DontDisplayLastUserName", 0, REG_SZ,
			     (CONST BYTE *) wcEnable, sizeof wcEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"DontDisplayLastUserName", 0, REG_SZ,
			     (CONST BYTE *) wcDisable, sizeof wcDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"DontDisplayLastUserName");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  uiSize = GetDlgItemText(hWnd, IDC_EDIT_USERINIT, wcDataBuffer,
			  sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
  if (uiSize == 0)
    lErrNo = RegDeleteValue(hKey, L"Userinit");
  else
    lErrNo = RegSetValueEx(hKey, L"Userinit", 0, REG_SZ,
			   (CONST BYTE *) wcDataBuffer,
			   (uiSize + 1) * sizeof(*wcDataBuffer));
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  uiSize = GetDlgItemText(hWnd, IDC_EDIT_SHELL, wcDataBuffer,
			  sizeof(wcDataBuffer)/sizeof(*wcDataBuffer));
  if (uiSize == 0)
    lErrNo = RegDeleteValue(hKey, L"Shell");
  else
    lErrNo = RegSetValueEx(hKey, L"Shell", 0, REG_SZ,
			   (CONST BYTE *) wcDataBuffer,
			   (uiSize + 1) * sizeof(*wcDataBuffer));
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_RUN_SCRIPTS_SYNC))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"RunLogonScriptSync", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"RunLogonScriptSync", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"RunLogonScriptSync");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_DISABLE_LOCK_WKS))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"DisableLockWorkstation", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"DisableLockWorkstation", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"DisableLockWorkstation");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_ALLOW_MULTISESS))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"AllowMultipleTSSessions", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"AllowMultipleTSSessions", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"AllowMultipleTSSessions");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  switch (IsDlgButtonChecked(hWnd, IDC_CHK_AUTO_RESTART_SHELL))
    {
    case BST_CHECKED:
      lErrNo = RegSetValueEx(hKey, L"AutoRestartShell", 0, REG_DWORD,
			     (CONST BYTE *) &dwEnable, sizeof dwEnable);
      break;
    case BST_UNCHECKED:
      lErrNo = RegSetValueEx(hKey, L"AutoRestartShell", 0, REG_DWORD,
			     (CONST BYTE *) &dwDisable, sizeof dwDisable);
      break;
    default:
      lErrNo = RegDeleteValue(hKey, L"AutoRestartShell");
    }
  if ((lErrNo != NO_ERROR) & (lErrNo != ERROR_FILE_NOT_FOUND))
    lLastErrNo = lErrNo;

  SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
  SetCursor(LoadCursor(NULL, IDC_ARROW));

  if (lLastErrNo != NO_ERROR)
    {
      LPSTR errmsg = win_errmsgA(lLastErrNo);
      LPSTR msg;
      LPVOID lpArgs[] = { (LPVOID)(LONG_PTR)lLastErrNo, errmsg };
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		     FORMAT_MESSAGE_FROM_STRING |
		     FORMAT_MESSAGE_ARGUMENT_ARRAY,
		     "Not all settings were saved. Reasons could be "
		     "insufficent permissions or lost network connection.%n"
		     "The Windows error number is %1!u! with description:%n%2",
		     0, 0, (LPSTR) &msg, 0, (va_list*)lpArgs);
      LocalFree(errmsg);
      MessageBoxA(hWnd, msg, "Error saving settings", MB_ICONEXCLAMATION);
      LocalFree(msg);
      return false;
    }
  else
    {
      MessageBoxA(hWnd, "Settings were successfully saved.", "Settings saved",
		  MB_ICONINFORMATION);
      bSettingsChanged = false;
      return true;
    }
}

INT_PTR CALLBACK
SelectComputerDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
    {
    case WM_INITDIALOG:
      {
	PWCHAR_BUFFER result = (PWCHAR_BUFFER) lParam;
	SendDlgItemMessage(hWnd, IDC_EDIT_COMPUTER, EM_SETLIMITTEXT,
			   (WPARAM) result->MaxChars - 1, 0);
	SetDlgItemText(hWnd, IDC_EDIT_COMPUTER, result->Buffer);
	SetProp(hWnd, L"ResultPtr", (HANDLE) lParam);
	return TRUE;
      }

    case WM_CLOSE:
      EndDialog(hWnd, IDCANCEL);
      return TRUE;

    case WM_DESTROY:
      RemoveProp(hWnd, L"ResultPtr");
      return TRUE;

    case WM_COMMAND:
      if (GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT_COMPUTER)) == 0)
	EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
      else
	EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);

      switch (LOWORD(wParam))
	{
	case IDOK:
	  {
	    if (GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT_COMPUTER)) == 0)
	      return TRUE;

	    PWCHAR_BUFFER result = (PWCHAR_BUFFER) GetProp(hWnd, L"ResultPtr");
	    GetDlgItemText(hWnd, IDC_EDIT_COMPUTER, result->Buffer,
			   result->MaxChars);
	    EndDialog(hWnd, IDOK);
	    return TRUE;
	  }

	case IDCANCEL:
	  EndDialog(hWnd, IDCANCEL);
	  return TRUE;

	case IDC_BTN_BROWSE_COMPUTER:
	  {
	    WCHAR wcDirName[MAX_PATH] = L"";

	    switch (CoInitialize(NULL))
	      {
	      case S_OK: case S_FALSE:
		break;
	      default:
		MessageBoxA(hWnd, "Cannot initialize COM library.", "Error",
			    MB_ICONSTOP);
		return TRUE;
	      }

	    LPITEMIDLIST pidlRoot = NULL;
	    SHGetSpecialFolderLocation(hWnd, CSIDL_NETWORK, &pidlRoot);

	    BROWSEINFO bif = { 0 };
	    bif.hwndOwner = hWnd;
	    bif.pidlRoot = pidlRoot;
	    bif.pszDisplayName = wcDirName;
	    bif.lpszTitle = L"Select remote computer";
	    bif.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_EDITBOX;

	    LPMALLOC Malloc;
	    SHGetMalloc(&Malloc);

	    LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bif);
	    if (pItemIDList == NULL)
	      {
		Malloc->Free(pidlRoot);
		CoUninitialize();
		return TRUE;
	      }

	    Malloc->Free(pItemIDList);
	    Malloc->Free(pidlRoot);
	    CoUninitialize();

	    SetDlgItemText(hWnd, IDC_EDIT_COMPUTER, wcDirName);
	    SendDlgItemMessage(hWnd, IDC_EDIT_COMPUTER, EM_SETSEL, 0,
			       (LPARAM) (INT) -1);
	    SetFocus(GetDlgItem(hWnd, IDC_EDIT_COMPUTER));
	    return TRUE;
	  }
	}
    }

  return FALSE;
}

bool
AskComputerName(HWND hWnd, LPWSTR lpComputerName, INT iMaxChars)
{
  WCHAR_BUFFER buffer;
  buffer.Buffer = lpComputerName;
  buffer.MaxChars = iMaxChars;
  return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SEL_COMPUTER), hWnd,
			SelectComputerDlgProc, (LPARAM) &buffer) == IDOK;
}

bool
AskSaveSettings(HWND hWnd)
{
  if (!bSettingsChanged)
    return true;

  switch (MessageBoxA(hWnd, "Saved changed settings?", "Settings changed",
		      MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON3))
    {
    case IDYES:
      return SaveSettings(hWnd);
    case IDNO:
      return true;
    }

  return false;
}

INT_PTR CALLBACK
MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
    {
    case WM_HELP:
      {
	if (!WinHelp(hWnd, HELP_FILE, HELP_CONTEXTPOPUP,
		     ((LPHELPINFO) lParam)->iCtrlId))
	  {
	    LPSTR errmsg = win_errmsgA(GetLastError());
	    MessageBoxA(hWnd, errmsg, "No help available", MB_ICONEXCLAMATION);
	    LocalFree(errmsg);
	  }
	return TRUE;
      }

    case WM_INITDIALOG:
      SetClassLongPtr(hWnd, GCLP_HICON,
		      (LONG_PTR) LoadIcon(hInstance,
					  MAKEINTRESOURCE(IDI_APPICON)));
      SendDlgItemMessage(hWnd, IDC_EDIT_USERNAME, EM_LIMITTEXT, MAX_PATH - 1,
			 0);
      SendDlgItemMessage(hWnd, IDC_EDIT_PASSWORD, EM_LIMITTEXT, MAX_PATH - 1,
			 0);
      SendDlgItemMessage(hWnd, IDC_EDIT_DOMAIN, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SendDlgItemMessage(hWnd, IDC_EDIT_SHELL, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SendDlgItemMessage(hWnd, IDC_EDIT_WFP_LIMIT, EM_LIMITTEXT, 10, 0);

      LoadSettings(hWnd, NULL);

      return TRUE;

    case WM_CLOSE:
      if (AskSaveSettings(hWnd))
	EndDialog(hWnd, IDOK);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
	{
	case IDC_EDIT_USERNAME:
	case IDC_EDIT_PASSWORD:
	case IDC_EDIT_DOMAIN:
	case IDC_EDIT_USERINIT:
	case IDC_EDIT_SHELL:
	  if (HIWORD(wParam) == EN_CHANGE)
	    bSettingsChanged = true;

	  return TRUE;

	case IDC_CHK_RUN_SCRIPTS_SYNC:
	case IDC_CHK_ALLOW_MULTISESS:
	case IDC_CHK_DISABLE_LOCK_WKS:
	case IDC_CHK_AUTO_RESTART_SHELL:
	case IDC_CHK_ENABLE_SHUTDOWN:
	case IDC_CHK_SHOW_ADV:
	case IDC_CHK_CAD_REQUIRED:
	case IDC_CHK_DISABLE_LAST_USER:
	case IDC_CHK_ENABLE_AUTOLOG:
	case IDC_CHK_ENABLE_SHIFT:
	case IDC_CHK_FORCE_AUTOLOGON:
	case IDC_CHK_DISABLE_WFP:
	  if (HIWORD(wParam) == BN_CLICKED)
	    bSettingsChanged = true;

	  return TRUE;

	case IDC_EDIT_WFP_LIMIT:
	  switch (HIWORD(wParam))
	    {
	    case EN_CHANGE:
	      bSettingsChanged = true;
	      return TRUE;

	    case EN_KILLFOCUS:
	      if (GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT_WFP_LIMIT))
		  > 0)
		{
		  bool bOldSettingsChanged = bSettingsChanged;

		  WCHAR wcBuffer[11] = L"";
		  GetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, wcBuffer,
				 sizeof(wcBuffer)/sizeof(*wcBuffer));
		  DWORD dwValue = wcstoul(wcBuffer, NULL, 0);
		  if (dwValue == (DWORD) -1)
		    SetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, L"");
		  else
		    {
		      _snwprintf(wcBuffer, _countof(wcBuffer),
                          L"%u", dwValue);
		      SetDlgItemText(hWnd, IDC_EDIT_WFP_LIMIT, wcBuffer);
		    }
		  
		  bSettingsChanged = bOldSettingsChanged;
		}
	      return TRUE;
	    }
	  return TRUE;

	case CM_HELP_CONTENTS:
	  if (!WinHelp(hWnd, HELP_FILE, HELP_FINDER, 0))
	    {
	      LPSTR errmsg = win_errmsgA(GetLastError());
	      MessageBoxA(hWnd, errmsg, "No help available",
			  MB_ICONEXCLAMATION);
	      LocalFree(errmsg);
	    }
	  return TRUE;

	case CM_HELP_INTRO:
	  if (!WinHelp(hWnd, HELP_FILE, HELP_CONTEXT, IDH_INTRO))
	    {
	      LPSTR errmsg = win_errmsgA(GetLastError());
	      MessageBoxA(hWnd, errmsg, "No help available",
			  MB_ICONEXCLAMATION);
	      LocalFree(errmsg);
	    }
	  return TRUE;

	case CM_HELP_HELPONHELP:
	  if (!WinHelp(hWnd, NULL, HELP_HELPONHELP, 0))
	    {
	      LPSTR errmsg = win_errmsgA(GetLastError());
	      MessageBoxA(hWnd, errmsg, "No help available",
			  MB_ICONEXCLAMATION);
	      LocalFree(errmsg);
	    }
	  return TRUE;

	case CM_HELP_ABOUT:
	  MessageBoxA(hWnd,
		      "Winlogon Configuration Tool, version 0.0.2.\r\n"
		      "Copyright (C) 2005-2007 Olof Lagerkvist\r\n"
		      "http://www.ltr-data.se   olof@ltr-data.se\r\n"
		      "\r\n"
		      "This program is free software; you can redistribute "
		      "it and/or modify it.\r\n"
		      "\r\n"
		      "This program is distributed in the hope that it will "
		      "be useful, but WITHOUT ANY WARRANTY; without even the "
		      "implied warranty of MERCHANTABILITY or FITNESS FOR A "
		      "PARTICULAR PURPOSE.", "About", MB_ICONINFORMATION);
	  return TRUE;

	case CM_EDIT_PROT_PWD:
	  {
	    WCHAR wcPwdChr = (WCHAR)
	      SendDlgItemMessage(hWnd, IDC_EDIT_PASSWORD, EM_GETPASSWORDCHAR,
				 0, 0);

	    if (wcPwdChr == 0)
	      {
		CheckMenuItem(GetMenu(hWnd), CM_EDIT_PROT_PWD, MF_CHECKED);
		SendDlgItemMessage(hWnd, IDC_EDIT_PASSWORD, EM_SETPASSWORDCHAR,
				   (WPARAM) (UINT) L'*', 0);
	      }
	    else
	      {
		CheckMenuItem(GetMenu(hWnd), CM_EDIT_PROT_PWD, MF_UNCHECKED);
		SendDlgItemMessage(hWnd, IDC_EDIT_PASSWORD, EM_SETPASSWORDCHAR,
				   0, 0);
	      }

	    RedrawWindow(GetDlgItem(hWnd, IDC_EDIT_PASSWORD), NULL, NULL,
			 RDW_INVALIDATE);

	    return TRUE;
	  }

	case CM_FILE_OPENLOCAL:
	  if (AskSaveSettings(hWnd))
	    LoadSettings(hWnd, NULL);
	  return TRUE;

	case CM_FILE_OPENREMOTE:
	  if (AskSaveSettings(hWnd))
	    {
	      WCHAR ComputerName[MAX_PATH] = L"";
	      if (AskComputerName(hWnd, ComputerName,
				  sizeof(ComputerName)/sizeof(*ComputerName)))
		LoadSettings(hWnd, ComputerName);
	    }
	  return TRUE;

	case CM_FILE_SAVE:
	  SaveSettings(hWnd);
	  return TRUE;

	case CM_FILE_EXIT:
	  if (AskSaveSettings(hWnd))
	    EndDialog(hWnd, IDOK);
	  return TRUE;
	}
    }

  return FALSE;
}

int
startup()
{
  hInstance = GetModuleHandleA(NULL);
  if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainDlgProc)
      != IDOK)
    {
      MessageBoxA(NULL,
		  "This program requires Windows NT, 2000, XP or Server 2003.",
		  "Incompatible Windows version", MB_ICONSTOP | MB_TASKMODAL);
      ExitProcess(0);
    }

  ExitProcess(1);
}
