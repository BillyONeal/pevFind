#ifndef REGSCRIPTCOMPILER_H_INCLUDED
#define REGSCRIPTCOMPILER_H_INCLUDED

#include <sstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <boost/range.hpp>

class regscriptCompiler
{
    enum
    {
        loadName,
        loadKeyRoot,
        loadData,
        createKey,
        closeKey,
        deleteKey,
        setValue,
        deleteValue
    };

    typedef struct
    {
        std::size_t code;
        std::size_t dataLength;
        void * data;
    } opCode;

    bool ansi;
    std::vector<opCode> parsedResults;
    std::size_t errorCount;
    std::size_t warningCount;
    std::size_t lineCount;
    std::wstring::const_iterator begin;
    std::wstring::const_iterator current;
    std::wstring::const_iterator end;
    void skipWhitespace();
    void parseVersion();
    bool parseKeySpec();
	bool parseValueSpec();
    bool loadNameAndRoot();
	void pushArgumentlessOpcode(const std::size_t code);
	std::wstring::const_iterator getEndOfKeyName();
	void parseCString(wchar_t ** str, std::size_t *length);
	void parseHexValueSpec(char ** str, std::size_t *length);
	DWORD parseDword();
    std::basic_stringstream<wchar_t> outputText;
	bool fixNull;
public:
    bool parse(boost::iterator_range<std::wstring::const_iterator> inputScript);
    ~regscriptCompiler();
    std::wstring getOutput() { return outputText.str(); };
	void printASM();
	void execute();
	bool succeeded();
};

#endif // REGSCRIPTCOMPILER_H_INCLUDED
