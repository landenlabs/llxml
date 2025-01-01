//-------------------------------------------------------------------------------------------------
//
//  llxml      Oct-2023      Dennis Lang
//
//  Convert xml
//
// Example input xml:
//   <!--
//     ~ Comment1 with multiple words
//     ~ Comment2 with multiple words
//     -->
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
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2023
// https://landenlabs.com/
//
// This file is part of llxml project.
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

// 4291 - No matching operator delete found
#pragma warning(disable : 4291)

// Project files
#include "ll_stdhdr.hpp"
#include "directory.hpp"
#include "split.hpp"
#include "xml.hpp"
#include "fileutil.hpp"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

using namespace std;

// Helper types
typedef std::vector<std::regex> PatternList;
typedef unsigned int uint;

// Runtime options
static PatternList includeFilePatList;
static PatternList excludeFilePatList;
static PatternList includePathPatList;
static PatternList excludePathPatList;
static StringList fileDirList;
static XmlBuffer xmlBuffer;

static bool showInfo = false;
static bool verbose = false;
static bool master = true;

static string outPath;
static string separator = ",";

static uint optionErrCnt = 0;
static uint patternErrCnt = 0;
static uint parseErrCnt = 0;

#ifdef WIN32

    #define strncasecmp _strnicmp
    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
        #define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
    #endif

#endif

/*
// -------------------------------------------------------------------------------------------------
// Extract name part from path.
static lstring& getName(lstring& outName, const lstring& inPath) {
    size_t nameStart = inPath.rfind(SLASH_CHAR) + 1;
    if (nameStart == 0)
        outName = inPath;
    else
        outName = inPath.substr(nameStart);
    return outName;
}
*/

// -------------------------------------------------------------------------------------------------
// Return true if inName matches pattern in patternList
static bool FileMatches(const lstring& inName, const PatternList& patternList, bool emptyResult) {
    if (patternList.empty() || inName.empty()) return emptyResult;

    for (size_t idx = 0; idx != patternList.size(); idx++)
        if (std::regex_match(inName.begin(), inName.end(), patternList[idx]))
            return true;

    return false;
}

// -------------------------------------------------------------------------------------------------
// Open, read and parse file.
static bool ParseFile(const lstring& filepath, const lstring& filename) {

    if (filepath == separator) {
        master = false;
        xmlBuffer.clearData();
        return false;
    }

    ifstream in;
    // ofstream out;
    struct stat filestat;
    bool parseOk = false;

    try {
        if (stat(filepath, &filestat) != 0) {
            cerr << "Error - empty or not a file: " << filepath << endl;
            return false;
        }

        in.open(filepath);
        if (in.good()) {
            xmlBuffer.resize(filestat.st_size + 1);
            streamsize inCnt = in.read(xmlBuffer.data(), xmlBuffer.size()).gcount();
            assert(inCnt < xmlBuffer.size());
            in.close();
            xmlBuffer.push_back('\0');

            parseOk = xmlBuffer.parse(std::cerr, filepath, master);

            if (! parseOk) {
                cerr << "Error - failed to parse: " << filepath << endl;
                parseErrCnt++;
            }
        } else {
            cerr << strerror(errno) << ", Unable to open: " << filepath << endl;
        }
    } catch (exception ex) {
        cerr << ex.what() << ", Error in file: " << filepath << endl;
    }

    if (verbose) cerr << (parseOk ? "Parsed: " : " Failed: ") << filepath << std::endl;
    return parseOk;
}

// -------------------------------------------------------------------------------------------------
// Locate matching files which are not in exclude list.
static size_t InspectFile(const lstring& fullname) {
    size_t fileCount = 0;
    string name, dirs;
    FileUtil::getName(name, fullname);
    FileUtil::getDirs(dirs, fullname);

    if (! name.empty()
        && ! FileMatches(name, excludeFilePatList, false)
        && FileMatches(name, includeFilePatList, true)
        && ! FileMatches(dirs, excludePathPatList, false)
        && FileMatches(dirs, includePathPatList, true)  ) {

        // if (verbose) cerr << fullname << std::endl;

        if (ParseFile(fullname, name)) {
            fileCount++;
            if (showInfo) {
                if (master) {
                    const FileData& fileData = xmlBuffer.filesData.at(fullname);
                    std::cout << "Parsed: " << fullname
                        << " rows=" << fileData.rows.size()
                        << " data=" << fileData.data.size()
                        << " meta=" << fileData.meta.size()
                        << std::endl;
                } else {
                    std::cout << "Parsed: " << fullname
                        << " updates=" <<  xmlBuffer.getUpdates()
                        << " extras=" << xmlBuffer.getExtras()
                        << std::endl;
                }
            }
        }
    }

    return fileCount;
}

