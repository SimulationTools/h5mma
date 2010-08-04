
#include <vector>
#include <cstring>
#include <iostream>
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

void nestedList(double *allData, hsize_t dims[], int rank)
{
  if (rank == 0)
  {
    cout << *allData;
  }
  else
  {
    int n = 1;
    for (int i = 1; i < rank; i++)
    {
      n *= dims[i];
    }
    cout << "{";
    for (int i = 0; i < dims[0]; i++)
    {
      nestedList(&(allData[i*n]), &dims[1], rank-1);
      if (i != dims[0] - 1)
        cout << ",";
    }
    cout << "}";
  }
}

void nestedList(int *allData, hsize_t dims[], int rank)
{
  if (rank == 0)
  {
    cout << *allData;
  }
  else
  {
    int n = 1;
    for (int i = 1; i < rank; i++)
    {
      n *= dims[i];
    }
    cout << "{";
    for (int i = 0; i < dims[0]; i++)
    {
      nestedList(&(allData[i*n]), &dims[1], rank-1);
      if (i != dims[0] - 1)
        cout << ",";
    }
    cout << "}";
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
//     cout << "size = " << size << endl;

     assert(size == 8);

     double *data = new double[n];

     dataset.read(data, floatType);

//      cout << "rank " << rank << ", dimensions " <<
//        (unsigned long)(dims_out[0]) << " x " <<
//        (unsigned long)(dims_out[1]) << endl;

     long int dims[rank];
     for (int i = 0; i < rank; i++)
     {
       dims[i] = dims_out[i];
     }

     MLPutRealArray(stdlink,data,dims,NULL,rank);

//     nestedList(data, dims_out, rank);

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

// static void read_o_s(char *fname, char *dname, double *ox, double *oy, double *oz,
//                      double *sx, double *sy, double *sz)
// {
//      int file_id, data_id, attr_id, attr_err;
//      double tmp[3];

//      file_id = H5Fopen(fname,H5F_ACC_RDONLY,H5P_DEFAULT);
//      data_id = H5Dopen(file_id,dname);
//      attr_id = H5Aopen_name(data_id, "origin");
//      attr_err = H5Aread(attr_id, H5T_IEEE_F64LE, &tmp);
//      *oz = tmp[0];
//      *oy = tmp[1];
//      *ox = tmp[2];
//      H5Aclose(attr_id);
//      H5Dclose(data_id);
//      H5Fclose(file_id);
//      file_id = H5Fopen(fname,H5F_ACC_RDONLY,H5P_DEFAULT);
//      data_id = H5Dopen(file_id,dname);
//      attr_id = H5Aopen_name(data_id, "delta");
//      attr_err = H5Aread(attr_id, H5T_IEEE_F64LE, &tmp);
//      *sz = tmp[0];
//      *sy = tmp[1];
//      *sx = tmp[2];
//      H5Aclose(attr_id);
//      H5Dclose(data_id);
//      H5Fclose(file_id);

//      return;
// }



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
//       cout << "attr: " << i << endl;
       Attribute attr = dataset.openAttribute(i);
       string name(attr.getName());
//       cout << "name == " << name << endl;
       DataType dataType = attr.getDataType();
//       cout << "dataType == " << dataType.fromClass() << endl;
//       cout << "dataType.getSize() == " << dataType.getSize() << endl;
       int size = dataType.getSize();

       cout << name << " -> ";
       MLPutFunction(stdlink, "Rule", 2);
       MLPutString(stdlink, name.c_str());

       if ((dataType.getClass() == H5T_INTEGER && size == 4) || 
           (dataType.getClass() == H5T_FLOAT) && size == 8)
       {
         DataSpace space(attr.getSpace());
         int nDims = space.getSimpleExtentNdims();
//         cout << "nDims == " << nDims << endl;
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
           nestedList((int *) values, dims, nDims);
         else if (dataType.getClass() == H5T_FLOAT)
           nestedList((double *) values, dims, nDims);

         cout << endl;

         if (dataType.getClass() == H5T_INTEGER)
           MLPutIntegerArray(stdlink,(int *) values, idims, 0, nDims);
         else if (dataType.getClass() == H5T_FLOAT)
           MLPutRealArray(stdlink,(double *) values, idims, 0, nDims);
       }
       else if (dataType.getClass() == H5T_STRING)
       {
         char str[size];
         attr.read(dataType, str);
         cout << str << endl;
         MLPutString(stdlink, str);
       }
       else
       {
         cout << "<unsupported attribute type>" << endl;
         MLPutSymbol(stdlink, "Null");
       }

       //     cout << endl;
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
     for (int i = 0; i < file.getNumObjs(); i++)
     {
       if (time(NULL) > lastSet + 1)
       {
         char status[1000];
         snprintf(status, sizeof(status), "dsIndex = 1.0*%d/%d\n", i, file.getNumObjs());
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
     snprintf(status, sizeof(status), "dsIndex = 1.0\n");
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
//  ReadDataset(argv[1], argv[2]);
//  ReadDatasetAttributes(argv[1], argv[2]);

  return MLMain(argc, argv); 
} 
