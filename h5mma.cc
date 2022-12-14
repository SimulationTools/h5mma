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

#include <vector>
#include <cstring>
#include <cstdlib>
#include <map>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <cmath>
#include <limits>

#include "h5mma.h"
#include "time.h"

using namespace std;

#if defined(_MSC_VER)
#define isnan(x) _isnan(x)
#define isfinite(x) _finite(x)
#define isinf(x) (!_finite(x) && !_isnan(x))
#endif

static int put_general_array(MLINK link, const double *fdata, long int dims[], int rank);

void fail(int type, const char *err)
{
  char err_msg[100];
  switch(type)
  {
  case FAIL_ML:
    sprintf(err_msg, "%s\"%.76s\"%s", "Message[h5mma::mlink,", MLErrorMessage(stdlink),"]");
    MLClearError(stdlink);
    break;
  case FAIL_HDF5:
    sprintf(err_msg, "%s\"%.76s\"%s", "Message[h5mma::mlink,", err,"]");
    break;
  case FAIL_INVALID:
    sprintf(err_msg, "%s\"%.76s\"%s", "Message[h5mma::mlink,", "Invalid arguments","]");
    break;
  default:
    sprintf(err_msg, "%s\"%.76s\"%s", "Message[h5mma::mlink,", "Unknown error","]");
  }

  MLNewPacket(stdlink);
  MLEvaluate(stdlink, err_msg);
  MLNextPacket(stdlink);
  MLNewPacket(stdlink);
  MLPutSymbol(stdlink, "$Failed");
}

/* Get the list of requested datasets from Mathematica */
long GetDatasetNames(vector<string> *datasetNames)
{
  long n;

  if(MLCheckFunction(stdlink, "List", &n)!=MLSUCCESS)
  {
    fail(FAIL_INVALID, NULL);
    return -1;
  }

  for(int j=0; j<n; j++)
  {
    const char *datasetName;
    if(MLGetString(stdlink, &datasetName))
    {
      datasetNames->push_back(datasetName);
      MLReleaseString(stdlink, datasetName);
    } else {
      fail(FAIL_ML, NULL);
      return -1;
    }
  }

  return n;
}

/* Get the list of requested dataset slabs from Mathematica */
long GetDatasetSlabs(vector<vector<vector<int> > > &datasetSlabs)
{
  long l, m, n;
  if(MLCheckFunction(stdlink, "List", &l)!=MLSUCCESS)
  {
    fail(FAIL_INVALID, NULL);
    return -1;
  }

  datasetSlabs.resize(l);

  for(int i=0; i<l; i++)
  {
    if(MLCheckFunction(stdlink, "List", &m)!=MLSUCCESS)
    {
      fail(FAIL_INVALID, NULL);
      return -1;
    }
    datasetSlabs[i].resize(m);

    for(int j=0; j<m; j++)
    {
      if(MLCheckFunction(stdlink, "Span", &n)!=MLSUCCESS || n != 3)
      {
        fail(FAIL_INVALID, NULL);
        return -1;
      }

      for(int k=0; k<n; k++)
      {
        int s;
        if(MLGetInteger(stdlink, &s))
        {
          datasetSlabs[i][j].push_back(s);
        } else {
          fail(FAIL_ML, NULL);
          return -1;
        }
      }
    }
  }

  return l;
}

