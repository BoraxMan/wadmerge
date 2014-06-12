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

#ifndef WADLUMPPTR_H
#define WADLUMPPTR_H


class ReferenceCount
{
  private:
    int count;
    
  public:
    void addRef();
    int delRef();
};  
  

class WadLumpPtr
{
  private:
    char* lumpdata;
    ReferenceCount* reference;
      
  public:
    WadLumpPtr();
    WadLumpPtr(char* pValue);
    WadLumpPtr(const WadLumpPtr& other);
    ~WadLumpPtr();
    char& operator* ();
    char* operator-> ();
    void assign(char *pValue);
    WadLumpPtr& operator= (const WadLumpPtr& other);
    WadLumpPtr& operator= ( char* pValue);
    bool operator== (const WadLumpPtr& other);
    bool operator!= (const WadLumpPtr& other);
};


#endif // WADLUMPPTR_H
