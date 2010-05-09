@ECHO OFF
PATH %path%;C:\Programmi\IZArc
COPY Release_Speed\IrfanPaint.dll .\Paint.dll
DEL Paint.zip
izarcc -a "%cd%\Paint.zip" Paint.dll
DEL Paint.dll
PAUSE 