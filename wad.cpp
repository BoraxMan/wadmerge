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

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iterator>
#include <stdio.h>
#include "wad.h"


const int numGroupTypes = 9; // Number of lump groupings.  This refers
// to all the enum_lumpTypes which end with _START.
const int lumpNameLength = 8; // The length of the lumpname in the WAD file.
const int wadLumpBeginOffset = 12; // The length in bytes of the WAD header.
const int mapEntries = 15; // The number of lump entries which make up a map for Doom (Hexen has one more).
const int glMapEntries = 5; // The number of lump entries for GL Nodes.

int findHigherPrime ( int start )
{
    // This function finds the next prime number higher than the start number.
    int count = 0;

    while ( 1 ) {
        for ( int a = 1; a <= start; ++a ) {
            if ( start % a == 0 ) {
                count++;
            }
        }

        if ( count == 2 ) {
            return start;
        } else {
            count = 0;
            start++; // Increment by one, and try again.
        }
    }
    return 0;
}

unsigned long hash ( const char *str )
{
    unsigned long hash = 5381;
    int c;

    for ( int x = 0; x < 8; ++x ) {
        c = *str++;
        hash = ( ( hash << 5 ) + hash ) + c; /* hash * 33 + c */
    }

    return hash;
}


const char *maplumpnames[] = {
    "THINGS",
    "LINEDEFS",
    "SIDEDEFS",
    "VERTEXES",
    "SEGS",
    "SSECTORS",
    "NODES",
    "SECTORS",
    "REJECT",
    "BLOCKMAP",
    "BEHAVIOR", // Hexen only.
    "GL_VERT",
    "GL_SEGS",
    "GL_SSECT",
    "GL_NODES",
    "GL_PVS"
};


Wad::Wad ( const Wad& obj )
{
    numlumps = obj.numlumps;
    sorted = obj.sorted;
    type = obj.type;
    wad_id = obj.wad_id;
    dirloc = obj.dirloc;
    wadlump = obj.wadlump;
    wadGameType = obj.wadGameType;
}

Wad::Wad ( Wad&& obj )
{
    numlumps = obj.numlumps;
    sorted = obj.sorted;
    type = obj.type;
    wad_id = std::move ( obj.wad_id );
    dirloc = obj.dirloc;
    wadlump = std::move ( obj.wadlump );
    wadGameType = obj.wadGameType;
}


Wad::Wad() : numlumps ( 0 ), type ( WAD_PWAD ), sorted ( false ), dirloc ( wadLumpBeginOffset ), hashsize ( 0 ), duplicatesFound ( 0 ), numDeduplicated ( 0 ), wadGameType ( G_UNKNOWN ),  hasherInitialised ( false )
{
    groupEndOffsets.resize ( numGroupTypes );
    this->wadType ( WAD_PWAD ); // Default to PWAD
}

Wad::Wad ( const char* filename ) : numlumps ( 0 ), type ( WAD_PWAD ), sorted ( false ), dirloc ( wadLumpBeginOffset ),  hashsize ( 0 ), duplicatesFound ( 0 ), numDeduplicated ( 0 ), wadGameType ( G_UNKNOWN ), hasherInitialised ( false )
{
    groupEndOffsets.resize ( numGroupTypes );
    this->load ( filename );
}

Wadlumpdata & Wad::operator[] ( int entrynum ) throw ( std::out_of_range )
{
    if ( sorted == false ) {
        this->updateIndexes ();
    }

    if ( entrynum >= numlumps ) {
        throw std::out_of_range ( "" );
    }

    std::vector < Wadlumpdata >::iterator it = wadlump.begin ();
    std::advance ( it, entrynum );
    return *it;
}

gameTypes Wad::getGameType()
{
    if ( wadGameType == G_UNKNOWN ) {
        this->determineWadGameType();
    }

    return wadGameType;

}

