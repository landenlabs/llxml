//
//  fileutil.cpp
//  llxml
//
//  Created by Dennis Lang on 10/21/23.
//  Copyright © 2023 Dennis Lang. All rights reserved.
//

#include "fileutil.hpp"
#include <sstream>

#ifdef WIN32
    const char SLASH_CHAR('\\');
    #include <assert.h>
    #define strncasecmp _strnicmp
    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
        #define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
    #endif
#else
    const char SLASH_CHAR('/');
    #include <assert.h>
#endif


// -------------------------------------------------------------------------------------------------
// Extract name part from path.
string& FileUtil::getName(string& outName, const string& inPath) {
    size_t nameStart = inPath.rfind(SLASH_CHAR) + 1;
    if (nameStart == 0)
        outName = inPath;
    else
        outName = inPath.substr(nameStart);
    return outName;
}

// -------------------------------------------------------------------------------------------------
// Extract name part from path.
string& FileUtil::getDirs(string& outDirs, const string& inPath) {
    size_t dirEnd = inPath.rfind(SLASH_CHAR);
    if (dirEnd == -1)
        outDirs = "";
    else
        outDirs = inPath.substr(0, dirEnd);
    return outDirs;
}

//-------------------------------------------------------------------------------------------------
string& FileUtil::getParts(string& outParts, const char* customFmt, const string& inPath) {
    stringstream sout;
    string name;
    string dirs;

    const char* fmt = customFmt;
    while (*fmt) {
        char c = *fmt++;
        if (c != '%') {
            sout << c;
        } else {
            c = *fmt++;
            switch (c) {
            case 'n':   // name
                sout << getName(name, inPath);
                break;
            case 'p':   // path
                sout << getDirs(dirs, inPath);
                break;
            default:
                sout << c;
                break;
            }
        }
    }

    outParts = sout.str();
    return outParts;
}
