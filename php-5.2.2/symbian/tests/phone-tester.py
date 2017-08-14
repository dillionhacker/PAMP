import e32
import os
import os.path
import re

"""Tool for running PHP tests (*.phpt) on a phone.
Modified from a remote tester tool in Nokia's PHP 5.1.4 port
by Mikko Heikelä.

The rest of this comment is outdated.

This is for advanced users only! You should understand
some Python to be able to run the tests.

Instructions:

* You'll need to copy the php.ini file from path tests/ to
  /Data/Apache/conf/php5/ on your phone.

* Then copy the contents of tests/phptest/ to /Data/Apache/htdocs/phptest/
  The most important here is remote_run.php (and _htaccess). Other files
  are needed by some tests.

* You should then be able to browse to
  http://my.site/phptest/remote_run.php
  Try it. You can run any code there by typing it to a form.
  It's very helpful in module development (and as you might guess,
  potentially very dangerous if not protected).

* Scroll a bit downwards in this file, you should see a line like this:
  remote_run = 'http://my.site/phptest/remote_run.php'
  Edit it to contain the correct address.

* Finally, scroll down to the very bottom to see
  how to invoke the tests of whatever module you want.
  You should configure the variable "root" there.


Some tests may require you to copy some extra files (usually *.inc)
from the path of the phpt-file to the path of the remote_run.php.
But tests/phptest/ already contains most (if not all) of these.
"""


#########################################################
# Configuration:
#########################################################

php_test_directories = [
                         r'C:\Data\PHP\tests\ext-gd'  #,
#                         r'C:\Data\PHP\tests\basic', 
#                         r'C:\Data\PHP\tests\func', 
#                         r'C:\Data\PHP\tests\classes', 
#                         r'C:\Data\PHP\tests\strings',
#                         r'C:\Data\PHP\tests\ext-sapi-cli',
#                         r'C:\Data\PHP\tests\ext-standard\assert',
#                         r'C:\Data\PHP\tests\ext-standard\strings',
#                         r'C:\Data\PHP\tests\ext-standard\file',
#                         r'C:\Data\PHP\tests\ext-standard\general_functions', 
#                         r'C:\Data\PHP\tests\ext-standard\math',
#                         r'C:\Data\PHP\tests\ext-standard\serialize',
#                         r'C:\Data\PHP\tests\ext-standard\time',
#                         r'C:\Data\PHP\tests\ext-standard\reg',
#                         r'C:\Data\PHP\tests\ext-standard\array',
#                         r'C:\Data\PHP\tests\ext-standard\filters',
#                         r'C:\Data\PHP\tests\ext-standard\image',
#                         r'C:\Data\PHP\tests\ext-standard\network',
#                         r'C:\Data\PHP\tests\ext-standard\versioning',
#                         r'C:\Data\PHP\tests\Zend',
#                         r'C:\Data\PHP\tests\lang',
#                         r'C:\Data\PHP\tests\run-test'
                        ]
php_exe = r'C:\sys\bin\php.exe'
workdir = r'C:\Data\PHP\tests'

php_output = r'C:\Data\PHP\php_output.txt'
zend_log = r'C:\Data\PHP\zend_log.txt'
python_output = workdir + r'\php_tester_output.txt'
python_log = workdir + r'\php_tester_log.txt'
output_file = open( python_output, 'w' )
log_file = open( python_log, 'w' )

MAX_PHP_OUTPUT_SIZE = 512 * 1024
PHP_OUTPUT_NOT_FOUND_MESSAGE = 'PHP output "' + php_output + '" not found.'

#########################################################
#########################################################

def runPhp(code, code_file_path):
    """Writes PHP code to a file, and runs the resulting script.

    php_output -- global variable that contains the output path
    of the CLI PHP.

    code -- the PHP code to run
    code_file_path -- the path to write the script to
    """
    
    code_file = open( code_file_path, 'w' )
    try:
        code_file.write( code )
    finally:
        code_file.close()
    
    e32.start_exe( php_exe, ' -f ' + code_file_path + r' -c C:\Data\PHP\php-cli.ini', True )

    def save_zend_log ():
        if os.path.isfile( zend_log ):
            e32.file_copy( code_file_path[:-4] + "_zend_log.txt", zend_log )
            os.remove( zend_log )

    if not os.path.isfile( php_output ):
        save_zend_log()
        return PHP_OUTPUT_NOT_FOUND_MESSAGE
    php_output_size = os.path.getsize( php_output )
    if php_output_size > MAX_PHP_OUTPUT_SIZE:
        e32.file_copy( code_file_path[:-4] + "_output.txt", php_output )
        save_zend_log()
        return 'PHP output "' + php_output + '" too big: '+ str( php_output_size ) + ' bytes.'
    try:
        php_output_file = open( php_output, 'r' )
        return php_output_file.read()
    finally:
        php_output_file.close()
        e32.file_copy( code_file_path[:-4] + "_output.txt", php_output )
        os.remove( code_file_path )
        save_zend_log()

