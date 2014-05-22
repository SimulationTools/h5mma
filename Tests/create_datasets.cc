/************************************************************

  Create a test dataset consisting of compund data

 ************************************************************/

#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int     intdata;
    float   floatdata;
    double  doubledata;
} compound_t;

int main (void)
{
    hid_t       h5file, filetype, memtype, dataspace, dataset;
    hsize_t     dims[1] = {3};
    compound_t  data[3];

    data[0].intdata = 1;
    data[0].floatdata = 43.21;
    data[0].doubledata = 12.31;
    data[1].intdata = 2;
    data[1].floatdata = 43.22;
    data[1].doubledata = 12.32;
    data[2].intdata = 3;
    data[2].floatdata = 43.23;
    data[2].doubledata = 12.33;

    h5file = H5Fcreate("compound.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    memtype = H5Tcreate(H5T_COMPOUND, sizeof(compound_t));
    H5Tinsert(memtype, "Integer data", HOFFSET(compound_t, intdata), H5T_NATIVE_INT);
    H5Tinsert(memtype, "Float data", HOFFSET(compound_t, floatdata), H5T_NATIVE_FLOAT);
    H5Tinsert(memtype, "Double data", HOFFSET(compound_t, doubledata), H5T_NATIVE_DOUBLE);

    filetype = H5Tcreate(H5T_COMPOUND, 4 + 4 + 8);
    H5Tinsert(filetype, "Integer data", 0, H5T_STD_I32LE);
    H5Tinsert(filetype, "Float data", 4, H5T_IEEE_F32LE);
    H5Tinsert(filetype, "Double data", 4 + 4, H5T_IEEE_F64LE);

    dataspace = H5Screate_simple(1, dims, NULL);

    dataset = H5Dcreate(h5file, "compound dataset", filetype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    H5Dclose(dataset);
    H5Sclose(dataspace);
    H5Tclose(filetype);
    H5Fclose(h5file);

    return 0;
}