/* Read dimensions of a given list of datasets in a file */
void ReadDatasetDimensions(const char *fileName)
{
  /* Get the list of datasets to be read from Mathematica*/
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);
  if(n<0)
    return;

  /* Create a loopback link to store the list until we are sure it can be fully
     filled. This way if something fails we can abort and send $Failed back. */
  MLINK loopback;
  loopback = MLLoopbackOpen(stdenv, NULL);

  if(!loopback) {
    fail(FAIL_ML, NULL);
    return;
  }

  try
  {
    /* Open requested file */
    H5F file(fileName);

    /* Loop over all requested datasets */
    MLPutFunction(loopback, "List", n);

    for(int i=0; i<n; i++)
    {
      /* Respond to abort from Mathematica */
      if(MLAbort)
      {
        MLPutFunction(stdlink, "Abort", 0);
        MLClose(loopback);
        return;
      }

      /* Open dataset and dataspace */
      H5D dataset(file, datasetNames[i]);
      H5S dataspace(dataset);

      /* Get the number of dimensions in this dataset */
      const int rank = dataspace.getSimpleExtentNDims();

      /*  Get the size of each dimension */
      vector<hsize_t> dims_out(rank);
      dataspace.getSimpleExtentDims(dims_out.data());

      vector<int> dims(rank);
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      /* Send the dimensions of this dataset to the loopback link */
      MLPutIntegerList(loopback, dims.data(), rank);
    }
  }
  catch(H5Exception err) {
    fail(FAIL_HDF5, err.getCMessage());
    MLClose(loopback);
    return;
  }

  /* Transfer data from loopback to actual Mathematica link */
  MLTransferToEndOfLoopbackLink(stdlink, loopback);

  MLClose(loopback);
}

