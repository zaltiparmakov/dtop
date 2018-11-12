C program which checks basic system info, lists processes, and lists open file descriptor of processes.

Za tisti program se uporablja NCURSES knjiznica za prikaz podatke.
Za pisanje (saljenje komande) in branje podatke iz /proc/meminfo in
/proc/stat v katerem se nahajajo podatke za CPU in Memory se uporabljajo cevi.
Za saljenje komande na cevi se uporablja awk jezik za text processing.
Novi thread se uporablja pri branje podatke iz teh fajlova, in podatki se
refreshirajo vsaki 2 sekunda.
V dtop.h header file se nahajajo function prototipe, konstante, in
stukture za CPU in Memory podatke.

Ko se program zagne, se prikaze glavno meni. S uporaba spodnja in zgornja
puscica, se lahko gre skozi meni.
Meni ima 3 opcije:
	- List processes with open file descriptors
	- Show system usage
	- Exit

* List processes with open file descriptors - pri taj meni item se otprejo
vsi procesi ki se nahajajo v /proc/ filesystem, in se v ncurses prikazujejo
z ID in ime procesa. Spodaj se nahaja string ki vprasa za process ID za
katero zelimo otpreti vse file descriptore in input za vnos PID.
* Show system usage - taj meni item prikaze podatke za sistema (zagotovo
podatke za CPU in Memory) in osveze podatke vsak 2 sekunda z uporabo nov
thread.
