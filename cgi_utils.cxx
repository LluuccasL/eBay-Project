#include "cgi_utils.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace cgi {

static std::string getEnv(const char* name)
{
    const char* v = std::getenv(name);
    return v ? std::string(v) : "";
}

std::string getRequestMethod()
{
    return getEnv("REQUEST_METHOD");
}

std::string getQueryString()
{
    return getEnv("QUERY_STRING");
}

std::string getPostBody()
{
    std::string lenStr = getEnv("CONTENT_LENGTH");
    if (lenStr.empty())
        return "";
    int len = std::atoi(lenStr.c_str());
    if (len <= 0 || len > 1024 * 1024)
        return "";
    std::string body;
    body.resize(static_cast<size_t>(len));
    std::cin.read(&body[0], static_cast<std::streamsize>(len));
    return body;
}

static void parsePairs(const std::string& s, std::map<std::string, std::string>& out)
{
    size_t i = 0;
    while (i < s.size())
    {
        size_t amp = s.find('&', i);
        std::string pair = (amp == std::string::npos) ? s.substr(i) : s.substr(i, amp - i);
        i = (amp == std::string::npos) ? s.size() : amp + 1;
        size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            std::string key = urlDecode(pair.substr(0, eq));
            std::string val = urlDecode(pair.substr(eq + 1));
            out[key] = val;
        }
        else if (!pair.empty())
            out[urlDecode(pair)] = "";
    }
}

std::map<std::string, std::string> parseFormUrlEncoded(const std::string& body)
{
    std::map<std::string, std::string> out;
    parsePairs(body, out);
    return out;
}

std::map<std::string, std::string> parseQueryString()
{
    return parseFormUrlEncoded(getQueryString());
}

std::map<std::string, std::string> getParams()
{
    std::string method = getRequestMethod();
    if (method == "POST")
    {
        std::string ct = getEnv("CONTENT_TYPE");
        if (ct.find("application/x-www-form-urlencoded") != std::string::npos)
            return parseFormUrlEncoded(getPostBody());
    }
    return parseQueryString();
}

std::string urlDecode(const std::string& encoded)
{
    std::string out;
    out.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); i++)
    {
        if (encoded[i] == '+')
            out += ' ';
        else if (encoded[i] == '%' && i + 2 < encoded.size() && std::isxdigit(static_cast<unsigned char>(encoded[i+1])) && std::isxdigit(static_cast<unsigned char>(encoded[i+2])))
        {
            int v = 0;
            for (int j = 1; j <= 2; j++)
                v = v * 16 + (std::isdigit(encoded[i+j]) ? encoded[i+j] - '0' : (std::tolower(static_cast<unsigned char>(encoded[i+j])) - 'a' + 10));
            out += static_cast<char>(v);
            i += 2;
        }
        else
            out += encoded[i];
    }
    return out;
}

std::string htmlEscape(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s)
    {
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '"') out += "&quot;";
        else if (c == '\'') out += "&#39;";
        else out += c;
    }
    return out;
}

} // namespace cgi