void ReadDatasets(const char *fileName)
{
  /* Get the list of datasets to be read from Mathematica*/
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);
  if(n<0)
    return;

  /* Get the list of dataset hyperslabs to be read from Mathematica */
  vector<vector<vector<int> > > datasetSlabs;
  long ns = GetDatasetSlabs(datasetSlabs);
  if(ns != n)
    return;

  /* Create a loopback link to store the list until we are sure it can be fully
     filled. This way if something fails we can abort and send $Failed back. */
  MLINK loopback;
  loopback = MLLoopbackOpen(stdenv, NULL);

  if(!loopback) {
    fail(FAIL_ML, NULL);
    return;
  }

  try
  {
    H5F file(fileName);

    MLPutFunction(loopback, "List", n);

    /* Loop over all requested datasets */
    for(int i=0; i<n; i++)
    {
      /* Respond to abort from Mathematica */
      if(MLAbort)
      {
        MLPutFunction(stdlink, "Abort", 0);
        MLClose(loopback);
        return;
      }

      H5D dataset(file, datasetNames[i]);

      /* Check we have 64 bit floating point numbers */
      H5T datatype(dataset);
      H5T_class_t typeclass = H5Tget_class(datatype.getId());
      size_t size = datatype.getSize();

      H5T_class_t superclass = typeclass == H5T_ARRAY ? H5Tget_class(datatype.getSuperId()) : H5T_NO_CLASS;
      size_t superSize = datatype.getSuperSize();

      /* Only accept a limited supset of the HDF5 data types */
      if (!((typeclass == H5T_INTEGER && (size == 8 || size == 4 || size == 2 || size == 1)) ||
            (typeclass == H5T_FLOAT && (size == 8 || size == 4)) ||
            (typeclass == H5T_STRING) ||
            (typeclass == H5T_ARRAY && superclass == H5T_FLOAT && (superSize == 4 || superSize == 8)) ||
            (typeclass == H5T_COMPOUND)))
      {
        stringstream ss;
        ss << "Unsupported datatype: ";
        if (typeclass == H5T_ARRAY)
          ss << "Array, class " << superclass << ", size " << superSize;
        else
          ss << "class " << typeclass << ", size " << size;
        string str = ss.str();
        throw(H5Exception(str));
      }

      if (typeclass == H5T_STRING)
      {
        size_t size = datatype.getSize();
        /* Include space for a null terminator in case it isn't in the data */
        vector<char> str(size+1);
        str[size] = '\0';
        if (H5Dread(dataset.getId(), datatype.getNativeId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, (void *)str.data()) < 0)
            throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
        MLPutString(loopback, str.data());
      } else {
        /* Numeric data */

        /* Get dimensions of this dataset */
        H5S dataspace(dataset);
        const int rank = dataspace.getSimpleExtentNDims();
        vector<hsize_t> dims_out(rank);

        if(datasetSlabs[i].size() > 0)
        {
          if(datasetSlabs[i].size() != rank)
            fail(FAIL_INVALID, NULL);
          vector<hsize_t> offsets(rank);
          vector<hsize_t> strides(rank);
          vector<hsize_t> lengths(rank);
          for (int j=0; j<rank; j++)
          {
            offsets[j] = datasetSlabs[i][j][0] - 1;
            strides[j] = datasetSlabs[i][j][2];
            lengths[j] = ceil(((double)(datasetSlabs[i][j][1] - offsets[j])) / ((double)strides[j]));
            dims_out[j] = lengths[j];
          }

          H5Sselect_hyperslab(dataspace.getId(), H5S_SELECT_SET, offsets.data(),
                              strides.data(), lengths.data(), NULL);
        } else {
          dataspace.getSimpleExtentDims(dims_out.data());
        }

        /* Read data */
        int nElems = 1;
        for (int j = 0; j < rank; j++)
        {
          nElems *= dims_out[j];
        }

        vector<long int> dims(rank);
        vector<int> dims2(rank);
        for (int j = 0; j < rank; j++)
        {
          dims[j] = dims_out[j];
          dims2[j] = dims_out[j];
        }

        hid_t memspace = H5Screate_simple(rank, dims_out.data(), NULL); 

        switch(typeclass)
        {
        case H5T_INTEGER:
          if(size == 8)
          {
            vector<mlint64> idata(nElems);
            if (H5Dread(dataset.getId(), datatype.getNativeId(), memspace, dataspace.getId(), H5P_DEFAULT, idata.data()) < 0)
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            if (rank == 0)
              MLPutInteger64(loopback, idata[0]);
            else
              MLPutInteger64Array(loopback, idata.data(), dims2.data(), 0, rank);
          } else if(size == 4) {
            vector<int> idata(nElems);
            if (H5Dread(dataset.getId(), datatype.getNativeId(), memspace, dataspace.getId(), H5P_DEFAULT, idata.data()) < 0)
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            if (rank == 0)
              MLPutInteger(loopback, idata[0]);
            else
              MLPutIntegerArray(loopback, idata.data(), dims.data(), 0, rank);
          } else if(size==2) {
            vector<short> sdata(nElems);
            if (H5Dread(dataset.getId(), H5T_NATIVE_SHORT, memspace, dataspace.getId(), H5P_DEFAULT, sdata.data()) < 0)
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            if (rank == 0)
              MLPutInteger16(loopback, sdata[0]);
            else
              MLPutInteger16Array(loopback, sdata.data(), dims2.data(), 0, rank);
          } else if(size==1) {
            vector<char> cdata(nElems);
            if (H5Dread(dataset.getId(), datatype.getNativeId(), memspace, dataspace.getId(), H5P_DEFAULT, cdata.data()) < 0)
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            MLPutString(loopback, cdata.data());
          }
          break;
        case H5T_FLOAT:
          if (size == 8)
          {
            double *fdata = 0;
            try
            {
              fdata = new double[nElems];
            }
            catch(bad_alloc e)
            {
              throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
            }
            if (H5Dread(dataset.getId(), datatype.getNativeId(), memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
            {
              delete [] fdata;
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            }

            bool numeric = true;
            for (int i = 0; i < nElems; i++)
            {
              numeric &= isfinite(fdata[i]);
            }

            if (numeric)
            {
              if (rank == 0)
                MLPutReal(loopback, fdata[0]);
              else
                MLPutRealArray(loopback, fdata, dims.data(), NULL, rank);
            }
            else
            {
              put_general_array(loopback, fdata, dims.data(), rank);
            }

            delete [] fdata;
          }
          else if (size == 4)
          {
            float *fdata = 0;
            try
            {
              fdata = new float[nElems];
            }
            catch(bad_alloc e)
            {
              throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
            }
            if (H5Dread(dataset.getId(), datatype.getId(), memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
            {
              delete [] fdata;
              throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
            }

            if (rank == 0)
              MLPutReal32(loopback, fdata[0]);
            else
              MLPutReal32Array(loopback, fdata, dims2.data(), NULL, rank);
            delete [] fdata;
          }
          else
          {
            assert(0);
          }
          break;

        case H5T_ARRAY:
          {
            const int array_rank = H5Tget_array_ndims(datatype.getId());
            vector<hsize_t> array_dims(rank);
            H5Tget_array_dims(datatype.getId(), array_dims.data());

            /* Determine size of array data */
            int nArrayElems = 1;
            for (int j = 0; j < array_rank; j++)
            {
              nArrayElems *= array_dims[j];
            }

            dims.resize(rank + array_rank);
            dims2.resize(rank + array_rank);
            for (int j = 0; j < array_rank; j++)
            {
              dims[j+rank] = array_dims[j];
              dims2[j+rank] = array_dims[j];
            }

            if (superSize == 8)
            {
              double *fdata = 0;
              try
              {
                fdata = new double[nElems*nArrayElems];
              }
              catch(bad_alloc e)
              {
                throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
              }
              if (H5Dread(dataset.getId(), datatype.getId(), memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
              {
                delete [] fdata;
                throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
              }

              bool numeric = true;
              for (int i = 0; i < nElems*nArrayElems; i++)
              {
                numeric &= isfinite(fdata[i]);
              }

              if (numeric)
              {
                MLPutRealArray(loopback, fdata, dims.data(), NULL, rank + array_rank);
              }
              else
              {
                put_general_array(loopback, fdata, dims.data(), rank + array_rank);
              }

              delete [] fdata;
            }
            else if (superSize == 4)
            {
              float *fdata = 0;
              try
              {
                fdata = new float[nElems*nArrayElems];
              }
              catch(bad_alloc e)
              {
                throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
              }
              if (H5Dread(dataset.getId(), datatype.getId(), memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
              {
                delete [] fdata;
                throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
              }

              MLPutReal32Array(loopback, fdata, dims2.data(), NULL, rank + array_rank);
              delete [] fdata;
            }
            else
            {
              assert(0);
            }
          }
          break;

        case H5T_COMPOUND:
          {
            /* Get information about compound dataset members */
            int nmembers = H5Tget_nmembers(datatype.getId());
            vector<H5T_class_t> memberClasses(nmembers);
            vector<string> memberNames(nmembers);
            vector<hid_t> memberTypes(nmembers);
            vector<size_t> memberSizes(nmembers);

            for (int j=0; j<nmembers; j++)
            {
              memberClasses[j] = H5Tget_member_class(datatype.getId(), j);
              char * memberName = H5Tget_member_name(datatype.getId(), j);
              memberNames[j] = string(memberName);
              H5free_memory(memberName);

              hid_t type = H5Tget_member_type(datatype.getId(), j);
              memberSizes[j] = H5Tget_size(type);
              memberTypes[j] = H5Tcreate(H5T_COMPOUND, memberSizes[j]);
              H5Tinsert(memberTypes[j], memberNames[j].data(), 0, type);
              H5Tclose(type);

              if (!((memberClasses[j] == H5T_INTEGER && (memberSizes[j] == 8 || memberSizes[j] == 4 || memberSizes[j] == 2 || memberSizes[j] == 1)) ||
                    (memberClasses[j] == H5T_FLOAT && (memberSizes[j] == 8 || memberSizes[j] == 4))))
              {
                stringstream ss;
                ss << "Unsupported datatype in compound datatype: "
                   << "class " << memberClasses[j] << ", size " << memberSizes[j];
                string str = ss.str();
                throw(H5Exception(str));
              }
            }

            MLPutFunction(loopback, "List", nmembers);
            for (int j=0; j<nmembers; j++)
            {
              MLPutFunction(loopback, "Rule", 2);
              MLPutString(loopback, memberNames[j].data());

              /* The following should really be factored out to avoid duplicating it */
              switch(memberClasses[j])
              {
              case H5T_INTEGER:
                if(memberSizes[j] == 8)
                {
                  vector<mlint64> idata(nElems);
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, idata.data()) < 0)
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  if (rank == 0)
                    MLPutInteger64(loopback, idata[0]);
                  else
                    MLPutInteger64Array(loopback, idata.data(), dims2.data(), 0, rank);
                } else if(memberSizes[j] == 4) {
                  vector<int> idata(nElems);
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, idata.data()) < 0)
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  if (rank == 0)
                    MLPutInteger(loopback, idata[0]);
                  else
                    MLPutIntegerArray(loopback, idata.data(), dims.data(), 0, rank);
                } else if(memberSizes[j]==2) {
                  vector<short> sdata(nElems);
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, sdata.data()) < 0)
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  if (rank == 0)
                    MLPutInteger16(loopback, sdata[0]);
                  else
                    MLPutInteger16Array(loopback, sdata.data(), dims2.data(), 0, rank);
                } else if(memberSizes[j]==1) {
                  vector<char> cdata(nElems);
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, cdata.data()) < 0)
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  MLPutString(loopback, cdata.data());
                }
                break;
              case H5T_FLOAT:
                if (memberSizes[j] == 8)
                {
                  double *fdata = 0;
                  try
                  {
                    fdata = new double[nElems];
                  }
                  catch(bad_alloc e)
                  {
                    throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
                  }
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
                  {
                    delete [] fdata;
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  }

                  bool numeric = true;
                  for (int i = 0; i < nElems; i++)
                  {
                    numeric &= isfinite(fdata[i]);
                  }

                  if (numeric)
                  {
                    if (rank == 0)
                      MLPutReal(loopback, fdata[0]);
                    else
                      MLPutRealArray(loopback, fdata, dims.data(), NULL, rank);
                  }
                  else
                  {
                    put_general_array(loopback, fdata, dims.data(), rank);
                  }

                  delete [] fdata;
                }
                else if (memberSizes[j] == 4)
                {
                  float *fdata = 0;
                  try
                  {
                    fdata = new float[nElems];
                  }
                  catch(bad_alloc e)
                  {
                    throw(H5Exception("Failed to allocate memory for dataset " + datasetNames[i]));
                  }
                  if (H5Dread(dataset.getId(), memberTypes[j], memspace, dataspace.getId(), H5P_DEFAULT, fdata) < 0)
                  {
                    delete [] fdata;
                    throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
                  }

                  if (rank == 0)
                    MLPutReal32(loopback, fdata[0]);
                  else
                    MLPutReal32Array(loopback, fdata, dims2.data(), NULL, rank);
                  delete [] fdata;
                }
                else
                {
                  assert(0);
                }
                break;

              default:
                throw(H5Exception("Data format not supported"));
              }
            }

            /* TODO: make sure this is always run */
            for (int j=0; j<nmembers; j++)
              H5Tclose(memberTypes[j]);
          }
          break;

        default:
          throw(H5Exception("Data format not supported"));
        }
      }

    }
  }
  catch(H5Exception err) {
    fail(FAIL_HDF5, err.getCMessage());
    MLClose(loopback);
    return;
  }

  /* Transfer data from loopback to actual Mathematica link */
  MLTransferToEndOfLoopbackLink(stdlink, loopback);

  MLClose(loopback);
}

