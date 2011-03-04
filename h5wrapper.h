/*
 * Copyright 2010-2011 Barry Wardell and Ian Hinder
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef H5WRAPPER_H
#define H5WRAPPER_H

#include "hdf5.h"
#include <string>

using namespace std;

class H5Exception
{
public:
  H5Exception(const string& message);
  const char* getCMessage();

private:
  string message;
};

class H5Base
{
public:
  hid_t getId() const;

protected:
  hid_t id;
};

class H5F : public H5Base
{
public:
  H5F(const string& filename);
  ~H5F();
};

class H5D : public H5Base
{
public:
  H5D(const H5F& file, const string& dataset);
  ~H5D();

  int getNumAttrs() const;
};

class H5A : public H5Base
{
public:
  H5A(hid_t location_id, const char* attr_name);
  ~H5A();
};

class H5S : public H5Base
{
public:
  H5S(const H5A& dataset);
  H5S(const H5D& dataset);
  ~H5S();

  int getSimpleExtentDims(hsize_t *dims) const;
  int getSimpleExtentNDims() const;
};

class H5T : public H5Base
{
public:
  H5T(const H5A& dataset);
  H5T(const H5D& dataset);
  ~H5T();

  size_t getSize() const;
};



#endif // H5WRAPPER_H
