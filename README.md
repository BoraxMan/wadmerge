WADMERGE
========

wadmerge - Merges WAD file for Doom engine games.

Author: Dennis Katsonis
dennisk@netspace.net.au.


Synopsis
--------
wadmerge [OPTIONS] 

Description
-----------

Merges .WAD files from Doom engine games, such as
Doom, Doom2, Hexen and Heretic.

Options
-------
-V
Print licence

-d
Allow duplicate lumps.

-i
Input wad filename.

-o
Output wad filename.

-P
Output wad is a PWAD.

-I
Output wad is an IWAD.

-c
Compact (deduplicate).  Store multiple lumps which have the same data once per wad.

Examples
--------
	wadmerge -i map01.wad -i graphics.wad -o megawad.wad

Merges map01.wad and graphics.wad into megawad.wad.

Notes
-----

By default, if the wads you are merging double up on lumps, wadmerge will not include them all, but just the first one.  For example, if you merge multiple wads each with a DSPOSSIT entry, only the first entry will make it into the final wad.

Deduplication means lumps which have different names, but the same data will share the same copy of data.

Limitations
-----------

Ensure that the output wad file is OK, before you remove the input wad files.  While this program has been tested and confirmed to work thoroughly, blind faith is no substitute for backups and testing.



Bugs
----
No known bugs.


