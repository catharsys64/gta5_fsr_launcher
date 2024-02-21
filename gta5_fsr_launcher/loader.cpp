#include "includes.h"

#ifdef GTA_BUILD
const string exec_name = "PlayGTAV.exe";
const string game_type = "";
#endif

#ifdef RPH_BUILD
const string exec_name = "RAGEPluginHook.exe";
const string game_type = "(RagePluginHook)";
#endif

const string proc_name = "GTA5.exe", dll_name = "fsr_dxhijack.dll"; 
string dll_path, exec_path;

int main(int argc, char** argv)
{
	SetConsoleTitleA(("FSR Launcher for GTA V " + game_type).c_str());

	string cur_dir = getCurrentDir();
	exec_path = cur_dir + "\\" + exec_name;
	dll_path = cur_dir + "\\" + dll_name;

	if (!filesystem::exists(exec_path)) {
		cout << exec_name << " is missing.\nMake sure to put FSR Launcher in GTA V's root directory.\n";
		std::cin.ignore();
		return 0;
	}
	else if (!filesystem::exists(dll_path)) {
		cout << dll_name << " is missing.\n";
		std::cin.ignore();
		return 0;
	}

	cout << "Launching GTA V. Do not close this window yet.\n";

	ShellExecute(NULL, "open", exec_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	while (getProcID(proc_name) == 0) {
		Sleep(1);
	}

	cout << "Loading.\n";

	int procId = getProcID(proc_name);
	if (!injectDLL(procId)) {
		cout << "Loading failed! Make sure to run FSR Launcher as admin!\nIf the problem persists, try disabling your antivirus.\n";
		std::cin.ignore();
		return 0;
	}

	system("cls");
	cout << "Success.\n";
	
	Sleep(700);
	return 0;
}

string getCurrentDir()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

bool injectDLL(const int& pid)
{
	long path_size = dll_path.length() + 1;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProc == NULL)
	{
		cerr << "[!] Failed to open handle to process." << endl;
		return false;
	}

	LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, path_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (MyAlloc == NULL)
	{
		cerr << "[!] Failed to allocate memory in process." << endl;
		return false;
	}

	int IsWriteOK = WriteProcessMemory(hProc, MyAlloc, dll_path.c_str(), path_size, 0);
	if (IsWriteOK == 0 || IsWriteOK == 6)
	{
		cerr << "[!] Failed to write to allocated memory." << endl;
		return false;
	}

	DWORD dWord;
	LPTHREAD_START_ROUTINE LoadLibraryPtr = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary("kernel32"), "LoadLibraryA");

	HANDLE threadHandle = CreateRemoteThread(hProc, NULL, 0, LoadLibraryPtr, MyAlloc, 0, &dWord);
	if (threadHandle == NULL)
	{
		cerr << "[!] Failed to create remote thread in process." << endl;
		return false;
	}

	if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (threadHandle != NULL))
	{
		return true;
	}

	return false;
}

int getProcID(const string& p_name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 entry = { 0 };

	entry.dwSize = sizeof(PROCESSENTRY32);

	if (snapshot == INVALID_HANDLE_VALUE) return 0;
	if (Process32First(snapshot, &entry) == FALSE) return 0;

	while (Process32Next(snapshot, &entry))
	{
		if (!strcmp(entry.szExeFile, p_name.c_str()))
		{
			CloseHandle(snapshot);
			return entry.th32ProcessID;
		}
	}
	CloseHandle(snapshot);
	return 0;
}