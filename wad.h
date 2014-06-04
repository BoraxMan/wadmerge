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

#include <list>
#include <string>
#include <algorithm>

#ifndef WAD_H
#define WAD_H

typedef enum enum_wadtypes {
  WAD_ERROR,
  WAD_IWAD,
  WAD_PWAD
} wadTypes;

typedef enum enum_lumptypes {
  T_GENERAL,
  T_F_START,
  T_F1_START,
  T_F2_START,
  T_F3_START,
  T_S_START,
  T_P_START,
  T_P1_START,
  T_P2_START,
  T_END,
  T_MAP
} lumpTypes;


typedef struct struct_wadlumpdata {
  int links; // This field is NOT exported to the Wad
  int type;  // Neither is this.
  int loc;
  int lumpsize;
  char name[8];
  char *lumpdata;
} wadlumpdata;


class Wad
{
private:
  int F_stack_pos;
  int F1_stack_pos;
  int F2_stack_pos;
  int F3_stack_pos;
  int S_stack_pos;
  int P_stack_pos;
  int P1_stack_pos;
  int P2_stack_pos;
  char wad_id[4];
  int numlumps;
  int dirloc;
  bool iwad; // True = IWAD, false = PWAD
  bool sorted;
  std::list< wadlumpdata > wadlump;
  std::list< wadlumpdata >::iterator it;


protected:
  int updateIndexes();
  lumpTypes getCurrentType(wadlumpdata &entry);
  int setCurrentType(wadlumpdata &entry);

  
public:
    Wad();
    Wad(const char* filename);
    ~Wad();
    int storeEntry(std::string lumpname);
    wadlumpdata& fetchEntry(int entrynum);
    int save(const char* filename);
    int load(const char* filename);
    bool mergeEntry(wadlumpdata& entry); // Returns "true" if the entry was a duplicate.
    int getNumLumps(void);
    int mergeWad(Wad &wad);
    wadTypes wadType();
    wadTypes wadType(wadTypes type);
};

#endif // WAD_H