class DiffError(RuntimeError):
    """Error indicating that expected and real output differ."""
    def __init__(self, real, expected):
        RuntimeError.__init__(self)
        self.expected = expected
        self.real = real

    def diff(self):
        # Don't have difflib in S60 python
        return "Expected:\n" + self.expected + "\nReal:\n" + self.real
#        import difflib
#        return '\n'.join(difflib.context_diff(self.expected.split('\n'), self.real.split('\n')))


class DiffError2(DiffError):
    """Error indicating that expected and real output differ."""
    def __init__(self, diffs):
        RuntimeError.__init__(self)
        self.diffs = diffs

    def diff(self):
        return '\n\n'.join(self.diffs)

class SkipError(RuntimeError):
    """Error indicating that the test case should be skipped."""
    pass


def testOk(real, sections):
    """Test that real output is same as expected output.
    Returns None for OK, may raise DiffError.

    real -- real output from PHP.
    sections -- dictionary containing expected values such as
                sections['EXPECT'] or sections['EXPECTF']
    """
    log_file.write('testOk entered\n')
    log_file.flush()
    tested = False
    real = real.strip().replace('\r\n', '\n')

    if 'EXPECT' in sections:
        tested = True
        wanted = sections['EXPECT'].strip().replace('\r\n', '\n')
        if real != wanted:
            raise DiffError(real, wanted)

    if 'EXPECTF' in sections or 'EXPECTREGEX' in sections:
        tested = True
        if 'EXPECTREGEX' in sections:
            wanted = sections['EXPECTREGEX']
        else:
            wanted = sections['EXPECTF']
        wanted_re = wanted.strip().replace('\r\n', '\n')
        if 'EXPECTF' in sections:
            wanted_re = re.sub(r'\%s[^ \n\r]*\.php', '%s', wanted_re)
            wanted_re = re.escape(wanted_re).replace('\\%', '%').replace('\\\n', '\n')
            wanted_re = wanted_re.replace("%s", ".+?")
            wanted_re = wanted_re.replace("%i", "[+\-]?[0-9]+")
            wanted_re = wanted_re.replace("%d", "[0-9]+",)
            wanted_re = wanted_re.replace("%x", "[0-9a-fA-F]+")
            wanted_re = wanted_re.replace("%f", "[+\-]?\.?[0-9]+\.?[0-9]*(E-?[0-9]+)?")
            wanted_re = wanted_re.replace("%c", ".")

        if len(real.split('\n')) != len(wanted_re.split('\n')):
            raise DiffError(real, wanted)

        err = []
        for line, line_wanted_re, line_wanted in zip(real.split('\n'), wanted_re.split('\n'), wanted.split('\n')):
            if not re.match(r'^'+line_wanted_re+r'$', line):
                err.append('%s\n%s' % (repr(line_wanted), repr(line)))
        if err:
#            file('out1', 'w').write(real)
#            file('out2', 'w').write(wanted)
            raise DiffError2(err)

#        rematcher = re.compile(r'^'+wanted_re+r'\n*$', re.DOTALL)
#         if not re.match(r'^'+wanted_re+r'\n*$', real):
#        if not rematcher.match(real):
# #             print wanted_re[-200:]
# #             print real[-200:]
# #             print repr(real[:20])
# #             print repr(wanted_re[:12])
# #             print re.match(wanted_re[:12], real[:20])
#            raise DiffError(real, wanted)
        log_file.write('testOk exit\n')
        log_file.flush()

    if not tested:
        log_file.write('testOk throw No expected value\n')
        log_file.flush()
        raise RuntimeError("No expected value")

_sectionSplit = re.compile('^--([A-Z]+)--[ \t\n\r]+', re.M)

def parseSections(data):
    """Parses the sections from a phpt file and returns a dictionary
    mapping section name to section contents."""
    pro = _sectionSplit.split(data)
    sections = {}
    for x in xrange(1, len(pro), 2):
        sections[pro[x]] = pro[x+1]
    return sections

findIni = re.compile('^([^=]+)=(.*)$', re.M)
findEka1Crash = re.compile(r'[^a-zA-Z_](INF|NAN|number_format)[^a-zA-Z_]')

