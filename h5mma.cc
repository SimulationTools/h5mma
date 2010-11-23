
#include <vector>
#include <cstring>
#include <map>
#include <iostream>
#include <assert.h>
#include "h5wrapper.h"
#include "mathlink.h" 
#include "time.h"

using namespace std;

#define FAIL_ML       1
#define FAIL_HDF5     2
#define FAIL_INVALID  3

extern "C"
{
  herr_t put_dataset_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
  herr_t put_dataset_name_fast(hid_t loc_id, const char *name, const H5L_info_t*, void *opdata);
  herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data);
}

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
      hsize_t dims_out[rank];
      dataspace.getSimpleExtentDims(dims_out);

      int dims[rank];
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      /* Send the dimensions of this dataset to the loopback link */
      MLPutIntegerList(loopback, dims, rank);
    }
  }
  catch(H5Exception err) {
    fail(FAIL_HDF5, err.getCMessage());
    MLClose(loopback);
    return;
  }

  /* Transfer data from loopback to actual Mathematica link */
  MLTransferExpression(stdlink, loopback);

  MLClose(loopback);
}

void ReadDatasets(const char *fileName)
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

      /* Check we have 64 bit floating point numbers */
      H5T datatype(dataset);
      if((H5Tget_class(datatype.getId()) != H5T_FLOAT) ||  (datatype.getSize() != 8))
        throw(H5Exception("Unsupported datatype"));

      /* Get dimensions of this dataset */
      H5S dataspace(dataset);
      const int rank = dataspace.getSimpleExtentNDims();
      hsize_t dims_out[rank];
      dataspace.getSimpleExtentDims(dims_out);

      /* Read data */
      int nElems = 1;
      for (int j = 0; j < rank; j++)
      {
        nElems *= dims_out[j];
      }

      double *data = new double[nElems];

      if (H5Dread(dataset.getId(), datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0)
        throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));

      long int dims[rank];
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      MLPutRealArray(loopback, data, dims, NULL, rank);
      delete[] data;
    }
  }
  catch(H5Exception err) {
    fail(FAIL_HDF5, err.getCMessage());
    MLClose(loopback);
    return;
  }

  /* Transfer data from loopback to actual Mathematica link */
  MLTransferExpression(stdlink, loopback);

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
  MLTransferExpression(stdlink, loopback);

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

  vector<string> datasetNames;

  H5Literate_by_name(file.getId(), "/", H5_INDEX_NAME, H5_ITER_NATIVE, NULL, put_dataset_name_fast, &datasetNames, H5P_DEFAULT);

  int numDatasets = datasetNames.size();
  MLPutFunction(stdlink, "List", numDatasets);
  for(int i=0; i<numDatasets; i++)
  {
    MLPutString(stdlink, datasetNames[i].c_str());
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
  vector<string> *datasetNames = (vector<string> *)opdata;
  datasetNames->push_back("/" + string(name));
  return 0;
}



herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data)
{
  MLINK loopback = *((MLINK*)op_data);

  H5A attr(location_id, attr_name);

  H5T datatype(attr);
  size_t size = datatype.getSize();

  MLPutFunction(loopback, "Rule", 2);
  MLPutString(loopback, attr_name);

  if ((H5Tget_class(datatype.getId()) == H5T_INTEGER && size == 4) ||
      (H5Tget_class(datatype.getId()) == H5T_FLOAT) && size == 8)
  {
    H5S dataspace(attr);
    const int rank = dataspace.getSimpleExtentNDims();
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims(dims);
    int nElems = 1;
    for (int k = 0; k < rank; k++)
    {
      nElems *= dims[k];
    }

    long int idims[rank];
    for (int k = 0; k < rank; k++)
    {
      idims[k] = dims[k];
    }

    char* values = new char[nElems*size];
    if(H5Aread(attr.getId(), datatype.getId(), (void *)values)<0)
      throw(H5Exception("Failed to read data for attribute"));

    if (H5Tget_class(datatype.getId()) == H5T_INTEGER)
      MLPutIntegerArray(loopback,(int *) values, idims, 0, rank);
    else if (H5Tget_class(datatype.getId()) == H5T_FLOAT)
      MLPutRealArray(loopback,(double *) values, idims, 0, rank);

    delete[] values;
  }
  else if (H5Tget_class(datatype.getId()) == H5T_STRING)
  {
    char str[size];
    if(H5Aread(attr.getId(), datatype.getId(), (void *)str)<0)
      throw(H5Exception("Failed to read data for attribute"));
    MLPutString(loopback, str);
  }
  else
  {
    MLPutSymbol(loopback, "Null");
  }

  return 0;
}

int main(int argc, char* argv[]) 
{
  return MLMain(argc, argv); 
} 
