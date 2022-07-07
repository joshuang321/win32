#include <stdio.h>
typedef unsigned short WCHAR;
typedef int UNICODE;

int main(int argc, char **argv)
{
	UNICODE dwUnicode;
	WCHAR firstSurrogate, secondSurrogate;
	if (argc !=2)
	{
		printf("Usage: unicode_to_utf16 [unicode_value]\r\n");
		return -1;
	}
	dwUnicode = atoi(argv[1]);
	if (dwUnicode < 0x10000)
	{
		printf("Invalid Unicode!");
		return -1;
	}
	firstSurrogate = ((dwUnicode - 0x10000) >> 0xA) + 0xD800;
	if (firstSurrogate < 0xD800)
	{
		printf("Invalid Unicode!");
		return -1;
	}
	secondSurrogate = (dwUnicode & 0x3FF) + 0xDC00;
	printf("utf16: %x %x\r\n", firstSurrogate, secondSurrogate);
	return 0;
}