int Wad::deduplicate ()
{
  //  This searches for any future entries which have the same data.
  // If the data is the same, we set the pointer to the first entries data, and set the location to be
  // a pointer to the first entry.
  // Deduped entries will then always refer to the original entry when queried for information.
  
  // This only handles deduplicating TWO entries where the data is the same as the second has to point to a first
  // which has original information.  We can't handle a deduplicated entry pointing to a deduplicated one and
  // I won't bother doing this as its unlikely to be an issue.

    if ( !sorted ) {
        updateIndexes();
    }

    std::vector < Wadlumpdata >::iterator index;

    for ( std::vector < Wadlumpdata >::iterator it = wadlump.begin (); it != wadlump.end (); it++ ) {
        for ( index = std::next ( it ); index != wadlump.end (); index++ ) {
            if ( ( index->lumpsize > 0 ) && ( index->lumpsize == it->lumpsize ) && ( index->deduped == false ) && ( it->deduped == false ) ) {

                if ( std::memcmp ( index->lumpdata.get(), it->lumpdata.get(), it->lumpsize ) == 0 ) {
                    index->lumpdata = it->lumpdata;
                    index->setLocation( &(*it) ); // We refer to the location by pointing to the original entry.
                    //  That way, if the original entry changes position, we always refer to the right one.
                    index->deduped = true;
                    ++numDeduplicated;
                }
            }
        }

    }
    // If we have deduplicated any, we will need to redo the indexes again before saving, otherwise
    // we are OK to go.
    // Deduplicated entries will move some entries further up the Wad.  
    if ( numDeduplicated ) {
      sorted = false;
    } else {
      sorted = true;
    }
    return 0;
}


unsigned int Wad::getNumLumps ( void ) const
{
    return wadlump.size();
}

int Wad::mergeWad ( Wad& wad, bool allowDuplicates ) throw()
{

    if ( ( hashsize != 0 ) && ( hasherInitialised == false ) ) {
        hasher.resize ( hashsize );
        hasherInitialised = true;
    }

    for ( int x = 0; x < wad.getNumLumps(); ++x ) {
        bool dup = this->storeEntry ( wad[x], allowDuplicates );
        if ( dup == true ) {
            // If it's a map, skip the next 10 lumps, as they are duplicates too.
            if ( ( ( wad[x] ).lumpsize <= 16 ) && std::equal ( ( wad[x] ).name.begin(), ( wad[x] ).name.begin() + 3, "MAP" ) ) {
                x += wad.wadGameType;
            
            // wadGameType enum = 10 for doom2 and 11 for hexen, which coincides with the number
            // of entries we have to skip to get to the next map.  We of course, use the wadtype of the
            // wad we are merging, not THIS one.
            } else if ( ( ( wad[x] ).lumpsize <= 16 ) && ( ( wad[x] ).name[0] == 'E' ) && ( ( wad[x] ).name[2] == 'M' ) ) {
                x += mapEntries;
                duplicatesFound += ( mapEntries + 1 );
            } else if ( std::equal ( ( wad[x] ).name.begin(), ( wad[x] ).name.begin() + 6, "GL_MAP" ) ) { 
              std::cout << "GL DOOM2" << std::endl;
                x += glMapEntries;
            } else if ( std::equal ( ( wad[x] ).name.begin(), ( wad[x] ).name.begin() + 4, "GL_E" ) && ( ( wad[x] ).name[5] == 'M' ) )  {
              std::cout << "GL DOOM" << std::endl;
                x += glMapEntries;
            } else {
                ++duplicatesFound;
            }
        }
    }

    return 0;
}

