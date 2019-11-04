NesHexDumpTool
=====

[Open this project in 8bitworkshop](http://8bitworkshop.com/redir.html?platform=nes&githubURL=https%3A%2F%2Fgithub.com%2Flucienmp-nes%2FNesHexDumpTool&file=fami.c).

This project is a small program used to explore hardware in an unknown NES emulation platform.
Allows for value read and write to a CPU space, and dumping of a section.  Mostly aimed at checking
the SPI flash paging for our needs.

Joystick LEFT/RIGHT - Move "*" cursor left/right
Joystick UP/DOWN    - For address, or value field changes the selected unit column UP/DOWN
A Button            - Depending on cursor either performs the single R/W, or sets the new hex dump address.



     1: Hex dump tool!
     2: 
     3: *                     <- cursor
     4: 1234 W 5a             <- Single R/W : ADDRESS, Read/Write, Value Read OR to Write
     5:
     6:
     7: 8000                  <- Address to hex dump; Button A to do the dump
     8:       0   1 ...
     9: 8000  xx yy           <- Dumped data
    10: 8008  xx yy           <- Dumped data+8
        ...
