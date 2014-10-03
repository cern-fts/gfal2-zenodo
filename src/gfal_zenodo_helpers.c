#include <gfal_api.h>
#include <json.h>
#include <string.h>
#include <utils/gfal_uri.h>
#include "gfal_zenodo_helpers.h"



int gfal2_zenodo_resource_from_uri(ZenodoResource* zr, const char* uri, GError** error)
{
	GError* tmp_err = NULL;
	gfal_uri parsed;

	if (gfal_parse_uri(uri, &parsed, &tmp_err) < 0) {
		gfal2_propagate_prefixed_error(error, tmp_err, __func__);
		return -1;
	}

	g_strlcpy(zr->domain, parsed.domain, sizeof(zr->domain));
	char *p = parsed.path;
	while (*p == '/')
		++p;
	char* slash = strchr(p, '/');

	if (slash) {
		*slash = 0;
		slash++;
		while (*slash == '/')
			++slash;
		g_strlcpy(zr->file, slash, sizeof(zr->file));
	}
	else {
		zr->file[0] = '\0';
	}
	g_strlcpy(zr->deposition, p, sizeof(zr->deposition));

	// Since we use colon to separate the id from the readable title, split on it
	p = strchr(zr->deposition, ':');
	if (p)
	    *p = '\0';
	p = strchr(zr->file, ':');
	if (p)
	    *p = '\0';

	if (zr->file[0])
		zr->type = ZenodoFile;
	else if (zr->deposition[0])
		zr->type = ZenodoDeposition;
	else
		zr->type = ZenodoRoot;

	gfal_log(GFAL_VERBOSE_DEBUG, "Zenodo resource %d [host: %s; deposition: %s; file: %s] %d",
			zr->type, zr->domain, zr->deposition, zr->file);

	return 0;
}


static int gfal2_zenodo_map_http_status(long response, GError** error, const char* func)
{
    if (response < 400)
        return 0;
    int errval = 0;
    if (response == 400)
        errval = EINVAL;
    else if (response == 401)
    	errval = EAGAIN;
    else if (response >= 402 && response <= 403)
        errval = EACCES;
    else if (response == 404)
        errval = ENOENT;
    else
        errval = EIO;

    gfal2_set_error(error, zenodo_domain(), errval, func, "HTTP Response %ld", response);
    return -1;
}


static void gfal2_zenodo_build_full_url(ZenodoHandle* handle, char* full,
		size_t fullsize, const char* domain, const char* uri, va_list args)
{
	int nchars = snprintf(full, fullsize, "https://%s", domain);
	va_list args_copy;
	va_copy(args_copy, args);
	nchars += vsnprintf(full + nchars, fullsize - nchars, uri, args_copy);
	va_end(args_copy);
}


static void gfal2_zenodo_append_access_token(ZenodoHandle* handle, const char* uri,
		char* out, size_t outsize)
{
	const gchar* access_token = gfal2_get_opt_string(handle->gfal2_context, "ZENODO", "ACCESS_TOKEN", NULL);
	if (access_token) {
		if (strchr(uri, '?'))
			snprintf(out, outsize, "%s&access_token=%s", uri, access_token);
		else
			snprintf(out, outsize, "%s?access_token=%s", uri, access_token);
	}
}


