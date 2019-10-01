
A slightly modified version of l0pht watch for IRIX 6.5
=============================================================================

If you use this with the regex functionality be careful what you do. For
instance: temp-watch -C '*' will default to watching the /tmp directory
and also placing the files it created in the /tmp directory. This will
create an inifinite loop.

Enabling Syslog also turns the process into a background daemon.

other than that - no big readme here, you have the source and I'm tired.
Mail back any bugs, fixes, enhancements, whatever and I'll incorporate
appropriate ones in. 

mudge@l0pht.com
1/8/99
