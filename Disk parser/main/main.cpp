#include <Windows.h>
#include <vector>
#include <string>
#include <strsafe.h>
#include <inttypes.h>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ShlObj.h>

#define NEW_WINDOW 0x1001
#define SCAN_PHYSICAL_DISK 0x1002
#define SCAN_VIRTUAL_VOLUME 0x1003
#define GO_TO_SECTOR 0x1004
#define PARSE_MFT_RECORD 0x1005
#define PARSE_INDX_RECORD 0x1006
#define CHANGE_READ_RATIO 0x1007
#define EXIT_APP 0x1008
#define ERROR_ALREADY_RUNNING 0x1009
#define ENTRY_START_INDEX 0x100a
#define GOTO_BUTTON_ID 0x100b
#define END_OF_MFT_RECORD_MARKER 0xffffffff

using namespace std;

typedef struct tagPHYSICAL_DISK {
	DWORD number;
	LARGE_INTEGER size;
	DISK_GEOMETRY geometry;
	DRIVE_LAYOUT_INFORMATION_EX* layout;
	HANDLE disk;
} PHYSICAL_DISK, * PPHYSICAL_DISK, * LPPHYSICAL_DISK;
typedef struct tagVOLUME_BOOT_SECTOR {
	BYTE jmp_opcode;
	WORD rel16_jmp_offset;
	WCHAR oem_id[0x8];
	LPBYTE bpb;
} VOLUME_BOOT_SECTOR, * PVOLUME_BOOT_SECTOR, * LPVOLUME_BOOT_SECTOR;
typedef struct tagVOLUME_INFO {
	DWORD cbSize;
	UINT disk_index;
	LPWSTR guid_path;
	LARGE_INTEGER offset, size;
	LPVOLUME_BOOT_SECTOR boot_sector;
} VOLUME_INFO, * PVOLUME_INFO, * LPVOLUME_INFO;

LRESULT __stdcall WndProc0x0(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall WndProc0x1(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall WndProc0x2(HWND, UINT, WPARAM, LPARAM);
DWORD __stdcall Init(INT, INT);
HRESULT __stdcall CreateWindowClass(LPCWSTR, LPCWSTR, LPCWSTR, SIZE_T, WNDPROC);
HANDLE __stdcall IsAlreadyRunning();
HRESULT __stdcall EnumerateMountedNTFSVolumes(vector<LPVOLUME_INFO>*);
HRESULT __stdcall GetPhysicalDisks(vector<PHYSICAL_DISK>*);
INT __stdcall RtlStartUserThreadEx(INT);
BOOLEAN __stdcall ReadRawDataFromDisk(PHYSICAL_DISK, LARGE_INTEGER, LPBYTE*, DWORD*);
void __stdcall EnableMenuItemEx(HWND, DWORD, BOOLEAN);
BOOLEAN __stdcall Parse0x0(HWND, WORD);
void __stdcall GetNTFSAttributeType(DWORD32, LPWSTR*);
void __stdcall ParseRunList(LPBYTE, vector<LPWSTR>*);
void __stdcall ParseRun(LPBYTE, LPBYTE*, LPBYTE*);
ULONGLONG __stdcall ByteArrayToHex(LPBYTE, BYTE);
void __stdcall ByteArrayToString(LPBYTE, SIZE_T, LPWSTR*);
void __stdcall Parse0x1(LPBYTE, vector<LPWSTR>*);
void __stdcall Parse0x2(LPBYTE, vector<LPWSTR>*);
DWORD __stdcall Parse0x3(LPBYTE, vector<LPWSTR>*);
BOOLEAN __stdcall IsRunningAsAdmin();
BOOLEAN __stdcall RegisterContextMenuHandler() {
	HKEY CLSID = NULL;
	DWORD create_dispo = 0x0;
	LSTATUS status = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0x0, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &CLSID, &create_dispo);
	if (status || create_dispo != REG_OPENED_EXISTING_KEY || !CLSID) return 0x0;
	HKEY DRIVE = NULL;
	status = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"DRIVE", 0x0, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &DRIVE, &create_dispo);
	if (status || create_dispo != REG_OPENED_EXISTING_KEY || !DRIVE) {
		RegCloseKey(CLSID);
		return 0x0;
	}
	HKEY InprocServer32 = NULL;
	status = RegCreateKeyExW(CLSID, L"{518f5e60-aac5-493a-8e9e-9afec4add54f}\\InprocServer32", 0x0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | DELETE, NULL, &InprocServer32, NULL);
	if (status || !InprocServer32) {
		RegCloseKey(CLSID);
		RegCloseKey(DRIVE);
		return 0x0;
	}

	LPSTR dll_path = (LPSTR)HeapAlloc(GetProcessHeap(), 0x8, MAX_PATH + 0x1);
	DWORD length = GetModuleFileNameA(NULL, dll_path, MAX_PATH + 0x1);
	StringCchCopyA(dll_path + length - 0x11, length - 0x11 + 0x22, "DiskAnalyserContextMenuHandler.dll");
	status = RegSetValueExA(InprocServer32, NULL, 0x0, REG_SZ, (const BYTE*)dll_path, length - 0x11 + 0x22);
	if (status) {
		RegCloseKey(InprocServer32);
		RegCloseKey(CLSID);
		RegCloseKey(DRIVE);
		return 0x0;
	}
	HeapFree(GetProcessHeap(), 0x0, dll_path);
	status = RegSetValueExA(InprocServer32, "ThreadingModel", 0x0, REG_SZ, (const BYTE*)"Apartment", 0x9);
	if (status) {
		RegCloseKey(InprocServer32);
		RegCloseKey(CLSID);
		RegCloseKey(DRIVE);
		return 0x0;
	}
	HKEY ContextMenuHandler = NULL;
	status = RegCreateKeyExW(DRIVE, L"shellex\\ContextMenuHandlers\\Disk Analyser", 0x0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | DELETE, NULL, &ContextMenuHandler, NULL);
	if (status || !ContextMenuHandler) {
		RegCloseKey(InprocServer32);
		RegCloseKey(CLSID);
		RegCloseKey(DRIVE);
		return 0x0;
	}
	status = RegSetValueExA(ContextMenuHandler, NULL, 0x0, REG_SZ, (const BYTE*)"{518f5e60-aac5-493a-8e9e-9afec4add54f}", 38);
	if (status) {
		RegCloseKey(ContextMenuHandler);
		RegCloseKey(InprocServer32);
		RegCloseKey(CLSID);
		RegCloseKey(DRIVE);
		return 0x0;
	}
	RegCloseKey(ContextMenuHandler);
	RegCloseKey(InprocServer32);
	RegCloseKey(CLSID);
	RegCloseKey(DRIVE);
	HMODULE shell32 = LoadLibraryExW(L"shell32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LPBYTE lpSHChangeNotify = (LPBYTE)GetProcAddress(shell32, "SHChangeNotify");
	if (lpSHChangeNotify) ((void(__stdcall*)(LONG, UINT, LPCVOID, LPCVOID))lpSHChangeNotify)(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	FreeLibrary(shell32);
	return 0x1;
}

SIZE_T threads_count = 0x0;
vector<LPVOLUME_INFO> volumes;
vector<PHYSICAL_DISK> disks;
HMENU menu_bar = NULL;
HACCEL accel = NULL;
CRITICAL_SECTION critical_section_info = { 0x0 };
UINT custom_message0x0 = 0x0;
LPBYTE lpStringFromCLSID = NULL;
LPBYTE lpCoTaskMemFree = NULL;

INT __stdcall wWinMain(HINSTANCE current_mod_base, HINSTANCE prev_mod_base, LPWSTR command_line, INT show_flag) {
	HANDLE exclusivity_mutex = IsAlreadyRunning();
	if (!exclusivity_mutex) return ERROR_ALREADY_RUNNING;

	if (!IsRunningAsAdmin()) {
		CloseHandle(exclusivity_mutex);
		return MessageBoxW(NULL, L"This program must be run as an admin", L"ERROR", MB_OKCANCEL | MB_ICONERROR);
	}

	RegisterContextMenuHandler();

	HMODULE shell32 = LoadLibraryExW(L"shell32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LPBYTE lpCommandLineToArgvW = (LPBYTE)GetProcAddress(shell32, "CommandLineToArgvW");
	if (!lpCommandLineToArgvW) {
		CloseHandle(exclusivity_mutex);
		return 0x0;
	}
	INT argc = 0x0;
	LPWSTR* argv = ((LPWSTR * (__stdcall*)(LPCWSTR, INT*))lpCommandLineToArgvW)(GetCommandLineW(), &argc);
	FreeLibrary(shell32);

	HRESULT rs = S_OK;

	if (argc > 0x2) {
		CloseHandle(exclusivity_mutex);
		LocalFree(argv);
		return MessageBoxW(NULL, L"COMMAND LINE ERROR : command line must contain only one argument", L"ERROR", MB_OKCANCEL | MB_ICONERROR);
	}
	rs = EnumerateMountedNTFSVolumes(&volumes);
	if (FAILED(rs)) {
		for (UINT i = 0x0; i < volumes.size(); i++) {
			for (UINT j = 0x0; j < ((volumes[i]->cbSize - sizeof VOLUME_INFO) / 0x8); j++)
				HeapFree(GetProcessHeap(), 0x0, *(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO + 0x8 * j));
			HeapFree(GetProcessHeap(), 0x0, volumes[i]);
		}
		CloseHandle(exclusivity_mutex);
		return HRESULT_CODE(rs);
	}

	rs = GetPhysicalDisks(&disks);
	if (FAILED(rs)) {
		for (UINT i = 0x0; i < volumes.size(); i++) {
			for (UINT j = 0x0; j < ((volumes[i]->cbSize - sizeof VOLUME_INFO) / 0x8); j++)
				HeapFree(GetProcessHeap(), 0x0, *(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO + 0x8 * j));
			HeapFree(GetProcessHeap(), 0x0, volumes[i]);
		}
		CloseHandle(exclusivity_mutex);
		return HRESULT_CODE(rs);
	}
	INT input_volume_index = -1;
	if (argc == 0x2) {
		for (UINT i = 0x0; i < volumes.size(); i++) {
			LPWSTR drive_letter = *(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO);
			if (!lstrcmpW(drive_letter, argv[0x1])) {
				input_volume_index = i;
				break;
			}
		}
		if (input_volume_index == -1) MessageBoxW(NULL, L"Volume not found !!", L"Warning", MB_OKCANCEL | MB_ICONWARNING);
	} 

	LocalFree(argv);
	InitializeCriticalSection(&critical_section_info);

	rs = CreateWindowClass(L"FileScannerWndClass0x0", IDI_APPLICATION, IDC_ARROW, 0x78, WndProc0x0);
	if (FAILED(rs)) {
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		return HRESULT_CODE(rs);
	}

	rs = CreateWindowClass(L"FileScannerWndClass0x1", IDI_APPLICATION, IDC_ARROW, 0x8, WndProc0x1);
	if (FAILED(rs)) {
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		return HRESULT_CODE(rs);
	}

	rs = CreateWindowClass(L"FileScannerWndClass0x2", IDI_APPLICATION, IDC_ARROW, 0x10, WndProc0x2);
	if (FAILED(rs)) {
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		return HRESULT_CODE(rs);
	}

	ACCEL* accel_info = (ACCEL*)HeapAlloc(GetProcessHeap(), 0x8, sizeof ACCEL * 0x8);

	accel_info[0x0].cmd = NEW_WINDOW;
	accel_info[0x0].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x0].key = 0x4e;

	accel_info[0x1].cmd = SCAN_VIRTUAL_VOLUME;
	accel_info[0x1].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x1].key = 0x42;

	accel_info[0x2].cmd = EXIT_APP;
	accel_info[0x2].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x2].key = 0x58;

	accel_info[0x3].cmd = SCAN_PHYSICAL_DISK;
	accel_info[0x3].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x3].key = 0x44;

	accel_info[0x4].cmd = GO_TO_SECTOR;
	accel_info[0x4].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x4].key = 0x47;

	accel_info[0x5].cmd = PARSE_MFT_RECORD;
	accel_info[0x5].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x5].key = 0x48;


	accel_info[0x6].cmd = PARSE_MFT_RECORD;
	accel_info[0x6].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x6].key = 0x49;

	accel_info[0x7].cmd = CHANGE_READ_RATIO;
	accel_info[0x7].fVirt = FVIRTKEY | FCONTROL;
	accel_info[0x7].key = 0x50;

	accel = CreateAcceleratorTableW(accel_info, 0x8);
	if (!accel) {
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		HeapFree(GetProcessHeap(), 0x0, accel_info);
		return GetLastError();
	}

	HeapFree(GetProcessHeap(), 0x0, accel_info);

	menu_bar = CreateMenu();
	if (!menu_bar) {
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
		DestroyAcceleratorTable(accel);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		return GetLastError();
	}

	MENUITEMINFOW super_item = { 0x0 };
	super_item.cbSize = sizeof MENUITEMINFOW;
	super_item.fMask = MIIM_STRING | MIIM_STATE | MIIM_SUBMENU;
	super_item.fState = MFS_ENABLED;
	super_item.cch = 0x4;
	super_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, super_item.cch * 0x2 + 0x2);
	super_item.hSubMenu = CreatePopupMenu();

	rs = StringCchCopyW(super_item.dwTypeData, 0x5, L"File");
	if (FAILED(rs)) {
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
		DestroyAcceleratorTable(accel);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		HeapFree(GetProcessHeap(), 0x0, super_item.dwTypeData);
		DestroyMenu(super_item.hSubMenu);
		DestroyMenu(menu_bar);
		return HRESULT_CODE(rs);
	}

	if (!InsertMenuItemW(menu_bar, 0x0, TRUE, &super_item)) {
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
		DestroyAcceleratorTable(accel);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		HeapFree(GetProcessHeap(), 0x0, super_item.dwTypeData);
		DestroyMenu(super_item.hSubMenu);
		DestroyMenu(menu_bar);
		return HRESULT_CODE(rs);
	}

	HeapFree(GetProcessHeap(), 0x0, super_item.dwTypeData);

	LPCWSTR item_strings[] = { L"New Window		Ctrl+N", L"Scan Physical Disk		Ctrl+B",  L"Scan NTFS Volume		Ctrl+D", L"Go to sector		Ctrl+G", L"Parse $MFT record		Ctrl+H", L"Parse INDX record		Ctrl+i", L"Change read ratio		Ctrl+J", L"Exit		Ctrl+X"};

	MENUITEMINFOW sub_item = { 0x0 };
	sub_item.cbSize = sizeof MENUITEMINFOW;
	sub_item.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID;
	for (UINT i = 0x0; i < 0x8; i++) {
		sub_item.cbSize = sizeof MENUITEMINFOW;
		sub_item.cch = lstrlenW(item_strings[i]);
		sub_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, sub_item.cch * 0x2 + 0x2);
		sub_item.wID = NEW_WINDOW + i;
		if (i == 0x3 || i == 0x4 || i == 0x5) sub_item.fState = MFS_DISABLED;
		else sub_item.fState = MFS_ENABLED;
		rs = StringCchCopyW(sub_item.dwTypeData, sub_item.cch + 0x1, item_strings[i]);
		if (FAILED(rs)) {
			UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
			UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
			UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
			DestroyAcceleratorTable(accel);
			DeleteCriticalSection(&critical_section_info);
			CloseHandle(exclusivity_mutex);
			HeapFree(GetProcessHeap(), 0x0, sub_item.dwTypeData);
			DestroyMenu(super_item.hSubMenu);
			DestroyMenu(menu_bar);
			return GetLastError();
		}
		if (!InsertMenuItemW(super_item.hSubMenu, i, TRUE, &sub_item)) {
			UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
			UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
			UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
			DestroyAcceleratorTable(accel);
			DeleteCriticalSection(&critical_section_info);
			CloseHandle(exclusivity_mutex);
			HeapFree(GetProcessHeap(), 0x0, sub_item.dwTypeData);
			DestroyMenu(super_item.hSubMenu);
			DestroyMenu(menu_bar);
			return GetLastError();
		}
		HeapFree(GetProcessHeap(), 0x0, sub_item.dwTypeData);
	}

	custom_message0x0 = RegisterWindowMessageW(L"FileScannerCustomMessage0x0");
	if (!custom_message0x0) {
		UnregisterClassW(L"FileScannerWndClass0x0", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x1", current_mod_base);
		UnregisterClassW(L"FileScannerWndClass0x2", current_mod_base);
		DestroyAcceleratorTable(accel);
		DeleteCriticalSection(&critical_section_info);
		CloseHandle(exclusivity_mutex);
		HeapFree(GetProcessHeap(), 0x0, sub_item.dwTypeData);
		DestroyMenu(super_item.hSubMenu);
		DestroyMenu(menu_bar);
		return GetLastError();
	}

	HMODULE ole32 = LoadLibraryW(L"ole32.dll");
	lpCoTaskMemFree = (LPBYTE)GetProcAddress(ole32, "CoTaskMemFree");
	lpStringFromCLSID = (LPBYTE)GetProcAddress(ole32, "StringFromCLSID");

	Init(show_flag, input_volume_index);

	while (TRUE) {
		EnterCriticalSection(&critical_section_info);
		if (!threads_count) break;
		LeaveCriticalSection(&critical_section_info);
	}

	DeleteCriticalSection(&critical_section_info);
	DestroyMenu(menu_bar);
	DestroyAcceleratorTable(accel);
	CloseHandle(exclusivity_mutex);
	FreeLibrary(ole32);

	return EXIT_SUCCESS;
}
HANDLE __stdcall IsAlreadyRunning() {
	HANDLE exclusivity_mutex = CreateMutexW(NULL, TRUE, L"FileScannerMutex0x0");
	if (!exclusivity_mutex) return NULL;
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(exclusivity_mutex);
		return NULL;
	}
	return exclusivity_mutex;
}
HRESULT __stdcall CreateWindowClass(LPCWSTR class_name, LPCWSTR icon_name, LPCWSTR cursor_name, SIZE_T extra_wnd_mem, WNDPROC MessageHandler) {
	WNDCLASSEXW wcex = { 0x0 };
	wcex.cbSize = sizeof WNDCLASSEXW;
	wcex.cbWndExtra = extra_wnd_mem;
	wcex.hInstance = GetModuleHandleW(NULL);
	wcex.hIcon = LoadIconW(NULL, icon_name);
	wcex.hCursor = LoadCursorW(NULL, cursor_name);
	wcex.hbrBackground = CreateSolidBrush(0xffffff);
	wcex.lpszClassName = class_name;
	wcex.lpfnWndProc = MessageHandler;
	return RegisterClassExW(&wcex) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}