static ssize_t gfal2_zenodo_nobody_internal(ZenodoHandle* handle, const char* method,
        char* buffer, size_t bufsize, const char *uri, GError** error)
{
	g_assert(handle != NULL && uri != NULL && buffer != NULL && error != NULL);

	char uri_with_token[1024];
	gfal2_zenodo_append_access_token(handle, uri, uri_with_token, sizeof(uri_with_token));

	// Perform
	curl_easy_setopt(handle->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

	char err_buffer[CURL_ERROR_SIZE];
    curl_easy_setopt(handle->curl_handle, CURLOPT_ERRORBUFFER, err_buffer);

    FILE* fd = fmemopen(buffer, bufsize, "wb");
    curl_easy_setopt(handle->curl_handle, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(handle->curl_handle, CURLOPT_WRITEFUNCTION, fwrite);

    curl_easy_setopt(handle->curl_handle, CURLOPT_URL, uri_with_token);

    if (strncmp(method, "GET", 3) == 0) {
        curl_easy_setopt(handle->curl_handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle->curl_handle, CURLOPT_NOBODY, 0);
    }
    else if (strncmp(method, "HEAD", 4) == 0) {
        curl_easy_setopt(handle->curl_handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle->curl_handle, CURLOPT_NOBODY, 1);
    }
    else {
        curl_easy_setopt(handle->curl_handle, CURLOPT_CUSTOMREQUEST, method);
    }

    gfal_log(GFAL_VERBOSE_VERBOSE, "%s %s", method, uri);

	int perform_result = curl_easy_perform(handle->curl_handle);

	fclose(fd);

	if (perform_result != 0) {
		gfal2_set_error(error, zenodo_domain(), EIO, __func__, "%s", err_buffer);
		return -1;
	}

    long response;
    curl_easy_getinfo(handle->curl_handle, CURLINFO_RESPONSE_CODE, &response);

    if (gfal2_zenodo_map_http_status(response, error, __func__) < 0)
        return -1;

    double total_size;
    curl_easy_getinfo(handle->curl_handle, CURLINFO_SIZE_DOWNLOAD, &total_size);
    return (ssize_t)(total_size);
}


static ssize_t gfal2_zenodo_post_internal(ZenodoHandle* handle, char* buffer, size_t bufsize,
		const char* body, size_t bodysize, const char *uri, int notoken, GError** error)
{
	g_assert(handle != NULL && uri != NULL && buffer != NULL && error != NULL);

	char uri_with_token[1024];
	if (notoken)
		g_strlcpy(uri_with_token, uri, sizeof(uri_with_token));
	else
		gfal2_zenodo_append_access_token(handle, uri, uri_with_token, sizeof(uri_with_token));

	// Perform
	curl_easy_setopt(handle->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

	char err_buffer[CURL_ERROR_SIZE];
    curl_easy_setopt(handle->curl_handle, CURLOPT_ERRORBUFFER, err_buffer);

    curl_easy_setopt(handle->curl_handle, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(handle->curl_handle, CURLOPT_POSTFIELDSIZE, bodysize);

    FILE* fd = fmemopen(buffer, bufsize, "wb");
    curl_easy_setopt(handle->curl_handle, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(handle->curl_handle, CURLOPT_WRITEFUNCTION, fwrite);

    curl_easy_setopt(handle->curl_handle, CURLOPT_URL, uri_with_token);

    curl_easy_setopt(handle->curl_handle, CURLOPT_POST, 1);

    gfal_log(GFAL_VERBOSE_VERBOSE, "POST %s", uri);
	int perform_result = curl_easy_perform(handle->curl_handle);

	fclose(fd);

	if (perform_result != 0) {
		gfal2_set_error(error, zenodo_domain(), EIO, __func__, "%s", err_buffer);
		return -1;
	}

    long response;
    curl_easy_getinfo(handle->curl_handle, CURLINFO_RESPONSE_CODE, &response);

    if (gfal2_zenodo_map_http_status(response, error, __func__) < 0)
        return -1;

    double total_size;
    curl_easy_getinfo(handle->curl_handle, CURLINFO_SIZE_DOWNLOAD, &total_size);
    return (ssize_t)(total_size);
}


static int gfal2_zenodo_refresh_token(ZenodoHandle* handle, const char* domain,
		char* buffer, size_t bufsize, GError** error)
{
	const gchar* client_id = gfal2_get_opt_string(handle->gfal2_context, "ZENODO", "APP_KEY", NULL);
	const gchar* client_secret = gfal2_get_opt_string(handle->gfal2_context, "ZENODO", "APP_SECRET", NULL);
	const gchar* refresh_token = gfal2_get_opt_string(handle->gfal2_context, "ZENODO", "REFRESH_TOKEN", NULL);

	char body[1024];
	size_t bodysize = snprintf(body, sizeof(body),
			"client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s&scope=deposit%%3Awrite+deposit%%3Aactions",
			client_id, client_secret, refresh_token
			);

	char oauth_uri[1024];
	snprintf(oauth_uri, sizeof(oauth_uri), "https://%s/oauth/token", domain);

	GError* tmp_err = NULL;
	ssize_t resp_size = gfal2_zenodo_post_internal(handle, buffer, bufsize,
			body, bodysize, oauth_uri, 1, &tmp_err);

	if (resp_size < 0) {
		gfal2_propagate_prefixed_error(error, tmp_err, __func__);
		return -1;
	}

	json_object* root = json_tokener_parse(buffer);
	if (!root) {
		gfal2_set_error(error, zenodo_domain(), EIO, __func__, "Could not parse the response of /oauth/token");
		return -1;
	}

	json_object* access_token_obj = NULL;
    json_object_object_get_ex(root, "access_token", &access_token_obj);
	if (!access_token_obj) {
		json_object_put(root);
		gfal2_set_error(error, zenodo_domain(), EIO, __func__, "Could not find access_token in the response of /oauth/token");
		return -1;
	}

	const char* access_token = json_object_get_string(access_token_obj);
	gfal2_set_opt_string(handle->gfal2_context, "ZENODO", "ACCESS_TOKEN", access_token, NULL);
	gfal_log(GFAL_VERBOSE_VERBOSE, "Zenodo got a new access token");

	json_object_put(root);
	return 0;
}


static ssize_t gfal2_zenodo_nobody(ZenodoHandle* handle, const char* method, char* buffer,
        size_t bufsize, GError** error, const char *domain, const char* uri, va_list args)
{
	ssize_t resp_size;
	char full_url[1024] = {0};

	gfal2_zenodo_build_full_url(handle, full_url, sizeof(full_url), domain, uri, args);

	resp_size = gfal2_zenodo_nobody_internal(handle, method, buffer, bufsize, full_url, error);

	if (resp_size < 0 && (*error)->code == EAGAIN) {
		gfal_log(GFAL_VERBOSE_VERBOSE, "Zenodo refresh token and try again");
		g_clear_error(error);

		if (gfal2_zenodo_refresh_token(handle, domain, buffer, bufsize, error) >= 0) {
			resp_size = gfal2_zenodo_nobody_internal(handle, method, buffer, bufsize, full_url, error);
			if (resp_size < 0 && (*error)->code == EAGAIN)
				(*error)->code = EACCES;
		}
	}

	return resp_size;
}


ssize_t gfal2_zenodo_get(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
                    const char *domain, const char* uri, ...)
{
    va_list args;
    va_start(args, uri);
    ssize_t ret = gfal2_zenodo_nobody(handle, "GET", buffer, bufsize, error, domain, uri, args);
    va_end(args);
    return ret;
}


ssize_t gfal2_zenodo_head(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
        const char *domain, const char* uri, ...)
{
    va_list args;
    va_start(args, uri);
    ssize_t ret = gfal2_zenodo_nobody(handle, "HEAD", buffer, bufsize, error, domain, uri, args);
    va_end(args);
    return ret;
}


ssize_t gfal2_zenodo_post(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
		const char* domain, const char* body, size_t bodysize, const char* uri, ...)
{
	ssize_t resp_size;
	char full_url[1024] = {0};

	va_list args;
	va_start(args, uri);
	gfal2_zenodo_build_full_url(handle, full_url, sizeof(full_url), domain, uri, args);
	va_end(args);

	resp_size = gfal2_zenodo_post_internal(handle, buffer, bufsize, body, bodysize, full_url, 0, error);

	if (resp_size < 0 && (*error)->code == EAGAIN) {
		gfal_log(GFAL_VERBOSE_VERBOSE, "Zenodo refresh token and try again");
		g_clear_error(error);

		if (gfal2_zenodo_refresh_token(handle, domain, buffer, bufsize, error) >= 0) {
			resp_size = gfal2_zenodo_post_internal(handle, buffer, bufsize, body, bodysize, full_url, 0, error);
			if (resp_size < 0 && (*error)->code == EAGAIN)
				(*error)->code = EACCES;
		}
	}

	return resp_size;
}


ssize_t gfal2_zenodo_delete(ZenodoHandle* handle, char* buffer, size_t bufsize, GError** error,
        const char *domain, const char* uri, ...)
{
    va_list args;
    va_start(args, uri);
    ssize_t ret = gfal2_zenodo_nobody(handle, "DELETE", buffer, bufsize, error, domain, uri, args);
    va_end(args);
    return ret;
}