void ReadDatasetAttributes(const char *fileName)
{
  /* Get the list of datasets to be read from Mathematica*/
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);
  if(n<0)
    return;

  /* Create a loopback link to store the list until we are sure it can be fully
     filled. This way if something fails we can abort and send $Failed back. */
  MLINK loopback;
  loopback = MLLoopbackOpen(stdenv, NULL);

  if(!loopback) {
    fail(FAIL_ML, NULL);
    return;
  }

  try
  {
    H5F file(fileName);

    MLPutFunction(loopback, "List", n);

    /* Loop over all requested datasets */
    for(int i=0; i<n; i++)
    {
      /* Respond to abort from Mathematica */
      if(MLAbort)
      {
        MLPutFunction(stdlink, "Abort", 0);
        MLClose(loopback);
        return;
      }

      H5D dataset(file, datasetNames[i]);
      int nAttrs = dataset.getNumAttrs();

      MLPutFunction(loopback, "List", nAttrs);

      H5Aiterate(dataset.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, 0, put_dataset_attribute, &loopback);
    }
  }
  catch(H5Exception err) {
    fail(FAIL_HDF5, err.getCMessage());
    MLClose(loopback);
    return;
  }

  /* Transfer data from loopback to actual Mathematica link */
  MLTransferToEndOfLoopbackLink(stdlink, loopback);

  MLClose(loopback);
}

