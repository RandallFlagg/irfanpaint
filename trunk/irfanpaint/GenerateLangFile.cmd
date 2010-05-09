@echo off
CALL "%VS71COMNTOOLS%\vsvars32.bat"
cl /D "RC_INVOKED" /E IrfanPaint.rc | IPLFGen\bin\Release\IPLFGen > IP_English.lng
