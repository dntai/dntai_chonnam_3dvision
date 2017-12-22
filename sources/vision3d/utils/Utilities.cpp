#include "Utilities.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <chrono>
#include <windows.h>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

// for windows mkdir
#ifdef _WIN32
#include <direct.h>
#endif

////////////////////////////////////////////////////////////////
// http://stackoverflow.com/questions/10402499/mkdir-c-function
////////////////////////////////////////////////////////////////
/**
* Checks if a folder exists
* @param foldername path to the folder to check.
* @return true if the folder exists, false otherwise.
*/
bool Utilities::folder_exists(std::string foldername)
{
	struct stat st;
	if (stat(foldername.c_str(), &st) != 0)
	{
		return 0;
	}
	return st.st_mode & S_IFDIR;
}

/**
* Portable wrapper for mkdir. Internally used by mkdir()
* @param[in] path the full path of the directory to create.
* @return zero on success, otherwise -1.
*/
int Utilities::_mkdir(const char *path)
{
#ifdef _WIN32
	return ::_mkdir(path);
#else
#if _POSIX_C_SOURCE
	return ::mkdir(path);
#else
	return ::mkdir(path, 0755); // not sure if this works on mac
#endif
#endif
}

/**
* Recursive, portable wrapper for mkdir.
* @param[in] path the full path of the directory to create.
* @return zero on success, otherwise -1.
*/
int Utilities::mkdir(const char *path)
{
	std::string current_level = "";
	std::string level;
	std::stringstream ss(path);

	// split path using slash as a separator
	while (std::getline(ss, level, '/'))
	{
		current_level += level; // append folder to the current level

		// create current level
		if (!folder_exists(current_level))
		{
			if (_mkdir(current_level.c_str()) != 0)
			{
				return -1;
			}
		}

		current_level += "/"; // don't forget to append a slash
	}

	return 0;
}

struct CompareFileOp
{
	bool operator()(std::string &a, std::string &b) const {
		if (a.size() < b.size()) return true;
		if (a.size() == b.size() && a.compare(b) < 0) return true;
		return false;
	}
} objCompareFileOp;

std::vector<std::string>  Utilities::GetFileNamesInDirectory(std::string dir, std::string ext) {
	std::vector<std::string> files;
	WIN32_FIND_DATAA fileData;

	HANDLE hFind;
	std::string full_path = dir + "\\" + ext;
	if (!((hFind = FindFirstFileA(full_path.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
		do{
			if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				files.push_back(dir + "\\" + std::string(fileData.cFileName));
			}
		} while (FindNextFileA(hFind, &fileData));
	}
	FindClose(hFind);
	std::sort(files.begin(), files.end(), objCompareFileOp);
	return files;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string Utilities::currentDateTime(const char *fmt) {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), fmt, &tstruct);
	return buf;
}

struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '\\' || ch == '/';
	}
};

std::string getFileNameFromPath(std::string const& pathname)
{
	return std::string(
		std::find_if(pathname.rbegin(), pathname.rend(),
		MatchPathSeparator()).base(),
		pathname.end());
}

std::string getFileName(std::string const& filename)
{
	std::string::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
	return pivot == filename.rend() ? filename
		 : std::string(filename.begin(), pivot.base() - 1);
}

std::string getFileExtension(std::string const& filename)
{
	std::string::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
	return pivot == filename.rend() ? string("")
		: std::string(pivot.base() + 1, filename.end() - 1);
}

std::string getFileNameOnlyNumber(std::string const& filename)
{
	std::string kq = "";
	for (int i = 0; i < filename.size(); i++)
	{
		if (filename[i] >= '0' && filename[i] <= '9')
		{
			kq = kq + filename[i];
		}
	}
	return kq;
}