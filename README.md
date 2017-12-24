# drealm
GPL sources for the v2.1b drealm UNIX BBS, http://groups.yahoo.com/group/drealmbbs

## To build:
Read README
(But basically it's just "make" to build).

### Dependencies
Drealm needs gcc, GNU make and ncurses. Only tried building on Linux, (Mint 18.3).

## Change history

2017-12-24:
Added relevant contents of dlm21b.diff file to bring version up to v2.1b

2017-12-22: 
Slightly tweaked to build on modern Linux with a C99 compiler. Release mode (ie only default GCC warnings), now builds cleanly. Basic changes were:

* "restrict" is a keyword in C99
* Assume all linuxes now have unix domain datagrams
* Use sigemptyset() rather than "= 0" for signal sets
* Some casts
* int / size_t differences
* execlp argument list termination
* Instances of printf(Ustring[123]) were changed to printf("%s", Ustring[123]). (No changes made where already more than one argument)

