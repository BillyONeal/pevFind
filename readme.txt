pevFind is under the BSL, for it's code. As this file is documentatoin, the
BSL is not appropriate. Therefore, I am releasing this file in the package
under the Creative Commons Attribution 3.0 Unported license. For more info,
see: http://creativecommons.org/licenses/by/3.0/

Note that pevFind depends on CryptoPP (under Public Domain) for it's hashing
functions, InfoZIP (under InfoZip's license) for it's zipping functions, and
more generally, Boost, which is also under the BSL. Unit tests depend on
Google Mock and Google Test, which are Google's property.

Before copying a file of this distribution into a commercial work, be sure to check the license of the file you are working with. Each file has the license and copyright holder listed a the top. :)

pevFind <subprogram> <options>

pevFind is a combination of an enhanced vFind and other tools.
    The first argument is the name of the subprogram inside of pevFind you wish to invoke.
    If no sub-program is specified, it will default to vFind behavior.
    
    
#### Subprogram: vFind  ##################################################################

  --c
  --custom  Use a custom line output format
    #1 = SHA-1
    #2 = SHA-224
    #3 = SHA-256
    #4 = SHA-384
    #5 = MD5
    #6 = SHA-512
    #7 = file is PE+ (if yes, then will be 7)
    #8 = file (8dot3 filename)
    ## = literal #
    #a = access time
    #b = tab
    #c = creation time
    #d = company
    #e = description
    #f = file
    #g = version
    #h = pe header time
    #i = product name
    #j = copyright
    #k = original file name
    #l = Trademark
    #m = modified time
    #n = newline
    #o = Internal Name
    #p = PE information
        123456 
        1      Is pe file
         2     Has debug info
          3    Signed
           4   DLL
            5  Checksum valid in pe header. Valid will be considered NULL for
               no checksum, or equal to the calculated checksum.
             6 Header time inside the PE file is valid
    #q = Comments
    #r = Private Build
    #s = size
    #t = types
    #u = unpadded size
    #v = PE signature is valid. Prints 7 for valid or - for not. There can be
        a valid signature even if the file even if the file contains no signature.
        This is because when checking for validity pevFind will also look in
        windows' catalogs for a matching hash for the file in question.
    #w = File is protected by windows file protection. 8 for protected, - for not.
    #x = Special Build
    #y = Header checksum in hex. Will be ten characters, 0xAAAAAAAA
    #z = Calculated checksum in hex. Will be ten characters, 0xAAAAAAAA
    Example: --custom:##c . #m #u #t #F# Will print something like 
    2008-01-01 17:01 . 2000-08-31 08:00 51200 --a------ C:\WINDOWS\NirCmd.exe
    The string must start with # and end with # -- this is what marks the bounds
    of the argument. Therefore, you DON'T NEED QUOTES when using --custom.

  --debug
    Displays debugging output.

  -e Display PE information -- This is the same as --custom:#[#p] #f#
  --peinfo
  
  --enable-filesystem-redirector-64 Enable WOW64's filesystem redirection functions.
  --fs32
  
  -ea Choose encoding (default)
  -eo
  -eu8
  -eu16
  --encodingACP
  --encodingOEM
  --encodingutf8
  --encodingutf16  (Note that windows doesn't take anything outside the Basic
  Multilingual Plane; Therefore this option could also correctly be called UCS-2.)
  
  Also note that when output is not redirected to a file, encoding has no effect --
  WriteConsole will output visible console characters no matter what the codepage, etc.
  Windows handles that automaticly.
  
  -ex
  --expand Expand all vFind regex directories to remove ~ pseudo-directories.
  
  -f  Show the full path, rather than the relative path
  --full
    Most of the time this command will have no effect. Whenever you prefix a regex
    with a path, the search will use that path. I don't like vFind's behavior on
    this... I don't think relative paths should ever be printed. However, because
    I intend this to be a drop in replacement, I had to replicate it's behavior.
    Using -f activates what I believe is the sane way -- it will always print the
    full path. If not using -f and using relative paths, do not use more than one
    regex. Such results are undefined. Because without this directive is designed
    soley to preserve compatability with vFind, and vFind supports only one regex
    at a time.
  
  --filelook Shows more information about the files. Same as --custom#---- #f ----
  #nCompany: #d#nFile Description: #e#nFile Version: #g#nProduct Name: #h#nCopyright:
  #i#nOriginal file name: #j#nFile Size: #u#nCreated Time: #c#nModified Time: #m#n
  Accessed Time: #a#nMD5: #5#nSHA1: #1#
  
  --files[:]["]path["]
  Example usage: pevFind -tp --filestemp00
  Example usage: pevFind -tf --files:temp00
  Reads newline delimited list of files from the file specified. When used by
  itself, this will be the only files listed. vFind regular expressions may be
  used as filters, but no directory enumeration will be performed when this switch
  is present. Only files which match the regex will be printed.
  Example usage: pevFind --peinfo --files"C:\Program Files\temp00" *.exe
  You may use it more than once:
  Example usage:
  pevFind -tp -sd:mdate --files:temp00 --filestemp02 --files"C:\Program Files\temp00"
  Multiple uses will simply add the files together.
  
  -k PEV Kill mode
    Searches for processes that match PEV's tree, and silently terminates them.
  
  -l  Use long listing matching vFind's L command.
  --long   This is the same as --custom:##t #s #m #f#
  
  -loadline[:]["]file["]
  Loads commands from file and replaces that in the command line stream.
  
  --limit[:]XX Limit count to XX items
  
  -m  Use short DOS filenames (Same as --custom:##8#)
  --short
  
  -md5[:]["]<<HASH>>["]
  Tests if file's MD5 matches "hash"
  
  -md5list[:]["]File["]
  -md5elist[:]["]File["]
  Loads a newline delimited list of MD5s from File to test
  The e list variant will return true on errors.
  
  -nrvf#regex#
  Creates a non-recursive VFindRegex object. This is for when you want a single
  regex to have non-recursive properties, as opposed to the -r switch, which
  changes the entire mode for pevFind.
  If it makes it any easier to remember, it stands for "Non Recursive Vfind Regex"
  
  -n  Print summary
  --summary
  
  -output[:]["]<FILE>["]
  Directs output to <FILE> rather than stdout.
  
  -preg#<REGEX>#
  Returns true when the file in question matches the specified perl
  regex (filename). Note that at least one vFind regex (or a use of
  the -files directive) is required to limit the scope of the search.
  
  -r  Disable recursion (Do not search subdirectories)
  --norecursion
  
  -sa Sort ascending by    NOTE: Default is UNSORTED!
    SIZE
    DATE (defaults to modified)
    ADATE Access Date
    MDATE Modified Date
    CDATE Created Date
    HDATE PE Header Date
    NAME
    INAME Case insensitive name sort. (Over 2x time of standard name sort)
    
  -sd Sort descending by
    SIZE
    DATE (defaults to modified)
    ADATE Access Date
    MDATE Modified Date
    CDATE Created Date
    HDATE PE Header Date
    NAME
    INAME Case insensitive name sort. (Over 2x time of standard name sort)
  
  Multiple sort commands will result in a lower order of sort. For example,
  to group items by size and then break ties by sorting by date, use
  something like -sa:size -sa:date. The order obtained should be the order
  in which the commmands are listed, if the sort commands are separated by
  "intersting" items, such as AND, OR, XOR, etc, the results of the sort
  are undefined.
  
  When a sort is performed, all entries which match the commandline's
  criteria will be copied into RAM. The entire sort operation is preformed
  in RAM, and will take at most n log n comparisons, where n is the number
  of found results. If running an excessively large sort, such as -tf C:\*,
  pevFind will not crash -- the standard list container it uses can handle
  more than four billion records -- but it will require quite a large amount
  of RAM while running. This should be fine, as this data is allowed to
  be cached by the Pagefile, but it would be wise to avoid using the
  sort methods if the results are expected to be exceedingly large.
  
  -sha1[:]["]<<HASH>>["]
  Tests if file's SHA1 matches "hash"
  
  -sha1list[:]["]File["]
  -sha1elist[:]["]File["]
  Loads a newline delimited list of SHA1s from File to test
  The elist variant will return true on error.

  -skip[:]"<path>"
  Directs pevFind to not enter <path> when calculating results.
  
  --tx
  --timeout Timeout after x number of ms.
  When this switch is present, pevFind starts a second thread which simply
  goes to sleep for the required time. After this time, the thread will wake up,
  and attempt to ask the main thread to cancel the search. The main thread will
  be given an additional 100ms in which to write the results to the resultant
  buffer. The errorlevel will be set to 1 in this case -- and the state of the
  resultant log will be incomplete, but otherwise valid. If the main thread is
  unable to finish in the additional 100ms, the sub-thread will terminate the
  program. In this case, errorlevel will be set to 2.

  -zip"filename"
  Entire contents of pevFind's file search are zipped into "filename"
  The shortest relative path which can contain all the files found will be used
  internally in the zip.

  SomeVFindRegex
    Any unknown sequence which has no - will be interpreted as a vFind regex.
    
  -t[:][!]... which is a type filter (Same as vfind)
        a Archive
        b 64 Bit PE file (PE+ format)
        c Compressed
        d Directory
        e Reparse Point (Such as a symlink on vista)
        f File
        g Signature is Valid. Note that this check relies on cryptographic services.
        h Hidden
        i Signature Present. Implies P.
        j PE Header Time is Valid (Valid is indicated as older than both the
            current system time, as well as the filesystem's modification
            time for the file)
        k Checksum Valid
        l DLL
        o SFC Protected
        p Valid PE File
        pMZ MZ header check valid executable (Possible DOS program)
        pLE LE format executable
        pNE NE format executable
        p2 Any format of executable where at least 2 magic numbers are valid. That is, PE, NE, and LE executables
        p64 PE+ (64 bit) executables
        r Read only
        s System
        t Temporary
        v Volume label
        w Writable
  
  -s[:][+|G|-|L|!|=] size above | below | not | equals
  
  -s[:]NUMBER-NUMBER size range
  
  -d[c|m|a][:][+|-|!|=] Date after / before / not / equals
  Default is modified, but c will use created and a will use access date.
  
  -d[c|m|a][:][G|L] Date more than x days ago or less than x days ago.
  Default is days (D) but supported suffixes are S(econds), MM(inutes), H(ours), D(ays), W(eeks), M(onths), Y(ears)
  At this point leap years aren't handled correctly but I don't think that's a
  major problem. When calculating this date, pevFind will use the UTC time and subtract
  the specified length from it. It will then pivot on that date as greater than or less than.
  
  Note: Date ranges are not supported because - specifies a range, but - s are used
  in the way dates are specified. Supporting - range would result in ambiguous
  cases. If you need a date range, use more than one -d argument.
  
  If you are using a absolute date which contains a space, you have an alternate method of
  specifying quotes which is a bit more readable. Instead of
  "-d+2008-01-02 11:32"
  you can put the quote after the +, -, !, =, G, or L, like so:
  -d+"2008-01-02 11:32"
  I find this quite a bit more readable.
  
  not
    Inverts the following directives
    
  and
    ANDs the following directives
  
  or
    ORs the following directives
    
  xor
    XORs the following directives
    
  Brackets -- That is, { }

  Order of operations:  Brackets {}
                        AND
                        OR
                        XOR
                        Everything else
                        
  Expression = andand AND andand
  AndAnd = orand OR orand
  OrAnd = xorand XOR xorand
  XorAnd = (Anything Else)* or  { expression }  or NOT expression.

pevFind will attempt to be smart about inserting quotes, space,
and -s in the right places. (For example, pevfind -ltf vfindregex will work just
fine) However, for best results, place a space between each commandline option.

A note about UNICODE:
pevFind will output data in ACP strings by default. Things are handled as UTF-16 internally, but converted before things are written to the file.

Errorlevels:
0 = Completed sucessfully, files found.
1 = Timeout was reached, but system was able to gracefully write results
2 = Timeout was reached, and program force-terminated. Discard the results (May be partially written)
3 = Commandline syntax error
4 = Completed sucessfully, no files were returned.

#6### Subprogram: VOLUME  #################################################################

This simple argumentless sub-program lists all the fixed drives on the system.
Only drives which are FIXED will be enumerated.
The GetDriveType function:
http://msdn.microsoft.com/en-us/library/aa364939(VS.85).aspx
is used to determine this. All other types are filtered.

#### Subprogram: CLSID Compressor  #######################################################

pevFind.exe CLSID <OPERATON> <INFILE> <OUTFILE>
Operations are:
C: Compress
 Compresses the ANSI or Unicode textfile containing CLSIDs into binary format.

D: Decompress
 Decompresses the binary format CLSIDs into an ANSI textfile. (Uses the ACP codepage)

DA: Decompress Append
 Decompresses the binary format CLSIDS into an ANSI textfile, appending to the existing file.

#### Subprogram: Time  ###################################################################

pevFind.exe TIME <UTC>
Displays the current time. If the argument UTC is specified, displays time in UTC.

#### Subprogram: EXEC  ###################################################################

pevFind.exe EXEC [/w|/s|/e] <COMMANDLINE>
Executes the <COMMANDLINE> following in a hidden window. Use -w or /w or -W or /W to wait
for the application to terminate.

/S will start the line using the LocalSystem account.

/E will start the line using a new environment. This cannot be used with /s.

#### Subprogram: UZIP  ###################################################################

pevFind.exe UZIP [/D] <INFILE> [<OUTDIR>]

Unzips <INFILE> into <OUTDIR>. If <OUTDIR> is not in the commandline, the contents of the zip file
are printed, rather than the file actually being inflated.

Directories are normally stripped from the zipfile (same as unzip.exe -j). To use the zip's
directory structure, supply the /D argument.

#### Subprogram: MOVEEX  #################################################################

pevFind.exe MOVEEX FILE [TARGET]

Copies file to target on reboot. If no target specified, file is deleted.

#### Subprogram: LINK  ###################################################################

pevFind.exe LINK FILE TARGET

Creates a hard filesystem link linking file to target. File is the already existing file,
target is to where the hard link is created.

#### Subprogram: PLIST  ##################################################################

Lists executable paths of all processes on the system.

#### Subprogram: CLIST  ##################################################################

Lists commandlines of all processes on the system.

#### Subprogram: SC  #####################################################################

CREATE: 
    NAME BINPATH TYPE START [DISPLAYNAME]
    Note that type and start take numbers, not the friendly names. Look up the values
    here: http://msdn.microsoft.com/en-us/library/ms682450(VS.85).aspx
    
DELETE:
    NAME
    
STOP:
    NAME

START:
    NAME
    
#### Subprogram: RIMPORT  ################################################################

pevFind RIMPORT [ LOOSE ] <FILE>
Where <FILE> is a regscript to be merged with the registry. If errors or warnings are
found in the regscript it will not be merged. Use LOOSE as the first option to force
the file to be imported even in presence of errors/warnings.

#### Subprogram: DDEV  ###################################################################

DDEV is a clone of Microsoft's DOSDEV.EXE, described at this location:
      http://blogs.msdn.com/adioltean/archive/2005/10/04/477164.aspx
The switches are exactly the same. There are a few extensions:

-N switch: DDD_NO_BROADCAST_SYSTEM flag is used when DefineDosDevice is called.

#### Subprogram: LINKRESOLVE #############################################################

Resolves shell link files entered as input. When one file is entered, then the simple path
of the file will be printed to the console. If more than one file is entered, then
they all will be listed using the format "SOURCE -> TARGET".

#### Subprogram: WAIT ####################################################################

Waits the specified number of milliseconds.
