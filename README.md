NesHexDumpTool
=====

[Open this project in 8bitworkshop](http://8bitworkshop.com/redir.html?platform=nes&githubURL=https%3A%2F%2Fgithub.com%2Flucienmp-nes%2FNesHexDumpTool&file=fami.c).

This project is a small program used to explore hardware in an unknown NES emulation platform.
Allows for value read and write to a CPU space, and dumping of a section.  Mostly aimed at checking
the SPI flash paging for our needs.

     1: Hex dump tool!
     2: 
     3: *                            <- pointer
     4: 1234 W 5a                    <- R/W to an address
     5:
     6:
     7: 8000                         <- Address to hex dump; Button A to do the dump
     8:       0   1 ...
     9: 8000  xx yy                  <- Dumped data
    10: 8008  xx yy                  <- Dumped data+8
        ...