def runTest(testFile, skipEka1Crash=False):
    """Runs given test file.

    testFile -- path to phpt file.
    """
    log_file.write('About to read ' + testFile + '\n')
    log_file.flush()
    data = file(testFile).read()
    log_file.write('read ' + testFile + '.  About to parseSections\n')
    log_file.flush()
    sections = parseSections(data)
    log_file.write('sections parsed\n')
    log_file.flush()
    for s in sections:
        assert s in ('TEST', 'EXPECT', 'EXPECTREGEX', 'EXPECTF', 'FILE', 'INI', 'SKIPIF', 'POST', 'GET', 'ARGS', 'FILEEOF', 'COOKIE', 'UEXPECT', 'UEXPECTF', 'UEXPECTREGEX', 'REDIRECTTEST' )

    if 'REDIRECTTEST' in sections:
        raise SkipError('requires REDIRECTTEST')
    if 'COOKIE' in sections:
        raise SkipError('requires COOKIE')
    if 'ARGS' in sections:
        raise SkipError('requires ARGS')

    if 'INI' in sections:
        raise SkipError('Skipping all that contain INI')

#    if 'INI' in sections and 'memory_limit' in sections['INI']:
#        raise SkipError('memory limit!')

    if 'SKIPIF' in sections:
#        raise SkipError('Skipping all that contain SKIPIF')
        skipTest = sections['SKIPIF']
        try:
            skipTestOutput = runPhp( skipTest, testFile[:-5] + "_skipif.php" )
        except Exception, e:
            raise SkipError('exception: ' + str(e) + ' in SKIPIF test.')
        if skipTestOutput[:4] == 'skip':
            raise SkipError(skipTestOutput[4:])
        if skipTestOutput == PHP_OUTPUT_NOT_FOUND_MESSAGE:
            raise SkipError("Skip test output not found.")

#     if 'SKIPIF' in sections and '("mbstring")' in sections['SKIPIF']:
#         raise SkipError('no mbstring extension')

    code = sections.get('FILE', sections.get('FILEEOF'))
    code = re.sub("['\"][^'\"]*quicktester.inc['\"]", '"/quicktester.inc"', code)

#     if 'symlink(' in code: raise SkipError('symlink')
#     if 'passthru(' in code and not 'fpassthru(' in code: raise SkipError('passthru')
#     if 'exec(' in code: raise SkipError('exec')
#     if 'proc_open(' in code: raise SkipError('proc_open')
#     if 'iconv(' in code: raise SkipError('iconv')
#     if skipEka1Crash and findEka1Crash.search(data): raise SkipError('EKA1 skip')

#     # Parse INI section. For each line containing a=b in ini section,
#     # add ini_set("a", "b") to the beginning of the code
#     inis = 'ini_set("log_errors", 0); '
#     if 'INI' in sections:
#         for line in findIni.findall(sections['INI']):
#             inis += 'ini_set("%s", "%s"); ' % line

#    code = '<?php %s ?>' % inis + code
    log_file.write('about to call runPhp()\n')
    log_file.flush()
    real = runPhp(code, testFile[:-1])
    log_file.write('runPhp returned\n')
    log_file.flush()
    #print real
    #tools.dataToFile('tmp', real)
    #real = tools.dataFromFile('tmp')
    
    # OOS: apache or connection crashed, no sense to carry on testing
#     if '>Out Of Site<' in real[:200]:
#         print 'OUT OF SITE'
#         raise RuntimeError('OUT OF SITE')
    testOk(real, sections)

def runAllTests(path, runAll=False, skipEka1Crash=False):
    """Runs all tests (*.phpt) from given path.

    Doesn't run passed tests again (uses the file "passed_tests" to store this)
    Except if runAll is True.

    If skipEka1Crash is True, then tests with words NAN, INF or number_format
    are skipped. Helpful for S60 version 2.x phones that crash with these.
    """
#    if not runAll:
#         try:
#             passed = tools.dataFromFile('passed_tests')
#         except IOError:
#             passed = {}
#         print len(passed), 'tests passed'
    passed = {}
    skipped = {}
    failed = {}
    output_file.write( "Running tests in " + path + "\n" )
    for f in os.listdir(path):
        if f[-4:] == 'phpt':
            if runAll or str(f) not in passed:
                try:
                    output_file.write( f + ": " )
                    runTest( os.path.join( path, f ),
                             skipEka1Crash=skipEka1Crash )
                except DiffError, e:
                    output_file.write( "** FAIL **\n" )
                    output_file.write( e.diff() + "\n" )
                    failed[str(f)] = 1
                except SkipError, e:
                    output_file.write( "** SKIP ** (%s)\n" % str(e) )
                    skipped[str(f)] = 1
                except Exception, e:
                    output_file.write( "Unknown exception (%s) from runTest\n" % str(e) )
                    output_file.flush()
                else:
                    output_file.write( "* OK *\n" )
                    passed[str(f)] = 1
                output_file.flush()
    output_file.write( "==================================================\n" )
    output_file.write( "Summary for tests in " + path + "\n" )
    output_file.write( "Passed (" + str(len(passed)) + "):\n" )
    for filename in passed.keys():
        output_file.write( filename + "\n" )
    output_file.write( "--------------------------------------------------\n" )
    output_file.write( "Failed (" + str(len(failed)) + "):\n" )
    for filename in failed.keys():
        output_file.write( filename + "\n" )
    output_file.write( "--------------------------------------------------\n" )
    output_file.write( "Skipped (" + str(len(skipped)) + "):\n" )
    for filename in skipped.keys():
        output_file.write( filename + "\n" )
    output_file.write( "==================================================\n" )
    output_file.flush()
