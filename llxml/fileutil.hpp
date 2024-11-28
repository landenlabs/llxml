//
//  fileutil.hpp
//  llxml
//
//  Created by Dennis Lang on 10/21/23.
//  Copyright © 2023 Dennis Lang. All rights reserved.
//

#ifndef fileutil_hpp
#define fileutil_hpp

#include <string>
using namespace std;

class FileUtil {
public:
    static string& getName(string& outName, const string& inPath);
    static string& getDirs(string& outDirs, const string& inPath);
    static string& getParts(string& outParts, const char* customFmt, const string& inPath);
};

#endif /* fileutil_hpp */
