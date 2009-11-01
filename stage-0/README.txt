BOOTSTRAP STAGE 0

unhex is the starting point for this bootstrap experiment.  It is a very
simple program for converting a stream of hexadecimal octets on standard
input into a binary file written to standard output.  The file format is 
very restrictive:

  XDIGIT := [0-9A-F]
  CHAR   := any character

  octet  := XDIGIT XDIGIT CHAR
  file   := octet*

The unhex.x file contains the hexadecimal quads for unhex.  Processing
it with unhex yields another copy of unhex.
