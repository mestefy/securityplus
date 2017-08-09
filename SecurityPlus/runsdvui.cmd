cd /d "D:\Work\SecurityPlus\SecurityPlus" & msbuild "SecurityPlus.vcxproj" /t:sdv /p:inputs="/devenv /clean" /p:configuration="Release" /p:platform="x64" 
exit %errorlevel% 