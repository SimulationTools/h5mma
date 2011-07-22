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

#include "h5wrapper.h"
#include "mathlink.h"

#define FAIL_ML       1
#define FAIL_HDF5     2
#define FAIL_INVALID  3

extern "C"
{
  herr_t put_dataset_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
  herr_t put_dataset_name_fast(hid_t loc_id, const char *name, const H5L_info_t*, void *opdata);
  herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data);
}

void fail(int type, const char *err);
long GetDatasetNames(vector<string> *datasetNames);
void ReadDatasetDimensions(const char *fileName);
void ReadDatasets(const char *fileName);
void ReadDatasetAttributes(const char *fileName);
void ReadDatasetNames(const char *fileName);
void ReadDatasetNamesFast(const char *fileName);

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
