#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
	DIR *dir;
	struct dirent *entry;
	dir = opendir(".");
	entry = readdir(dir);
	std::cout << entry->d_type << std::endl;
}
