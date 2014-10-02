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
#pragma once
#ifndef _GFAL_ZENODO_H
#define _GFAL_ZENODO_H

#include <curl/curl.h>
#include <gfal_api.h>
#include <gfal_plugins_api.h>


/*
 * Internal plugin context
 */
struct ZenodoHandle {
    CURL* curl_handle;
    gfal2_context_t gfal2_context;
};
typedef struct ZenodoHandle ZenodoHandle;

/*
 * Domain
 */
const GQuark zenodo_domain();

/*
 * Plugin name
 */
const char* gfal2_zenodo_getName();

/*
 * Directory operations
 */
gfal_file_handle gfal2_zenodo_opendir(plugin_handle, const char*, GError**);
int gfal2_zenodo_closedir(plugin_handle, gfal_file_handle, GError**);
struct dirent* gfal2_zenodo_readdir(plugin_handle, gfal_file_handle, GError**);
struct dirent* gfal2_zenodo_readdirpp(plugin_handle, gfal_file_handle, struct stat*, GError**);

/*
 * Namespace operations
 */
int gfal2_zenodo_stat(plugin_handle, const char*, struct stat*, GError**);
int gfal2_zenodo_mkdir(plugin_handle, const char*, mode_t, gboolean, GError**);
int gfal2_zenodo_rmdir(plugin_handle, const char*, GError**);
int gfal2_zenodo_unlink(plugin_handle, const char*, GError**);
int gfal2_zenodo_rename(plugin_handle, const char*, const char*, GError**);

/*
 * IO operations
 */
gfal_file_handle gfal2_zenodo_fopen(plugin_handle, const char*, int, mode_t, GError**);
ssize_t gfal2_zenodo_fread(plugin_handle, gfal_file_handle, void*, size_t, GError**);
ssize_t gfal2_zenodo_fwrite(plugin_handle, gfal_file_handle, const void*, size_t count, GError**);
int gfal2_zenodo_fclose(plugin_handle, gfal_file_handle, GError **);
off_t gfal2_zenodo_fseek(plugin_handle, gfal_file_handle, off_t, int, GError**);

#endif
