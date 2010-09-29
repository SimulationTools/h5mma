
#include <vector>
#include <cstring>
#include <iostream>
#include <assert.h>
#include "H5Cpp.h"
#include "mathlink.h" 
#include "time.h"

using namespace H5;
using namespace std;

herr_t put_dataset_name(hid_t loc_id, const char *name, void *opdata);

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

  try
  {
    H5File  file(fileName, H5F_ACC_RDONLY);

    MLPutFunction(stdlink, "List", n);

    /* Loop over all requested datasets */
    for(int i=0; i<n; i++)
    {
      DataSet dataset = file.openDataSet(datasetNames[i].c_str());
      DataSpace dataspace = dataset.getSpace();
      const int rank = dataspace.getSimpleExtentNdims();
      hsize_t dims_out[rank];
      dataspace.getSimpleExtentDims(dims_out, NULL);

      int dims[rank];
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      MLPutIntegerList(stdlink, dims, rank);
    }
  }

  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
     fail();
     error.printError();
     return;
  }
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
    fail();
    error.printError();
    return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
    fail();
    error.printError();
    return;
  }
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

  try
  {
    H5File  file(fileName, H5F_ACC_RDONLY);

    MLPutFunction(stdlink, "List", n);

    /* Loop over all requested datasets */
    for(int i=0; i<n; i++)
    {
      DataSet dataset = file.openDataSet(datasetNames[i].c_str());
      H5T_class_t type_class = dataset.getTypeClass();
      assert(type_class == H5T_FLOAT);
      DataSpace dataspace = dataset.getSpace();
      const int rank = dataspace.getSimpleExtentNdims();
      hsize_t dims_out[rank];
      dataspace.getSimpleExtentDims(dims_out, NULL);
      int nElems = 1;
      for (int j = 0; j < rank; j++)
      {
        nElems *= dims_out[j];
      }

      FloatType floatType = dataset.getFloatType();
      size_t size = floatType.getSize();

      assert(size == 8);

      double *data = new double[nElems];

      dataset.read(data, floatType);

      long int dims[rank];
      for (int j = 0; j < rank; j++)
      {
        dims[j] = dims_out[j];
      }

      MLPutRealArray(stdlink,data,dims,NULL,rank);
      delete[] data;
    }
  }  // end of try block

  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
     error.printError();
     fail();
     return;
  }

  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     fail();
     error.printError();
     return;
  }
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

  try
  {
    H5File  file(fileName, H5F_ACC_RDONLY);

    MLPutFunction(stdlink, "List", n);

    /* Loop over all requested datasets */
    for(int i=0; i<n; i++)
    {
      DataSet dataset = file.openDataSet(datasetNames[i].c_str());

      int nAttrs = dataset.getNumAttrs();

      MLPutFunction(stdlink, "List", nAttrs);

      for (int j = 0; j < nAttrs; j++)
      {
        Attribute attr = dataset.openAttribute(j);
        string name(attr.getName());
        DataType dataType = attr.getDataType();
        int size = dataType.getSize();

        MLPutFunction(stdlink, "Rule", 2);
        MLPutString(stdlink, name.c_str());

        if ((dataType.getClass() == H5T_INTEGER && size == 4) ||
            (dataType.getClass() == H5T_FLOAT) && size == 8)
        {
          DataSpace space(attr.getSpace());
          int nDims = space.getSimpleExtentNdims();
          hsize_t dims[nDims];
          space.getSimpleExtentDims(dims);
          int nElems = 1;
          for (int k = 0; k < nDims; k++)
          {
            nElems *= dims[k];
          }

          long int idims[nDims];
          for (int k = 0; k < nDims; k++)
          {
            idims[k] = dims[k];
          }

          char* values = new char[nElems*size];
          attr.read(dataType, (void *)values);

          if (dataType.getClass() == H5T_INTEGER)
            MLPutIntegerArray(stdlink,(int *) values, idims, 0, nDims);
          else if (dataType.getClass() == H5T_FLOAT)
            MLPutRealArray(stdlink,(double *) values, idims, 0, nDims);

          delete[] values;
        }
        else if (dataType.getClass() == H5T_STRING)
        {
          char str[size];
          attr.read(dataType, str);
          MLPutString(stdlink, str);
        }
        else
        {
          MLPutSymbol(stdlink, "Null");
        }
      }
    }
  }  // end of try block

  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     fail();
     error.printError();
     return;
  }
}


void ReadDatasetNames(const char *fileName)
{
  try
  {
    H5File  file(fileName, H5F_ACC_RDONLY);
    vector<string> datasetNames;

    file.iterateElems("/", NULL, put_dataset_name, &datasetNames);

    int numDatasets = datasetNames.size();
    MLPutFunction(stdlink, "List", numDatasets);
    for(int i=0; i<numDatasets; i++)
    {
      MLPutString(stdlink, datasetNames[i].c_str());
    }
  }  // end of try block

  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
     error.printError();
     fail();
     return;
  }

  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     fail();
     error.printError();
     return;
  }

  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     fail();
     error.printError();
     return;
  }

  return;
}

herr_t put_dataset_name(hid_t loc_id, const char *name, void *opdata)
{
  vector<string> *datasetNames = (vector<string> *)opdata;
  datasetNames->push_back("/" + string(name));
  return 0;
}

int main(int argc, char* argv[]) 
{
  return MLMain(argc, argv); 
} 
