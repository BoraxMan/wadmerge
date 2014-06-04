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
#include <fstream>
#include <cstring>
#include "wad.h"

const int lumpNameLength = 8;
const int wadLumpBeginOffset = 12;

char *maplumpnames[] = {
    "THINGS",
    "LINEDEFS",
    "SIDEDEFS",
    "VERTEXES",
    "SEGS",
    "SSECTORS",
    "NODES",
    "SECTORS",
    "REJECT",
    "BLOCKMAP"
  };


Wad::Wad() : numlumps(0), iwad(false), F_stack_pos(0), F1_stack_pos(0),
	    F2_stack_pos(0), F3_stack_pos(0), S_stack_pos(0), P_stack_pos(0),
	    P1_stack_pos(0), P2_stack_pos(0), sorted(0)
{

}

Wad::Wad(const char* filename): Wad()
{

  this->load(filename);
}

wadlumpdata& Wad::fetchEntry(int entrynum)
{
  if (!sorted)
    this->updateIndexes();
 
  if (entrynum >= numlumps)
    entrynum = (numlumps -1);
  
  it = wadlump.begin();

  std::advance(it, entrynum);

  return *it;
}

int Wad::getNumLumps(void)
{
  return numlumps;
}

int Wad::mergeWad(Wad &wad)
{
  int dup;
  for (int x = 0; x < wad.getNumLumps(); x++)
    {
      wadlumpdata x3 = wad.fetchEntry(x);
      dup = this->mergeEntry(wad.fetchEntry(x));
      if (dup == true)
      { // If it's a map, skip the next 10 lumps, as they are duplicates too.
	if ((strncmp(x3.name, "MAP", 3) == 0) && (x3.lumpsize == 0))
	  x +=10;
	if ((x3.name[0] == 'E') && (x3.name[2] == 'M') && (x3.lumpsize == 0))
	  x += 10;
      }
    }

  return 0;
}

  

bool Wad::mergeEntry(wadlumpdata& entry)
{
  bool ismap = false;
  bool duplicate = false;
  
  std::list< wadlumpdata >::iterator insertPosition;
  
  insertPosition = wadlump.end(); // By default, add to the end.
  
  for (it = wadlump.begin(); it != wadlump.end(); it++)
  {
    for (int z = 0; z <= 9; z++)
    {
      if (strncmp(entry.name, maplumpnames[z], strlen(maplumpnames[z])) == 0)
	ismap = true;
    } // It's part of a map.
    
    if((strncmp(entry.name, it->name, lumpNameLength) == 0) && (ismap == false))
      duplicate = true;  // If NOT part of a map, and already exist, its a duplicate.
    
    // The following inserts the entry into the correct position.  Some
    // lumps need to be placed within *_START and *_END labels.  If this lump
    // was loaded from between these labels, find the existing labels
    // and insert it there.  There are all types which aren't T_GENERAL
    
    if ((insertPosition == wadlump.end()) && (it->type != T_GENERAL))
      if(it->type == entry.type)
	insertPosition = it;   // Change position if it matches the current.  
	// This also means we won't be checking this again.  This obviously points to the start.
  }
  
  if(entry.type != T_GENERAL)
  { // We've found the start of the right section, but we want to append to end.
    for (it = insertPosition; it != wadlump.end(); it++)
    {
      if(it->type == T_GENERAL) // This means we've found the end of the section
      {
	//--it; //And we will add it just before the end tag.
	break;
      }
    }
  }
  else it = wadlump.end(); // If it didn't belong to an existing section, add to the end.

  
  if(!duplicate)
  {
    entry.links++;
    wadlump.insert(it, entry);
    sorted = false;
  }

  return duplicate;
}


Wad::~Wad()
{

  for (it = wadlump.begin(); it != wadlump.end(); it++)
  { 
  if(--it->links == 0)
    delete it->lumpdata;
  }
}

int Wad::updateIndexes()
{
  numlumps = 0;
  dirloc = wadLumpBeginOffset; // We start 12 positions in
  for (it = wadlump.begin(); it != wadlump.end(); it++)
  { 
    numlumps++;
    it->loc = dirloc;
    dirloc += it->lumpsize;
  }
  return dirloc;
}

