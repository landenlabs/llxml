//-------------------------------------------------------------------------------------------------
//
// File: xml.cpp   Author: Dennis Lang  Desc: Get files from directories
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
// ----- License ----
//
// Copyright (c) 2023 Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "xml.hpp"

#include <errno.h>
#include <stdio.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <regex>

#include "directory.hpp"
#include "ll_stdhdr.hpp"
#include "fileutil.hpp"

#ifdef HAVE_WIN
    #include <windows.h>
#endif

#ifdef WIN32
#else
#endif

#if 1
static std::regex_constants::match_flag_type rxFlags =
    std::regex_constants::match_flag_type(std::regex_constants::match_default +
        std::regex_constants::extended);
#else
static std::regex_constants::match_flag_type rxFlags =
    std::regex_constants::match_flag_type(std::regex_constants::match_default +
        std::regex_constants::match_not_eol +
        std::regex_constants::match_not_bol);
#endif

#define sizeStr(x)  sizeof(x)-1

static regex begPat("<.");
static regex xmlPatEnd("[?][>]( |\r|\n)*");
static regex commentPatEnd("-->( |\r|\n)*");
static regex stringPatEnd("</string>( |\r|\n)*");
static regex stringPat("<string.*name=.([^'\"]+).[^>]*>(.|\r|\n)*</string>( |\r|\n)*");
static regex anyPat("<([a-z]+).*name=.([^'\"]+).[^>]*>(.|\r|\n)*</([a-z]+)>");
static regex eoxPat("<[^<]+>( |\r|\n)*");
static regex eolPat("( |\r|\n)*");


//-------------------------------------------------------------------------------------------------
const char* XmlBuffer::getNext(const std::regex& xmlPatBeg) const {
    std::match_results<const char*> match;
    const char* begPtr = (const char*)data() + pos;
    const char* endPtr = (const char*)data() + size();

    // std::string test = std::string(begPtr, endPtr);

    const char* nextPtr = nullptr;
    size_t length = 0;
    if (std::regex_search(begPtr, endPtr, match, xmlPatBeg, rxFlags)) {
        length = match.position();
        const_cast<XmlBuffer*> (this)->pos += length;
        nextPtr = begPtr + length;
        length += match.length();
    }

    return (length > 0) ? nextPtr : nullptr;
}

//-------------------------------------------------------------------------------------------------
bool XmlBuffer::getStatement(const std::regex& xmlPatEnd, std::string& outStatement) const {
    std::match_results<const char*> match;
    const char* begPtr = (const char*)data() + pos;
    const char* endPtr = (const char*)data() + size();

    if (std::regex_search(begPtr, endPtr, match, xmlPatEnd, rxFlags)) {
        size_t length = match.position() + match.length();
        outStatement = std::string(begPtr, begPtr + length);
        const_cast<XmlBuffer*> (this)->pos += length;
        return true;
    }

    return false;
}

#define META_PREFIX  "_#"
static char META_FMT[] = META_PREFIX "%d";

// -------------------------------------------------------------------------------------------------
static void nextKey(unsigned num, string& numStr) {
    static char tagStr[10];
    snprintf(tagStr, sizeof(tagStr), META_FMT, num);
    numStr = tagStr;
}

// -------------------------------------------------------------------------------------------------
static string clean(const string& str) {
    // string out = str;
    // out.erase(remove(out.begin(), out.end(), '\n'), out.cend());
    string out(str.length(), '\0');
    int oIdx = 0;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] != '\n') {
            out[oIdx++] = str[i];
        }
    }
    out.resize(oIdx);
    return out;
}

// -------------------------------------------------------------------------------------------------
static void checkDuplicate(ostream& err, const XmlData& data, const string& key,
    const string& value, const string& filePath) {
    if (data.find(key) != data.end() && data.at(key) != value) {
        err << "Warning - duplicate: " << key << " in " << filePath << std::endl;
        err << " Old=" << data.at(key) << std::endl;
        err << " New=" << value << std::endl;
    }
}

// -------------------------------------------------------------------------------------------------
static bool equalIgnoreWhite(const string& str1, const string& str2) {

    const char* p1 = str1.c_str();
    const char* p2 = str2.c_str();
    while (*p1) {
        while (isspace(*p1)) p1++;  // space= ' ', '\t', '\n', '\v', '\f', '\r'
        while (isspace(*p2)) p2++;
        if (*p1 != *p2) return false;
        if (*p1 == '\0') break;
        p1++;
        p2++;
    }
    return (*p2 == '\0');
}

// -------------------------------------------------------------------------------------------------
unsigned int XmlBuffer::lineAt(size_t pos) const {
    return (unsigned) std::count(data(), data() + pos, '\n');
}

