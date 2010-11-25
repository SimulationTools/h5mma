/****************************************************************************
 * Copyright (C) 2010 Ian Hinder and Barry Wardell
 *****************************************************************************/

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
