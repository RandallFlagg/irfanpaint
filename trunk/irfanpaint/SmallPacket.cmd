@ECHO OFF
PATH %path%;c:\Program Files (x86)\IZArc
COPY Release_Speed\IrfanPaint.dll .\Paint.dll
DEL Paint.zip
izarcc -a "%cd%\Paint.zip" Paint.dll
DEL Paint.dll
PAUSE 