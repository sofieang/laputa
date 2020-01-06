#include "Files.h"
#include "App.h"
#include "Utility.h"
#ifdef __linux__
#include <FL/Fl_File_Chooser.H>
#else
#include <FL/Fl_Native_File_Chooser.H>
#endif

//-----------------------------------------------------------------------------------------------------------------------

string OpenFileDialog(const char* title, const char* filetypes) {
#ifdef __linux__
	char homeDir[FL_PATH_MAX];
	fl_filename_expand(homeDir, "~");
	Fl_File_Chooser fc(homeDir, filetypes, Fl_File_Chooser::SINGLE, title);
	fc.preview(0);
	fc.show();
	while(fc.shown()) Fl::wait();
	if (fc.value()) return string(fc.value());
	else return string("");
#else
	Fl_Native_File_Chooser fc;
	fc.title(title);
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.filter(filetypes);
	if (fc.show() == 0) return fc.filename();
	else return string("");
#endif
}

//-----------------------------------------------------------------------------------------------------------------------

string SaveFileDialog(const char* title, const char* filetypes, const char* preset) {
#ifdef __linux__
	string startFileString = "~/" + string(preset);
	char startFile[FL_PATH_MAX];
	fl_filename_expand(startFile, startFileString.c_str());
	Fl_File_Chooser fc(startFile, filetypes, Fl_File_Chooser::CREATE, title);
	fc.preview(0);
	fc.show();
	while(fc.shown()) Fl::wait();
	if (fc.value()) return string(fc.value());
	else return string("");
#else
	Fl_Native_File_Chooser fc;
	fc.title(title);
	fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc.options(Fl_Native_File_Chooser::NEW_FOLDER | Fl_Native_File_Chooser::SAVEAS_CONFIRM);
	fc.preset_file(preset);
	fc.filter(filetypes);
	if (fc.show() == 0) return fc.filename();
	else return string("");
#endif
}

//-----------------------------------------------------------------------------------------------------------------------

