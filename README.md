<table border="0">
  <tr>
    <td>
      <!-- VERSION -->v6.07.10<br>
      <!-- DATE -->12-Jul-2026<br>
      macOS<br>
      <a href="https://landenlabs.com">Home</a>
    </td>
    <td>
      <a href="https://landenlabs.com">
        <img src="screens/landen_labs_300.webp" width="300" alt="LanDen Labs">
      </a>
    </td>
  </tr>
</table>

# llxml <!-- VERSION -->v6.07.10<!-- DATE -->12-Jul-2026
 
OSX / Linux / DOS  Parse XML resource files

  [![Build status](https://travis-ci.org/landenlabs/llxml.svg?branch=master)](https://travis-ci.org/landenlabs/llxml)
  [![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE.txt)
  ![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
  ![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows-lightgrey.svg)

### Description

LLXml parses XML resource files (such as Android `strings.xml`) and extracts
entries tagged between `<!-- BEGIN NAMESPACE name -->` and
`<!-- END NAMESPACE name -->` comment markers. Extracted entries for each
input file can be written to a new file using the `-outFmt` path pattern
(`%p`=directory path, `%f`=filename), which is useful for splitting a shared
resource file into separate per-namespace files while preserving comments.

### Dependencies
* [llcommon](https://github.com/landenlabs/llcommon) - shared LanDen Labs utility library (git submodule)

This input xml file:

<pre>
&lt;?xml version="1.0" encoding="utf-8"?&gt;
&lt;resources&gt;
    &lt;string name="language" translatable="false"&gt;English&lt;/string&gt;

    &lt;!-- BEGIN NAMESPACE test1 --&gt;
    &lt;string name="word1"&gt;Your Drive&lt;/string&gt;
    &lt;string name="word2"&gt;Radar&lt;/string&gt;
    &lt;!-- END NAMESPACE test1 --&gt;
&lt;/resources&gt;
</pre>

Run with an output pattern to split out the tagged namespace:
<pre>
llxml -outFmt=%p-test1/%f strings.xml
</pre>

Writes the entries between the `test1` BEGIN/END markers to `strings.xml`
under a new `-test1` sibling directory.

Visit home website

[https://landenlabs.com](https://landenlabs.com)


Help Banner:
<pre>
llxml  Dennis Lang v1.2 (landenlabs.com) Nov 26 2024

Des: Xml parse and ??
Use: llxml [options] directories...   or  files

 Options (only unique characters required, can be repeated, case ignored):
   -fileInclude=<filePattern>
   -fileExclude=<filePattern>
   -pathInclude=<pathPattern>
   -pathExclude=<pathPattern>
   -showInput
   -verbose
   -outFmt=%p-AA/%f

 Example:
   llxml -inc=\*xml -excludePath=\*value-\*
   llxml main1.xml dir2/main2.xml , child1.xml child2.xml

 Example input xml:
    <?xml version="1.0" encoding="utf-8"?>
    <!-- comment-->
    <resources>
        <string name="language" translatable="false">English</string>
        <!-- comment -->
        <string name="word1">Your Drive</string>
        <string name="word2">Radar</string>
    </resources>

   Output:
    Files written per -outFmt pattern for each BEGIN/END NAMESPACE region found
</pre>

### License

```
Copyright 2026 Dennis Lang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
See [LICENSE.txt](LICENSE.txt) for the full license text.