#                        tools.dataToFile('passed_tests', passed)


def runSingleTest(filePath):
    """Runs a single test file and prints some info."""
    print filePath
    try:
        runTest(filePath)
        print 'OK'
    except DiffError, e:
        print e.diff()

###########################################################################
###########################################################################

# Old 5.1.4 port test info:

# The following tests have been ran:

#BASIC MODULES
# \ext\date\tests
# \ext\pcre\tests
# \ext\tokenizer\tests
# \ext\ctype\tests
# \ext\calendar\tests
# \ext\xml\tests
# \ext\session\tests
# \ext\gd\tests

#STANDARD LIBRARY
# \ext\standard\tests\array
# \ext\standard\tests\assert
# \ext\standard\tests\file
# \ext\standard\tests\filters
# \ext\standard\tests\general_functions
# \ext\standard\tests\image
# \ext\standard\tests\math
# \ext\standard\tests\network
# \ext\standard\tests\reg
# \ext\standard\tests\serialize
# \ext\standard\tests\strings
# \ext\standard\tests\time
# \ext\standard\tests\versioning

#ZEND
# \tests\basic
# \tests\classes
# \tests\func
# \tests\lang
# \tests\run-test
# \tests\strings



# Configure the root of your PHP installation here (without trailing \)
# e.g. root = r'C:\Symbian\8.0a\S60_2nd_FP2\mws\phone\httpd\php-5.1.4'

#root = r'C:\Symbian\8.0a\S60_2nd_FP2\mws\phone\httpd\php-5.1.4'
# root = r'C:\Symbian\9.1\S60_3rd\mws\phone\httpd\php-5.1.4'

# List of tests for Series 60 modules:

#tests = [
#     root + r'\ext\s60_inbox\tests\basic.phpt',
#     root + r'\ext\s60_e32db\tests\basic.phpt',
#     root + r'\ext\s60_e32db\tests\crash.phpt',
#     root + r'\ext\s60_e32db\tests\sprintf.phpt',
#     root + r'\ext\s60_e32db\tests\long.phpt',
#     root + r'\ext\s60_e32db\tests\types.phpt',
#     root + r'\ext\s60_e32db\tests\types2.phpt',
#     root + r'\ext\s60_e32db\tests\misc.phpt',
#     root + r'\ext\s60_e32db\tests\multiconn.phpt',
#     root + r'\ext\s60_e32db\tests\unset.phpt',
#     root + r'\ext\s60_sysinfo\tests\basic.phpt',
#     root + r'\ext\s60_contacts\tests\basic.phpt',         # run only on eka1 platform
#     root + r'\ext\s60_contacts\tests\fields.phpt',        # run only on eka1 platform
#     root + r'\ext\s60_contacts\tests\all_fields.phpt',    # run only on eka1 platform
#     root + r'\ext\s60_contacts\tests\remove.phpt',        # run only on eka1 platform
#     root + r'\ext\s60_contacts\tests\crash.phpt',         # run only on eka1 platform
#     root + r'\ext\s60_calendar\tests\simple.phpt',
#     root + r'\ext\s60_calendar\tests\many.phpt',
#     root + r'\ext\s60_calendar\tests\repeat.phpt',
#     root + r'\ext\s60_calendar\tests\todorepeat.phpt',
#     root + r'\ext\s60_calendar\tests\eka1todolists.phpt', # run only on eka1 platform
#     root + r'\ext\s60_calendar\tests\eka1todolists2.phpt', # run only on eka1 platform
#     root + r'\ext\s60_calendar\tests\eka1.phpt',          # run only on eka1 platform
#     root + r'\ext\s60_calendar\tests\eka2.phpt',          # run only on eka2 platform
#     root + r'\ext\s60_calendar\tests\eka2repeat.phpt',    # run only on eka2 platform
#     root + r'\ext\s60_calendar\tests\eka2todo.phpt',      # run only on eka2 platform
#    ]


# Use this to run Series 60 module tests from the list above
#for testFile in tests: runSingleTest(testFile)

# Use this to run all tests in some path
#runAllTests(root + r'\ext\pcre\tests')

for test_dir in php_test_directories:
    runAllTests( test_dir )
output_file.close()
log_file.close()
