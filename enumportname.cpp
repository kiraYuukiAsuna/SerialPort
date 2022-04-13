#include <windows.h>

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

// multi byte to wide char:
std::wstring s2ws(const std::string& str) {
	int size_needed =
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0],
						size_needed);
	return wstrTo;
}

// wide char to multi byte:
std::string ws2s(const std::wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(),
										  int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1),
						&strTo[0], size_needed, 0, 0);
	return strTo;
}

//将TCHAR转为char
//*tchar是TCHAR类型指针，*_char是char类型指针
void TcharToChar(const WCHAR* tchar, char* _char) {
	int iLength;
	//获取字节长度
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

//同上
void CharToTchar(const char* _char, WCHAR* tchar) {
	int iLength;

	iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, tchar, iLength);
}

bool cmp(string s1, string s2) {
	if (atoi(s1.substr(3).c_str()) < atoi(s2.substr(3).c_str())) {
		return true;
	} else {
		return false;
	}
}

void GetComList_Reg(vector<string>& comList) {
	HKEY hKey;
	int result;
	int i = 0;
	WCHAR chs[256];
	CharToTchar("Hardware\\DeviceMap\\SerialComm", chs);
	result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, chs, NULL, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS) {
		WCHAR portName[256], commName[256];
		DWORD dwLong, dwSize;
		do {
			dwSize = sizeof(portName) / sizeof(TCHAR);
			dwLong = dwSize;
			result = RegEnumValueW(hKey, i, portName, &dwLong, NULL, NULL,
								   (LPBYTE)commName, &dwSize);
			if (result == ERROR_NO_MORE_ITEMS) {
				break;
			}
			char ch[256];
			TcharToChar(portName, ch);
			comList.push_back(ch);
			i++;
		} while (true);
		RegCloseKey(hKey);
	}
}

int main() {
	vector<string> comList;
	GetComList_Reg(comList);
	sort(comList.begin(), comList.end(), cmp);

	for (int i = 0; i < comList.size(); i++) {
		cout << comList[i] << endl;
	}
}
