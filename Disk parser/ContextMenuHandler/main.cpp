#include "pch.h"

HRESULT __stdcall DllGetClassObject(REFCLSID, REFIID, LPVOID*);
HRESULT __stdcall DllCanUnloadNow();

ULONG dll_ref_count = 0x0;

class ContextMenuHandler : public IShellExtInit, public IContextMenu {
private:
	ULONG ref_count;
	vector<LPWSTR> target_items;
public:
	ContextMenuHandler() {
		InterlockedIncrement(&dll_ref_count);
		ref_count = 0x0;
	}
	~ContextMenuHandler() {
		InterlockedDecrement(&dll_ref_count);
	}

	ULONG __stdcall AddRef() {
		InterlockedIncrement(&ref_count);
		return ref_count;
	}
	ULONG __stdcall Release() {
		InterlockedDecrement(&ref_count);
		if (!ref_count) {
			delete this;
			return 0x0;
		}
		return ref_count;
	}
	HRESULT __stdcall QueryInterface(REFIID interface_id, LPVOID* interface_buffer) {
		if (!interface_buffer) return E_INVALIDARG;
		if (interface_id == IID_IShellExtInit) {
			*interface_buffer = (IShellExtInit*)this;
			AddRef();
			return S_OK;
		}
		if (interface_id == IID_IContextMenu) {
			*interface_buffer = (IContextMenu*)this;
			AddRef();
			return S_OK;
		}
		*interface_buffer = NULL;
		return E_NOINTERFACE;
	}

	HRESULT __stdcall Initialize(LPCITEMIDLIST folder_idl, IDataObject* data_object, HKEY prog_id) {
		FORMATETC shell_format = { 0x0 };
		shell_format.cfFormat = CF_HDROP;
		shell_format.dwAspect = DVASPECT::DVASPECT_CONTENT;
		shell_format.lindex = -1;
		shell_format.tymed = TYMED_HGLOBAL;
		STGMEDIUM transfer_medium = { 0x0 };
		data_object->AddRef();
		if (FAILED(data_object->GetData(&shell_format, &transfer_medium)) || !transfer_medium.hGlobal) return E_UNEXPECTED;

		UINT items_count = DragQueryFileW((HDROP)transfer_medium.hGlobal, 0xffffffff, NULL, 0x0);
		for (UINT i = 0x0; i < items_count; i++) {
			LPWSTR item_name = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, MAX_PATH * 0x2 + 0x2);
			target_items.push_back(item_name);
			DragQueryFileW((HDROP)transfer_medium.hGlobal, i, item_name, MAX_PATH + 0x1);
		}

		ReleaseStgMedium(&transfer_medium);
		data_object->Release();
		return S_OK;
	}

	HRESULT __stdcall GetCommandString(UINT_PTR command, UINT type, UINT * reserved, CHAR* out_string, UINT cch_max) {
		if (!out_string || !cch_max) return E_INVALIDARG;
		if (type == GCS_VALIDATEA || type == GCS_VALIDATEW) return command == 0x0 ? S_OK : S_FALSE;
		ZeroMemory(out_string, 0x6);
		switch (type) {
			case GCS_VERBA: StringCchCopyA(out_string, 0x8, "command"); break;
			case GCS_VERBW: StringCchCopyW((LPWSTR)out_string, 0x8, L"command"); break;
			case GCS_HELPTEXTA: StringCchCopyA(out_string, 0x8, "command"); break;
			case GCS_HELPTEXTW: StringCchCopyW((LPWSTR)out_string, 0x8, L"command"); break;
		}
		return S_OK;
	}
	HRESULT __stdcall InvokeCommand(CMINVOKECOMMANDINFO* command_context) {
		BOOLEAN is_supported = 0x0;
		if (IS_INTRESOURCE(command_context->lpVerb)) {
			LPSTR verb = (LPSTR)HeapAlloc(GetProcessHeap(), 0x8, 0x6);
			if (FAILED(GetCommandString(0x0, GCS_VERBA, NULL, verb, 0x5))) {
				HeapFree(GetProcessHeap(), 0x0, verb);
				return E_FAIL;
			}
			is_supported = (!lstrcmpA(verb, "command"));
			HeapFree(GetProcessHeap(), 0x0, verb);
		}
		else is_supported = (!lstrcmpA(command_context->lpVerb, "command"));
		if (is_supported) {
			for (UINT i = 0x0; i < target_items.size(); i++) MessageBox(0, target_items[i], 0, 0);
			return S_OK;
		}
		return E_FAIL;
	}
	HRESULT __stdcall QueryContextMenu(HMENU context_menu, UINT item_position, UINT min_id, UINT max_id, UINT flags) {
		if (target_items.size() == 0x1) {
			if (!(flags & (CMF_EXTENDEDVERBS | CMF_DISABLEDVERBS))) { // adding normal commands only , no disabled or special commands ( displayed when the user right click the item with pressing the shift key )
				MENUITEMINFOW menu_item = { 0x0 };
				menu_item.cbSize = sizeof MENUITEMINFOW;
				menu_item.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
				menu_item.fState = MFS_ENABLED;
				menu_item.cch = 0x7;
				menu_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), 0x8, menu_item.cch * 0x2 + 0x2);
				StringCchCopyW(menu_item.dwTypeData, menu_item.cch + 0x1, L"Command");
				menu_item.wID = min_id;
				if (!InsertMenuItemW(context_menu, item_position, TRUE, &menu_item)) { // Inserting the menu item in the suitable position
					HeapFree(GetProcessHeap(), 0x0, menu_item.dwTypeData);
					return MAKE_HRESULT(0x0, 0x0, 0x0);
				}
				HeapFree(GetProcessHeap(), 0x0, menu_item.dwTypeData);
				return MAKE_HRESULT(0x0, 0x0, 0x1);
			}
			return MAKE_HRESULT(0x0, 0x0, 0x0);
		}
		return MAKE_HRESULT(0x0, 0x0, 0x0);
	}
};
class ClassFactory : public IClassFactory {
private:
	ULONG ref_count;
	BOOL locked;
public:
	ClassFactory() {
		InterlockedIncrement(&dll_ref_count);
		ref_count = 0x0;
		locked = TRUE;
	}
	~ClassFactory() {
		InterlockedDecrement(&dll_ref_count);
	}

