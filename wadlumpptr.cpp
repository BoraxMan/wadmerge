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
#include "wadlumpptr.h"

void ReferenceCount::addRef()
{
  count++;
}
int ReferenceCount::delRef()
{
  return count--;
}

WadLumpPtr::WadLumpPtr() : lumpdata(0), reference(0)
{
  reference = new ReferenceCount();
  reference->addRef();

}

void WadLumpPtr::assign ( char* pValue )
{
  //std::cout << "Lumpdata " << lumpdata << "pValue " << pValue << std::endl;
  lumpdata = pValue;
  reference->addRef();
}

WadLumpPtr::WadLumpPtr(char* pValue) : lumpdata(pValue), reference(0)
{
  reference = new ReferenceCount();
  reference->addRef();
}

WadLumpPtr::WadLumpPtr( const WadLumpPtr& other ) : lumpdata(other.lumpdata), reference(other.reference)
{
  reference->addRef();
}

WadLumpPtr::~WadLumpPtr()
{
   
  if (reference->delRef() == 0)
  {
    delete lumpdata;
    delete reference;
  }
}

char& WadLumpPtr::operator*()
{
  return *lumpdata;
}

char* WadLumpPtr::operator->()
{
  return lumpdata;
}

WadLumpPtr& WadLumpPtr::operator= ( const WadLumpPtr& other )
{

  if (this != &other)
  {
    if (reference->delRef() == 0)
    {
      delete lumpdata;
      delete reference;
    }
    lumpdata = other.lumpdata;
    reference = other.reference;
    reference->addRef();
  }
  return *this;
}


WadLumpPtr& WadLumpPtr::operator= ( char* pValue)
{
  
  lumpdata = pValue;
  reference->addRef();
  
  return *this;
}


bool WadLumpPtr::operator== ( const WadLumpPtr& other )
{
  if (lumpdata = other.lumpdata)
    return true;
  else
    return false;
}

bool WadLumpPtr::operator!= ( const WadLumpPtr& other )
{
  if (lumpdata != other.lumpdata)
    return true;
  else
    return false;
}

