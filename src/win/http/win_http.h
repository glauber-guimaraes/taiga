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

#ifndef TAIGA_WIN_HTTP_H
#define TAIGA_WIN_HTTP_H

#define TAIGA_WIN_HTTP_MULTITHREADED

#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#ifndef HTTP_ONLY
#define HTTP_ONLY
#endif

#include <windows.h>
#include <string>
#include <vector>

#include "base/map.h"
#include "third_party/curl/curl.h"
#include "win/win_thread.h"

namespace win {
namespace http {

typedef base::multimap<std::wstring, std::wstring> header_t;
typedef base::multimap<std::wstring, std::wstring> query_t;

enum ContentEncoding {
  kContentEncodingNone,
  kContentEncodingGzip
};

enum Protocol {
  kHttp,
  kHttps
};

class Request {
public:
  Request();
  virtual ~Request() {}

  void Clear();

  Protocol protocol;
  std::wstring method;
  std::wstring host;
  std::wstring path;
  query_t query;
  header_t header;
  std::wstring body;

  std::wstring uuid;
  LPARAM parameter;
};

class Response {
public:
  Response();
  virtual ~Response() {}

  void Clear();

  unsigned int code;
  header_t header;
  std::wstring body;

  std::wstring uuid;
  LPARAM parameter;
};

class CurlGlobal {
public:
  CurlGlobal();
  ~CurlGlobal();

  bool initialized() const;

private:
  bool initialized_;
};

class Client : public win::Thread {
public:
  Client();
  virtual ~Client();

  void Cleanup();
  bool MakeRequest(Request request);

  const Request& request() const;
  const Response& response() const;
  curl_off_t content_length() const;
  curl_off_t current_length() const;

  void set_auto_redirect(bool enabled);
  void set_download_path(const std::wstring& download_path);
  void set_proxy(
      const std::wstring& host,
      const std::wstring& username,
      const std::wstring& password);
  void set_referer(const std::wstring& referer);
  void set_user_agent(const std::wstring& user_agent);

  virtual void OnError(CURLcode error_code) {}
  virtual bool OnHeadersAvailable() { return false; }
  virtual bool OnProgress() { return false; }
  virtual bool OnReadComplete() { return true; }  // TODO: Why "true"?
  virtual bool OnRedirect(const std::wstring& address) { return false; }

  DWORD ThreadProc();

protected:
  Request request_;
  Response response_;

  ContentEncoding content_encoding_;
  curl_off_t content_length_;
  curl_off_t current_length_;
  std::string write_buffer_;

  bool auto_redirect_;
  std::wstring download_path_;
  std::wstring proxy_host_;
  std::wstring proxy_password_;
  std::wstring proxy_username_;
  std::wstring referer_;
  bool secure_transaction_;
  std::wstring user_agent_;

private:
  static size_t HeaderFunction(void*, size_t, size_t, void*);
  static size_t WriteFunction(char*, size_t, size_t, void*);
  static int DebugCallback(CURL*, curl_infotype, char*, size_t, void*);
  static int XferInfoFunction(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
  int ProgressFunction(curl_off_t, curl_off_t);

  bool Initialize();
  bool SetRequestOptions();
  bool SendRequest();
  bool Perform();

  void BuildRequestHeader();
  bool GetResponseHeader(const std::wstring& header);
  bool ParseResponseHeader();

  static CurlGlobal curl_global_;
  CURL* curl_handle_;

  curl_slist* header_list_;
  std::string optional_data_;
};

class Url {
public:
  Url() {}
  Url(const std::wstring& url);
  virtual ~Url() {}

  void Crack(std::wstring url);

  Url& operator=(const Url& url);
  void operator=(const std::wstring& url);

  std::wstring scheme;
  std::wstring host;
  std::wstring path;
};

}  // namespace http
}  // namespace win

#endif  // TAIGA_WIN_HTTP_H