DWORD __stdcall Init(INT show_flag, INT input_volume_index) {

	EnterCriticalSection(&critical_section_info);
	threads_count++;
	LeaveCriticalSection(&critical_section_info);

	POINT screen_size = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	HWND top_window = CreateWindowExW(0x0, L"FileScannerWndClass0x0", L"Disk Parser 1.0", WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, screen_size.x / 2 - 550,
		screen_size.y / 2 - 360, 1100, 720, NULL, menu_bar, GetModuleHandleW(NULL), NULL);
	if (!top_window) return HRESULT_FROM_WIN32(GetLastError());

	ShowWindow(top_window, show_flag);
	UpdateWindow(top_window);

	if (input_volume_index != -1) SendMessageW(top_window, custom_message0x0, 0x0, (LPARAM)input_volume_index);

	MSG message_info = { 0x0 };
	while (GetMessageW(&message_info, NULL, 0x0, 0x0)) {
		if (!TranslateAcceleratorW(message_info.hwnd, accel, &message_info)) {
			TranslateMessage(&message_info);
			DispatchMessageW(&message_info);
		}
	}

	EnterCriticalSection(&critical_section_info);
	threads_count--;
	LeaveCriticalSection(&critical_section_info);

	return message_info.wParam;
}
LRESULT __stdcall WndProc0x0(HWND window, UINT message, WPARAM msg_param0, LPARAM msg_param1) {
	if (InSendMessage()) ReplyMessage(0x0);
	if (message == WM_CREATE) {
		vector<LPWSTR>* texts = new vector<LPWSTR>();
		vector<pair<pair<DWORD, DWORD>, COLORREF>>* residents = new vector<pair<pair<DWORD, DWORD>, COLORREF>>();
		SetWindowLongPtrW(window, 0x68, (LONG_PTR)residents);
		SetWindowLongPtrW(window, 0x0, (LONG_PTR)texts);
		SetWindowLongPtrW(window, 0x30, 0x1);
		return 0x0;
	}
	if (message == WM_PAINT) {
		RECT client_area = { 0x0 };
		GetClientRect(window, &client_area);
		ValidateRect(window, &client_area);
		HDC dc = GetDC(window);
		LOGFONTW font_info = { 0x0 };
		font_info.lfWeight = 400;
		font_info.lfHeight = 14;
		CopyMemory(font_info.lfFaceName, L"Lucida Console", 0x1c);
		HFONT font = CreateFontIndirectW(&font_info);
		SelectObject(dc, font);
		HBRUSH brush = CreateSolidBrush(0xffffff);
		SelectObject(dc, brush);
		FillRect(dc, &client_area, brush);
		HPEN pen = CreatePen(PS_SOLID, 0x1, 0x0);
		SelectObject(dc, pen);
		Rectangle(dc, 0xa, 0xa, client_area.right / 0x2 - 0x5, client_area.bottom - 0xa);
		vector<LPWSTR> texts = *(vector<LPWSTR>*)GetWindowLongPtrW(window, 0x0);
		if (texts.size() > 0x0) {
			DWORD scroll_pos_y = (DWORD)GetWindowLongPtrW(window, 0x8);
			DWORD scroll_pos_x = (DWORD)GetWindowLongPtrW(window, 0x58);
			for (UINT i = scroll_pos_y; i < min(scroll_pos_y + ((client_area.bottom - 30) / 0x14), texts.size()); i++) {
				if (lstrcmpW(texts[i], L"0")) TextOutW(dc, 0x14, 0x14 + 0x14 * (i - scroll_pos_y), texts[i] + scroll_pos_x, min(((client_area.right / 0x2 - 0x14) / (font_info.lfHeight / 0x2 + 0x1)) - 0x2, lstrlenW(texts[i] + scroll_pos_x)));
				else {
					MoveToEx(dc, 0xa, 0x9 + 0x14 * (i + 1 - scroll_pos_y), NULL);
					LineTo(dc, client_area.right / 0x2 - 0x5, 0x9 + 0x14 * (i + 1 - scroll_pos_y));
				}
			}
		}
		Rectangle(dc, client_area.right / 0x2 + 0x5, 0xa, client_area.right - 0xa, client_area.bottom - 0xa);
		DWORD size = (DWORD)GetWindowLongPtrW(window, 0x20);
		if (size > 0x0) {
			LPBYTE raw_data = (LPBYTE)GetWindowLongPtrW(window, 0x18);
			LPPHYSICAL_DISK disk = (LPPHYSICAL_DISK)GetWindowLongPtrW(window, 0x28);
			vector<pair<pair<DWORD, DWORD>, COLORREF>>* resident_attrs = (vector<pair<pair<DWORD, DWORD>, COLORREF>>*)GetWindowLongPtrW(window, 0x68);
			DWORD scroll_pos = (DWORD)GetWindowLongPtrW(window, 0x10);
			LPSTR byte = (LPSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x3);
			UINT byte_index = scroll_pos * 0x10;
			BOOLEAN full_data = 0x0;
			UINT color = 255;
			DWORD64 coef = ceil((DOUBLE)(client_area.right - 0xc - (client_area.right / 0x2 + 0x8)) / 0x10);	
			for (UINT i = 0x0; i < 0x20; i++) {
				for (UINT j = 0x0; j < 0x10; j++) {
					BOOLEAN is_resident_content = 0x0;
					UINT k;
					for (k = 0x0; k < resident_attrs->size(); k++) if (byte_index >= resident_attrs->at(k).first.first && byte_index < (resident_attrs->at(k).first.first + resident_attrs->at(k).first.second)) {
						is_resident_content = 0x1;
						break;
					}
					if(!is_resident_content) SetTextColor(dc, 0x0);
					else SetTextColor(dc, resident_attrs->at(k).second);
					StringCchPrintfA(byte, 0x3, "%s%X", raw_data[byte_index] <= 0xf ? "0" : "", raw_data[byte_index]);
					TextOutA(dc, client_area.right / 0x2 + 0xe + coef * j, 0xd + ceil(((DOUBLE)(client_area.bottom - 0x1a) / 0x20)) * i, byte, 0x2);
					byte_index++;
					if (byte_index == size) {
						full_data = 0x1;
						break;
					}
					if (!(byte_index % disk->geometry.BytesPerSector)) {
						MoveToEx(dc, client_area.right / 0x2 + 0x5, 0xd + 17 + ceil(((DOUBLE)(client_area.bottom - 0x1a) / 0x20)) * i, NULL);
						LineTo(dc, client_area.right - 0xa, 0xd + 17 + ceil(((DOUBLE)(client_area.bottom - 0x1a) / 0x20)) * i);
					}
				}
				if (full_data) break;
			}
			HeapFree(GetProcessHeap(), 0x0, byte);
		}
		DeleteObject(pen);
		DeleteObject(brush);
		DeleteObject(font);
		ReleaseDC(window, dc);
		return 0x0;
	}
	if (message == WM_SYSCOMMAND) {
		LRESULT rv = DefWindowProcW(window, message, msg_param0, msg_param1);
		if (msg_param0 == SC_RESTORE || msg_param0 == SC_MAXIMIZE) {
			SetWindowLongPtrW(window, 0x60, msg_param0 == SC_RESTORE ? 0x0 : 0x1);
			if (msg_param0 == SC_RESTORE) {
				RECT client_area = { 0x0 };
				GetClientRect(window, &client_area);
				InvalidateRect(window, &client_area, FALSE);
				UpdateWindow(window);
			}
		}
		return rv;
	}
	if (message == WM_MOUSEWHEEL) {
	HANDLE_Y_SCROLL:
		signed short wheel_distance = HIWORD(msg_param0);
		POINT cur_pos = { 0x0 };
		GetCursorPos(&cur_pos);
		WINDOWPLACEMENT wnd_placement = { 0x0 };
		GetWindowPlacement(window, &wnd_placement);
		BYTE maximaized_flag = (BYTE)GetWindowLongPtrW(window, 0x60);
		if (!maximaized_flag) {
			cur_pos.x -= wnd_placement.rcNormalPosition.left;
			cur_pos.y -= wnd_placement.rcNormalPosition.top;
		}
		RECT client_area = { 0x0 };
		GetClientRect(window, &client_area);
		RECT left_rect = { 0x0 };
		left_rect.left = 0xa;
		left_rect.right = client_area.right / 0x2 - 0x5;
		left_rect.top = 0xa;
		left_rect.bottom = client_area.bottom - 0xa;
		RECT right_rect = { 0x0 };
		right_rect.left = client_area.right / 0x2 + 0x5;
		right_rect.right = client_area.right - 0xa;
		right_rect.top = 0xa;
		right_rect.bottom = client_area.bottom - 0xa;
		vector<LPWSTR> texts = *(vector<LPWSTR>*)GetWindowLongPtrW(window, 0x0);
		DWORD size = (DWORD)GetWindowLongPtrW(window, 0x20);
		if (PtInRect(&left_rect, cur_pos)) {
			DWORD scroll_pos = (DWORD)GetWindowLongPtrW(window, 0x8);
			if ((wheel_distance < 0x0 && (scroll_pos + ceil(((DOUBLE)(client_area.bottom - 0x1a)) / 0x14)) >= texts.size()) || (wheel_distance > 0x0 && scroll_pos == 0x0)) return 0x0;
			if (wheel_distance < 0x0) scroll_pos++;
			else scroll_pos--;
			if (scroll_pos < 0x0) return 0x0;
			SetWindowLongPtrW(window, 0x8, scroll_pos);
			InvalidateRect(window, &left_rect, FALSE);
		}
		else if (PtInRect(&right_rect, cur_pos)) {
			DWORD scroll_pos = (DWORD)GetWindowLongPtrW(window, 0x10);
			if ((wheel_distance < 0x0 && ((scroll_pos + ceil(((DOUBLE)(client_area.bottom - 0x1a) / 0x14))) * 0x10 >= size)) || (wheel_distance > 0x0 && scroll_pos == 0x0)) return 0x0;
			if (wheel_distance < 0x0) scroll_pos++;
			else scroll_pos--;
			if (scroll_pos < 0x0) return 0x0;
			SetWindowLongPtrW(window, 0x10, scroll_pos);
			InvalidateRect(window, &right_rect, FALSE);
		}
		UpdateWindow(window);
		return 0x0;
	}
	if (message == custom_message0x0) {
		RECT client_area = { 0x0 };
		GetClientRect(window, &client_area);
		InvalidateRect(window, &client_area, FALSE);
		UpdateWindow(window);
		SetWindowLongPtrW(window, 0x8, 0x0);
		SetWindowLongPtrW(window, 0x10, 0x0);
		UINT entry_index = (UINT)msg_param1;
		BOOLEAN is_disk = (BOOLEAN)msg_param0;
		PHYSICAL_DISK disk;
		LPPHYSICAL_DISK lpDisk = (LPPHYSICAL_DISK)GetWindowLongPtrW(window, 0x28);
		if (lpDisk) HeapFree(GetProcessHeap(), 0x0, lpDisk);
		lpDisk = (LPPHYSICAL_DISK)HeapAlloc(GetProcessHeap(), 0x8, sizeof PHYSICAL_DISK);
		if (is_disk) {
			disk = disks[entry_index];
			EnableMenuItemEx(window, PARSE_MFT_RECORD, 0x0);
			EnableMenuItemEx(window, GO_TO_SECTOR, 0x1);
			EnableMenuItemEx(window, PARSE_INDX_RECORD, 0x0);
		}
		else {
			disk = disks[volumes[entry_index]->disk_index];
			SetWindowLongPtrW(window, 0x50, (LONG_PTR)volumes[entry_index]);
			EnableMenuItemEx(window, PARSE_MFT_RECORD, 0x1);
			EnableMenuItemEx(window, GO_TO_SECTOR, 0x1);
			EnableMenuItemEx(window, PARSE_INDX_RECORD, 0x1);
		}
		*lpDisk = disk;
		SetWindowLongPtrW(window, 0x28, (LONG_PTR)lpDisk);
		LARGE_INTEGER offset;
		offset.QuadPart = is_disk ? 0x0 : volumes[entry_index]->offset.QuadPart;
		SetWindowLongPtrW(window, 0x38, offset.QuadPart / disk.geometry.BytesPerSector);
		DWORD size = disk.geometry.BytesPerSector * (DWORD)GetWindowLongPtrW(window, 0x30);
		vector<LPWSTR>* texts = (vector<LPWSTR>*)GetWindowLongPtrW(window, 0x0);
		for (UINT i = 0x0; i < texts->size(); i++) HeapFree(GetProcessHeap(), 0x0, texts->at(i));
		texts->clear();
		LPBYTE raw_data = (LPBYTE)GetWindowLongPtrW(window, 0x18);
		if (raw_data) HeapFree(GetProcessHeap(), 0x0, raw_data);
		if (is_disk) {
			LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Disk number : %d", disk.number);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Bytes per sector : %d", disk.geometry.BytesPerSector);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Sectors per track : %d", disk.geometry.SectorsPerTrack);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Tracks per cylinder : %d", disk.geometry.TracksPerCylinder);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Cylinders : %lld", disk.geometry.Cylinders.QuadPart);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Media : %X", disk.geometry.MediaType);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Total size (GB) : %.2f", (DOUBLE)disk.size.QuadPart / (1024 * 1024 * 1024));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Partition style : %s", disk.layout->PartitionStyle == PARTITION_STYLE_GPT ? L"GPT" : (disk.layout->PartitionStyle == PARTITION_STYLE_MBR ? L"MBR" : L"RAW"));
			texts->push_back(text);
			DWORD count = 0x0;
			for (UINT i = 0x0; i < volumes.size(); i++) if (volumes[i]->disk_index == entry_index) count += 0x1;
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"NTFS Volumes count : %d", count);
			texts->push_back(text);
			if (disk.layout->PartitionStyle == PARTITION_STYLE_GPT) {
				LPOLESTR str_guid = NULL;
				((HRESULT(__stdcall*)(REFCLSID, LPOLESTR*))lpStringFromCLSID)(disk.layout->Gpt.DiskId, &str_guid);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Disk ID : %s", str_guid);
				((void(__stdcall*)(LPVOID))lpCoTaskMemFree)(str_guid);
				texts->push_back(text);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Max partitions count : %d", disk.layout->Gpt.MaxPartitionCount);
				texts->push_back(text);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"First usable sector : %lld", disk.layout->Gpt.StartingUsableOffset.QuadPart / disk.geometry.BytesPerSector);
				texts->push_back(text);
			}
			else if (disk.layout->PartitionStyle == PARTITION_STYLE_MBR) {
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Signature : %X", disk.layout->Mbr.Signature);
				texts->push_back(text);
			}
			for (UINT i = 0x0; i < disk.layout->PartitionCount; i++) {
				if (!disk.layout->PartitionEntry[i].PartitionNumber) continue;
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
				CopyMemory(text, L"0", 0x2);
				texts->push_back(text);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Partiton %d : ", disk.layout->PartitionEntry[i].PartitionNumber);
				texts->push_back(text);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 120, L"Starting sector : %d", disk.layout->PartitionEntry[i].StartingOffset.QuadPart / disk.geometry.BytesPerSector);
				texts->push_back(text);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 120, L"Size (GB) : %.2f", (DOUBLE)disk.layout->PartitionEntry[i].PartitionLength.QuadPart / (1024 * 1024 * 1024));
				texts->push_back(text);
				if (disk.layout->PartitionStyle == PARTITION_STYLE_MBR) {
					LPOLESTR str_guid = NULL;
					((HRESULT(__stdcall*)(REFCLSID, LPOLESTR*))lpStringFromCLSID)(disk.layout->PartitionEntry[i].Mbr.PartitionId, &str_guid);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 120, L"ID : %s", str_guid);
					((void(__stdcall*)(LPVOID))lpCoTaskMemFree)(str_guid);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Bootable : %s", disk.layout->PartitionEntry[i].Mbr.BootIndicator ? L"Yes" : L"No");
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Recognized : %s", disk.layout->PartitionEntry[i].Mbr.RecognizedPartition ? L"Yes" : L"No");
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Hidden sectors : %d", disk.layout->PartitionEntry[i].Mbr.HiddenSectors);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Type : %X", disk.layout->PartitionEntry[i].Mbr.PartitionType);
					texts->push_back(text);
				}
				else if (disk.layout->PartitionStyle == PARTITION_STYLE_GPT) {
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 120, L"Name : %s", lstrlenW(disk.layout->PartitionEntry[i].Gpt.Name) ? disk.layout->PartitionEntry[i].Gpt.Name : L"????????");
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 120, L"Attributes : %X", disk.layout->PartitionEntry[i].Gpt.Attributes);
					texts->push_back(text);
				}
			}
		}
		else {
			LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Drive letter : %s", *(LPWSTR*)((LPBYTE)volumes[entry_index] + sizeof VOLUME_INFO));
			texts->push_back(text);
			for (UINT i = 0x1; i < ((volumes[entry_index]->cbSize - sizeof VOLUME_INFO - 0x1) / 0x8); i++) {
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Mounted Folder %d : %s", i + 1, *(LPWSTR*)((LPBYTE)volumes[entry_index] + sizeof VOLUME_INFO + 0x8 * i));
				texts->push_back(text);
			}
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"GUID path : %s", volumes[entry_index]->guid_path);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Phyiscal disk : PhysicalDrive%d", disks[volumes[entry_index]->disk_index].number);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Starting sector : %lld", volumes[entry_index]->offset.QuadPart / disks[volumes[entry_index]->disk_index].geometry.BytesPerSector);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Size (GB) : %.2f", (DOUBLE)volumes[entry_index]->size.QuadPart / (1024 * 1024 * 1024));
			texts->push_back(text);
		}
		RECT invalid_rect = { 0x0 };
		invalid_rect.left = 0xa;
		invalid_rect.right = client_area.right / 0x2 - 0x5;
		invalid_rect.top = 0xa;
		invalid_rect.bottom = client_area.bottom - 0xa;
		InvalidateRect(window, &invalid_rect, FALSE);
		UpdateWindow(window);

		if (!ReadRawDataFromDisk(disk, offset, &raw_data, &size)) return MessageBoxW(window, L"ReadRawDataFromDisk() failed", L"ERROR", MB_ICONERROR | MB_OKCANCEL);
		SetWindowLongPtrW(window, 0x18, (LONG_PTR)raw_data);
		SetWindowLongPtrW(window, 0x20, (LONG_PTR)size);

		invalid_rect.left = client_area.right / 0x2 + 0x5;
		invalid_rect.right = client_area.right - 0xa;
		invalid_rect.top = 0xa;
		invalid_rect.bottom = client_area.bottom - 0xa;
		InvalidateRect(window, &invalid_rect, FALSE);
		UpdateWindow(window);

		if (!is_disk) {
			if (!volumes[entry_index]->boot_sector) {
				volumes[entry_index]->boot_sector = (LPVOLUME_BOOT_SECTOR)HeapAlloc(GetProcessHeap(), 0x8, sizeof VOLUME_BOOT_SECTOR);
				volumes[entry_index]->boot_sector->jmp_opcode = (BYTE)raw_data[0x0];
				volumes[entry_index]->boot_sector->rel16_jmp_offset = *(WORD*)(raw_data + 0x1);
				MultiByteToWideChar(CP_UTF8, 0x0, (LPCCH)raw_data + 0x3, 0x8, volumes[entry_index]->boot_sector->oem_id, 0x8);
				volumes[entry_index]->boot_sector->bpb = (LPBYTE)HeapAlloc(GetProcessHeap(), 0x8, 0x4c);
				CopyMemory(volumes[entry_index]->boot_sector->bpb, raw_data + 0xb, 0x4c);
			}

			LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"X68 jmp offset : %d", volumes[entry_index]->boot_sector->rel16_jmp_offset);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"OEM ID : %s", volumes[entry_index]->boot_sector->oem_id);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
			CopyMemory(text, L"0", 0x2);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Serial number : %lld", *(DWORD64*)(volumes[entry_index]->boot_sector->bpb + 0x3d));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Media : %X", volumes[entry_index]->boot_sector->bpb[0xa]);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Bytes per sector : %d", *(WORD*)volumes[entry_index]->boot_sector->bpb);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Sectors per cluster : %d", volumes[entry_index]->boot_sector->bpb[0x2]);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Total sectors : %lld", *(DWORD64*)(volumes[entry_index]->boot_sector->bpb + 0x1d));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Reserved sectors : %d", *(WORD*)(volumes[entry_index]->boot_sector->bpb + 0x3));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"$Mft starting sector : %lld", (volumes[entry_index]->offset.QuadPart / disk.geometry.BytesPerSector) + *(DWORD64*)(volumes[entry_index]->boot_sector->bpb + 0x25) * volumes[entry_index]->boot_sector->bpb[0x2]);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"$MftMirr starting sector : %lld", (volumes[entry_index]->offset.QuadPart / disk.geometry.BytesPerSector) + *(DWORD64*)(volumes[entry_index]->boot_sector->bpb + 0x2d) * volumes[entry_index]->boot_sector->bpb[0x2]);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			DWORD size = volumes[entry_index]->boot_sector->bpb[0x35] <= 0x7f ? (volumes[entry_index]->boot_sector->bpb[0x35] * volumes[entry_index]->boot_sector->bpb[0x2] * *(WORD*)volumes[entry_index]->boot_sector->bpb) : pow(2, (volumes[entry_index]->boot_sector->bpb[0x35] - 0x1) ^ 0xff);
			StringCchPrintfW(text, 100, L"Sectors per MFT record : %d", size / (*(WORD*)volumes[entry_index]->boot_sector->bpb));
			texts->push_back(text);
			SetWindowLongPtrW(window, 0x40, size);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			size = volumes[entry_index]->boot_sector->bpb[0x39] <= 0x7f ? (volumes[entry_index]->boot_sector->bpb[0x39] * volumes[entry_index]->boot_sector->bpb[0x2] * *(WORD*)volumes[entry_index]->boot_sector->bpb) : pow(2, volumes[entry_index]->boot_sector->bpb[0x39] & 0x7f);
			StringCchPrintfW(text, 100, L"Sectors per INDX record : %d", size / (*(WORD*)volumes[entry_index]->boot_sector->bpb));
			texts->push_back(text);
			SetWindowLongPtrW(window, 0x48, size);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
			CopyMemory(text, L"0", 0x2);
			texts->push_back(text);

			invalid_rect.left = 0xa;
			invalid_rect.right = 495;
			invalid_rect.top = 0xa;
			invalid_rect.bottom = 650;
			InvalidateRect(window, &invalid_rect, FALSE);
			UpdateWindow(window);
		}
		return 0x0;
	}
	if (message == WM_COMMAND) {
		if (HIWORD(msg_param0) == 0x0 || HIWORD(msg_param0) == 0x1) {
			if (LOWORD(msg_param0) == NEW_WINDOW) return RtlStartUserThreadEx(SW_SHOWNORMAL);
			if (LOWORD(msg_param0) == EXIT_APP) {
				DestroyWindow(window);
				return 0x0;
			}
			else if (LOWORD(msg_param0) == SCAN_VIRTUAL_VOLUME || LOWORD(msg_param0) == SCAN_PHYSICAL_DISK) {
				WINDOWPLACEMENT pos = { 0x0 };
				GetWindowPlacement(window, &pos);
				EnableWindow(window, FALSE);
				DWORD window_type = LOWORD(msg_param0);
				DWORD height = 50 + (LOWORD(msg_param0) == SCAN_VIRTUAL_VOLUME ? volumes.size() * 50 : disks.size() * 50);
				HWND choose_window = CreateWindowExW(0x0, L"FileScannerWndClass0x1", L"", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
					pos.rcNormalPosition.left + (pos.rcNormalPosition.right - pos.rcNormalPosition.left) / 0x2 - 250,
					pos.rcNormalPosition.top + (pos.rcNormalPosition.bottom - pos.rcNormalPosition.top) / 0x2 - height / 0x2,
					500, height, window, NULL, GetModuleHandleW(NULL), &window_type);
				if (!choose_window) return MessageBoxW(0, L"CreateWindowExW() failed", 0, 0);
				SetWindowTextW(choose_window, LOWORD(msg_param0) == SCAN_VIRTUAL_VOLUME ? L"Scan NTFS Volume" : L"Scan Physical Disk");
				ShowWindow(choose_window, SW_SHOWNORMAL);
				UpdateWindow(choose_window);
			}
			else if (LOWORD(msg_param0) == GO_TO_SECTOR || LOWORD(msg_param0) == CHANGE_READ_RATIO) {
				WINDOWPLACEMENT pos = { 0x0 };
				GetWindowPlacement(window, &pos);
				EnableWindow(window, FALSE);
				DWORD window_type = LOWORD(msg_param0);
				HWND goto_window = CreateWindowExW(0x0, L"FileScannerWndClass0x2", L"", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
					pos.rcNormalPosition.left + ((pos.rcNormalPosition.right - pos.rcNormalPosition.left) / 0x2) - 100,
					pos.rcNormalPosition.top + ((pos.rcNormalPosition.bottom - pos.rcNormalPosition.top) / 0x2) - 55, 200, 110, window, (HMENU)0x0, GetModuleHandleW(NULL),
					&window_type);
				if (!goto_window) return MessageBoxW(0, L"CreateWindowExW() failed", 0, 0);
				SetWindowTextW(goto_window, LOWORD(msg_param0) == GO_TO_SECTOR ? L"Go To Sector" : L"Change Read Ratio");
				ShowWindow(goto_window, SW_SHOWNORMAL);
				UpdateWindow(goto_window);
			}
			else if (LOWORD(msg_param0) == PARSE_MFT_RECORD || LOWORD(msg_param0) == PARSE_INDX_RECORD) {
				if (!Parse0x0(window, LOWORD(msg_param0))) return MessageBoxW(window, LOWORD(msg_param0) == PARSE_MFT_RECORD ? L"Invalid MFT record" : L"Invalid INDX record", L"ERROR", MB_ICONERROR | MB_OKCANCEL);
				SetWindowLongPtrW(window, 0x8, 0x0);
				SetWindowLongPtrW(window, 0x10, 0x0);
				RECT client_area = { 0x0 };
				GetClientRect(window, &client_area);
				RECT invalid_rect = { 0x0 };
				invalid_rect.left = 0xa;
				invalid_rect.right = client_area.right / 0x2 - 0x5;
				invalid_rect.top = 0xa;
				invalid_rect.bottom = client_area.bottom - 0xa;
				InvalidateRect(window, &invalid_rect, FALSE);
				UpdateWindow(window);
				invalid_rect.left = client_area.right / 0x2 + 0x5;
				invalid_rect.right = client_area.right - 0xa;
				invalid_rect.top = 0xa;
				invalid_rect.bottom = client_area.bottom - 0xa;
				InvalidateRect(window, &invalid_rect, FALSE);
				UpdateWindow(window);
				return 0x0;
			}
		}
		return 0x0;
	}
	if (message == WM_KEYDOWN) {
		DWORD scroll_pos_x = (DWORD)GetWindowLongPtrW(window, 0x58);
		RECT client_area = { 0x0 };
		GetClientRect(window, &client_area);
		if (msg_param0 == VK_LEFT) {
			if (scroll_pos_x == 0x0) return 0x0;
			scroll_pos_x--;
			SetWindowLongPtrW(window, 0x58, scroll_pos_x);
			RECT invalid_rect = { 0x0 };
			invalid_rect.left = 0xa;
			invalid_rect.right = client_area.right / 0x2 - 0x5;
			invalid_rect.top = 0xa;
			invalid_rect.bottom = client_area.bottom - 0xa;
			InvalidateRect(window, &invalid_rect, FALSE);
			UpdateWindow(window);
		}
		else if (msg_param0 == VK_RIGHT) {
			vector<LPWSTR>* texts = (vector<LPWSTR>*)GetWindowLongPtrW(window, 0x0);
			DWORD max = 0x0;
			for (UINT i = 0x0; i < texts->size(); i++) if (lstrlenW(texts->at(i)) > max) max = lstrlenW(texts->at(i));
			if ((scroll_pos_x + ((client_area.right / 0x2 - 0x14) / 0x8) - 0x2) >= max) return 0x0;
			scroll_pos_x++;
			SetWindowLongPtrW(window, 0x58, scroll_pos_x);
			RECT invalid_rect = { 0x0 };
			invalid_rect.left = 0xa;
			invalid_rect.right = client_area.right / 0x2 - 0x5;
			invalid_rect.top = 0xa;
			invalid_rect.bottom = client_area.bottom - 0xa;
			InvalidateRect(window, &invalid_rect, FALSE);
			UpdateWindow(window);
		}
		else  if (msg_param0 == VK_DOWN || msg_param0 == VK_UP) {
			msg_param0 = MAKEWPARAM(0x0, msg_param0 == VK_UP ? 120 : -120);
			goto HANDLE_Y_SCROLL;
		}
		return 0x0;
	}
	if (message == WM_CLOSE) {
		DestroyWindow(window);
		return 0x0;
	}
	if (message == WM_DESTROY) {
		PostQuitMessage(EXIT_SUCCESS);
		return 0x0;
	}
	return DefWindowProcW(window, message, msg_param0, msg_param1);

}
HRESULT __stdcall EnumerateMountedNTFSVolumes(vector<LPVOLUME_INFO>* volumes) {
	LPWSTR volume_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	HANDLE volume_enumerator = FindFirstVolumeW(volume_name, 100);
	if (!volume_enumerator) {
		HeapFree(GetProcessHeap(), 0x0, volume_name);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	while (TRUE) {
		LPWSTR mount_points = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x1);
		DWORD required_chars_count = 0x0;
		GetVolumePathNamesForVolumeNameW(volume_name, mount_points, 0x1, &required_chars_count);
		mount_points = (LPWSTR)HeapReAlloc(GetProcessHeap(), 0x8, mount_points, required_chars_count * 0x2);
		if (!GetVolumePathNamesForVolumeNameW(volume_name, mount_points, required_chars_count, &required_chars_count) && GetLastError() != ERROR_MORE_DATA) {
			HeapFree(GetProcessHeap(), 0x0, mount_points);
			HeapFree(GetProcessHeap(), 0x0, volume_name);
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (lstrlenW(mount_points) != 0x0) {
			DWORD mount_points_count = 0x0;
			for (UINT i = 0x0; i < (required_chars_count - 0x1); i++) if (mount_points[i] == '\0') mount_points_count++;
			LPVOLUME_INFO volume_info = (LPVOLUME_INFO)HeapAlloc(GetProcessHeap(), 0x8, sizeof VOLUME_INFO + 0x8 * mount_points_count);
			volume_info->cbSize = sizeof VOLUME_INFO + 0x8 * mount_points_count;
			volume_info->guid_path = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, lstrlenW(volume_name) * 0x2 + 0x2);
			StringCchCopyW(volume_info->guid_path, lstrlenW(volume_name) + 0x1, volume_name);
			DWORD start = 0x0;
			DWORD index = 0x0;
			HRESULT rs;
			for (UINT i = 0x0; i < (required_chars_count - 0x1); i++) {
				if (mount_points[i] == '\0') {
					*(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index) = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, (i - start + 1) * 0x2);
					if (!*(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index)) {
						HeapFree(GetProcessHeap(), 0x0, mount_points);
						HeapFree(GetProcessHeap(), 0x0, volume_name);
						return HRESULT_FROM_WIN32(GetLastError());
					}
					rs = StringCchCopyW(*(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index), i - start + 1, mount_points + start);
					if (FAILED(rs)) {
						HeapFree(GetProcessHeap(), 0x0, *(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index));
						HeapFree(GetProcessHeap(), 0x0, mount_points);
						HeapFree(GetProcessHeap(), 0x0, volume_name);
						return rs;
					}
					if (start == 0x0) {
						LPWSTR fs_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, MAX_PATH * 0x2 + 0x2);
						if (!GetVolumeInformationW(*(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index), NULL, 0x0, NULL, 0x0, NULL, fs_name, MAX_PATH + 0x1)) {
							HeapFree(GetProcessHeap(), 0x0, fs_name);
							HeapFree(GetProcessHeap(), 0x0, *(LPWSTR*)((LPBYTE)volume_info + sizeof VOLUME_INFO + 0x8 * index));
							goto LOOP_END;
						}
						wstring lower_case_fs_name = fs_name;
						HeapFree(GetProcessHeap(), 0x0, fs_name);
						for (UINT j = 0x0; j < lower_case_fs_name.size(); j++) lower_case_fs_name[j] = tolower(lower_case_fs_name[j]);
						if (lstrcmpW(lower_case_fs_name.c_str(), L"ntfs")) goto LOOP_END;
					}
					start = i + 0x1;
					index++;
				}
			}
			volumes->push_back(volume_info);
		}
	LOOP_END:
		HeapFree(GetProcessHeap(), 0x0, mount_points);

		BOOL keep_enumrating = FindNextVolumeW(volume_enumerator, volume_name, 100);
		if (!keep_enumrating) {
			HeapFree(GetProcessHeap(), 0x0, volume_name);
			return GetLastError() == ERROR_NO_MORE_FILES ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	}
}
INT __stdcall RtlStartUserThreadEx(INT show_flag) {
	HANDLE thread = CreateThread(NULL, 0x0, NULL, NULL, CREATE_SUSPENDED, NULL);
	if (!thread) return 0x1;

	CONTEXT context = { 0x0 };
	context.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(thread, &context)) {
		CloseHandle(thread);
		return 0x1;
	}

	context.Rcx = show_flag;
	context.Rdx = (DWORD64)-1;
	context.Rsp -= 40;

	LPBYTE exiting_thread = (LPBYTE)VirtualAlloc(0x0, 15, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	BYTE shellcode[] = {
		0x48, 0x33, 0xC9, // xor rcx, rcx
		0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // mov rax, ExitThread
		0xFF, 0xD0, // call rax
	};
	*(DWORD64*)(shellcode + 0x5) = (DWORD64)ExitThread;

	WriteProcessMemory(GetCurrentProcess(), exiting_thread, shellcode, 15, NULL);

	context.Rsp -= 0x8;
	*(DWORD64*)(context.Rsp) = (DWORD64)exiting_thread;

	context.Rip = (DWORD64)Init;

	if (!SetThreadContext(thread, &context)) {
		CloseHandle(thread);
		return 0x1;
	}

	ResumeThread(thread);
	CloseHandle(thread);
	return 0x0;
}
HRESULT __stdcall GetPhysicalDisks(vector<PHYSICAL_DISK>* physical_disks) {
	if (!physical_disks) {
		SetLastError(87);
		return HRESULT_FROM_WIN32(87);
	}
	for (UINT i = 0x0; i < volumes.size(); i++) {
		LPWSTR symbolic_link = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, lstrlenW(*(LPCWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO)) * 0x2 + 0x8);
		if (FAILED(StringCchCopyW(symbolic_link, 0x5, L"\\\\.\\"))) {
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			continue;
		}
		CopyMemory(symbolic_link + 0x4, *(LPCWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO), (lstrlenW(*(LPCWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO)) - 0x1) * 0x2);
		HANDLE volume = CreateFileW(symbolic_link, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (volume == INVALID_HANDLE_VALUE) {
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			continue;
		}

		VOLUME_DISK_EXTENTS* disk_extents = (VOLUME_DISK_EXTENTS*)HeapAlloc(GetProcessHeap(), 0x8, sizeof VOLUME_DISK_EXTENTS);
		DWORD required_size = 0x0;
		if (!DeviceIoControl(volume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0x0, disk_extents, sizeof VOLUME_DISK_EXTENTS, &required_size, NULL)) {
			disk_extents = (VOLUME_DISK_EXTENTS*)HeapReAlloc(GetProcessHeap(), 0x8, disk_extents, sizeof VOLUME_DISK_EXTENTS + (disk_extents->NumberOfDiskExtents - 0x1) * sizeof DISK_EXTENT);
			if (!DeviceIoControl(volume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0x0, disk_extents, sizeof VOLUME_DISK_EXTENTS, &required_size, NULL)) {
				HeapFree(GetProcessHeap(), 0x0, disk_extents);
				HeapFree(GetProcessHeap(), 0x0, symbolic_link);
				CloseHandle(volume);
				continue;
			}
		}

		BOOLEAN is_already_found = 0x0;
		UINT j;
		for (j = 0x0; j < disks.size(); j++) if (disks[j].number == disk_extents->Extents[0x0].DiskNumber) {
			is_already_found = 0x1;
			break;
		}

		if (is_already_found) {
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			CloseHandle(volume);
			volumes[i]->offset = disk_extents->Extents[0x0].StartingOffset;
			volumes[i]->size = disk_extents->Extents[0x0].ExtentLength;
			volumes[i]->disk_index = j;
			HeapFree(GetProcessHeap(), 0x0, disk_extents);
			continue;
		}

		PHYSICAL_DISK disk = { 0x0 };
		disk.number = disk_extents->Extents[0x0].DiskNumber;


		HeapFree(GetProcessHeap(), 0x0, symbolic_link);
		symbolic_link = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 38);
		if (FAILED(StringCchCopyW(symbolic_link, 0x12, L"\\\\.\\PhysicalDrive"))) {
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			continue;
		}
		WCHAR disk_number_as_char[200];
		_itow_s(disk_extents->Extents[0x0].DiskNumber, disk_number_as_char, 0xa);
		CopyMemory(symbolic_link + 0x11, disk_number_as_char, lstrlenW(disk_number_as_char) * 0x2);
		volumes[i]->offset = disk_extents->Extents[0x0].StartingOffset;
		volumes[i]->size = disk_extents->Extents[0x0].ExtentLength;
		volumes[i]->disk_index = i;
		HANDLE disk_handle = CreateFileW(symbolic_link, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);

		if (disk_handle == INVALID_HANDLE_VALUE) {
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			HeapFree(GetProcessHeap(), 0x0, disk_extents);
			CloseHandle(volume);
			continue;
		}

		required_size = 0x0;
		disk.layout = (DRIVE_LAYOUT_INFORMATION_EX*)HeapAlloc(GetProcessHeap(), 0x8, sizeof DRIVE_LAYOUT_INFORMATION_EX + 127 * sizeof PARTITION_INFORMATION_EX);
		if (DeviceIoControl(disk_handle, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0x0, disk.layout, sizeof DRIVE_LAYOUT_INFORMATION_EX + 127 * sizeof PARTITION_INFORMATION_EX, &required_size, NULL)) {
			disk.layout = (DRIVE_LAYOUT_INFORMATION_EX*)HeapReAlloc(GetProcessHeap(), 0x8, disk.layout, required_size);
			if (!DeviceIoControl(disk_handle, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0x0, disk.layout, required_size, NULL, NULL)) {
				HeapFree(GetProcessHeap(), 0x0, disk.layout);
				CloseHandle(disk_handle);
				HeapFree(GetProcessHeap(), 0x0, symbolic_link);
				HeapFree(GetProcessHeap(), 0x0, disk_extents);
				CloseHandle(volume);
				continue;
			}
		}

		if (!DeviceIoControl(disk_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0x0, &disk.geometry, sizeof DISK_GEOMETRY, NULL, 0x0)) {
			HeapFree(GetProcessHeap(), 0x0, disk.layout);
			CloseHandle(disk_handle);
			HeapFree(GetProcessHeap(), 0x0, symbolic_link);
			HeapFree(GetProcessHeap(), 0x0, disk_extents);
			CloseHandle(volume);
			continue;
		}

		disk.size.QuadPart = disk.geometry.BytesPerSector * disk.geometry.SectorsPerTrack * disk.geometry.TracksPerCylinder * disk.geometry.Cylinders.QuadPart;

		disk.disk = disk_handle;
		HeapFree(GetProcessHeap(), 0x0, symbolic_link);
		HeapFree(GetProcessHeap(), 0x0, disk_extents);
		CloseHandle(volume);

		physical_disks->push_back(disk);
	}
	return S_OK;
}
LRESULT __stdcall WndProc0x1(HWND window, UINT message, WPARAM wp, LPARAM lp) {
	if (InSendMessage()) ReplyMessage(0x0);
	switch (message) {
		case WM_CREATE: {
			DWORD window_type = *(DWORD*)((LPCREATESTRUCTW)lp)->lpCreateParams;
			RECT client_area = { 0x0 };
			GetClientRect(window, &client_area);
			LOGFONTW font_info = { 0x0 };
			CopyMemory(font_info.lfFaceName, L"Lucida Console", 0x1c);
			font_info.lfWeight = 300;
			font_info.lfHeight = 14;
			HFONT font = CreateFontIndirectW(&font_info);
			if (window_type == SCAN_PHYSICAL_DISK) {
				for (UINT i = 0x0; i < disks.size(); i++) {
					WCHAR disk_number[200] = { 0x0 };
					_itow_s(disks[i].number, disk_number, 10);
					LPWSTR symbolic_link = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 28 + lstrlenW(disk_number) * 0x2);
					CopyMemory(symbolic_link, L"PhysicalDrive", 26);
					CopyMemory(symbolic_link + 13, disk_number, lstrlenW(disk_number) * 0x2);
					LPWSTR original_disk_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					QueryDosDeviceW(symbolic_link, original_disk_name, 100);
					HWND disk_entry = CreateWindowExW(0x0, L"button", original_disk_name, WS_CHILD | WS_BORDER, 0xa, 0xa + 50 * i, client_area.right - client_area.left - 20,
						40, window, (HMENU)(i + ENTRY_START_INDEX), GetModuleHandleW(NULL), NULL);
					SendMessageW(disk_entry, WM_SETFONT, (WPARAM)font, (LPARAM)TRUE);
					ShowWindow(disk_entry, SW_SHOWNORMAL);
					UpdateWindow(disk_entry);
					HeapFree(GetProcessHeap(), 0x0, symbolic_link);
					HeapFree(GetProcessHeap(), 0x0, original_disk_name);
				}
			}
			else if (window_type == SCAN_VIRTUAL_VOLUME) {
				for (UINT i = 0x0; i < volumes.size(); i++) {
					LPWSTR drive_letter = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, lstrlenW(*(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO)) * 0x2 + 0x2);
					CopyMemory(drive_letter, *(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO), (lstrlenW(*(LPWSTR*)((LPBYTE)volumes[i] + sizeof VOLUME_INFO)) - 0x1) * 0x2);
					LPWSTR original_volume_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					QueryDosDeviceW(drive_letter, original_volume_name, 100);
					HWND volume_entry = CreateWindowExW(0x0, L"button", original_volume_name, WS_CHILD | WS_BORDER, 0xa, 0xa + 50 * i, client_area.right - client_area.left - 20,
						40, window, (HMENU)(i + ENTRY_START_INDEX), GetModuleHandleW(NULL), NULL);
					SendMessageW(volume_entry, WM_SETFONT, (WPARAM)font, (LPARAM)TRUE);
					ShowWindow(volume_entry, SW_SHOWNORMAL);
					UpdateWindow(volume_entry);
					HeapFree(GetProcessHeap(), 0x0, drive_letter);
					HeapFree(GetProcessHeap(), 0x0, original_volume_name);
				}
			}
			SetWindowLongPtrW(window, 0x0, window_type == SCAN_VIRTUAL_VOLUME ? 0x0 : 0x1);
			return 0x0;
		}
		case WM_COMMAND: {
			if (HIWORD(wp) == BN_CLICKED || BN_DOUBLECLICKED) {
				HWND main_window = GetWindow(window, GW_OWNER);
				WPARAM _wp = (WPARAM)GetWindowLongPtrW(window, 0x0);
				EnableWindow(main_window, TRUE);
				SendMessageW(window, WM_CLOSE, 0x0, 0x0);
				SendMessageW(main_window, custom_message0x0, _wp, (LPARAM)(LOWORD(wp) - ENTRY_START_INDEX));
			}
			return 0x0;
		}
		case WM_CLOSE: {
			EnableWindow(GetWindow(window, GW_OWNER), TRUE);
			DestroyWindow(window);
			return 0x0;
		}
		default: return DefWindowProcW(window, message, wp, lp);
	}
}
BOOLEAN __stdcall ReadRawDataFromDisk(PHYSICAL_DISK disk, LARGE_INTEGER offset, LPBYTE* out_buffer, DWORD* size) {
	if (*size % disk.geometry.BytesPerSector != 0x0) *size = ((*size) / disk.geometry.BytesPerSector) + disk.geometry.BytesPerSector;
	*out_buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0x8, *size);
	if (!(*out_buffer)) return 0x0;
	if (SetFilePointer(disk.disk, offset.LowPart, &offset.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		HeapFree(GetProcessHeap(), 0x0, *out_buffer);
		return 0x0;
	}
	if (!ReadFile(disk.disk, *out_buffer, *size, NULL, NULL)) {
		HeapFree(GetProcessHeap(), 0x0, *out_buffer);
		return 0x0;
	}
	if (SetFilePointer(disk.disk, 0x0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		HeapFree(GetProcessHeap(), 0x0, *out_buffer);
		return 0x0;
	}
	return 0x1;
}
void __stdcall EnableMenuItemEx(HWND wnd, DWORD id, BOOLEAN bEnable) {
	HMENU menu_bar = GetMenu(wnd);
	if (!menu_bar) return;
	MENUITEMINFOW menu_item = { 0x0 };
	menu_item.cbSize = sizeof MENUITEMINFOW;
	menu_item.fMask = MIIM_SUBMENU;
	if (!GetMenuItemInfoW(menu_bar, 0x0, TRUE, &menu_item) || !menu_item.hSubMenu) return;
	HMENU sub_menu = menu_item.hSubMenu;
	menu_item.cbSize = sizeof MENUITEMINFOW;
	menu_item.fMask = MIIM_STATE;
	menu_item.fState = bEnable ? MFS_ENABLED : MFS_DISABLED;
	SetMenuItemInfoW(sub_menu, id, FALSE, &menu_item);
}
LRESULT __stdcall WndProc0x2(HWND window, UINT message, WPARAM msg_param0x0, LPARAM msg_param0x1) {
	if (InSendMessage()) ReplyMessage(0x0);
	if (message == WM_CREATE) {
		DWORD window_type = *(DWORD*)((LPCREATESTRUCTW)msg_param0x1)->lpCreateParams;
		LOGFONTW font_info = { 0x0 };
		CopyMemory(font_info.lfFaceName, L"Lucida Console", 0x1c);
		font_info.lfWeight = 400;
		font_info.lfHeight = 14;
		HFONT font = CreateFontIndirectW(&font_info);
		HWND main_window = GetWindow(window, GW_OWNER);
		ULONGLONG current_value = window_type == GO_TO_SECTOR ? (ULONGLONG)GetWindowLongPtrW(main_window, 0x38) : (ULONGLONG)GetWindowLongPtrW(main_window, 0x30);
		LPWSTR current_value_as_wstr = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x16);
		StringCchPrintfW(current_value_as_wstr, 0xb, L"%lld", current_value);
		HWND edit_control = CreateWindowExW(0x0, L"EDIT", current_value_as_wstr, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER | WS_BORDER, 0xa, 0xa, 163, 22, window, (HMENU)0x0, GetModuleHandleW(NULL), NULL);
		HeapFree(GetProcessHeap(), 0x0, current_value_as_wstr);
		SendMessageW(edit_control, WM_SETFONT, (WPARAM)font, (LPARAM)0x0);
		ShowWindow(edit_control, SW_SHOWNORMAL);
		UpdateWindow(edit_control);
		HWND button = CreateWindowExW(0x0, L"BUTTON", window_type == GO_TO_SECTOR ? L"Go >" : L"Change", WS_CHILD | BS_CENTER | BS_PUSHBUTTON | BS_NOTIFY, 0xa, 35, 163, 25, window, (HMENU)GOTO_BUTTON_ID, GetModuleHandleW(NULL), NULL);
		SendMessageW(button, WM_SETFONT, (WPARAM)font, (LPARAM)0x0);
		ShowWindow(button, SW_SHOWNORMAL);
		UpdateWindow(button);
		SetWindowLongPtrW(window, 0x0, window_type);
		SetWindowLongPtrW(window, 0x8, (LONG_PTR)edit_control);
		return 0x0;
	}
	else if (message == WM_COMMAND) {
		if ((HIWORD(msg_param0x0) == BN_CLICKED || HIWORD(msg_param0x0) == BN_DOUBLECLICKED) && LOWORD(msg_param0x0) == GOTO_BUTTON_ID) {
			HWND main_window = GetWindow(window, GW_OWNER);
			DWORD window_type = (DWORD)GetWindowLongPtrW(window, 0x0);
			HWND edit_control = (HWND)GetWindowLongPtrW(window, 0x8);
			LPPHYSICAL_DISK disk = (LPPHYSICAL_DISK)GetWindowLongPtrW(main_window, 0x28);
			DWORD length = GetWindowTextLengthW(edit_control);
			if (length == 0x0) return 0x0;
			LPWSTR input_text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, length * 0x2 + 0x2);
			GetWindowTextW(edit_control, input_text, length + 0x1);
			ULONGLONG input_value = _wtoi64(input_text);
			HeapFree(GetProcessHeap(), 0x0, input_text);
			if (window_type == CHANGE_READ_RATIO) {
				SetWindowLongPtrW(main_window, 0x30, input_value != 0x0 ? input_value : 0x1);
				EnableWindow(main_window, TRUE);
				DestroyWindow(window);
				return 0x0;
			}
			SetWindowLongPtrW(main_window, 0x38, input_value);
			LPBYTE raw_data = (LPBYTE)GetWindowLongPtrW(main_window, 0x18);
			if (raw_data) HeapFree(GetProcessHeap(), 0x0, raw_data);
			LARGE_INTEGER offset = { 0x0 };
			offset.QuadPart = input_value;
			offset.QuadPart *= disk->geometry.BytesPerSector;
			DWORD size = disk->geometry.BytesPerSector * (DWORD)GetWindowLongPtrW(main_window, 0x30);
			if (!ReadRawDataFromDisk(*disk, offset, &raw_data, &size)) return MessageBoxW(window, L"ReadRawDataFromDisk() failed", L"ERROR", MB_ICONERROR | MB_OKCANCEL);
			SetWindowLongPtrW(main_window, 0x18, (LONG_PTR)raw_data);
			SetWindowLongPtrW(main_window, 0x20, (LONG_PTR)size);
			SetWindowLongPtrW(main_window, 0x10, (LONG_PTR)0x0);
			EnableWindow(main_window, TRUE);
			DestroyWindow(window);
			RECT invalid_rect = { 0x0 };
			invalid_rect.left = 515;
			invalid_rect.right = 1075;
			invalid_rect.top = 0xa;
			invalid_rect.bottom = 650;
			InvalidateRect(main_window, &invalid_rect, FALSE);
			UpdateWindow(main_window);
		}
		return 0x0;
	}
	else if (message == WM_CLOSE) {
		EnableWindow(GetWindow(window, GW_OWNER), TRUE);
		DestroyWindow(window);
		return 0x0;
	}
	return DefWindowProcW(window, message, msg_param0x0, msg_param0x1);
}
void __stdcall GetNTFSAttributeType(DWORD32 attr_id, LPWSTR* attr_type) {
	if (!attr_type) {
		SetLastError(87);
		return;
	}
	LPCWSTR attr_types[] = { L"$STANDARD_INFORMATION", L"$ATTRIBUTE_LIST", L"$FILE_NAME", L"$OBJET_ID", L"$SECURITY_DESCRIPTOR", L"$VOLUME_NAME", L"$VOLUME_INFORATION",
		L"$DATA", L"$INDEX_ROOT", L"$INDEX_ALLOCATION", L"$BITMAP", L"$REPARSE_POINT", L"$EA_INFORMATION", L"$EA", L"", L"$LOGGED_UTILITY_STREAM" };
	*attr_type = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, lstrlenW(attr_types[attr_id / 0x10 - 0x1]) * 0x2 + 0x2);
	StringCchCopyW(*attr_type, lstrlenW(attr_types[attr_id / 0x10 - 0x1]) + 0x1, attr_types[attr_id / 0x10 - 0x1]);
}
void __stdcall ByteArrayToString(LPBYTE byte_array, SIZE_T cch, LPWSTR* out) {
	if (!out) {
		SetLastError(87);
		return;
	}
	*out = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, cch * 0x2 + 0x2);
	StringCchCopyW(*out, cch + 0x1, (LPWSTR)byte_array);
}
ULONGLONG __stdcall ByteArrayToHex(LPBYTE byte_array, BYTE array_size) {
	if (!byte_array || !array_size) {
		SetLastError(87);
		return NULL;
	}
	wstring output = L"";
	LPWSTR byte_as_str = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x6);
	for (signed int i = (array_size - 0x1); i >= 0x0; i--) {
		StringCchPrintfW(byte_as_str, 0x3, L"%s%X", byte_array[i] <= 0xf ? L"0" : L"", byte_array[i]);
		output += byte_as_str;
	}
	HeapFree(GetProcessHeap(), 0x0, byte_as_str);

	return stoul(output, nullptr, 0x10);
}
BOOLEAN __stdcall Parse0x0(HWND window, WORD record_type) {
	if (!window || (record_type != PARSE_MFT_RECORD && record_type != PARSE_INDX_RECORD)) {
		SetLastError(87);
		return 0x0;
	}
	vector<LPWSTR>* texts = (vector<LPWSTR>*)GetWindowLongPtrW(window, 0x0);
	for (UINT i = 0x0; i < texts->size(); i++) HeapFree(GetProcessHeap(), 0x0, texts->at(i));
	texts->clear();
	LPBYTE raw_data = (LPBYTE)GetWindowLongPtrW(window, 0x18);
	DWORD size = (DWORD)GetWindowLongPtrW(window, 0x20);
	DWORD mft_record_size = (DWORD)GetWindowLongPtrW(window, 0x40);
	DWORD indx_record_size = (DWORD)GetWindowLongPtrW(window, 0x48);
	LPVOLUME_INFO cw_volume = (LPVOLUME_INFO)GetWindowLongPtrW(window, 0x50);
	LPPHYSICAL_DISK cw_disk = (LPPHYSICAL_DISK)GetWindowLongPtrW(window, 0x28);
	if (size < mft_record_size) return 0x0;
	LPSTR signature = (LPSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x5);
	StringCchCopyA(signature, 0x5, (LPSTR)raw_data);
	if (lstrcmpA(signature, record_type == PARSE_MFT_RECORD ? "FILE" : "INDX")) {
		HeapFree(GetProcessHeap(), 0x0, signature);
		return 0x0;
	}
	HeapFree(GetProcessHeap(), 0x0, signature);
	for (UINT i = 0x1; i <= (*(WORD*)(raw_data + 0x6) - 0x1); i++) {
		CopyMemory(raw_data + cw_disk->geometry.BytesPerSector * i - 0x2, raw_data + (*(WORD*)(raw_data + 0x4)) + i * 0x2, 0x1);
		CopyMemory(raw_data + cw_disk->geometry.BytesPerSector * i - 0x1, raw_data + (*(WORD*)(raw_data + 0x4)) + i * 0x2 + 0x1, 0x1);
	}
	if (record_type == PARSE_INDX_RECORD) {
		LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Log file transaction number : %lld", *(DWORD64*)(raw_data + 0x8));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"VCN : %d", *(DWORD64*)(raw_data + 0x10));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		CopyMemory(text, L"0", 0x2);
		texts->push_back(text);
		Parse0x2(raw_data + 0x18, texts);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
		CopyMemory(text, L"0", 0x2);
		texts->push_back(text);
		return 0x1;
	}
	LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Record number : ( %d:%d )", *(WORD*)(raw_data + 0x2c), *(WORD*)(raw_data + 0x10));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Log file transaction number : %lld", *(LONGLONG*)(raw_data + 0x8));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Hard links : %d", *(WORD*)(raw_data + 0x12));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Header size (B) : %d", *(WORD*)(raw_data + 0x14));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Allocation state : %s", *(WORD*)(raw_data + 0x16) == 0x0 ? L"Deleted file" : (*(WORD*)(raw_data + 0x16) == 0x1 ? L"Allocated file" : (*(WORD*)(raw_data + 0x16) == 0x2 ? L"Deleted Directory" : (*(WORD*)(raw_data + 0x16) == 0x3 ? L"Allocated Directory" : L"??????"))));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Record actual size (B) : %d", *(DWORD32*)(raw_data + 0x18));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Record physical size (B) : %d", *(DWORD32*)(raw_data + 0x1c));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Base record number : %lld", *(LONGLONG*)(raw_data + 0x20));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Next attribute id : %d", *(WORD*)(raw_data + 0x28));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
	CopyMemory(text, L"0", 0x2);
	texts->push_back(text);
	for (DWORD32 i = *(WORD*)(raw_data + 0x14); *(DWORD32*)(raw_data + i) != END_OF_MFT_RECORD_MARKER; i += (*(DWORD32*)(raw_data + i + 0x4))) {
		LPWSTR attr_id = NULL;
		GetNTFSAttributeType(*(DWORD32*)(raw_data + i), &attr_id);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		LPWSTR name = NULL;
		ByteArrayToString(raw_data + i + *(WORD*)(raw_data + i + 0xa), *(raw_data + i + 0x9), &name);
		StringCchPrintfW(text, 100, L"Attribute : %s:%s", attr_id, *(raw_data + i + 0x9) == 0x0 ? L"Unnamed" : name);
		HeapFree(GetProcessHeap(), 0x0, name);
		HeapFree(GetProcessHeap(), 0x0, attr_id);
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Total size (B) : %d", *(DWORD32*)(raw_data + i + 0x4));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"type : % s", !(* (raw_data + i + 0x8)) ? L"Resident" : L"Non resident");
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Content state : %s", *(WORD*)(raw_data + i + 0xc) == 0x0 ? L"Normal" : (*(WORD*)(raw_data + i + 0xc) == 0x1 ? L"Compressed" : (*(WORD*)(raw_data + i + 0xc) == 0x40 ? L"Encrypted" : (*(WORD*)(raw_data + i + 0xc) == 0x80 ? L"Sparse" : L"??????"))));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"id : %d", *(WORD*)(raw_data + i + 0xe));
		texts->push_back(text);
		if (!(*(raw_data + i + 0x8))) {
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Content offset : %d", *(WORD*)(raw_data + i + 0x14));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Content size (B) : %d", *(DWORD32*)(raw_data + i + 0x10));
			texts->push_back(text);
			LPBYTE content = raw_data + i + *(WORD*)(raw_data + i + 0x14);
			switch (*(DWORD32*)(raw_data + i)) {
				case 0x10: {
					LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
					CopyMemory(text, L" ", 0x2);
					texts->push_back(text);
					SYSTEMTIME sys_time = { 0x0 };
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					LPFILETIME file_time = (LPFILETIME)content;
					FileTimeToSystemTime(file_time, &sys_time);
					StringCchPrintfW(text, 100, L"Creation timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay <= 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					file_time = (LPFILETIME)(content + 0x8);
					FileTimeToSystemTime(file_time, &sys_time);
					StringCchPrintfW(text, 100, L"Last modfied timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay <= 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					file_time = (LPFILETIME)(content + 0x10);
					FileTimeToSystemTime(file_time, &sys_time);
					StringCchPrintfW(text, 100, L"Record last modified timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay <= 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					file_time = (LPFILETIME)(content + 0x18);
					FileTimeToSystemTime(file_time, &sys_time);
					StringCchPrintfW(text, 100, L"Lastg accessed timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay <= 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Attributes : %X", *(DWORD32*)(content + 0x20));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Max versions : %d", *(DWORD32*)(content + 0x24));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Version : %d", *(DWORD32*)(content + 0x28));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Class ID : %d", *(DWORD32*)(content + 0x2c));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Owner ID : %d", *(DWORD32*)(content + 0x30));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Security Descriptior ID : %d", *(DWORD32*)(content + 0x34));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Quota charge : %lld", *(DWORD64*)(content + 0x38));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Last USN : %lld", *(DWORD64*)(content + 0x40));
					texts->push_back(text);
				} break;
				case 0x20: for (UINT i = 0x0; i < (*(DWORD32*)(raw_data + i + 0x10)); i += Parse0x3(content + i, texts)); break;
				case 0x30: {
					LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
					CopyMemory(text, L" ", 0x2);
					texts->push_back(text);
					Parse0x1(content, texts);
				}break;
				case 0x40: {
					LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
					CopyMemory(text, L" ", 0x2);
					texts->push_back(text);
					LPOLESTR str_guid = NULL;
					((HRESULT(__stdcall*)(REFCLSID, LPOLESTR*))lpStringFromCLSID)(*(GUID*)content, &str_guid);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Id : %s", str_guid);
					((void(__stdcall*)(LPVOID))lpCoTaskMemFree)(str_guid);
					texts->push_back(text);
				} break;
				case 0x90: {
					LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
					CopyMemory(text, L" ", 0x2);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"$INDEX ROOT header :");
					texts->push_back(text);
					LPWSTR attr_type = NULL;
					GetNTFSAttributeType(*(DWORD32*)content, &attr_type);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Attribute type : %s", attr_type);
					HeapFree(GetProcessHeap(), 0x0, attr_type);
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Sorting rule : %d", *(DWORD32*)(content + 0x4));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"INDX record size (B) : %d", *(DWORD32*)(content + 0x8));
					texts->push_back(text);
					text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
					StringCchPrintfW(text, 100, L"Clusters per INDX record : %d", content[0xc]);
					texts->push_back(text);
					Parse0x2(content + 0x10, texts);
				} break;
				default: {
					vector<pair<pair<DWORD, DWORD>, COLORREF>>* resident_attrs = (vector<pair<pair<DWORD, DWORD>, COLORREF>>*)GetWindowLongPtrW(window, 0x68);
					if (!resident_attrs) {
						resident_attrs = new vector<pair<pair<DWORD, DWORD>, COLORREF>>();
						SetWindowLongPtrW(window, 0x68, (LONG_PTR)resident_attrs);
					}

					resident_attrs->push_back(make_pair(make_pair(i + *(WORD*)(raw_data + i + 0x14), *(DWORD32*)(raw_data + i + 0x10)), 0xff));
				}
			}
		}
		else {
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"First VCN : %lld", *(LONGLONG*)(raw_data + i + 0x10));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Last VCN : %lld", *(LONGLONG*)(raw_data + i + 0x18));
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Phyiscal size (KB) : %.2f", (DOUBLE)(*(DWORD64*)(raw_data + i + 0x28)) / 1024);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Actual size (KB) : %.2f", (DOUBLE)(*(DWORD64*)(raw_data + i + 0x30)) / 1024);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchPrintfW(text, 100, L"Initialized size (KB) : %.2f", (DOUBLE)(*(DWORD64*)(raw_data + i + 0x38)) / 1024);
			texts->push_back(text);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			StringCchCopyW(text, 100, L"Run list : ");
			texts->push_back(text);
			LPBYTE runlist = raw_data + i + *(WORD*)(raw_data + i + 0x20);
			text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
			LARGE_INTEGER sector = { 0x0 };
			sector.QuadPart = (cw_volume->offset.QuadPart / cw_disk->geometry.BytesPerSector);
			UINT run = 0x1;
			for (UINT j = 0x0; runlist[j] != 0x0; j += (((runlist[j] & 0xf0) >> 0x4) + (runlist[j] & 0xf) + 0x1)) {
				sector.QuadPart += (ByteArrayToHex(runlist + j + 0x1 + (runlist[j] & 0xf), (runlist[j] & 0xf0) >> 0x4) * cw_volume->boot_sector->bpb[0x2]);
				text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
				StringCchPrintfW(text, 100, L"Run %d : { sector : %lld, size (KB) : %.2f }", run, sector.QuadPart ,
					(DOUBLE)(ByteArrayToHex(runlist + j + 0x1, runlist[j] & 0xf) * cw_volume->boot_sector->bpb[0x2] * (*(WORD*)cw_volume->boot_sector->bpb)) / 1024);
				texts->push_back(text);
				run++;
			}
		}
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
		CopyMemory(text, L"0", 0x2);
		texts->push_back(text);
	}
	return 0x1;
}
void __stdcall Parse0x1(LPBYTE raw_data, vector<LPWSTR>* texts) {
	LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 400);
	LPWSTR file_name = NULL;
	ByteArrayToString(raw_data + 0x42, *(raw_data + 0x40), &file_name);
	StringCchPrintfW(text, 200, L"File name : %s", file_name);
	HeapFree(GetProcessHeap(), 0x0, file_name);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Parent directory : ( %d:%d )", ByteArrayToHex(raw_data, 0x6), *(WORD*)(raw_data + 0x6));
	texts->push_back(text);
	SYSTEMTIME sys_time = { 0x0 };
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	LPFILETIME file_time = (LPFILETIME)(raw_data + 0x8);
	FileTimeToSystemTime(file_time, &sys_time);
	StringCchPrintfW(text, 100, L"Creation timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay < 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	file_time = (LPFILETIME)(raw_data + 0x10);
	FileTimeToSystemTime(file_time, &sys_time);
	StringCchPrintfW(text, 100, L"Last modifed timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay < 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	file_time = (LPFILETIME)(raw_data + 0x18);
	FileTimeToSystemTime(file_time, &sys_time);
	StringCchPrintfW(text, 100, L"Record last modifed timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay < 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	file_time = (LPFILETIME)(raw_data + 0x20);
	FileTimeToSystemTime(file_time, &sys_time);
	StringCchPrintfW(text, 100, L"Last accessed timestamp : %s%d/%s%d/%d %s%d:%s%d:%s%d", sys_time.wDay < 10 ? L"0" : L"", sys_time.wDay, sys_time.wMonth <= 10 ? L"0" : L"", sys_time.wMonth, sys_time.wYear, sys_time.wHour <= 10 ? L"0" : L"", sys_time.wHour, sys_time.wMinute <= 10 ? L"0" : L"", sys_time.wMinute, sys_time.wSecond <= 10 ? L"0" : L"", sys_time.wSecond);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Physical size (KB) : %.2f", (DOUBLE)(*(DWORD64*)(raw_data + 0x28)) / 1024);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Actual size (KB) : %.2f", (DOUBLE)(*(DWORD64*)(raw_data + 0x30)) / 1024);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Attributes : %d", *(DWORD32*)(raw_data + 0x38));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Extended attributes : %d", *(DWORD32*)(raw_data + 0x3c));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"File name namespace : %s", *(raw_data + 0x41) == 0x0 ? L"Posix" : (*(raw_data + 0x41) == 0x1 ? L"Win32" : (*(raw_data + 0x41) == 0x2 ? L"Dos" : *(raw_data + 0x41) == 0x3 ? L"Win32 & Dos" : L"??????")));
	texts->push_back(text);
}
void __stdcall Parse0x2(LPBYTE raw_data, vector<LPWSTR>*texts) {
	LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
	CopyMemory(text, L" ", 0x2);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"NODE header :");
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Total size (B) : %d", (*(DWORD32*)(raw_data + 0x08)));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Content offset : %d", *(DWORD32*)raw_data);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Content size (B) : %d", (*(DWORD32*)(raw_data + 0x04)) - (*(DWORD32*)raw_data));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Flags : %d", *(DWORD32*)(raw_data + 0x0C));
	texts->push_back(text);
	LPBYTE entries = raw_data + (*(DWORD32*)raw_data);
	DWORD j = 0x0;
	UINT entry_index = 0x1;
	while ((j + *(WORD*)(entries + j + 0x8)) <= ((*(DWORD32*)(raw_data + 0x04)) - *(DWORD32*)raw_data) && (*(WORD*)(entries + j + 0x8))) {
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
		CopyMemory(text, L"0", 0x2);
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Entry %d :", entry_index);
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"File record : ( %d:%d )", ByteArrayToHex(entries + j, 0x6), ByteArrayToHex(entries + j + 0x6, 0x2));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Total size (B) : %d", *(WORD*)(entries + j + 0x8));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Content size (B) : %d", *(WORD*)(entries + j + 0xa));
		texts->push_back(text);
		text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
		StringCchPrintfW(text, 100, L"Flags : %d", *(DWORD32*)(entries + j + 0xc));
		texts->push_back(text);
		if (*(WORD*)(entries + j + 0xa)) Parse0x1(entries + j + 0x10, texts);
		entry_index++;
		j += (*(WORD*)(entries + j + 0x8));
	}
}
DWORD __stdcall Parse0x3(LPBYTE raw_data, vector<LPWSTR>* texts) {
	LPWSTR text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x4);
	CopyMemory(text, L" ", 0x2);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	LPWSTR attr_type = NULL;
	GetNTFSAttributeType(*(DWORD32*)raw_data, &attr_type);
	StringCchPrintfW(text, 100, L"Attribute type : %s", attr_type);
	HeapFree(GetProcessHeap(), 0x0, attr_type);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Total size (KB) : %.2f", (FLOAT)(*(WORD*)(raw_data + 0x4)) / 1024);
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Attribute name : %s", *(raw_data + 0x6) ? (LPWSTR)(raw_data + *(raw_data + 0x7)): L"Unnamed");
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Starting VCN : %lld", *(LONGLONG*)(raw_data + 0x8));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Base record : %lld : %d", ByteArrayToHex(raw_data + 0x10, 0x6), ByteArrayToHex(raw_data + 0x16, 0x2));
	texts->push_back(text);
	text = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, 200);
	StringCchPrintfW(text, 100, L"Attribute ID : %d", *(WORD*)(raw_data + 0x12));
	texts->push_back(text);
	return (*(WORD*)(raw_data + 0x4));
}
BOOLEAN __stdcall IsRunningAsAdmin() {
	HANDLE access_token = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &access_token)) return 0x0;
	DWORD required_size = 0x0;
	GetTokenInformation(access_token, TokenPrivileges, NULL, 0x0, &required_size);
	PTOKEN_PRIVILEGES privileges = (PTOKEN_PRIVILEGES)HeapAlloc(GetProcessHeap(), 0x8, required_size);
	if (!GetTokenInformation(access_token, TokenPrivileges, privileges, required_size, &required_size)) {
		HeapFree(GetProcessHeap(), 0x0, privileges);
		CloseHandle(access_token);
		return 0x0;
	}
	BOOLEAN is_admin = 0x0;
	LPWSTR privilege_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8,200);
	for (UINT i = 0x0; i < privileges->PrivilegeCount; i++) {
		DWORD cch = 101;
		if (!LookupPrivilegeNameW(NULL, &privileges->Privileges[i].Luid, privilege_name, &cch)) continue;
		if (!lstrcmpW(privilege_name, SE_SECURITY_NAME)) {
			is_admin = 0x1;
			break;
		}
	}
	HeapFree(GetProcessHeap(), 0x0, privilege_name);
	HeapFree(GetProcessHeap(), 0x0, privileges);
	CloseHandle(access_token);
	return is_admin;
}