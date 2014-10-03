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
#ifndef _GFAL_ZENODO_HELPERS_H
#define _GFAL_ZENODO_HELPERS_H

#include <limits.h>
#include "gfal_zenodo.h"

/*
 * Resource representation
 * In Zenodo we care about root, depositions and files inside depositions
 */
typedef enum {ZenodoRoot, ZenodoDeposition, ZenodoFile} ZenodoResourceType;

struct ZenodoResource {
	char domain[HOST_NAME_MAX];
	char deposition[PATH_MAX];
	char file[PATH_MAX];

	ZenodoResourceType type;
};
typedef struct ZenodoResource ZenodoResource;

/*
 * Initialize a Zenodo resource from a URI
 */
int gfal2_zenodo_resource_from_uri(ZenodoResource*, const char*, GError**);

/*
 * Perform a GET
 */
ssize_t gfal2_zenodo_get(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
		const char *domain, const char* uri, ...);

/*
 * Perform a HEAD
 */
ssize_t gfal2_zenodo_head(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
        const char *domain, const char* uri, ...);

/*
 * Perform a POST
 */
ssize_t gfal2_zenodo_post(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
		const char* domain, const char* body, size_t bodysize, const char* uri, ...);

/*
 * Perform a DELETE
 */
ssize_t gfal2_zenodo_delete(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
        const char *domain, const char* uri, ...);

#endif