int Wad::save(const char* filename)
{
  std::ofstream fout;
  char wad_id[4];
  int count = 0;
  
  fout.open(filename, std::ios_base::binary);
  if (!fout.is_open())
  {
    return 1;
  }
  
  
  if (sorted == false)
    this->updateIndexes();
  
  if (iwad)
    fout.write("IWAD",4);
  else
    fout.write("PWAD",4);
  
  fout.write(reinterpret_cast<char *>(&numlumps), sizeof(int32_t));
  fout.write(reinterpret_cast<char *>(&dirloc), sizeof(int32_t));
  
  for (it = wadlump.begin(); it != wadlump.end(); it++)
  { // Write the data
    fout.write(reinterpret_cast<char *>(it->lumpdata), it->lumpsize);
  }
  
  // Now write the index
  for (it = wadlump.begin(); it != wadlump.end(); it++)
  {
    fout.write(reinterpret_cast<char *>(&it->loc), sizeof(int32_t));
    fout.write(reinterpret_cast<char *>(&it->lumpsize), sizeof(int32_t));
    fout.write(reinterpret_cast<char *>(&it->name), 8);
  }
  
  
return 0;
}

int Wad::load(const char* filename)
{
  std::ifstream fin;
  char wad_id[4];
  wadlumpdata wadindex;
  
  fin.open(filename, std::ios_base::binary);
  
  if (!fin.is_open())
  {
    return -1;
  }
  
  fin.read(reinterpret_cast<char *>(wad_id), 4);
  fin.read(reinterpret_cast<char *>(&numlumps), sizeof(int32_t));
  fin.read(reinterpret_cast<char *>(&dirloc), sizeof(int32_t));
  fin.seekg(dirloc, std::ios::beg);
  
  for (int count = 0; count < numlumps; count++)
  { // We read the lump information.
    fin.read(reinterpret_cast<char *>(&wadindex.loc), sizeof(int32_t));
    fin.read(reinterpret_cast<char *>(&wadindex.lumpsize), sizeof(int32_t));
    fin.read(reinterpret_cast<char *>(&wadindex.name), 8);

    wadindex.type = this->getCurrentType(wadindex);
    wadindex.links = 1;
    wadlump.push_back(wadindex);
  }
  
  // Now we will read the lump data.
  for (it = wadlump.begin(); it != wadlump.end(); it++)
  {
    fin.seekg(it->loc, std::ios::beg);
    it->lumpdata = new char[it->lumpsize];
    fin.read(reinterpret_cast<char *>(it->lumpdata), it->lumpsize);
    if (fin.gcount() != it->lumpsize)
    {
      std::cout << "Read error!" << std::endl;
      return -1;
    }     
  }
  sorted = true;
  return 0;
}

lumpTypes Wad::getCurrentType(wadlumpdata& entry)
{
  static lumpTypes currenttype = T_GENERAL;
  
  if (!(strncmp(entry.name, "F_START", 7)))
    currenttype = T_F_START;
  else if (!(strncmp(entry.name, "F1_START", 8)))
    currenttype = T_F1_START;
  else if (!strncmp(entry.name, "F2_START", 8))
    currenttype = T_F2_START;
  else if (!strncmp(entry.name, "F3_START", 8))
    currenttype = T_F3_START;
  else if (!strncmp(entry.name, "S_START", 7))
    currenttype = T_S_START;
  else if (!strncmp(entry.name, "P_START", 7))
    currenttype = T_P_START;
  else if (!strncmp(entry.name, "P1_START", 8))
    currenttype = T_P1_START;
  else if (!strncmp(entry.name, "P2_START", 8))
    currenttype = T_P2_START;
  else if (!strncmp(entry.name, "F_END", 5))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "F1_END", 6))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "F2_END", 6))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "S_END", 5))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "P_END", 5))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "P1_END", 6))
    currenttype = T_GENERAL;
  else if (!strncmp(entry.name, "P2_END", 6))
    currenttype = T_GENERAL;
  
  return currenttype;
  
}


wadTypes Wad::wadType()
{
  wadTypes type;
  if (numlumps == 0)
    type = WAD_ERROR;
  else if (strncmp(wad_id, "IWAD", 4) == 0)
    type = WAD_IWAD;
  else if (strncmp(wad_id, "PWAD", 4) == 0)
    type = WAD_PWAD;
  
  return type;
}

wadTypes Wad::wadType(wadTypes type)
{
  if (numlumps == 0)
    type = WAD_ERROR;
  
  if (type == WAD_IWAD)
  {
    iwad = true;
    strncpy(wad_id, "IWAD", 4);
  }
  else if (type == WAD_PWAD)
  {
    iwad = false;
    strncpy(wad_id, "PWAD", 4);
  }
  return type;
}