gameTypes Wad::determineWadGameType ()
{
    // We need this to know the map format.

    for ( std::vector < Wadlumpdata >::const_iterator it_lump = wadlump.begin(); it_lump != wadlump.end(); ++it_lump ) {
        if ( it_lump->lumpsize <= 32 ) {
            if ( ( wadGameType == G_UNKNOWN ) && ( it_lump->lumpsize < 32 ) && ( std::equal ( it_lump->name.begin(), it_lump->name.begin() +3 , "MAP" )  ) )

            {
                if ( std::equal ( ( it_lump + G_HEXEN )->name.begin(), ( it_lump + G_HEXEN )->name.end(), maplumpnames[10] ) ) {

                    // maplumpnames[10] = BEHAVIOR
                    // As this only appears in HEXEN, if the 10th one after the MAP entry is BEHAVIOR
                    // then we've established that it is a Hexen map and not a Doom 2 one.
                    wadGameType = G_HEXEN;
                    break;
                } else {
                    wadGameType = G_DOOM2;
                    break;
                }
            } else if ( ( wadGameType == G_UNKNOWN ) && ( it_lump->lumpsize < 32 ) && ( it_lump->name[0] == 'E' ) && ( it_lump->name[2] == 'M' ) ) {
                // This also applies to HERETIC and Ultimate Doom
                wadGameType = G_DOOM;
                break;
            } else {
                wadGameType = G_UNKNOWN;
            }
        }
    } // End 'for' loop
    return wadGameType;
}


bool Wad::storeEntry ( const Wadlumpdata& entry, bool allowDuplicates ) throw()
{
    bool ismap = false;
    bool duplicate = false;
    int hashValue;
    bool collision = false;
    std::vector< Wadlumpdata >::iterator it;

    if ( allowDuplicates == false )
        // If no duplicates, check and return we've found one, if it is indeed one
    {

        for ( int z = 0; z <= ( mapEntries - 1 ); ++z ) {
            if ( std::equal ( entry.name.begin(), entry.name.begin() + strlen ( maplumpnames[z] ), maplumpnames[z] ) ) {
                ismap = true;
            }
        } // It's part of a map.  We don't check these for duplicates.

        if ( ( ismap == false ) && ( hashsize != 0 ) ) {
            // So its not part of a map.  Have we come accross this entryname before?
            // We can do a quick hash search, IF the hashtable has been initialised.
            hashValue = ( hash ( entry.name.data() ) ) %hashsize;

            if ( hasher[hashValue] == 0 ) {
                duplicate = false;
                hasher[hashValue] = 1;
            } else {
                collision = true;
            }
        }
        if ( ( collision == true ) || ( hashsize == 0 ) ) {
            // If its a collision, or we haven't initialised the hash table, do a slow search.

            for ( it = wadlump.begin(); it != wadlump.end(); ++it ) {
                if ( ( std::equal ( entry.name.begin(), entry.name.end(), it->name.begin() ) ) && ( ismap == false ) ) {
                    duplicate = true;    // So we've determined its a duplicate, for real..
                }
            } // End for loop.
        }
    } // end if (allowDuplicates == false)

    if ( !duplicate || ( allowDuplicates == true ) ) {
        // If not a duplicate, we can add it, OR if we've allowed them
        if ( ( entry.type != T_GENERAL ) && ( groupEndOffsets[entry.type] != 0 ) ) {
            it = wadlump.begin();
            std::advance ( it, groupEndOffsets[entry.type] ); // Jump to the end of the segment.

            wadlump.insert ( it, entry ); // Insert.
            groupEndOffsets[entry.type]++; // Move end of segment up one space, as we've
            // inserted an entry before it.

            for ( int x = 0; x < numGroupTypes; ++x ) {
                // All end of segment marker entries  this, also get moved up one space .
                if ( groupEndOffsets[x] > groupEndOffsets[entry.type] ) {
                    groupEndOffsets[x]++;
                }
                // Because these are kept up to date, we don't need to recalculate the offsets
                // for the next merge.
            }
        } else {
            wadlump.push_back ( entry );    // Otherwise, we just append it.
        }
        sorted = false;
    }

    return duplicate;  // The calling function might want to know whether this was
    // a duplicate or not.
}


Wad::~Wad()
{

}

int Wad::updateIndexes()
{
    std::vector < Wadlumpdata >::iterator it;

    numlumps = wadlump.size();
    dirloc = wadLumpBeginOffset; // We start after the WAD header.

    std::for_each ( wadlump.begin(), wadlump.end(), [&] ( Wadlumpdata &x ) {
        if ( x.deduped == false ) {
            x.setLocation ( dirloc );
            dirloc += x.lumpsize;
        }
    } );

    return dirloc;
}

