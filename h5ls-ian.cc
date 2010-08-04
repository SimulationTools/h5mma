
#include <vector>
#include <cstring>
#include <iostream>
#include "H5Cpp.h"

using namespace H5;
using namespace std;
using std::string;

int main(int argc, char *argv[])
{
   try
   {
     H5File file("phi.0.xy.h5", H5F_ACC_RDONLY );

     for (int i = 0; i < file.getNumObjs(); i++)
     {
       string name(file.getObjnameByIdx(i));
       H5G_stat_t object_info;
       file.getObjinfo(name, object_info);
       if (object_info.type == H5G_DATASET)
       {
         cout << "/" + name << endl;
       }      
     }
   }  // end of try block

   // catch failure caused by the H5File operations
   catch( FileIException error )
   {
      error.printError();
      return -1;
   }

   // catch failure caused by the DataSet operations
   catch( DataSetIException error )
   {
      error.printError();
      return -1;
   }

   // catch failure caused by the DataSpace operations
   catch( DataSpaceIException error )
   {
      error.printError();
      return -1;
   }

   // catch failure caused by the DataSpace operations
   catch( DataTypeIException error )
   {
      error.printError();
      return -1;
   }


  return 0;
}
