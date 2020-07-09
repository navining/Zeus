@set IP=127.0.0.1
@set PORT=4567
@set CLIENT=1000
@set THREAD=2

@cd ../bin/Win32/Release
client IP=%IP% PORT=%PORT% CLIENT=%CLIENT% THREAD=%THREAD%

@pause