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
#include "wad.h"


int main(int argc, char **argv) 
{
  Wad test;
  Wad test3;
  wadlumpdata entry1;
  wadTypes x;
    
  test.load("1.wad");
  Wad test2("2.wad");
  x = test.wadType();
    
  if (x == WAD_IWAD)
    test2.wadType(WAD_IWAD);
    
  test3.mergeWad(test2);
  test3.mergeWad(test);

  test3.save("test.wad");
     
}
