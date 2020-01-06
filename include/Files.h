#ifndef __FILES_H__
#define __FILES_H__
#include "Prefix.h"
#include "tinyxml.h"
#include "zip.h"
#include <stdio.h>
#include <string>

#define TYPE_NONE 0
#define TYPE_INT 1
#define TYPE_DOUBLE 2
#define TYPE_STRING 3

using namespace std;

class XMLData {
public:
	t_int type = TYPE_NONE;
	t_int intVal = 0;
	double dbVal = 0;
	string strVal;

	XMLData() {}

	void setInt(t_int iv) {intVal = iv; type = TYPE_INT;}
	void setDouble(double dv) {dbVal = dv; type = TYPE_DOUBLE;}
	void setString(const string& sv) {strVal = sv; type = TYPE_STRING;}
};

bool SaveDataAsSpreadsheet(const XMLData* data, t_int width, t_int height, t_int depth, string *sheetNames, const char* filename);
void AddFileToZipArchive(const char* srcfile, const char* dstname, zipFile zfile, bool removeAfterCopy = false);
TiXmlText* ConvertToXMLLines(const string& s);

// functions for generating files in .ods file
bool MakeContentFile(const XMLData* data, t_int width, t_int height, t_int depth, string *sheetNames, const char* path);

// General functions encapsulating file chooser dialogs
string OpenFileDialog(const char* title, const char* filetypes);
string SaveFileDialog(const char* title, const char* filetypes, const char* preset);

// dirty stuff for handling paths
void MakePathNative(char* path);
void MakePathNative(string& path);
#endif
