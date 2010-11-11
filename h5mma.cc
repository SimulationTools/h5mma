
#include <vector>
#include <cstring>
#include <map>
#include <iostream>
#include <assert.h>
#include "hdf5.h"
#include "mathlink.h" 
#include "time.h"

using namespace std;

extern "C"
{
  herr_t put_dataset_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
  herr_t put_dataset_name_fast(hid_t loc_id, const char *name, const H5L_info_t*, void *opdata);
  herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data);
}

void fail()
{
  MLPutSymbol(stdlink, "$Failed");
}

/* Get the list of requested datasets from Mathematica */
long GetDatasetNames(vector<string> *datasetNames)
{
  long n;

  MLCheckFunction(stdlink, "List", &n);

  for(int j=0; j<n; j++)
  {
    const char *datasetName;
    if(MLGetString(stdlink, &datasetName))
    {
      datasetNames->push_back(datasetName);
      MLReleaseString(stdlink, datasetName);
    } else {
      cout << "Error receiving dataset names: " << MLError(stdlink) << endl;
      return -1;
    }
  }

  return n;
}

void ReadDatasetDimensions(const char *fileName)
{
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);

  if(n<0)
  {
    fail();
    return;
  }

  hid_t file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file < 0) {fail(); return;};

  MLPutFunction(stdlink, "List", n);

  /* Loop over all requested datasets */
  for(int i=0; i<n; i++)
  {
    hid_t dataset = H5Dopen(file, datasetNames[i].c_str(), H5P_DEFAULT);
    if (dataset < 0) {fail(); return;};
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {fail(); return;};
    const int rank = H5Sget_simple_extent_ndims(dataspace);
    hsize_t dims_out[rank];
    H5Sget_simple_extent_dims(dataspace, dims_out, NULL);

    int dims[rank];
    for (int j = 0; j < rank; j++)
    {
      dims[j] = dims_out[j];
    }

    MLPutIntegerList(stdlink, dims, rank);
    if (H5Sclose(dataspace) < 0) {fail(); return;};
    if (H5Dclose(dataset) < 0) {fail(); return;};
  }
  if (H5Fclose(file) < 0) {fail(); return;};
}

void ReadDatasets(const char *fileName)
{
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);

  if(n<0)
  {
    fail();
    return;
  }

  static map<string,hid_t> files;

  hid_t file;

  if (files.count(fileName) > 0)
  {
          file = files[fileName];
  }
  else
  {
          file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
          files[fileName] = file;
  }
  if (file < 0) {fail(); return;}

  MLPutFunction(stdlink, "List", n);

  /* Loop over all requested datasets */
  for(int i=0; i<n; i++)
  {
    hid_t dataset = H5Dopen(file, datasetNames[i].c_str(), H5P_DEFAULT);
    if (file < 0) {fail(); return;}
    
    hid_t datatype = H5Dget_type(dataset);
    if (datatype < 0) {fail(); return;}
    if (H5Tget_class(datatype) != H5T_FLOAT) {fail(); return;}
    
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {fail(); return;}

    const int rank = H5Sget_simple_extent_ndims(dataspace);
    if (rank < 0) {fail(); return;}

    hsize_t dims_out[rank];
    H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
    int nElems = 1;
    for (int j = 0; j < rank; j++)
    {
      nElems *= dims_out[j];
    }

    size_t size = H5Tget_size(datatype);

    if (size != 8) {fail(); return;}

    double *data = new double[nElems];

    if (H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0)
      {fail(); return;}

    long int dims[rank];
    for (int j = 0; j < rank; j++)
    {
      dims[j] = dims_out[j];
    }

    MLPutRealArray(stdlink, data, dims, NULL, rank);
    delete[] data;
    if (H5Sclose(dataspace) < 0) {fail(); return;};
    if (H5Tclose(datatype) < 0) {fail(); return;};
    if (H5Dclose(dataset) < 0) {fail(); return;};
  }
//  if (H5Fclose(file) < 0) {fail(); return;};

}