// -------------------------------------------------------------------------------------------------
// Recurse over directories, locate files.
static size_t InspectFiles(const lstring& dirname) {
    Directory_files directory(dirname);
    lstring fullname;

    size_t fileCount = 0;

    struct stat filestat;
    try {
        if (stat(dirname, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
            fileCount += InspectFile(dirname);
        } else if (dirname == separator) {
            InspectFile(dirname);
        }
    } catch (exception ex) {
        // Probably a pattern, let directory scan do its magic.
        // std::cerr << ex.what() << std::endl;
    }

    while (directory.more()) {
        directory.fullName(fullname);
        if (directory.is_directory()) {
            fileCount += InspectFiles(fullname);
        } else if (fullname.length() > 0) {
            fileCount += InspectFile(fullname);
        }
    }

    return fileCount;
}

// -------------------------------------------------------------------------------------------------
// Return compiled regular expression from text.
static std::regex getRegEx(const char* value) {
    try {
        std::string valueStr(value);
        return std::regex(valueStr);
        // return std::regex(valueStr, regex_constants::icase);
    } catch (const std::regex_error& regEx) {
        std::cerr << regEx.what() << ", Pattern=" << value << std::endl;
    }

    patternErrCnt++;
    return std::regex("");
}

// -------------------------------------------------------------------------------------------------
// Validate option matchs and optionally report problem to user.
static bool ValidOption(const char* validCmd, const char* possibleCmd, bool reportErr = true) {
    // Starts with validCmd else mark error
    size_t validLen = strlen(validCmd);
    size_t possibleLen = strlen(possibleCmd);

    if (strncasecmp(validCmd, possibleCmd, std::min(validLen, possibleLen)) == 0)
        return true;

    if (reportErr) {
        std::cerr << "Unknown option:'" << possibleCmd << "', expect:'" << validCmd << "'\n";
        optionErrCnt++;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc == 1) {
        cerr << "\n"
            << argv[0] << "  Dennis Lang v1.2 (landenlabs.com) " __DATE__
            << "\n"
            << "\nDes: Xml parse and ?? \n"
                      "Use: llxml [options] directories...   or  files\n"
                      "\n"
                      " Options (only unique characters required, can be repeated, case ignored):\n"
                      "   -fileInclude=<filePattern>\n"
                      "   -fileExclude=<filePattern>\n"
                      "   -pathInclude=<pathPattern>\n"
                      "   -pathExclude=<pathPattern>\n"
                      "   -showInput\n"
                      "   -verbose\n"
                      "   -outFmt=%p-AA/%f \n"
                      "\n"
                      " Example:\n"
                      "   llxml -inc=\\*xml -excludePath=\\*value-\\* \n"
                      "   llxml main1.xml dir2/main2.xml , child1.xml child2.xml \n"
                      "\n"
                      " Example input xml:\n"
                      "    <?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      "    <!-- comment-->\n"
                      "    <resources>\n"
                      "        <string name=\"language\" translatable=\"false\">English</string>\n"
                      "        <!-- comment -->\n"
                      "        <string name=\"word1\">Your Drive</string>\n"
                      "        <string name=\"word2\">Radar</string>\n"
                      "    </resources>\n"
                      "\n"
                      "   Output:\n"
                      "\n";
    } else {
        bool doParseCmds = true;
        string endCmds = "--";
        for (int argn = 1; argn < argc; argn++) {
            if (*argv[argn] == '-' && doParseCmds) {
                lstring argStr(argv[argn]);
                Split cmdValue(argStr, "=", 2);
                if (cmdValue.size() == 2) {
                    lstring cmd = cmdValue[0];
                    lstring value = cmdValue[1];

                    switch (cmd[(unsigned)1]) {
                    case 'f':  // fileExclude=<pat>
                        if (ValidOption("fileExclude", cmd + 1, false)) {
                            ReplaceAll(value, "*", ".*");
                            excludeFilePatList.push_back(getRegEx(value));
                        } else if (ValidOption("fileInclude", cmd + 1)) {
                            ReplaceAll(value, "*", ".*");
                            includeFilePatList.push_back(getRegEx(value));
                        }
                        break;
                    case 'p':  // includeFile=<pat>
                        if (ValidOption("pathExclude", cmd + 1, false)) {
                            ReplaceAll(value, "*", ".*");
                            excludePathPatList.push_back(getRegEx(value));
                        } else if (ValidOption("pathInclude", cmd + 1)) {
                            ReplaceAll(value, "*", ".*");
                            includePathPatList.push_back(getRegEx(value));
                        }
                        break;
                    case 'o':   // main=outMain.xml
                        if (ValidOption("outpath", cmd + 1)) {
                            outPath = value;
                        }
                        break;

                    default:
                        std::cerr << "Unknown command " << cmd << std::endl;
                        optionErrCnt++;
                        break;
                    }
                } else {
                    switch (argStr[(unsigned)1]) {
                    case 's':  // -show info about parsed files
                        showInfo = true;
                        continue;
                    case 'v':  // -v=true or -v=anyThing
                        verbose = true;
                        continue;
                    }

                    if (endCmds == argv[argn]) {
                        doParseCmds = false;
                    } else {
                        std::cerr << "Unknown command " << argStr << std::endl;
                        optionErrCnt++;
                    }
                }
            } else {
                // Store file directories
                fileDirList.push_back(argv[argn]);
            }
        }

        if (patternErrCnt == 0 && optionErrCnt == 0 &&
                    fileDirList.size() != 0) {
            if (fileDirList.size() == 1 && fileDirList[0] == "-") {
                string filePath;
                while (std::getline(std::cin, filePath)) {
                    InspectFiles(filePath);
                }
            } else {
                for (auto const& filePath : fileDirList) {
                    InspectFiles(filePath);
                }
            }
        }

        xmlBuffer.writeFilesTo(outPath, verbose);

        std::cerr << std::endl;
    }

    return 0;
}