int Wad::save ( const char *filename ) throw ( std::string )
{
    std::vector< Wadlumpdata >::iterator it;
    std::ofstream fout;
    fout.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

    try {
        fout.open ( filename, std::ios_base::binary );
    } catch ( std::ifstream::failure &e ) {
        throw ( std::string ( "Error saving file." ) );
    }

    if ( sorted == false ) {
        this->updateIndexes ();
    }

    unsigned int z = wadlump.size ();

    try {
        fout.write ( reinterpret_cast < char *> ( wad_id.data() ), wad_id.size() );
        fout.write ( reinterpret_cast < char *> ( &z ), sizeof ( int32_t ) ) ;
        fout.write ( reinterpret_cast < char *> ( &dirloc ), sizeof ( int32_t ) );

        for ( it = wadlump.begin (); it != wadlump.end (); ++it ) {
            // Write the data
            fout.seekp ( it->getLocation(), std::ios::beg );
            if ( it->deduped == false ) {
                fout.write ( reinterpret_cast < char *> ( it->lumpdata.get() ), it->lumpsize );
            }
        }
        // Now write the index
        fout.seekp ( dirloc, std::ios::beg );

        for ( it = wadlump.begin (); it != wadlump.end (); ++it ) {
            int32_t loc = it->getLocation();
            fout.write ( reinterpret_cast < char *> ( &loc ), sizeof ( int32_t ) ) ;
            fout.write ( reinterpret_cast < char *> ( &it->lumpsize ), sizeof ( int32_t ) );
            fout.write ( reinterpret_cast < char *> ( &it->name ), lumpNameLength );
        }
    } // End of try
    catch ( std::ifstream::failure &e ) {
        fout.close();
        throw ( std::string ( "Error saving file." ) );
    }
    fout.close ();
    return 0;
}



int Wad::load ( const char* filename ) throw ( std::string )
{
    std::ifstream fin;
    fin.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    Wadlumpdata wadindex;

    try {
        fin.open ( filename, std::ios_base::binary );
    } catch ( std::istream::failure &e ) {
        throw ( std::string ( "Error loading file." ) );
    }

    try {
        fin.read ( reinterpret_cast<char *> ( wad_id.data() ), wad_id.size() );
        fin.read ( reinterpret_cast<char *> ( &numlumps ), sizeof ( int32_t ) );
        fin.read ( reinterpret_cast<char *> ( &dirloc ), sizeof ( int32_t ) );
        fin.seekg ( dirloc, std::ios::beg );

        wadlump.reserve ( numlumps );

        for ( int count = 0; count < numlumps; ++count ) {
            // We read the lump information.
            int32_t x;
            fin.read ( reinterpret_cast<char *> ( &x ), sizeof ( int32_t ) );
            wadindex.setLocation( x );
            fin.read ( reinterpret_cast<char *> ( &wadindex.lumpsize ), sizeof ( int32_t ) );
            fin.read ( reinterpret_cast<char *> ( &wadindex.name ), lumpNameLength );
            wadindex.deduped = false;
            wadindex.type = this->getCurrentType ( wadindex );
            wadlump.push_back ( std::move ( wadindex ) );
        }

        // Now we will read the lump data.
        for ( std::vector < Wadlumpdata >::iterator it = wadlump.begin(); it != wadlump.end(); ++it ) {
            fin.seekg ( it->getLocation(), std::ios::beg );
            //it->lumpdata.assign ( new unsigned char[it->lumpsize] );
            it->lumpdata = make_shared_array<char> ( it->lumpsize );
            //  = std::make_shared<char *>(new char[it->lumpsize])
            fin.read ( reinterpret_cast<char *> ( it->lumpdata.get() ), it->lumpsize );
        }

    } // End of try block
    catch ( std::istream::failure &e ) {
        fin.close();
        throw ( std::string ( "Error reading file." ) );
    }

    this->calcLabelOffsets();  // Calculate where the end group tags are.
    sorted = true;
    this->wadType();  // This fetches the WAD type (PWAD/IWAD), but also sets
    // the 'iwad' flag to true or false.  Call this to set the flag to the correct value.

    fin.close();
    this->determineWadGameType();
    return 0;
}