void ReadDatasetAttributes(const char *fileName)
{
  vector<string> datasetNames;
  long n = GetDatasetNames(&datasetNames);

  if(n<0)
  {
    fail();
    return;
  }

  hid_t file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file < 0) {fail(); return;}

  MLPutFunction(stdlink, "List", n);

  /* Loop over all requested datasets */
  for(int i=0; i<n; i++)
  {
    hid_t dataset = H5Dopen(file, datasetNames[i].c_str(), H5P_DEFAULT);
    if (dataset < 0) {fail(); return;}
    
    H5O_info_t object_info;
    H5Oget_info(dataset, &object_info);
    int nAttrs = object_info.num_attrs;

    MLPutFunction(stdlink, "List", nAttrs);

    H5Aiterate(dataset, H5_INDEX_NAME, H5_ITER_NATIVE, 0, put_dataset_attribute, NULL);
    if (H5Dclose(dataset) < 0) {fail(); return;};
  }
  if (H5Fclose(file) < 0) {fail(); return;};
}

void ReadDatasetNames(const char *fileName)
{
  hid_t file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file < 0) {fail(); return;};
  vector<string> datasetNames;

  H5Ovisit(file, H5_INDEX_NAME, H5_ITER_NATIVE, put_dataset_name, &datasetNames);

  int numDatasets = datasetNames.size();
  MLPutFunction(stdlink, "List", numDatasets);
  for(int i=0; i<numDatasets; i++)
  {
    MLPutString(stdlink, datasetNames[i].c_str());
  }

  if (H5Fclose(file) < 0) {fail(); return;};
  return;
}

void ReadDatasetNamesFast(const char *fileName)
{
  hid_t file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file < 0) {fail(); return;};
  vector<string> datasetNames;

  H5Literate_by_name(file, "/", H5_INDEX_NAME, H5_ITER_NATIVE, NULL, put_dataset_name_fast, &datasetNames, H5P_DEFAULT);

  int numDatasets = datasetNames.size();
  MLPutFunction(stdlink, "List", numDatasets);
  for(int i=0; i<numDatasets; i++)
  {
    MLPutString(stdlink, datasetNames[i].c_str());
  }

  if (H5Fclose(file) < 0) {fail(); return;};
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
  hid_t attr = H5Aopen(location_id, attr_name, H5P_DEFAULT);
  if (attr < 0) {fail(); return -1;};
  hid_t datatype = H5Aget_type(attr);
  if (datatype < 0) {fail(); return -1;}
  H5T_class_t typeclass = H5Tget_class(datatype);
  size_t size = H5Tget_size(datatype);

  MLPutFunction(stdlink, "Rule", 2);
  MLPutString(stdlink, attr_name);

  if ((typeclass == H5T_INTEGER && size == 4) ||
      (typeclass == H5T_FLOAT) && size == 8)
  {
    hid_t dataspace = H5Aget_space(attr);
    const int rank = H5Sget_simple_extent_ndims(dataspace);
    hsize_t dims[rank];
    H5Sget_simple_extent_dims(dataspace, dims, NULL);
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
    H5Aread(attr, datatype, (void *)values);

    if (typeclass == H5T_INTEGER)
      MLPutIntegerArray(stdlink,(int *) values, idims, 0, rank);
    else if (typeclass == H5T_FLOAT)
      MLPutRealArray(stdlink,(double *) values, idims, 0, rank);

    delete[] values;
    if (H5Sclose(dataspace) < 0) {fail(); return -1;};
  }
  else if (typeclass == H5T_STRING)
  {
    char str[size];
    H5Aread(attr, datatype, str);
    MLPutString(stdlink, str);
  }
  else
  {
    MLPutSymbol(stdlink, "Null");
  }

  if (H5Tclose(datatype) < 0) {fail(); return -1;};
  if (H5Aclose(attr) < 0) {fail(); return -1;};
  return 0;
}

int main(int argc, char* argv[]) 
{
  return MLMain(argc, argv); 
} 
