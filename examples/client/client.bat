@set IP=127.0.0.1
@set PORT=4567
@set CLIENT=10000
@set THREAD=1
@set MSG=10
@set SLEEP=100

@cd ../bin/Win32/Release
client IP=%IP% PORT=%PORT% CLIENT=%CLIENT% THREAD=%THREAD% MSG=%MSG% SLEEP=%SLEEP%

@pause