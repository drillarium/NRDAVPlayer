echo off
echo -----------------------------------------------
echo -   VECTOR 3
echo -   Arxiu .BAT per a accedir a la carpeta de versions del NRDAVPlayer
echo -   Data : 21.11.2014
echo -----------------------------------------------

rem set V3Version=5
rem set V3Revision=01
rem set V3VerRev=%V3Version%.%V3Revision%

set V3ZipDir=\\Together3\DADES\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Bin
echo. 
echo. 
echo -
echo - Obrir explorador a:
echo -    %V3ZipDir%
echo -

pause
explorer "%V3ZipDir%"