void ReadDatasetNames(const char *fileName)
{
  H5F file(fileName);

  vector<string> datasetNames;

  H5Ovisit(file.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, put_dataset_name, &datasetNames);

  int numDatasets = datasetNames.size();
  MLPutFunction(stdlink, "List", numDatasets);
  for(int i=0; i<numDatasets; i++)
  {
    MLPutString(stdlink, datasetNames[i].c_str());
  }

  return;
}

void ReadDatasetNamesFast(const char *fileName)
{
  H5F file(fileName);

  H5G_info_t group_info;
  H5Gget_info_by_name(file.getId(), "/", &group_info, H5P_DEFAULT);
  int num_links = group_info.nlinks;

  DatasetNames dns;
  dns.num_links = num_links;

  herr_t err = H5Literate_by_name(file.getId(), "/", H5_INDEX_NAME, H5_ITER_NATIVE, NULL, 
                                  put_dataset_name_fast, &dns, H5P_DEFAULT);

  if (err < 0)
  {
    MLPutFunction(stdlink, "Abort", 0);
    return;
  }

  int numDatasets = dns.datasetNames.size();

  MLPutFunction(stdlink, "List", numDatasets);
  for(int i=0; i<numDatasets; i++)
  {
    MLPutString(stdlink, dns.datasetNames[i].c_str());
  }

  return;
}

