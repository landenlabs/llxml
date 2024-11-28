//-------------------------------------------------------------------------------------------------
//
// File: json.hpp  Author: Dennis Lang  Desc: Parse json
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2023
// https://landenlabs.com
//
// This file is part of llxml project.
//
// Parse example:
//
//   <?xml version="1.0" encoding="utf-8"?>
//
//   <!--
//     ~ Comment1 with multiple words
//     ~ Comment2 with multiple words
//     -->
//
//   <resources>
//       <string name="language" translatable="false">English</string>
//
//       <!-- BEGIN NAMESPACE test1 -->
//       <!-- Translation notes, blah blah blah notes 1 -->
//       <string name="word1">Your Drive</string>
//       <string name="word2">Radar</string>
//       <string name="word_3">Daily</string>
//       <!-- END NAMESPACE test1 -->
//
//       <!-- BEGIN NAMESPACE test2 -->
//       <!-- Translation notes, blah blah blah notes 2 -->
//       <string name="another_word1">Your Drive</string>
//       <!-- Translation notes, blah blah blah notes 3 -->
//       <string name="another_word2">Radar</string>
//       <!-- END NAMESPACE test2 -->
//
//   </resources>
//
//
// ----- License ----
//
// Copyright (c) 2023 Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef xml_h
#define xml_h

#include <vector>
#include <exception>
#include <map>
#include <string>
#include <ostream>
#include <regex>

#include "lstring.hpp"

using namespace std;

typedef vector<lstring> StringList;
typedef vector<string> Strings;
typedef map<string, string> XmlData;

struct FileData {
    Strings rows;
    XmlData meta;
    XmlData data;
    XmlData updates;
    XmlData extra;
};

// String buffer being parsed
class XmlBuffer : public std::vector<char> {
public:
    map<string, FileData> filesData;

    bool parse(ostream& err, string filePath, bool append);
    void clearData();
    void writeFilesTo(const string& outPathFmt, bool verbose) const;
    unsigned int getUpdates() const;
    unsigned int getExtras() const;

private:
    size_t pos = 0;
    const char* getNext(const regex& xmlPatBeg) const;
    bool getStatement(const regex& xmlPatEnd, string& outStatement) const;

    bool update(const string& key, const string& statement);
    unsigned int lineAt(size_t pos) const;
};


#endif /* xml */

