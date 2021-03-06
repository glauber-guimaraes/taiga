/*
** Taiga
** Copyright (C) 2010-2014, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "win_http.h"

#include "base/encoding.h"
#include "base/file.h"
#include "base/foreach.h"
#include "base/gzip.h"
#include "base/logger.h"
#include "base/string.h"

namespace win {
namespace http {

bool Client::MakeRequest(Request request) {
  // Close any previous connection
  Cleanup();

  // Set the new request
  request_ = request;
  LOG(LevelDebug, L"ID: " + request.uuid);

  // Set secure transaction state
  secure_transaction_ = request.protocol == kHttps;

  // Ensure that the response has the same parameter and UUID as the request
  response_.parameter = request.parameter;
  response_.uuid = request.uuid;

  if (Initialize())
    if (SetRequestOptions())
      if (SendRequest())
        return true;

  Cleanup();
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Client::Initialize() {
  if (!curl_global_.initialized())
    return false;

  curl_handle_ = curl_easy_init();

  return curl_handle_ != nullptr;
}

bool Client::SetRequestOptions() {
  CURLcode code = CURLE_OK;

  #define TAIGA_CURL_SET_OPTION(option, value) \
    if ((code = curl_easy_setopt(curl_handle_, option, value)) != CURLE_OK) { \
      OnError(code); \
      return false; \
    }

  //////////////////////////////////////////////////////////////////////////////
  // Callback options

#ifdef _DEBUG
  TAIGA_CURL_SET_OPTION(CURLOPT_VERBOSE, TRUE);
  TAIGA_CURL_SET_OPTION(CURLOPT_DEBUGFUNCTION, DebugCallback);
  TAIGA_CURL_SET_OPTION(CURLOPT_DEBUGDATA, this);
#endif

  TAIGA_CURL_SET_OPTION(CURLOPT_HEADERFUNCTION, HeaderFunction);
  TAIGA_CURL_SET_OPTION(CURLOPT_HEADERDATA, this);

  TAIGA_CURL_SET_OPTION(CURLOPT_WRITEFUNCTION, WriteFunction);
  TAIGA_CURL_SET_OPTION(CURLOPT_WRITEDATA, &write_buffer_);

  TAIGA_CURL_SET_OPTION(CURLOPT_NOPROGRESS, FALSE);
  TAIGA_CURL_SET_OPTION(CURLOPT_XFERINFOFUNCTION, XferInfoFunction);
  TAIGA_CURL_SET_OPTION(CURLOPT_XFERINFODATA, this);

  //////////////////////////////////////////////////////////////////////////////
  // Network options

  // Set URL
  std::string url;
  switch (request_.protocol) {
    case kHttp:
    default:
      url += "http://";
      break;
    case kHttps:
      url += "https://";
      break;
  }
  url += WstrToStr(request_.host);
  std::wstring path = request_.path;
  if (!request_.query.empty()) {
    std::wstring query_string;
    foreach_(it, request_.query) {  // Append the query component to the path
      query_string += query_string.empty() ? L"?" : L"&";
      query_string += it->first + L"=" + EncodeUrl(it->second, false);
    }
    path += query_string;
  }
  url += WstrToStr(path);
  TAIGA_CURL_SET_OPTION(CURLOPT_URL, url.c_str());
  LOG(LevelDebug, L"Host: " + request_.host);
  LOG(LevelDebug, L"Path: " + path);

  // Set protocol
  int protocol = secure_transaction_ ? CURLPROTO_HTTPS : CURLPROTO_HTTP;
  TAIGA_CURL_SET_OPTION(CURLOPT_PROTOCOLS, protocol);
  TAIGA_CURL_SET_OPTION(CURLOPT_REDIR_PROTOCOLS, protocol);

  // Set proxy
  if (!proxy_host_.empty()) {
    std::string proxy_host = WstrToStr(proxy_host_);
    TAIGA_CURL_SET_OPTION(CURLOPT_PROXY, proxy_host.c_str());
    if (!proxy_username_.empty()) {
      std::string proxy_username = WstrToStr(proxy_username_);
      TAIGA_CURL_SET_OPTION(CURLOPT_PROXYUSERNAME,
                            proxy_username.c_str());
    }
    if (!proxy_password_.empty()) {
      std::string proxy_password = WstrToStr(proxy_password_);
      TAIGA_CURL_SET_OPTION(CURLOPT_PROXYPASSWORD,
                            proxy_password.c_str());
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // HTTP options

  // Set auto-redirect
  if (auto_redirect_) {
    TAIGA_CURL_SET_OPTION(CURLOPT_FOLLOWLOCATION, TRUE);
  }

  // Set method
  if (request_.method == L"POST") {
    optional_data_ = WstrToStr(request_.body);
    TAIGA_CURL_SET_OPTION(CURLOPT_POSTFIELDS, optional_data_.c_str());
    TAIGA_CURL_SET_OPTION(CURLOPT_POSTFIELDSIZE, optional_data_.size());
    TAIGA_CURL_SET_OPTION(CURLOPT_POST, TRUE);
  }

  // Set referrer
  if (!referer_.empty()) {
    std::string referer = WstrToStr(referer_);
    TAIGA_CURL_SET_OPTION(CURLOPT_REFERER, referer.c_str());
  }

  // Set user agent
  if (!user_agent_.empty()) {
    std::string user_agent = WstrToStr(user_agent_);
    TAIGA_CURL_SET_OPTION(CURLOPT_USERAGENT, user_agent.c_str());
  }

  // Set custom headers
  BuildRequestHeader();
  TAIGA_CURL_SET_OPTION(CURLOPT_HTTPHEADER, header_list_);

  //////////////////////////////////////////////////////////////////////////////
  // Security options

#ifdef TAIGA_WIN_HTTP_SSL_UNSECURE
  TAIGA_CURL_SET_OPTION(CURLOPT_SSL_VERIFYPEER, 0L);
  TAIGA_CURL_SET_OPTION(CURLOPT_SSL_VERIFYHOST, 0L);
#endif

  #undef TAIGA_CURL_SET_OPTION

  return true;
}

bool Client::SendRequest() {
#ifdef TAIGA_WIN_HTTP_MULTITHREADED
  return CreateThread(nullptr, 0, 0);
#else
  return Perform();
#endif
}



bool Client::Perform() {
  CURLcode code = curl_easy_perform(curl_handle_);

  if (code == CURLE_OK) {
    if (!write_buffer_.empty()) {
      if (content_encoding_ == kContentEncodingGzip) {
        std::string compressed;
        std::swap(write_buffer_, compressed);
        UncompressGzippedString(compressed, write_buffer_);
      }
      response_.body = StrToWstr(write_buffer_);
    }

    if (!download_path_.empty())
      SaveToFile((LPCVOID)&write_buffer_.front(), write_buffer_.size(),
                 download_path_);

    if (!OnReadComplete())
      return true;

  } else {
    OnError(code);
  }

  Cleanup();

  return code == CURLE_OK;
}

DWORD Client::ThreadProc() {
  return Perform();
}

////////////////////////////////////////////////////////////////////////////////

void Client::BuildRequestHeader() {
  // Set acceptable types for the response
  if (!request_.header.count(L"Accept"))
    request_.header[L"Accept"] = L"*/*";

  // Set content type for POST and PUT requests
  if (request_.method == L"POST" || request_.method == L"PUT")
    if (!request_.header.count(L"Content-Type"))
      request_.header[L"Content-Type"] = L"application/x-www-form-urlencoded";

  // Append available header fields
  foreach_(it, request_.header) {
    std::string header = WstrToStr(it->first);
    if (!it->second.empty()) {
      header += ": " + WstrToStr(it->second);
    } else {
      header += ";";
    }
    header_list_ = curl_slist_append(header_list_, header.c_str());
  }
}

}  // namespace http
}  // namespace win