herr_t put_dataset_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data)
{
  vector<string> *datasetNames = (vector<string> *)op_data;
  if(object_info->type == H5O_TYPE_DATASET)
    datasetNames->push_back("/" + string(name));
  return 0;
}

herr_t put_dataset_name_fast(hid_t loc_id, const char *name, const H5L_info_t*, void *opdata)
{
  DatasetNames *dns = (DatasetNames *) opdata;
  dns->datasetNames.push_back("/" + string(name));

  double progress_frac = (double) dns->datasetNames.size() / (double) dns->num_links;

  if (progress_frac - dns->last_progress >= 0.01)
  {
    ostringstream progress;
    progress << "h5mma`ReadDatasetsProgress = " << progress_frac;
    MLEvaluateString(stdlink, (char *) progress.str().c_str());
    dns->last_progress = progress_frac;
  }

  // Return a negative number to indicate an error if Mathematica is
  // requesting an Abort
  return MLAbort ? -1 : 0;
}


herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data)
{
  MLINK loopback = *((MLINK*)op_data);

  H5A attr(location_id, attr_name);

  H5T datatype(attr);
  H5T_class_t typeclass = H5Tget_class(datatype.getId());
  size_t size = datatype.getSize();

  MLPutFunction(loopback, "Rule", 2);
  MLPutString(loopback, attr_name);

  if ((typeclass == H5T_INTEGER && size == 4) ||
      (typeclass == H5T_FLOAT && size == 8))
  {
    H5S dataspace(attr);
    const int rank = dataspace.getSimpleExtentNDims();
    vector<hsize_t> dims(rank);
    dataspace.getSimpleExtentDims(dims.data());
    int nElems = 1;
    for (int k = 0; k < rank; k++)
    {
      nElems *= dims[k];
    }

    vector<long int> idims(rank);
    for (int k = 0; k < rank; k++)
    {
      idims[k] = dims[k];
    }

    char* values = new char[nElems*size];
    if(H5Aread(attr.getId(), datatype.getNativeId(), (void *)values)<0)
      throw(H5Exception("Failed to read data for attribute"));

    if (typeclass == H5T_INTEGER)
      MLPutIntegerArray(loopback,(int *) values, idims.data(), 0, rank);
    else if (typeclass == H5T_FLOAT)
      MLPutRealArray(loopback,(double *) values, idims.data(), 0, rank);

    delete[] values;
  }
  else if (typeclass == H5T_STRING)
  {
    /* Include space for a null terminator in case it isn't in the attribute */
    vector<char> str(size+1);
    str[size] = '\0';
    if(H5Aread(attr.getId(), datatype.getNativeId(), (void *)str.data())<0)
      throw(H5Exception("Failed to read data for attribute"));
    MLPutString(loopback, str.data());
  }
  else
  {
    MLPutSymbol(loopback, "Null");
  }

  return 0;
}