bool SaveDataAsSpreadsheet(const XMLData* data, t_int width, t_int height, t_int depth, string* sheetNames, const char* filename) {
	// get path
	char name[FL_PATH_MAX];
	strcpy(name, fl_filename_name(filename));
	char path[FL_PATH_MAX];
	strcpy(path, filename);
	path[strlen(filename) - strlen(name)] = 0;

	// create a zip from the files
	zipFile f = zipOpen(filename, APPEND_STATUS_CREATE);
	AddFileToZipArchive(app->dataFile("ods/mimetype").c_str(), "mimetype", f);
	AddFileToZipArchive(app->dataFile("/ods/styles.xml").c_str(), "styles.xml", f);
	AddFileToZipArchive(app->dataFile("/ods/meta.xml").c_str(), "meta.xml", f);
	AddFileToZipArchive(app->dataFile("/ods/META-INF/manifest.xml").c_str(), "META-INF/manifest.xml", f);
	AddFileToZipArchive(app->dataFile("/ods/manifest.rdf").c_str(), "manifest.rdf", f);
	if(!MakeContentFile(data, width, height, depth, sheetNames, path)) {
		zipClose(f, NULL);
		remove(filename);
		return false;
	}
	char cntStr[FL_PATH_MAX];
	strcpy(cntStr, path);
	strcat(cntStr, "content.xml");
	AddFileToZipArchive(cntStr, "content.xml", f, true);
	zipClose(f, NULL);
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

void AddFileToZipArchive(const char* srcfile, const char* dstname, zipFile zfile, bool removeAfterCopy) {
	zip_fileinfo zi;
	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
	char *buf = new char[0xFFFF];

	zipOpenNewFileInZip(zfile, dstname, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	FILE *f = fopen(srcfile, "r");
	int sz;
	do {
		sz = (int)fread(buf, 1, 0xFFFF, f);
		if (sz > 0) zipWriteInFileInZip (zfile, buf, sz);
	} while (sz > 0);
	fclose(f);
	zipCloseFileInZip(zfile);

	// delete file
	if(removeAfterCopy) remove(srcfile);
	delete[] buf;
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlText* ConvertToXMLLines(const string& s) {
	string output = "";

	t_int start = 0;
	do {
		t_int end = s.find('\r', start);
		if(end < s.size()) {
			output += s.substr(start, end) + string("<br />");
			if(end < s.size() - 1) {
				if(s[end + 1] == '\n') ++end;
			}
			start = end + 1;
		}
	} while (start < s.size());
	TiXmlText *text = new TiXmlText(output.c_str());

	return text;
}

//-----------------------------------------------------------------------------------------------------------------------

bool MakeContentFile(const XMLData* data, t_int width, t_int height, t_int depth, string *sheetNames, const char* path) {
	// create header
	TiXmlDocument f;
	//TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "true");
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "yes");
	f.LinkEndChild(decl);

	TiXmlElement *root = new TiXmlElement("office:document-content");
	root->SetAttribute("office:version", "1.1");
	root->SetAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
	root->SetAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
	root->SetAttribute("xmlns:content", "urn:oasis:names:tc:opendocument:xmlns:content:1.0");
	root->SetAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
	root->SetAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
	root->SetAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
	root->SetAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
	root->SetAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
	root->SetAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
	root->SetAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
	root->SetAttribute("xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");
	root->SetAttribute("xmlns:msoxl", "http://schemas.microsoft.com/office/excel/formula");
	f.LinkEndChild(root);

	TiXmlElement *fontfacedecls = new TiXmlElement("office:font-face-decls");
	root->LinkEndChild(fontfacedecls);
	TiXmlElement *style = new TiXmlElement("style:font-face");
	style->SetAttribute("style:name", "Calibri");
	style->SetAttribute("svg:font-family", "Calibri");
	fontfacedecls->LinkEndChild(style);

	TiXmlElement *autostyles = new TiXmlElement("office:automatic-styles");
	style = new TiXmlElement("style:style");
	style->SetAttribute("style:name", "ce1");
	style->SetAttribute("style:family", "table-cell");
	style->SetAttribute("style:parent-style-name", "Default");
	style->SetAttribute("style:data-style-name", "N0");
	autostyles->LinkEndChild(style);

	style = new TiXmlElement("style:style");
	style->SetAttribute("style:name", "co1");
	style->SetAttribute("style:family", "table-column");
	TiXmlElement *style2 = new TiXmlElement("style:table-column-properties");
	style2->SetAttribute("fo:break-before", "auto");
	style2->SetAttribute("style:column-width", "2cm");
	style->LinkEndChild(style2);
	autostyles->LinkEndChild(style);

	style = new TiXmlElement("style:style");
	style->SetAttribute("style:name", "ro1");
	style->SetAttribute("style:family", "table-row");
	style2 = new TiXmlElement("style:table-row-properties");
	style2->SetAttribute("style:row-height", "15pt");
	style2->SetAttribute("style:use-optimal-row-height", "true");
	style2->SetAttribute("fo:break-before", "auto");
	style->LinkEndChild(style2);
	autostyles->LinkEndChild(style);

	style = new TiXmlElement("style:style");
	style->SetAttribute("style:name", "ta1");
	style->SetAttribute("style:family", "table");
	style->SetAttribute("style:master-page-name", "mp1");
	style2 = new TiXmlElement("style:table-properties");
	style2->SetAttribute("table:display", "true");
	style2->SetAttribute("style:writing-mode", "lr-tb");
	style->LinkEndChild(style2);
	autostyles->LinkEndChild(style);
	root->LinkEndChild(autostyles);

	TiXmlElement *body = new TiXmlElement("office:body");
	root->LinkEndChild(body);
	TiXmlElement *ss = new TiXmlElement("office:spreadsheet");
	body->LinkEndChild(ss);

	TiXmlElement *calcsettings = new TiXmlElement("table:calculation-settings");
	if(!calcsettings) return false;
	calcsettings->SetAttribute("table:case-sensitive", "false");
	calcsettings->SetAttribute("table:search-criteria-must-apply-to-whole-cell", "false");
	ss->LinkEndChild(calcsettings);

	// create tables
	for(t_int k = 0; k < depth; ++k) {
		TiXmlElement *table = new TiXmlElement("table:table");
		if(!table) return false;
		table->SetAttribute("table:name", sheetNames[k].c_str());
		table->SetAttribute("table:style-name", "ta1");
		ss->LinkEndChild(table);
		TiXmlElement *tcol = new TiXmlElement("table:table-column");
		if(!tcol) return false;
		tcol->SetAttribute("table:style-name", "co1");
		tcol->SetAttribute("table:default-cell-style-name", "ce1");
		tcol->SetAttribute("table:number-columns-repeated", 16384);
		table->LinkEndChild(tcol);
		for(t_int j = 0; j < height; ++j) {
			TiXmlElement *row = new TiXmlElement("table:table-row");
			if(!row) return false;
			row->SetAttribute("table:style-name", "ro1");
			table->LinkEndChild(row);
			for(t_int i = 0; i < width; ++i) {
				TiXmlElement *cell = new TiXmlElement("table:table-cell");
				cell->SetAttribute("table:style-name", "ce1");
				if(!cell) return false;
				if(data[k * width * height + j * width + i].type == TYPE_DOUBLE) {
					cell->SetAttribute("office:value-type", "float");
					cell->SetDoubleAttribute("office:value", data[k * width * height + j * width + i].dbVal);
				}
				else if(data[k * width * height + j * width + i].type == TYPE_INT) {
					cell->SetAttribute("office:value-type", "float");
					cell->SetAttribute("office:value", data[k * width * height + j * width + i].intVal);
				}
				else {
					cell->SetAttribute("office:value-type", "string");
					TiXmlElement *text = new TiXmlElement("text:p");
					cell->LinkEndChild(text);
					TiXmlText* txt = new TiXmlText(data[k * width * height + j * width + i].strVal.c_str());
					text->LinkEndChild(txt);
				}
				row->LinkEndChild(cell);
			}

			// fill rest of row
			TiXmlElement *rowFill = new TiXmlElement("table:table-cell");
			if(!rowFill) return false;
			rowFill->SetAttribute("table:number-columns-repeated", IntToString(16384 - width));
			row->LinkEndChild(rowFill);
		}
		// fill rest of table
		TiXmlElement *tableFill = new TiXmlElement("table:table-row");
		if(!tableFill) return false;
		tableFill->SetAttribute("style-name", "ro1");
		tableFill->SetAttribute("table:number-rows-repeated", IntToString(1048576 - height));
		table->LinkEndChild(tableFill);
		TiXmlElement* rowFill = new TiXmlElement("table:table-cell");
		if(!rowFill) return false;
		rowFill->SetAttribute("table:number-columns-repeated", IntToString(16384));
		tableFill->LinkEndChild(rowFill);
	}

	// write out everything
	char filename[FL_PATH_MAX];
	strcpy(filename, path);
	strcat(filename, "content.xml");
	f.SaveFile(filename);
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

void MakeDirectory(const char *dirName) {
#ifdef _WINDOWS
	wchar_t wtext[FL_PATH_MAX];
	mbstowcs(wtext, dirName, strlen(dirName) + 1);
	t_int err = CreateDirectory(dirName, 0);
	t_int err2 = GetLastError();
#else
	mkdir(dirName, 0775);
#endif
}

//-----------------------------------------------------------------------------------------------------------------------

void MakePathNative(char* path) {
	t_int pos = 0;
	bool lastWasDelimiter = false;
	while (path[pos] != 0) {
#ifdef _WINDOWS
		if (path[pos] == '/') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else {
				path[pos] = '\\';
				++pos;
			}
			lastWasDelimiter = true;
		}
		else if (path[pos] == '\\') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else ++pos;
			lastWasDelimiter = true;
		}
		else {
			lastWasDelimiter = false;
			++pos;
		}
#endif
#ifdef __APPLE__
        if (path[pos] == '\\') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else {
				path[pos] = '/';
				++pos;
			}
			lastWasDelimiter = true;
		}
		else if (path[pos] == '/') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else ++pos;
			lastWasDelimiter = true;
		}
		else {
			lastWasDelimiter = false;
			++pos;
		}

#endif
#ifdef __linux__
        if (path[pos] == '\\') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else {
				path[pos] = '/';
				++pos;
			}
			lastWasDelimiter = true;
		}
		else if (path[pos] == '/') {
			if (lastWasDelimiter) for (t_int i = pos; path[i] != 0; ++i) path[i] = path[i + 1];
			else ++pos;
			lastWasDelimiter = true;
		}
		else {
			lastWasDelimiter = false;
			++pos;
		}

#endif
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void MakePathNative(string& path) {
	char cpath[FL_PATH_MAX];
	strcpy(cpath, path.c_str());
	MakePathNative(cpath);
	path = string(cpath);
}
