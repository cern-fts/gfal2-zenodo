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

// Directory listing functions

#include "gfal_zenodo.h"
#include <logger/gfal_logger.h>
#include <common/gfal_common_err_helpers.h>
#include <json.h>
#include <string.h>
#include <time.h>


gfal_file_handle gfal2_zenodo_opendir(plugin_handle plugin_data,
        const char* url, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return NULL;
}


int gfal2_zenodo_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc,
        GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return -1;
}


struct dirent* gfal2_zenodo_readdir(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return NULL;
}


struct dirent* gfal2_zenodo_readdirpp(plugin_handle plugin_data,
        gfal_file_handle dir_desc, struct stat* st, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__, "Not implemented");
    return NULL;
}
