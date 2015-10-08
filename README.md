# Evil Islands DataBase Editor

## Files description

**Main program**
* DBEditor.exe

**Examples**
* database.xlsx    -- example of Single Player Database file in XLSX format
* databaselmp.xlsx -- example of LAN Multi Player Database file in XLSX format

**Configuration files**
* dbfiles.txt      -- database files descriptor
* dbblocks.txt     -- database blocks descriptor
* dbtypes.txt      -- database block fields types descriptor
* dbheaders.txt    -- database block fields names headers descriptor
* dbinits.zip      -- database initial data archive
* icons            -- icons folder (it is used in dbfiles.txt and dbblocks.txt)

**Settings**
* settings.ini     -- settings file (see section #3 "Settings file")

## Command line options

This program works in two main modes: user interaction mode and command line mode.

If program is launched without any command line options, so program starts in user interaction mode (with main window and data tables).
If program is launched with command line options, so program starts in command line mode without any main window and without any data tables.

In current version there is only two commands is allowed:

a) DBEditor.exe path\to\filename.res
Program just convert path\to\filename.res to path\to\filename.xlsx

b) DBEditor.exe path\to\filename.xlsx
Program just convert path\to\filename.xlsx to path\to\filename.res

This program is not console program, so while running it creates gui thread and returns to console simultaneously. To wait the end of program processing use this MS Windows cmd method:

start /wait DBEditor.exe path\to\filename.res

In this example process will wait until dbeditor.exe done conversion.

## Settings file

Settings file is generated automatically and not necessary needed.

This is list of options that can be changed only manually:

### Data codepage

Data codepage option used to define the codepage of any text data stored in database fields.
Usually for Evil Islands it is "Windows-1251"

Example of usage (write it in settings.ini file):
[Main]
DataCodePage=Windows-1251

### Console silent mode

This option is used in command line options only.
If no command options passed to running program, then this option is ignored.

Set this option to "true" to have no gui windows while program running with command options.
Set this option to "false" to show progress bars while program running with command options.

Example of usage. Set silent mode to "true" (write it in settings.ini file):
[Console]
Silent=true

## Some comments about .RES writing

If XLSX file is loaded firstly and than saved to RES file, then RES file NOT be overwritten at whole. The target RES file will be unpacked, only *db files will be overwriteen, and new data will be packed to final RES file. So it keeps original unsupported content of target RES archive.

It *db files is loaded firstly and than saved to RES files, then it works same as in previos case (unpack RES -> overwrite *db files -> pack RES).

If RES file is loaded firstly, then program keeps in memory all original files from source RES archive. Also it indicates the saved source RES file in program title.
So saving RES archive after that is works with follow steps: 1. unpack source RES file contents; 2. rewrite *db files to new; 3. pack new RES file contents to overwrite source RES file.