// Output a list of lists representing the array of doubles.  
static int put_general_array(MLINK link, const double *fdata, long int dims[], int rank)
{
  int npoints = 1;
  for (int i = 0; i < rank; i++)
  {
    npoints *= dims[i];
  }

  if (rank == 0) {
    if (isfinite(fdata[0]))
    {
      MLPutReal(link, fdata[0]);
    }
    else if (isnan(fdata[0]))
    {
      MLPutSymbol(link, "Indeterminate");
    }
    else if (isinf(fdata[0]))
    {
      MLPutFunction(link, "DirectedInfinity", 1);
      if(fdata[0]==std::numeric_limits<double>::infinity())
        MLPutInteger(link, 1);
      else
        MLPutInteger(link, -1);
    }
    else
    {
      // Not sure what to do here.  Can this happen?
      MLPutSymbol(link, "Unknown");
    }
  } else if (rank == 1) {
    MLPutFunction(link, "List", dims[0]);
    for (int i = 0; i < dims[0]; i++)
    {
      if (isfinite(fdata[i]))
      {
        MLPutReal(link, fdata[i]);
      }
      else if (isnan(fdata[i]))
      {
        MLPutSymbol(link, "Indeterminate");
      }
      else if (isinf(fdata[i]))
      {
        MLPutFunction(link, "DirectedInfinity", 1);
        if(fdata[i]==std::numeric_limits<double>::infinity())
          MLPutInteger(link, 1);
        else
          MLPutInteger(link, -1);
      }
      else
      {
        // Not sure what to do here.  Can this happen?
        MLPutSymbol(link, "Unknown");
      }
    }
  }
  else
  {
    MLPutFunction(link, "List", dims[0]);
    int nsubpoints = npoints/dims[0];
    for (int i = 0; i < dims[0]; i++)
    {
      put_general_array(link, fdata+i*nsubpoints, dims+1, rank-1);
    }
  }
  return 0;
}

#if WINDOWS_MATHLINK

int WINAPI WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
  char buff[512];
  char FAR * buff_start = buff;
  char FAR * argv[32];
  char FAR * FAR * argv_end = argv + 32;

  MLScanString(argv, &argv_end, &lpszCmdLine, &buff_start);
  return MLMain((int)(argv_end - argv), argv);
}

#else
int main(int argc, char* argv[])
{
  return MLMain(argc, argv);
}

#endif