	ULONG __stdcall AddRef() {
		InterlockedIncrement(&ref_count);
		return ref_count;
	}
	ULONG __stdcall Release() {
		InterlockedDecrement(&ref_count);
		if (ref_count == 0x0 && !locked) {
			delete this;
			return 0x0;
		}
		return ref_count;
	}
	HRESULT __stdcall QueryInterface(REFIID interface_id, LPVOID* interface_buffer) {
		if (!interface_buffer) return E_INVALIDARG;
		if (interface_id == IID_IUnknown) {
			*interface_buffer = (IUnknown*)this;
			AddRef();
			return S_OK;
		}
		if (interface_id == IID_IClassFactory) {
			*interface_buffer = (IClassFactory*)this;
			AddRef();
			return S_OK;
		}
		*interface_buffer = NULL;
		return E_NOINTERFACE;
	}

	HRESULT __stdcall LockServer(BOOL lock) {
		InterlockedExchange((ULONGLONG*)&locked, lock);
		return S_OK;
	}
	HRESULT __stdcall CreateInstance(IUnknown* OuterIUnknown, REFIID interface_id, LPVOID* interface_buffer) {
		if (!interface_buffer) return E_INVALIDARG;
		if (OuterIUnknown) {
			*interface_buffer = NULL;
			return CLASS_E_NOAGGREGATION;
		}
		ContextMenuHandler* ctx_menu_instance = new ContextMenuHandler();
		HRESULT rs = ctx_menu_instance->QueryInterface(interface_id, interface_buffer);
		if (FAILED(rs)) delete ctx_menu_instance;
		return rs;
	}
}; 

HRESULT __stdcall DllCanUnloadNow() {
	return dll_ref_count ? S_OK : S_FALSE;
}
HRESULT __stdcall DllGetClassObject(REFCLSID class_id, REFIID interface_id, LPVOID* interface_buffer) {
	HMODULE ole32 = LoadLibraryExW(L"ole32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (!ole32) {
		if (interface_buffer) *interface_buffer = NULL;
		return HRESULT_FROM_WIN32(GetLastError());
	}
	LPBYTE lpCLSIDFromString = (LPBYTE)GetProcAddress(ole32, "CLSIDFromString");
	if (!lpCLSIDFromString) {
		FreeLibrary(ole32);
		if (interface_buffer) *interface_buffer = NULL;
		return HRESULT_FROM_WIN32(GetLastError());
	}
	CLSID ext_clsid = { 0x0 };
	HRESULT rs = ((HRESULT(__stdcall*)(LPCOLESTR, LPCLSID))lpCLSIDFromString)(L"{424a995d-246c-4ada-bd98-3a1db4868241}", &ext_clsid);
	FreeLibrary(ole32);
	if (FAILED(rs)) {
		if (interface_buffer) *interface_buffer = NULL;
		return rs;
	}
	if (ext_clsid != class_id) {
		if (interface_buffer) *interface_buffer = NULL;
		return CLASS_E_CLASSNOTAVAILABLE;
	}
	if (interface_id != IID_IClassFactory) {
		if (interface_buffer) *interface_buffer = NULL;
		return E_NOINTERFACE;
	}
	ClassFactory* class_factory_instance = new ClassFactory();
	rs = class_factory_instance->QueryInterface(interface_id, interface_buffer);
	if (FAILED(rs)) delete class_factory_instance;
	return rs;
}