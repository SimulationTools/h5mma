/************************************************************

  Create a test dataset consisting of array data

 ************************************************************/

#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>

/* Dataset size */
#define NXD  3
#define NYD  4
#define DATASET_RANK 2

/* Array size */
#define NXA  5
#define NYA  6
#define ARRAY_RANK 2

int main (void)
{
  hid_t   h5file, dataspace, datatype, dataset;
  hsize_t array_dims[2] = {NXA, NYA};
  hsize_t dataset_dims[2] = {NXD, NYD};
  double  data[NXA*NYA*NXD*NYD];

  for(int i = 0; i < NXA*NYA*NXD*NYD; ++i)
    data[i] = i;

  h5file = H5Fcreate("array.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  dataspace = H5Screate_simple(DATASET_RANK, dataset_dims, NULL);
  datatype = H5Tarray_create(H5T_NATIVE_DOUBLE, ARRAY_RANK, array_dims);
  dataset = H5Dcreate(h5file, "array dataset", datatype, dataspace,
                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

  H5Dclose(dataset);
  H5Sclose(dataspace);
  H5Tclose(datatype);
  H5Fclose(h5file);

  return 0;
}
