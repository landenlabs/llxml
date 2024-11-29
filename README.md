#llxml
OSX / Linux / DOS  Parse XML

### TODO - add notes here

  [![Build status](https://travis-ci.org/landenlabs/llxml.svg?branch=master)](https://travis-ci.org/landenlabs/llxml)


This input json file:

<pre>

</pre>

Is converted to CSV columns as:
<pre>

</pre>

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
    TODO - add notes here
</pre>
