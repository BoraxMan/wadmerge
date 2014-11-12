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

#include <string>
#include <vector>
#include <algorithm>
#include "wadlumpptr.h"

#ifndef WAD_H
#define WAD_H


unsigned long hash(char *str);
int findHigherPrime(int start);


template <typename ForwardIt>
ForwardIt next ( ForwardIt it,
                 typename std::iterator_traits<ForwardIt>::difference_type n = 1 )
{
    std::advance ( it, n );
    return it;
}

enum flags
{
    F_ALLOW_DUPLICATES 	= 0x1,
    F_DEDUP		= 0x2,
    F_IWAD		= 0x4,
    F_PWAD		= 0x8
};

typedef enum enum_wadtypes
{
    WAD_ERROR,
    WAD_IWAD,
    WAD_PWAD
} wadTypes;

typedef enum enum_lumptypes
{
    T_F_START,
    T_F1_START,
    T_F2_START,
    T_F3_START,
    T_S_START,
    T_P_START,
    T_P1_START,
    T_P2_START,
    T_C_START,
    T_END,
    T_MAP,
    T_GENERAL
} lumpTypes;

typedef enum enum_gametypes
{
  G_UNKNOWN,
  G_DOOM, 
  G_HERETIC = 9,
  G_DOOM2 = 10,  // This coincides with the number of map entries in a Doom 2 map for a reason
  G_HEXEN = 11, // This coincides with the number of map entries in a Hexen map for a reason
} gameTypes;
// That reason is, so that we can use the gameTypes value to advances the right number of lump entries
// to get the next map.

typedef struct struct_wadlumpdata
{
    lumpTypes type;  // This is not written to the wad.
    int loc;
    int lumpsize;
    char name[8];
    WadLumpPtr lumpdata;
    bool deduped; // Neither is this.

} wadlumpdata;


class Wad
{
private:
    std::vector< int > groupEndOffsets;
    char wad_id[4];
    int numlumps;
    bool iwad; // True = IWAD, false = PWAD
    bool sorted;
    int dirloc;
    int hashsize;
    int duplicatesFound; // Defaults to zero, increments each time a lump
			// was added which is a duplicate of a previous one.
    int numDeduplicated;
    gameTypes wadGameType;

    std::vector< int > hasher;
    bool hasherInitialised;
    std::vector< wadlumpdata > wadlump;
    gameTypes determineWadGameType();

    int updateIndexes();
    int calcLabelOffsets();
    lumpTypes getCurrentType ( const wadlumpdata &entry ) throw();

public:
    //Wad& operator=(const Wad& obj);
    Wad ( const Wad& obj );
    Wad();
    Wad ( const char* filename );
    ~Wad();
    int deduplicate();
    wadlumpdata& operator[] ( int entrynum ) throw();
    int save ( const char* filename ) throw (std::string);
    int load ( const char* filename ) throw (std::string);
    bool storeEntry ( const wadlumpdata& entry, bool allowDuplicates ) throw(); // Returns "true" if the entry was a duplicate.
    int getNumLumps ( void );
    void stats(void);
    int mergeWad ( Wad& wad, bool allowDuplicates) throw();
    wadTypes wadType();
    void setHashSize(int hashsz);
    wadTypes wadType ( wadTypes type );
    gameTypes getGameType();

};

#endif // WAD_H