int Wad::calcLabelOffsets () throw()
{
    lumpTypes thisType = T_GENERAL;
    lumpTypes currType = T_GENERAL;
    int count = 0;

    for ( std::vector < Wadlumpdata >::const_iterator it = wadlump.begin (); it != wadlump.end (); ++it ) {
        currType = thisType;
        thisType = it->type;

        if ( ( thisType == T_GENERAL ) && ( currType != T_GENERAL ) ) {
            if ( ( currType == T_F1_START ) || ( currType == T_F2_START )
                    || ( currType == T_F3_START ) || ( currType == T_F_START )
                    || ( currType == T_P1_START ) || ( currType == T_P2_START )
                    || ( currType == T_P_START ) || ( currType == T_S_START )
                    || ( currType == T_C_START ) ) {
                groupEndOffsets[currType] = count;
            }
        }
        ++count;
    }
    return 0;
}


lumpTypes Wad::getCurrentType ( const Wadlumpdata & entry ) const throw()
{
    static lumpTypes currenttype = T_GENERAL;

    if ( std::equal ( entry.name.begin(), entry.name.begin() + 7, "F_START" ) ) {
        currenttype = T_F_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 8, "F1_START" ) ) {
        currenttype = T_F1_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 8, "F2_START" ) ) {
        currenttype = T_F2_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 8, "F3_START" ) ) {
        currenttype = T_F3_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 7, "S_START" ) ) {
        currenttype = T_S_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 7, "P_START" ) ) {
        currenttype = T_P_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 8, "P1_START" ) ) {
        currenttype = T_P1_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 8, "P2_START" ) ) {
        currenttype = T_P2_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 7, "C_START" ) ) {
        currenttype = T_C_START;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 5, "F_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 6, "F1_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 6, "F2_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 5, "S_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 5, "P_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 6, "P1_END" ) ) {
        currenttype = T_GENERAL;
    } else if ( std::equal ( entry.name.begin(), entry.name.begin() + 6, "P2_END" ) ) {
        currenttype = T_GENERAL;
    }

    return currenttype;

}


wadTypes Wad::wadType ()
{
    wadTypes type;

    if ( std::equal ( wad_id.begin(), wad_id.end(), "IWAD" ) ) {
        type = WAD_IWAD;
    } else if ( std::equal ( wad_id.begin(), wad_id.end(), "PWAD" ) ) {
        type = WAD_PWAD;
    } else {
        type = WAD_ERROR;
    }

    return type;
}

wadTypes Wad::wadType ( wadTypes type )
{

    if ( type == WAD_IWAD )
        wad_id = {'I','W','A','D'};
    else if ( type == WAD_PWAD )
        wad_id = {'P','W','A','D'};

    return type;
}

void Wad::setHashSize ( int hashsz )
{
    hashsize = hashsz;
}

void Wad::stats ( void ) const
{
    // outputs some statistics.  Only really useful for the output wad.
    std::cout << "Number of lumps : " << numlumps << std::endl;

    if ( duplicatesFound ) {
        std::cout << "Duplicates entries discarded : " << duplicatesFound << std::endl;
    }

    if ( numDeduplicated ) {
        std::cout << "Entries deduplicated : " << numDeduplicated << std::endl;
    }

    std::cout << "Output WAD file size " << dirloc + ( numlumps * ( lumpNameLength + ( sizeof ( uint32_t ) * 2 ) ) ) << " bytes" << std::endl;
}


Wadlumpdata::Wadlumpdata() : ptr( nullptr )
{
}

int Wadlumpdata::getLocation()
{
  if (ptr == nullptr) {
    return location;
  } else if ( deduped == true ) {
    return ptr->getLocation();
  } else throw ( std::string ( "Non deduplicated entry with pointer to another entry...\n" ) );
}

void Wadlumpdata::setLocation(int loc)
{
  location = loc;
}

void Wadlumpdata::setLocation(Wadlumpdata* p)
{
  ptr = p;
}
