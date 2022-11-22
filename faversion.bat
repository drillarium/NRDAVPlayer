@echo off
echo -----------------------------------------------
echo -   VECTOR 3
echo -   Archivo .BAT para crear versiones de NRDAVPlayer
echo -----------------------------------------------

set V3ExeFile=".\x64\Release\NRDAVPlayer.exe"
set V3PdbFile=".\x64\Release\NRDAVPlayer.pdb"

set V3BatPwd=%CD%
set V3SchemaDir=%V3BatPwd%\run
set V3LocalZipDir=%V3BatPwd%\Releases

:GET_VER
REM -- Use of GetExeVer.bat --
set V3GetVer=\Projectes\Scripts\GetExeVer.bat
if not exist %V3GetVer% (
echo.
echo **
echo ** Please, Check Out from SVN:
echo **    "%V3GetVer%" 
echo **
goto END
)

call %V3GetVer% %V3ExeFile%

if "%V3Version%" EQU "" (
echo.
echo **
echo ** Error getting version from "%V3ExeFile%"
echo **
goto END
)

:GET_MFORMATS_VER
set V3MFormatsSuffix=
echo.
if "%V3MFormatsVersion%" EQU "" (
echo * Please set current MFormats SDK version, usual vaules:
echo *  2.7.0.13138 ^(default^)
echo *  2.6.0.12514
echo *
echo * Suggestion: set V3MFormatsVersion environment variable for future releases
set V3MFormatsVer=2.7.0.13138
) else (
echo * Defaulting to V3MFormatsVersion=%V3MFormatsVersion%
set V3MFormatsVer=%V3MFormatsVersion%
)
echo.
set /p V3MFormatsVer= MFormats Version (%V3MFormatsVer%, 'S' to skip version): 
if "%V3MFormatsVer%"=="s" set V3MFormatsVer=S
if "%V3MFormatsVer%"=="S" (set V3MFormatsVer=
goto :SHOW_VER)
set V3MFormatsSuffix=.MFormats%V3MFormatsVer%

:SHOW_VER
echo -----------------------------------------------
echo - NRDAVPlayer %V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%
echo -----------------------------------------------
echo. 

rem :GEN_SCH
rem pushd %V3SchemaDir%
rem echo - Genera esquema: %CD%\NRDAVPlayerXmlSetup.XML
rem echo. 
rem pause
rem %V3BatPwd%\%V3ExeFile% -e -WriteSchema
rem popd

:MD_LOCAL_ZIP
if NOT EXIST "%V3LocalZipDir%" (
echo. 
echo - Crea "%V3LocalZipDir%"
echo.
pause
md "%V3LocalZipDir%"
)

:ZIP_EXE
echo. 
echo - Genera zips: NRDAVPlayer.%V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%[.pdb].zip
echo. 
pause

%V3MakeZip% %V3LocalZipDir%\NRDAVPlayer.%V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%.zip %V3ExeFile% %V3SchemaDir%\NRDAVPlayer.sample.ini
%V3MakeZip% %V3LocalZipDir%\NRDAVPlayer.%V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%.pdb.zip %V3PdbFile%

rem echo. 
rem echo - Obrir explorador a:
rem echo -    %V3AppZipsRoot%\NRDAVPlayer\
rem pause
rem explorer %V3AppZipsRoot%\NRDAVPlayer\

:COPY_ZIPS
set V3AppDir=%V3AppZipsRoot%\NRDAVPlayer
set V3ZipDir=%V3AppDir%\Bin
set V3PdbDir=%V3ZipDir%\_PDB_
echo. 
echo -----------------------------------------------
echo Copia els zips al servidor ( CTRL-C per abortar )
echo -----------------------------------------------
echo. 


if NOT EXIST "%V3AppDir%" (
echo. 
echo - Crea "%V3AppDir%"
echo.
pause
md "%V3AppDir%"
)


if NOT EXIST "%V3ZipDir%" (
echo. 
echo - Crea "%V3ZipDir%"
echo.
pause
md "%V3ZipDir%"
)


if NOT EXIST "%V3PdbDir%" (
echo. 
echo - Crea "%V3PdbDir%"
echo.
pause
md "%V3PdbDir%"
echo. 
)


echo **
echo ** APPLICATION = %V3AppDir%
echo ** INSTALL     = %V3ZipDir%
echo ** PDB         = %V3PdbDir%
echo **
echo. 
pause

copy %V3LocalZipDir%\NRDAVPlayer.%V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%.zip		%V3ZipDir%
copy %V3LocalZipDir%\NRDAVPlayer.%V3Version%.%V3Revision%.r%V3Release%%V3Build%%V3MFormatsSuffix%.pdb.zip 	%V3PdbDir%
copy NRDAVPlayer.README.txt									%V3AppDir%

:END
pause
