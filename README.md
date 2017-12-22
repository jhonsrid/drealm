# drealm
GPL sources for the v2.1a drealm UNIX BBS, http://groups.yahoo.com/group/drealmbbs

Slightly tweaked to build on modern Linux with a C99 compiler. Release mode (ie only default GCC warnings), now builds cleanly. Basic changes were:

* "restrict" is a keyword in C99
* Assume all linuxes now have unix domain datagrams
* Use sigemptyset() rather than "= 0" for signal sets
* Some casts
* int / size_t differences
* execlp argument list termination
* Instances of printf(Ustring[123]) were changed to printf("%s", Ustring[123]). (No changes made where already more than one argument)

## TODO 
* Tackle -Wall or at least a reasonable subset of it.
* Replace use of non-POSIX "CBAUD"
* Replace use of XOPEN_SOURCE
* Build on MacOS


