@ECHO OFF
ECHO Copia del plugin...
PATH %path%;C:\Programmi\IZArc
COPY ..\Release_Speed\IrfanPaint.dll IVDir\Plugins\Paint.dll
SET /P vn="Inserisci il numero di versione o Invio per uscire: "
IF "%vn%"=="" GOTO :EOF
cd IVDir
SET packetname="%cd%\..\IrfanPaint_%vn%_bin+IV.zip"
if exist %packetname% del %packetname%
izarcc -a -Pr %packetname% i_view32.exe Plugins
PAUSE
