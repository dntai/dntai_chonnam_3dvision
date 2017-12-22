#pragma once

#include <iostream>
#include <string>
#include <vector>

class Utilities
{
public:
	static bool folder_exists(std::string foldername);
	static int _mkdir(const char *path);
	static int mkdir(const char *path);
	static std::vector<std::string>  GetFileNamesInDirectory(std::string dir, std::string ext);
	static const std::string currentDateTime(const char *fmt);
};

