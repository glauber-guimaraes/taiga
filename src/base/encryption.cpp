/*
** Taiga
** Copyright (C) 2010-2013, Eren Okka
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

#include "encryption.h"
#include "string.h"
#include "third_party/base64/base64.h"

#define MIN_LENGHT 16
#define SIMPLE_KEY L"Tenori Taiga"

using std::wstring;

////////////////////////////////////////////////////////////////////////////////

wstring Base64Decode(const wstring& str, bool for_filename) {
  if (str.empty()) return L"";
  Base64Coder coder;
  string buff = ToANSI(str);
  coder.Decode((BYTE*)buff.c_str(), buff.length());
  if (for_filename) {
    wstring msg = ToUTF8(coder.DecodedMessage());
    ReplaceChar(msg, '-', '/');
    return msg;
  } else {
    return ToUTF8(coder.DecodedMessage());
  }
}

wstring Base64Encode(const wstring& str, bool for_filename) {
  if (str.empty()) return L"";
  Base64Coder coder;
  string buff = ToANSI(str);
  coder.Encode((BYTE*)buff.c_str(), buff.length());
  if (for_filename) {
    wstring msg = ToUTF8(coder.EncodedMessage());
    ReplaceChar(msg, '/', '-');
    return msg;
  } else {
    return ToUTF8(coder.EncodedMessage());
  }
}

////////////////////////////////////////////////////////////////////////////////

wstring XOR(wstring str, wstring key) {
  for (unsigned int i = 0; i < str.length(); i++) {
    str[i] = str[i] ^ key[i % key.length()];
  }
  return str;
}

wstring SimpleEncrypt(wstring str) {
  // Set minimum length
  if (str.length() > 0 && str.length() < MIN_LENGHT) {
    str.append(MIN_LENGHT - str.length(), '?');
  }
  // Encrypt
  str = XOR(str, SIMPLE_KEY);
  // Convert to hexadecimal string
  wstring buffer;
  for (unsigned int i = 0; i < str.length(); i++) {
    wchar_t c[32] = {'\0'};
    _itow_s(str[i], c, 32, 16);
    if (wcslen(c) == 1) buffer.push_back('0');
    buffer += c;
  }
  ToUpper(buffer);
  // Return
  return buffer;
}

wstring SimpleDecrypt(wstring str) {
  // Convert from hexadecimal string
  wstring buffer;
  for (unsigned int i = 0; i < str.length(); i = i + 2) {
    wchar_t c = static_cast<wchar_t>(wcstoul(str.substr(i, 2).c_str(), NULL, 16));
    buffer.push_back(c);
  }
  // Decrypt
  buffer = XOR(buffer, SIMPLE_KEY);
  // Trim
  TrimRight(buffer, L"?");
  // Return
  return buffer;
}