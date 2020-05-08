#ifndef __WEBI_H__
#define __WEBI_H__

const char *index_cont = "\
<!DOCTYPE html>\
<html lang='cz'>\
<head>\
    <meta charset='utf-8'>\
    <title>scale</title>\
    <script src=\"jquery-3.5.1.min.js\"></script>\
    <script>\
        function load() {\
            var url = \"http://192.168.42.1/data.json\";\
            var jqxhr = $.getJSON( url, function( data ) {\
                $(\"#raw\").text(data.raw);\
            })\
        }\
\
        var timer = window.setInterval(doSometimes, 500);\
\
        function doSometimes() {\
          load();\
        }\
    </script>\
</head>\
<body onload=\"load();\">\
    <h1>Kegator</h1>\
    <p>Raw data: <span id='raw'>---</span></p>\
</body>\
</html>\
\0";

#endif __WEBI_H__
