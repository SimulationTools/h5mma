
#include <vector>
#include <cstring>
#include <iostream>
#include <assert.h>
#include "H5Cpp.h"
#include "mathlink.h" 
#include "time.h"

using namespace H5;
using namespace std;
using std::string;

void fail()
{
  MLPutSymbol(stdlink, "$Failed");
}

void ReadDatasetDimensions(const char *fileName, const char *datasetName)
{
  try
   {
     H5File  file(fileName, H5F_ACC_RDONLY);
     DataSet dataset = file.openDataSet(datasetName);
     DataSpace dataspace = dataset.getSpace();
     const int rank = dataspace.getSimpleExtentNdims();
     hsize_t dims_out[rank];
     int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);

     int dims[rank];
     for (int i = 0; i < rank; i++)
     {
       dims[i] = dims_out[i];
     }

     MLPutIntegerList(stdlink, dims, rank);
   }

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
}

void ReadDataset(const char *fileName, const char *datasetName)
{
  try
   {
     H5File  file(fileName, H5F_ACC_RDONLY);
     DataSet dataset = file.openDataSet(datasetName);
     H5T_class_t type_class = dataset.getTypeClass();
     assert(type_class == H5T_FLOAT);
     DataSpace dataspace = dataset.getSpace();
     const int rank = dataspace.getSimpleExtentNdims();
     hsize_t dims_out[rank];
     int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
     int n = 1;
     for (int i = 0; i < rank; i++)
     {
       n *= dims_out[i];
     }

     FloatType floatType = dataset.getFloatType();
     size_t size = floatType.getSize();

     assert(size == 8);

     double *data = new double[n];

     dataset.read(data, floatType);

     long int dims[rank];
     for (int i = 0; i < rank; i++)
     {
       dims[i] = dims_out[i];
     }

     MLPutRealArray(stdlink,data,dims,NULL,rank);

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

void ReadDatasetAttributes(const char *fileName, const char *datasetName)
{
  try
   {
     H5File  file(fileName, H5F_ACC_RDONLY);
     DataSet dataset = file.openDataSet(datasetName);

     int nAttrs = dataset.getNumAttrs();

     MLPutFunction(stdlink, "List", nAttrs);

     for (int i = 0; i < nAttrs; i++)
     {
       Attribute attr = dataset.openAttribute(i);
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
         int n = 1;
         for (int j = 0; j < nDims; j++)
         {
           n *= dims[j];
         }

         long int idims[nDims];
         for (int i = 0; i < nDims; i++)
         {
           idims[i] = dims[i];
         }

         void *values = new char[n*size];
         attr.read(dataType, values);

         if (dataType.getClass() == H5T_INTEGER)
           MLPutIntegerArray(stdlink,(int *) values, idims, 0, nDims);
         else if (dataType.getClass() == H5T_FLOAT)
           MLPutRealArray(stdlink,(double *) values, idims, 0, nDims);
       }
       else if (dataType.getClass() == H5T_STRING)
       {
         char str[size];
         attr.read(dataType, str);
         MLPutString(stdlink, str);
       }
       else
       {
         cout << "<unsupported attribute type>" << endl;
         MLPutSymbol(stdlink, "Null");
       }
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


void ReadDatasets(const char *fileName)
{
   try
   {
     H5File  file(fileName, H5F_ACC_RDONLY);
     vector<string>  datasets;
     time_t  lastSet = time(NULL);
     int numObjs = file.getNumObjs();

     for (int i = 0; i < numObjs; i++)
     {
       /* Update the dsIndex progress variable in Mathematica every second*/
       if (time(NULL) > lastSet + 1)
       {
         char status[1000];
         snprintf(status, sizeof(status), "h5mma`Private`dsIndex = 1.0*%d/%d\n", i, numObjs);
         MLEvaluateString(stdlink, status);

         if (MLError(stdlink))
         {
           fail();
           return;
         }
         lastSet = time(NULL);
       }

       string name(file.getObjnameByIdx(i));
       H5G_stat_t object_info;
       file.getObjinfo(name, object_info);
       if (object_info.type == H5G_DATASET)
       {
         datasets.push_back("/"+name);
       }      
       if (MLAbort)
       {
         fail();
         return;
       }
     }
     char status[1000];
     snprintf(status, sizeof(status), "h5mma`Private`dsIndex = 1.0\n");
     MLEvaluateString(stdlink, status);

     MLPutFunction(stdlink, "List", datasets.size());
     for (int i = 0; i < datasets.size(); i++)
     {
       MLPutString(stdlink, datasets[i].c_str());
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

int main(int argc, char* argv[]) 
{
  return MLMain(argc, argv); 
} 
