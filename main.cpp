#define VERSION "0.8.1"
/*
 * Wadmerge: Merges WAD files used for Doom/Doom2/Hexen/Heretic
 * Copyright (C) 2014  Dennis Katsonis dennisk@netspace.net.au
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include "wad.h"
#include "wadlumpptr.h"

void printLicense( void )
{
  std::cout << "This program is free software: you can redistribute it and/or modify" << std::endl;
  std::cout << "it under the terms of the GNU General Public License as published by" << std::endl;
  std::cout << "the Free Software Foundation, either version 3 of the License, or" << std::endl;
  std::cout << "(at your option) any later version." << std::endl;
  std::cout << std::endl;
  std::cout << "This program is distributed in the hope that it will be useful," << std::endl;
  std::cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl;
  std::cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << std::endl;
  std::cout << "GNU General Public License for more details." << std::endl;
  std::cout << "You should have received a copy of the GNU General Public License" << std::endl;
  std::cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>." << std::endl;
}

void print_help ( void )
{
  std::cout << "By default, wadmerge will not include duplicate wad lumps.  The first entry\n";
  std::cout << "encountered will be the one used in the output file.  If the input wads double\n";
  std::cout << "up on wad data, put the wad with the data to keep first.\n\n";
  std::cout << "By default, output wad will be a PWAD, unless at least one of the input wads is\n";
  std::cout << "an IWAD.  Specify -I or -P to override this behaiviour.\n\n";
  std::cout << "Usage : wadmerge [options] -i input1.wad -i input2.wad -i input3 -o output.wad\n\n";
  std::cout << "Options :\n";
  std::cout << " -d Allow duplicate lumps.\t\t-o Output filename.\n";
  std::cout << " -I Output file is an IWAD.\t\t-P Output file is a PWAD.\n";
  std::cout << " -i Input Wad filename.\n";
  std::cout << " -c Compact (deduplicate).  Store multiple lumps with the same data\n";
  std::cout << "    only once per wad." << std::endl;
  std::cout << " -V Show license." << std::endl;
}


int main (int argc, char **argv)
{
  int optch;
  int optimalHashSize = 0;
  std::vector < Wad > inputfiles;
  std::vector < Wad >::iterator it_inputfiles;
  Wad output;
  std::string outputfile = "invalid";
  unsigned char flags = 0;

  std::cout << "WADMERGE: Joins/merges WAD files for Doom and Doom engine based games.\n";
  std::cout << "(C) Dennis Katsonis (2014).\t\tVersion " << VERSION << "\n\n";
  
  if (argc <= 1)
    {
      print_help ();
      return 0;
    }
  
  
  while ((optch = getopt (argc, argv, "dVIo:i:Pc")) != -1)
    {
      switch (optch)
	{
	case 'V':
	  printLicense();
	  return 0;
	  break;
	case 'd':
	  flags |= F_ALLOW_DUPLICATES;
	  break;
	case 'I':
	  flags |= F_IWAD;
	  break;
	case 'P':
	  flags |= F_PWAD;
	  if (flags & F_IWAD)
	  {
	    std::cout << "Can't set output as both IWAD and PWAD." << std::endl;
	    return 1;
	  }
	  break;
	case 'c':
	  flags |= F_DEDUP;
	  break;
	case 'o':
	  outputfile = optarg;
	  break;
	case 'i':
	  std::cout << "Loading " << optarg << std::endl;
	  try
	  {  // 'Wad' class may throw an exception of the file 
	     // specified by 'optarg' is not a valid and complete .WAD file.
	    inputfiles.push_back (Wad (optarg));
	  }
	  catch(std::string err)
	  {
	    std::cout << err << " : " << optarg << std::endl; 
	    return 1;
	  }
	  break;

	}			// End switch.
    }				// End while.

  if (inputfiles.size () == 0)
    {
      std::cout << "No input WAD files specified." << std::endl;
      return -1;
    }
  if (outputfile == "invalid")
    {
      std::cout << "No valid output WAD files specified." << std::endl;
      return -1;
    }


  // We'll determine the optimal hashsize.  The size doesnt' really matter, but we get more speed when it can cover
  // the largest wad file we can create.  
  for (std::vector < Wad >::iterator c = inputfiles.begin(); c != inputfiles.end(); ++c)
  {
    optimalHashSize += c->getNumLumps();
  }
   
  optimalHashSize = findHigherPrime(optimalHashSize); // We then find the next prime number.
  output.setHashSize(optimalHashSize); // and set it.  Note that we can still merge wads
  // without setting a hash value.  Duplicates will simply be found using a slower method instead.

  std::cout << "Merging...\n";
  for (std::vector < Wad >::iterator c = inputfiles.begin(); c != inputfiles.end(); ++c)
  { // If any wads are IWADS and user hasn't selected an option, make the ouput IWAD. PWAD is default.
    if (!(flags & F_IWAD) && !(flags & F_PWAD))
    {
      if (c->wadType() == WAD_IWAD)
      {
	output.wadType(WAD_IWAD);
      }
    }
    output.mergeWad(*c);
  }

  if (flags & F_IWAD) // If the user has selected IWAD or PWAD, override.
    output.wadType (WAD_IWAD);  
  else if (flags & F_PWAD)
    output.wadType( WAD_PWAD);
  
  std::cout << "Writing " << outputfile << "..." << std::endl;  
  if (flags & F_DEDUP)
  {
    std::cout << "Deduplicating..." << std::endl;
    output.deduplicate();
  }
  try
  {
    output.save (outputfile.c_str ());
  }
  catch(std::string err)
  {
    std::cout << err << " : " << optarg << std::endl; 
    return 1;
  }
  
  return 0;
}