// -------------------------------------------------------------------------------------------------
bool XmlBuffer::parse(ostream& err, string filePath, bool master) {

    smatch match;
    vector<string> blockKeys;
    unsigned row = 0;
    string key;
    string statement;
    size_t lastPos = 0;
    const char* nextPtr;
    pos = 0;

    static FileData noData;
    FileData& fileData = master ? filesData[filePath] : noData;

    while ((nextPtr = getNext(begPat)) != nullptr) {
        if (pos > lastPos + 1) {
            nextKey(row++, key);
            if (master) {
                fileData.rows.push_back(key);
                statement = string(data() + lastPos, data() + pos);
                checkDuplicate(err, fileData.meta, key, statement, filePath);
                fileData.meta[key] = statement;
            }
        }

        bool okay = false;
        bool isMeta = true;

        switch (nextPtr[1]) {
        case '?':  // xml header <?xml .... ?>
            okay = getStatement(xmlPatEnd, statement);
            nextKey(row++, key);
            break;
        case '!':  // comment <!-- xxxx -->
            okay = getStatement(commentPatEnd, statement);
            nextKey(row++, key);
            break;
        case '/':   // end of a block, </resources>
            key = blockKeys.empty() ? "" : blockKeys.back();
            if (strncmp(key.c_str() + 1, nextPtr + 2, key.length() - 1) == 0) {
                okay = getStatement(eoxPat, statement);
                nextKey(row++, key);
                blockKeys.pop_back();
            }
            break;
        case 's':
            // <string name="key" opt="flags">String Value</string>
            if (strncmp("<string ", nextPtr, 8) == 0) {
                okay = getStatement(stringPatEnd, statement);
                string test = clean(statement);
                okay &= std::regex_search(test, match, stringPat, rxFlags);
                if (okay) {
                    // match[0]=whole string; match[1]=first capture group.
                    if (match.size() >= 2) {
                        key = match[1].str();
                        // err << "in=" << test << " key=" << key << std::endl;
                        isMeta = false;
                    }
                } else {
                    okay = false;
                    err << "Error - Line: " << lineAt(pos) << " Unknown: " << clean(statement) << ", In:" << filePath << std::endl;
                }
            }
            break;
        }

        if (okay) {
            if (master) {
                fileData.rows.push_back(key);
                if (isMeta) {
                    checkDuplicate(err, fileData.meta, key, statement, filePath);
                    fileData.meta[key] = statement;
                } else {
                    checkDuplicate(err, fileData.data, key, statement, filePath);
                    fileData.data[key] = statement;
                    // err << "Added [" << key << "]=" << statement << std::endl;
                }
            } else if (! isMeta) {
                if (! update(key, statement))
                    err << "Warning - extra: " << clean(statement) << ", In:" << filePath << std::endl;
            }
        } else {
            okay = getStatement(eoxPat, statement);
            if (okay) {
                blockKeys.push_back(statement);
                if (master) {
                    nextKey(row++, key);
                    fileData.rows.push_back(key);
                    checkDuplicate(err, fileData.meta, key, statement, filePath);
                    fileData.meta[key] = statement;
                }
            } else {
                err << "Error - Line: " << lineAt(pos) << " Unknown: " << string(nextPtr, nextPtr + 10) << ", In:" << filePath << std::endl;
                return false;
            }
        }

        lastPos = pos;
    }

    return filesData.size() > 0;
}

// -------------------------------------------------------------------------------------------------
void XmlBuffer::clearData() {
    for (auto& file : filesData) {
        for (auto& data : file.second.data) {
            data.second.clear();
        }
    }
}

// -------------------------------------------------------------------------------------------------
bool XmlBuffer::update(const string& key, const string& statement) {
    bool updated = false;
    for (auto& file : filesData) {
        FileData& fileData = file.second;
        if (fileData.data.find(key) != fileData.data.end()) {
            if (updated) {
                if (fileData.data.at(key) != statement) {
                    std::cerr << "Warning - duplicate: " << key << ", file=" << file.first << endl;
                }
            } else {
                const string& prevStatement = fileData.data.at(key);
                if (prevStatement.empty() || ! equalIgnoreWhite(prevStatement, statement)) {
                    fileData.updates[key] = prevStatement;
                }
                fileData.data[key] = statement;
                updated = true;
            }
        } else  {
            fileData.extra[key] = statement;
        }
    }
    return updated;
}

// -------------------------------------------------------------------------------------------------
unsigned int XmlBuffer::getUpdates() const {
    unsigned int updates = 0;
    for (const auto& file : filesData) {
        const FileData& fileData = file.second;
        updates += fileData.updates.size();
    }
    return updates;
}

// -------------------------------------------------------------------------------------------------
unsigned int XmlBuffer::getExtras() const {
    unsigned int extras = 0;
    for (const auto& file : filesData) {
        const FileData& fileData = file.second;
        extras += fileData.extra.size();
    }
    return extras;
}

// -------------------------------------------------------------------------------------------------
// Dump parsed json in json format.
void XmlBuffer::writeFilesTo(const string& outFmt, bool verbose) const {
    if (outFmt.length() == 0) {
        return;
    }

    for (const auto& file : filesData) {
        const FileData& fileData = file.second;

        const string& filePath = file.first;
        const Strings& fileRow = fileData.rows;
        const XmlData& xmlMeta = fileData.meta;
        const XmlData& xmlData = fileData.data;
        const XmlData& updates = fileData.updates;


        string outPath;
        FileUtil::getParts(outPath, outFmt.c_str(), filePath);
        bool toStdout = (outPath == "-");

        if (updates.empty() && ! toStdout) {
            cerr << "No updates to: " << outPath << std::endl;
            continue;
        }

        ostream* pOut = &cout;
        ofstream outF;
        if (! toStdout) {
            outF.open(outPath);
            if (! outF) {
                cerr << "Failed creation of: " << outPath << " outFmt: " << outFmt << " filePath: " << filePath << std::endl;
                continue;
            }
            pOut = &outF;
            cerr << "Saved " << updates.size() << " updates to: " << outPath << endl;
        } else if (verbose) {
            cout << "\n==== File: " << filePath << endl;
        }

        if (verbose) {
            for (const auto& upd : updates) {
                cerr << "   Update: [" << upd.first << "]=" << upd.second << " To:" << xmlData.at(upd.first) << std::endl;
            }
        }

        for (const string& key : fileRow) {
            const string& str = (key.compare(0, sizeStr(META_PREFIX), META_PREFIX) == 0)
                ? xmlMeta.at(key)
                : xmlData.at(key);
            (*pOut) << str;
        }

        if (outF.is_open())
            outF.close();
    }
}


