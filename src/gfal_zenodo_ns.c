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

#include <json.h>
#include <string.h>
#include "gfal_zenodo.h"
#include "gfal_zenodo_helpers.h"


int gfal2_zenodo_stat(plugin_handle plugin_data, const char* url,
        struct stat *buf, GError** error)
{
	GError* tmp_err = NULL;
	ZenodoResource zr;

	if (gfal2_zenodo_resource_from_uri(&zr, url, &tmp_err) < 0) {
		gfal2_propagate_prefixed_error(error, tmp_err, __func__);
		return -1;
	}

    char buffer[102400];
    json_object* root;
    struct dirent dent;

	memset(buf, 0, sizeof(*buf));
	switch(zr.type) {
		case ZenodoRoot:
			buf->st_mode = S_IFDIR;
			break;
		case ZenodoDeposition:
            if (gfal2_zenodo_get(plugin_data, buffer, sizeof(buffer), &tmp_err, zr.domain,
                    "/api/deposit/depositions/%s", zr.deposition) < 0) {
                gfal2_propagate_prefixed_error(error, tmp_err, __func__);
                return -1;
            }

            root = json_tokener_parse(buffer);
            gfal2_zenodo_deposition_to_stat(root, &dent, buf);
            json_object_put(root);

			break;
		case ZenodoFile:
            if (gfal2_zenodo_get(plugin_data, buffer, sizeof(buffer), &tmp_err, zr.domain,
                    "/api/deposit/depositions/%s/files/%s", zr.deposition, zr.file) < 0) {
                gfal2_propagate_prefixed_error(error, tmp_err, __func__);
                return -1;
            }

		    root = json_tokener_parse(buffer);
            gfal2_zenodo_file_to_stat(root, &dent, buf);
            json_object_put(root);

            break;
	}

	return *error ? -1 : 0;
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
    GError* tmp_err = NULL;
    ZenodoResource zr;

    if (gfal2_zenodo_resource_from_uri(&zr, url, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    if (zr.type != ZenodoDeposition) {
        gfal2_set_error(error, zenodo_domain(), ENOTDIR, __func__, "rmdir can only be called on a deposition");
        return -1;
    }

    char buffer[512];
    if (gfal2_zenodo_delete(plugin_data, buffer, sizeof(buffer), &tmp_err, zr.domain,
            "/api/deposit/depositions/%s", zr.deposition) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    return 0;
}


int gfal2_zenodo_unlink(plugin_handle plugin_data, const char* url,
        GError** error)
{
    GError* tmp_err = NULL;
    ZenodoResource zr;

    if (gfal2_zenodo_resource_from_uri(&zr, url, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    if (zr.type != ZenodoFile) {
        gfal2_set_error(error, zenodo_domain(), EISDIR, __func__, "rmdir can only be called on a deposition");
        return -1;
    }

    char buffer[512];
    if (gfal2_zenodo_delete(plugin_data, buffer, sizeof(buffer), &tmp_err, zr.domain,
            "/api/deposit/depositions/%s/files/%s", zr.deposition, zr.file) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    return 0;
}


int gfal2_zenodo_rename(plugin_handle plugin_data, const char * oldurl,
        const char * urlnew, GError** error)
{
	gfal2_set_error(error, zenodo_domain(), EPERM, __func__, "Rename operation not supported");
    return -1;
}
