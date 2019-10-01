
A slightly modified version of l0pht watch for IRIX 6.5
=============================================================================
larry@mathom:~/l0pht-watch$ make IRIX65
        
        make -e temp-watch  \
        XFLAGS='-DIRIX65 -DREGEX -DREGCOMP_3C' CC="gcc -O -Wall"
        gcc -O -Wall  -DIRIX65 -DREGEX -DREGCOMP_3C  -c check_tmp.c
        gcc -O -Wall  -DIRIX65 -DREGEX -DREGCOMP_3C  -c daemon.c
        gcc -O -Wall  -DIRIX65 -DREGEX -DREGCOMP_3C  -c list_utils.c
        gcc -O -Wall  -DIRIX65 -DREGEX -DREGCOMP_3C  -c usage.c 
        gcc -O -Wall -s     -DIRIX65 -DREGEX -DREGCOMP_3C  -o temp-watch 
        check_tmp.o daemon.o list_utils.o usage.o 

============================================================================

If you use this with the regex functionality be careful what you do. For
instance: temp-watch -C '*' will default to watching the /tmp directory
and also placing the files it created in the /tmp directory. This will
create an inifinite loop.

Enabling Syslog also turns the process into a background daemon.

other than that - no big readme here, you have the source and I'm tired.
Mail back any bugs, fixes, enhancements, whatever and I'll incorporate
appropriate ones in. 

mudge at l0pht.com
1/8/99
