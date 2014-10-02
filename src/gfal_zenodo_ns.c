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

// Namespace operations, except listing dir

#include "gfal_zenodo.h"
#include <common/gfal_common_err_helpers.h>
#include <json.h>
#include <string.h>


int gfal2_zenodo_stat(plugin_handle plugin_data, const char* url,
        struct stat *buf, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


int gfal2_zenodo_mkdir(plugin_handle plugin_data, const char* url, mode_t mode,
        gboolean rec_flag, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


int gfal2_zenodo_rmdir(plugin_handle plugin_data, const char* url,
        GError** error)
{
    return gfal2_zenodo_unlink(plugin_data, url, error);
}


int gfal2_zenodo_unlink(plugin_handle plugin_data, const char* url,
        GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


int gfal2_zenodo_rename(plugin_handle plugin_data, const char * oldurl,
        const char * urlnew, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}
