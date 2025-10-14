#include "httpUtils.h"

const char *httpMethodToString(HTTPMethod method)
{
    switch (method)
    {
    case HTTP_GET:
        return "GET";
    case HTTP_POST:
        return "POST";
    case HTTP_DELETE:
        return "DELETE";
    case HTTP_PUT:
        return "PUT";
    case HTTP_PATCH:
        return "PATCH";
    default:
        return "UNKNOWN";
    }
}
