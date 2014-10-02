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

// Plugin entry point

#include "gfal_zenodo.h"
#include <gfal_plugins_api.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


const GQuark zenodo_domain() {
    return g_quark_from_static_string("zenodo");
}


// Plugin unique name
const char* gfal2_zenodo_getName()
{
    return "zenodo";
}


// Returns true if the plugin recognizes the URL
static gboolean gfal2_zenodo_check_url(plugin_handle plugin_data,
        const char* url, plugin_mode operation, GError** err)
{
    switch (operation) {
        case GFAL_PLUGIN_RENAME:
        case GFAL_PLUGIN_STAT:
        case GFAL_PLUGIN_LSTAT:
        case GFAL_PLUGIN_MKDIR:
        case GFAL_PLUGIN_RMDIR:
        case GFAL_PLUGIN_OPENDIR:
        case GFAL_PLUGIN_OPEN:
        case GFAL_PLUGIN_UNLINK:
            return strncmp(url, "zenodo:", 7) == 0;
        default:
            return FALSE;
    }
}


// Frees the memory used by the plugin data
static void gfal2_zenodo_delete_data(plugin_handle plugin_data)
{
    ZenodoHandle* zenodo = (ZenodoHandle*)(plugin_data);
    curl_easy_cleanup(zenodo->curl_handle);
    free(zenodo);
}


// Logging callback
static int gfal2_zenodo_debug_callback(CURL *handle, curl_infotype type,
        char *data, size_t size, void *userptr)
{
    char msg_fmt[64];
    switch (type) {
        case CURLINFO_TEXT:
            snprintf(msg_fmt, sizeof(msg_fmt), "INFO: %%.%zds", size - 1); // Mute \n
            gfal_log(GFAL_VERBOSE_VERBOSE, msg_fmt, data);
            break;
        case CURLINFO_HEADER_IN:
            snprintf(msg_fmt, sizeof(msg_fmt), "HEADER IN: %%.%zds", size - 2); // Mute \n\r
            gfal_log(GFAL_VERBOSE_DEBUG, msg_fmt, data);
            break;
        case CURLINFO_HEADER_OUT:
            snprintf(msg_fmt, sizeof(msg_fmt), "HEADER OUT: %%.%zds", size - 2); // Mute \n\r
            gfal_log(GFAL_VERBOSE_DEBUG, msg_fmt, data);
            break;
        case CURLINFO_DATA_IN:
            snprintf(msg_fmt, sizeof(msg_fmt), "DATA IN: %%.%zds", size);
            //gfal_log(GFAL_VERBOSE_TRACE, msg_fmt, data);
            break;
        case CURLINFO_DATA_OUT:
            snprintf(msg_fmt, sizeof(msg_fmt), "DATA OUT: %%.%zds", size);
            gfal_log(GFAL_VERBOSE_TRACE, msg_fmt, data);
            break;
        default:
            break;
    }
    return 0;
}


// Set logging
static void gfal2_zenodo_set_logging(ZenodoHandle* zenodo)
{
    curl_easy_setopt(zenodo->curl_handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(zenodo->curl_handle, CURLOPT_DEBUGFUNCTION, gfal2_zenodo_debug_callback);
}

// GFAL2 will look for this symbol to register the plugin
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err)
{
    gfal_plugin_interface zenodo_plugin;
    memset(&zenodo_plugin, 0, sizeof(gfal_plugin_interface));

    ZenodoHandle* zenodo = calloc(1, sizeof(ZenodoHandle));
    zenodo->curl_handle = curl_easy_init();
    zenodo->gfal2_context = handle;

    gfal2_zenodo_set_logging(zenodo);

    zenodo_plugin.plugin_data = zenodo;
    zenodo_plugin.plugin_delete = gfal2_zenodo_delete_data;

    zenodo_plugin.getName = gfal2_zenodo_getName;
    zenodo_plugin.check_plugin_url = gfal2_zenodo_check_url;

    zenodo_plugin.opendirG = gfal2_zenodo_opendir;
    zenodo_plugin.readdirG = gfal2_zenodo_readdir;
    zenodo_plugin.readdirppG = gfal2_zenodo_readdirpp;
    zenodo_plugin.closedirG = gfal2_zenodo_closedir;

    zenodo_plugin.statG = gfal2_zenodo_stat;
    zenodo_plugin.lstatG = gfal2_zenodo_stat;
    zenodo_plugin.mkdirpG = gfal2_zenodo_mkdir;
    zenodo_plugin.rmdirG = gfal2_zenodo_rmdir;
    zenodo_plugin.unlinkG = gfal2_zenodo_unlink;
    zenodo_plugin.renameG = gfal2_zenodo_rename;

    zenodo_plugin.openG = gfal2_zenodo_fopen;
    zenodo_plugin.closeG = gfal2_zenodo_fclose;
    zenodo_plugin.readG = gfal2_zenodo_fread;
    zenodo_plugin.writeG = gfal2_zenodo_fwrite;
    zenodo_plugin.lseekG = gfal2_zenodo_fseek;

    return zenodo_plugin;
}
