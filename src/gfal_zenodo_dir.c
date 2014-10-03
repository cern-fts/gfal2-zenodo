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

#include <json.h>
#include <string.h>
#include <time.h>
#include "gfal_zenodo.h"
#include "gfal_zenodo_helpers.h"


struct ZenodoDir {
    struct json_object* root;
    struct array_list* contents;
	int content_length;
	int i;
	struct dirent ent;
    ZenodoResourceType type;
};
typedef struct ZenodoDir ZenodoDir;


gfal_file_handle gfal2_zenodo_opendir(plugin_handle plugin_data,
        const char* url, GError** error)
{
	GError* tmp_err = NULL;
	ZenodoResource zr;

	if (gfal2_zenodo_resource_from_uri(&zr, url, &tmp_err) < 0) {
		gfal2_propagate_prefixed_error(error, tmp_err, __func__);
		return NULL;
	}

	ZenodoDir* dir = NULL;
	char buffer[102400];
	json_object* root;

	switch (zr.type) {
		case ZenodoRoot:
		    if (gfal2_zenodo_get(plugin_data, buffer, sizeof(buffer), &tmp_err,
		    		zr.domain, "/api/deposit/depositions") < 0) {
				gfal2_propagate_prefixed_error(error, tmp_err, __func__);
				return NULL;
			}
			break;
		case ZenodoDeposition:
		    if (gfal2_zenodo_get(plugin_data, buffer, sizeof(buffer), &tmp_err,
                    zr.domain, "/api/deposit/depositions/%s/files", zr.deposition) < 0) {
                gfal2_propagate_prefixed_error(error, tmp_err, __func__);
                return NULL;
            }
		    root = json_tokener_parse(buffer);
		    break;
		case ZenodoFile:
			gfal2_set_error(error, zenodo_domain(), ENOTDIR, __func__, "Can not list a file");
			return NULL;
	}

    root = json_tokener_parse(buffer);
    if (!root) {
        gfal2_set_error(error, zenodo_domain(), EIO, __func__, "Could not parse the response");
        return NULL;
    }
    dir = g_malloc0(sizeof(*dir));
    dir->root = root;
    dir->type = zr.type;
    dir->contents = json_object_get_array(root);
    dir->content_length = json_object_array_length(root);

	return gfal_file_handle_new2(gfal2_zenodo_getName(), dir, NULL, url);
}


int gfal2_zenodo_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc,
        GError** error)
{
	ZenodoDir* dir_handle = gfal_file_handle_get_fdesc(dir_desc);
    json_object_put(dir_handle->root);
    g_free(dir_handle);
    gfal_file_handle_delete(dir_desc);
    return 0;
}


struct dirent* gfal2_zenodo_readdir(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** error)
{
    struct stat _;
    return gfal2_zenodo_readdirpp(plugin_data, dir_desc, &_, error);
}


struct dirent* gfal2_zenodo_deposition_to_stat(json_object* entry,
        struct dirent* dent, struct stat* st)
{
    memset(st, 0, sizeof(*st));
    memset(dent, 0, sizeof(dent));

    st->st_mode = 0750 | S_IFDIR;

    json_object* id = NULL, *aux = NULL;
    json_object_object_get_ex(entry, "id", &id);

    // Name
    json_object_object_get_ex(entry, "title", &aux);
    if (aux) {
        dent->d_reclen = snprintf(dent->d_name, sizeof(dent->d_name), "%d:%s",
                json_object_get_int(id), json_object_get_string(aux));
    }
    else {
        dent->d_reclen = snprintf(dent->d_name, sizeof(dent->d_name), "%d",
                json_object_get_int(id));
    }

    // nlinks
    json_object_object_get_ex(entry, "files", &aux);
    if (aux) {
        st->st_nlink = json_object_array_length(aux);
    }

    // Created
    struct tm time;
    json_object_object_get_ex(entry, "created", &aux);
    if (aux) {
        strptime(json_object_get_string(aux), "%Y-%m-%dT%H:%M:%S%z", &time);
        st->st_ctime = mktime(&time);
    }

    // Modified
    json_object_object_get_ex(entry, "modified", &aux);
    if (aux) {
        strptime(json_object_get_string(aux), "%Y-%m-%dT%H:%M:%S%z", &time);
        st->st_mtime = mktime(&time);
    }

    // Access
    st->st_atime = st->st_mtime;

    // UID
    json_object_object_get_ex(entry, "owner", &aux);
    if (aux) {
        st->st_uid = json_object_get_int(aux);
    }

    return dent;
}


struct dirent* gfal2_zenodo_file_to_stat(json_object* entry,
        struct dirent* dent, struct stat* st)
{
    memset(st, 0, sizeof(*st));
    memset(dent, 0, sizeof(dent));

    st->st_mode = 0640 | S_IFREG;

    json_object *id = NULL, *aux = NULL;

    // Name
    json_object_object_get_ex(entry, "id", &id);

    json_object_object_get_ex(entry, "filename", &aux);
    if (aux) {
        dent->d_reclen = snprintf(dent->d_name, sizeof(dent->d_name), "%s:%s",
                json_object_get_string(id), json_object_get_string(aux));
    } else {
        dent->d_reclen = snprintf(dent->d_name, sizeof(dent->d_name), "%s",
                        json_object_get_string(id));
    }

    // nlinks
    st->st_nlink = 1;

    // filesize
    json_object_object_get_ex(entry, "filesize", &aux);
    if (aux) {
        st->st_size = json_object_get_int64(aux);
    }

    return dent;
}


struct dirent* gfal2_zenodo_readdirpp(plugin_handle plugin_data,
        gfal_file_handle dir_desc, struct stat* st, GError** error)
{
    ZenodoDir* dir_handle = gfal_file_handle_get_fdesc(dir_desc);

    if (dir_handle->i >= dir_handle->content_length)
        return NULL;

    json_object* entry = (json_object*)array_list_get_idx(dir_handle->contents, dir_handle->i++);

    switch (dir_handle->type) {
    case ZenodoRoot:
        return gfal2_zenodo_deposition_to_stat(entry, &dir_handle->ent, st);
    case ZenodoDeposition:
        return gfal2_zenodo_file_to_stat(entry, &dir_handle->ent, st);
    default:
        gfal2_set_error(error, zenodo_domain(), ENOSYS, __func__,
                "Not implemented for this type of resource");
        return NULL;
    }
}
