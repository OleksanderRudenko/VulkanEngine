#include "stdafx.h"
#include "tools.h"

namespace xengine
{

std::vector<char> ReadFile(const std::string& _filename, std::ios_base::openmode _mode)
{
	std::ifstream file(_filename, _mode);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

}