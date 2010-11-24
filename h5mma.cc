
#include <vector>
#include <cstring>
#include <map>
#include <iostream>
#include <sstream>
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

      /* Only accept 4 byte integers, 8 byte floats or 1 byte integers (as characters) */
      if (!((typeclass == H5T_INTEGER && size == 4) ||
            (typeclass == H5T_FLOAT && size == 8) ||
            (typeclass == H5T_INTEGER && size == 1) ))
      {
        throw(H5Exception("Unsupported datatype"));
      }

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

      long int dims[rank];
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      switch(typeclass)
      {
      case H5T_INTEGER:
        if(size == 4)
        {
          int idata[nElems];
          if (H5Dread(dataset.getId(), datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, idata) < 0)
            throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
          MLPutIntegerArray(loopback, idata, dims, 0, rank);
        } else if(size==1) {
          char cdata[nElems];
          if (H5Dread(dataset.getId(), datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, cdata) < 0)
            throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
          MLPutString(loopback, cdata);
        }
        break;
      case H5T_FLOAT:
        {
          double fdata[nElems];
          if (H5Dread(dataset.getId(), datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, fdata) < 0)
            throw(H5Exception("Failed to read data for dataset " + datasetNames[i]));
          MLPutRealArray(loopback, fdata, dims, NULL, rank);
        }
        break;
      default:
        throw(H5Exception("Data format not supported"));
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

class DatasetNames
{
public:
  DatasetNames()
  {
    num_links = 0;
    last_progress = 0.0;
  }
  vector<string> datasetNames;
  int num_links;
  double last_progress;
};

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
      (typeclass == H5T_FLOAT) && size == 8)
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

    if (typeclass == H5T_INTEGER)
      MLPutIntegerArray(loopback,(int *) values, idims, 0, rank);
    else if (typeclass == H5T_FLOAT)
      MLPutRealArray(loopback,(double *) values, idims, 0, rank);

    delete[] values;
  }
  else if (typeclass == H5T_STRING)
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
