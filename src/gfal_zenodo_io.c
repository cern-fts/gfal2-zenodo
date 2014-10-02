/*
 *  Copyright 2014 CERN
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
**/

// Input/Output functions

#include "gfal_zenodo.h"
#include <common/gfal_common_err_helpers.h>
#include <json.h>
#include <string.h>


gfal_file_handle gfal2_zenodo_fopen(plugin_handle plugin_data, const char* url,
        int flag, mode_t mode, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
	return NULL;
}


ssize_t gfal2_zenodo_fread(plugin_handle plugin_data, gfal_file_handle fd, void* buff,
        size_t count, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


ssize_t gfal2_zenodo_fwrite(plugin_handle plugin_data, gfal_file_handle fd,
        const void* buff, size_t count, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


int gfal2_zenodo_fclose(plugin_handle plugin_data, gfal_file_handle fd, GError **error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


off_t gfal2_zenodo_fseek(plugin_handle plugin_data, gfal_file_handle fd, off_t offset,
        int whence, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}
