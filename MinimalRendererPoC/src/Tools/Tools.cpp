#include "MrpocPch.h"
#include <Tools/Tools.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#if defined (PLATFORM_SWITCH)
#include <nn/time.h>
#endif


using namespace std;
using namespace mrpoc;

#if defined (PLATFORM_SWITCH)
void mrpoc::strcpy_s(char* dest, rsize_t destsz, const char* src)
{
	strcpy(dest, src);
}
#endif

string mrpoc::ReadFileToString(const string& filename)
{
	unsigned int length;
	char* buffer;

	ifstream is;
	is.open(filename.c_str(), ifstream::in);

	// Check for bad paths
	if (!is.is_open())
	{
		spdlog::error("Unable to open file: {}", filename);
		return "";
	}

	// get length of file:
	is.seekg(0, ios::end);
	length = static_cast<unsigned int>(is.tellg());
	is.seekg(0, ios::beg);

	//!! Create a null terminated string !!

	// allocate memory
	buffer = new char[length + 1];
	memset(buffer, 0, length + 1);

	// read data as a block:
	is.read(buffer, length + 1);
	is.close();

	// NB: specifying a length parameter will solve any null byte issues, even when
	// reading arbitrary binary data.
	string ret = string(buffer, length + 1);
	delete[] buffer;

	return ret;
}

bool mrpoc::ReadFileToBuffer(const std::string& filename, unsigned char*& buffer, int& length)
{
#if defined(_WIN32)
	FILE* f = nullptr;
	fopen_s(&f, filename.c_str(), "rb");
#else 
	FILE* f = fopen(filename.c_str(), "rb");
#endif

	if (!f)
		return false;

	fseek(f, 0, SEEK_END);
	length = static_cast<int>(ftell(f));
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	buffer = static_cast<unsigned char*>(malloc(length + 1));
	fread(buffer, length, 1, f);
	fclose(f);

	return true;
}

bool mrpoc::SaveFile(const std::string& filename,
	const std::string& text)
{
	ofstream os;
	os.open(filename.c_str(), ofstream::out);

	// Check for bad paths
	if (!os.is_open())
	{
#ifdef DEBUG
		spdlog::error("Unable to open file for writing: {}", filename);
#endif
		return false;
	}


	// get length of file:
	os.write(text.c_str(), text.size());

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// String hashing function
// Taken from http://www.cse.yorku.ca/~oz/hash.html (public domain)
////////////////////////////////////////////////////////////////////////////////
unsigned long long mrpoc::StringHash(const std::string& str)
{
	unsigned long long hash = 0;
	int c;
	const char* cstr = str.c_str();

	while ((c = *cstr++))
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

////////////////////////////////////////////////////////////////////////////////
// StringReplace
// Courtesy of: http://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
////////////////////////////////////////////////////////////////////////////////
string mrpoc::StringReplace(
	const std::string& subject,
	const std::string& search,
	const std::string& replace)
{
	std::string result(subject);
	size_t pos = 0;

	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		result.replace(pos, search.length(), replace);
		pos += search.length();
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
// StringStartsWith
// Determine if a string starts with a given string.
////////////////////////////////////////////////////////////////////////////////
bool mrpoc::StringStartsWith(const std::string& subject, const std::string& prefix)
{
	// Early out, prefix is longer than the subject:
	if (prefix.length() > subject.length())
	{
		return false;
	}

	// Compare per character:
	for (size_t i = 0; i < prefix.length(); ++i)
	{
		if (subject[i] != prefix[i])
		{
			return false;
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// StringEndsWith
// Determine if a string ends with a given string.
////////////////////////////////////////////////////////////////////////////////
bool mrpoc::StringEndsWith(const std::string& subject, const std::string& suffix)
{
	// Early out test:
	if (suffix.length() > subject.length())
	{
		return false;
	}

	// Resort to difficult to read C++ logic:
	return subject.compare(
		subject.length() - suffix.length(),
		suffix.length(),
		suffix
	) == 0;
}

void mrpoc::StringCapitalizeFirst(std::string& str)
{
	if (!str.empty())
	{
		auto& c = str[0];
		c = toupper(c);
	}
}

std::string mrpoc::UnicodeToUTF8(unsigned int codepoint)
{
	std::string out;

	if (codepoint <= 0x7f)
		out.append(1, static_cast<char>(codepoint));
	else if (codepoint <= 0x7ff)
	{
		out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else if (codepoint <= 0xffff)
	{
		out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else
	{
		out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	return out;
}

std::string mrpoc::ToString(float f, int precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << f;
	return stream.str();
}

std::string mrpoc::ToHexString(int number)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(int) * 2)
		<< std::hex << number;
	return stream.str();
}


long long mrpoc::Now()
{

#if defined (PLATFORM_SWITCH)

	nn::time::SystemClockTraits::time_point tp = nn::time::StandardUserSystemClock::now();
	time_t tt = nn::time::StandardUserSystemClock::to_time_t(tp);
	return tt;
#else

	return time(nullptr);

#endif
}
