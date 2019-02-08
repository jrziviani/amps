#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"

#include "amps_wrapper.h"

#define MAX_HANDLER 1

typedef int (*method_handler)(request_rec *r);

static int get_handler(request_rec *r);

/* The sample content handler */
static int cool_framework_handler(request_rec *r)
{
    if (strcmp(r->handler, "cool_framework")) {
        return DECLINED;
    }

    method_handler methods[MAX_HANDLER] = {get_handler};

    if (r->method_number >= MAX_HANDLER || r->method_number < 0) {
        return DECLINED;
    }

    return methods[r->method_number](r);
}

static int get_handler(request_rec *r)
{
    char *result = NULL;

    if (r->header_only || r->args == 0) {
        return OK;
    }

    // r->content_type = "text/html";
    get_template(r, &result);

    /* something bad happened */
    if (result == NULL) {
        return DECLINED;
    }

    ap_rputs(result, r);

    free(result);

    return OK;
}

static void cool_framework_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(cool_framework_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA cool_framework_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    cool_framework_register_hooks  /* register hooks                      */
};

