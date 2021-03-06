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

#include <algorithm>
#include <functional>
#include <iomanip>
#include <locale>
#include <sstream>

#include "string.h"

using std::string;
using std::vector;
using std::wstring;

////////////////////////////////////////////////////////////////////////////////
// Erasing

void Erase(wstring& str1, const wstring& str2, bool case_insensitive) {
  if (str2.empty() || str1.length() < str2.length())
    return;

  auto it = str1.begin();

  do {
    if (case_insensitive) {
      it = std::search(it, str1.end(), str2.begin(), str2.end(), &IsCharsEqual);
    } else {
      it = std::search(it, str1.end(), str2.begin(), str2.end());
    }

    if (it != str1.end())
      str1.erase(it, it + str2.length());

  } while (it != str1.end());
}

void EraseChars(wstring& str, const wchar_t chars[]) {
  size_t pos = 0;

  do {
    pos = str.find_first_of(chars, pos);

    if (pos != wstring::npos)
      str.erase(pos, 1);

  } while (pos != wstring::npos);
}

void ErasePunctuation(wstring& str, bool keep_trailing) {
  auto rlast = str.rbegin();

  if (keep_trailing)
    rlast = std::find_if(str.rbegin(), str.rend(),
        [](wchar_t c) -> bool {
          return !(c == L'!' || // "Hayate no Gotoku!", "K-ON!"...
                   c == L'+' || // "Needless+"
                   c == L'\''); // "Gintama'"
        });

  auto it = std::remove_if(str.begin(), rlast.base(),
      [](int c) -> bool {
        // Control codes, white-space and punctuation characters
        if (c <= 255 && !isalnum(c))
          return true;
        // Unicode stars, hearts, notes, etc. (0x2000-0x2767)
        if (c > 8192 && c < 10087)
          return true;
        // Valid character
        return false;
      });
  
  if (keep_trailing)
    std::copy(rlast.base(), str.end(), it);

  str.resize(str.size() - (rlast.base() - it));
}

void EraseLeft(wstring& str1, const wstring str2, bool case_insensitive) {
  if (str1.length() < str2.length())
    return;

  if (case_insensitive) {
    if (!std::equal(str2.begin(), str2.end(), str1.begin(), &IsCharsEqual))
      return;
  } else {
    if (!std::equal(str2.begin(), str2.end(), str1.begin()))
      return;
  }

  str1.erase(str1.begin(), str1.begin() + str2.length());
}

void EraseRight(wstring& str1, const wstring str2, bool case_insensitive) {
  if (str1.length() < str2.length())
    return;

  if (case_insensitive) {
    if (!std::equal(str2.begin(), str2.end(), str1.end() - str2.length(),
                    &IsCharsEqual))
      return;
  } else {
    if (!std::equal(str2.begin(), str2.end(), str1.end() - str2.length()))
      return;
  }

  str1.resize(str1.length() - str2.length());
}

void RemoveEmptyStrings(vector<wstring>& input) {
  if (input.empty())
    return;

  input.erase(std::remove_if(input.begin(), input.end(),
      [](const wstring& s) -> bool { return s.empty(); }),
      input.end());
}

void StripHtmlTags(wstring& str) {
  int index_begin = -1;
  int index_end = -1;

  do {
    index_begin = InStr(str, L"<", 0);
    if (index_begin > -1) {
      index_end = InStr(str, L">", index_begin);
      if (index_end > -1) {
        str.erase(index_begin, index_end - index_begin + 1);
      } else {
        break;
      }
    }
  } while (index_begin > -1);
}

////////////////////////////////////////////////////////////////////////////////
// Searching and comparison

wstring CharLeft(const wstring& str, int length) {
  return str.substr(0, length);
}

wstring CharRight(const wstring& str, int length) {
  if (length > static_cast<int>(str.length())) {
    return str.substr(0, str.length());
  } else {
    return str.substr(str.length() - length, length);
  }
}

int CompareStrings(const wstring& str1, const wstring& str2,
                   bool case_insensitive, size_t max_count) {
  if (case_insensitive) {
    return _wcsnicmp(str1.c_str(), str2.c_str(), max_count);
  } else {
    return wcsncmp(str1.c_str(), str2.c_str(), max_count);
  }
}

int InStr(const wstring& str1, const wstring str2, int pos,
          bool case_insensitive) {
  if (str1.empty())
    return -1;
  if (str2.empty())
    return 0;
  if (str1.length() < str2.length())
    return -1;

  if (case_insensitive) {
    auto i = std::search(str1.begin() + pos, str1.end(),
                         str2.begin(), str2.end(),
                         &IsCharsEqual);
    return (i == str1.end()) ? -1 : i - str1.begin();
  } else {
    size_t i = str1.find(str2, pos);
    return (i != wstring::npos) ? i : -1;
  }
}

wstring InStr(const wstring& str1, const wstring& str2_left,
              const wstring& str2_right) {
  wstring output;

  int index_begin = InStr(str1, str2_left);

  if (index_begin > -1) {
    index_begin += str2_left.length();
    int index_end = InStr(str1, str2_right, index_begin);
    if (index_end > -1)
      output = str1.substr(index_begin, index_end - index_begin);
  }

  return output;
}

int InStrRev(const wstring& str1, const wstring str2, int pos) {
  size_t i = str1.rfind(str2, pos);
  return (i != wstring::npos) ? i : -1;
}

int InStrChars(const wstring& str1, const wstring str2, int pos) {
  size_t i = str1.find_first_of(str2, pos);
  return (i != wstring::npos) ? i : -1;
}

int InStrCharsRev(const wstring& str1, const wstring str2, int pos) {
  size_t i = str1.find_last_of(str2, pos);
  return (i != wstring::npos) ? i : -1;
}

bool IsAlphanumeric(const wchar_t c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}
bool IsAlphanumeric(const wstring& str) {
  if (str.empty())
    return false;

  for (size_t i = 0; i < str.length(); i++)
    if (!IsAlphanumeric(str[i]))
      return false;

  return true;
}

inline bool IsCharsEqual(const wchar_t c1, const wchar_t c2) {
  return tolower(c1) == tolower(c2);
}

bool IsEqual(const wstring& str1, const wstring& str2) {
  if (str1.length() != str2.length())
    return false;

  return std::equal(str1.begin(), str1.end(), str2.begin(), &IsCharsEqual);
}

bool IsHex(const wchar_t c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}
bool IsHex(const wstring& str) {
  if (str.empty())
    return false;

  for (size_t i = 0; i < str.length(); i++)
    if (!IsHex(str[i]))
      return false;

  return true;
}

bool IsNumeric(const wchar_t c) {
  return c >= '0' && c <= '9';
}
bool IsNumeric(const wstring& str) {
  if (str.empty())
    return false;

  for (size_t i = 0; i < str.length(); i++)
    if (!IsNumeric(str[i]))
      return false;

  return true;
}

bool IsWhitespace(const wchar_t c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

bool StartsWith(const wstring& str1, const wstring& str2) {
  return str1.compare(0, str2.length(), str2) == 0;
}

bool EndsWith(const wstring& str1, const wstring& str2) {
  if (str2.length() > str1.length())
    return false;

  return str1.compare(str1.length() - str2.length(), str2.length(), str2) == 0;
}

size_t LongestCommonSubsequenceLength(const wstring& str1,
                                      const wstring& str2) {
  if (str1.empty() || str2.empty())
    return 0;

  const size_t len1 = str1.length();
  const size_t len2 = str2.length();

  vector<vector<size_t>> table(len1 + 1);
  for (auto it = table.begin(); it != table.end(); ++it)
    it->resize(len2 + 1);

  for (size_t i = 0; i < len1; i++) {
    for (size_t j = 0; j < len2; j++) {
      if (str1[i] == str2[j]) {
        table[i + 1][j + 1] = table[i][j] + 1;
      } else {
        table[i + 1][j + 1] = max(table[i + 1][j], table[i][j + 1]);
      }
    }
  }

  return table.back().back();
}

size_t LongestCommonSubstringLength(const wstring& str1, const wstring& str2) {
  if (str1.empty() || str2.empty())
    return 0;

  const size_t len1 = str1.length();
  const size_t len2 = str2.length();

  vector<vector<size_t>> table(len1);
  for (auto it = table.begin(); it != table.end(); ++it)
    it->resize(len2);

  size_t longest_length = 0;

  for (size_t i = 0; i < len1; i++) {
    for (size_t j = 0; j < len2; j++) {
      if (str1[i] == str2[j]) {
        if (i == 0 || j == 0) {
          table[i][j] = 1;
        } else {
          table[i][j] = table[i - 1][j - 1] + 1;
        }
        if (table[i][j] > longest_length) {
          longest_length = table[i][j];
        }
      } else {
        table[i][j] = 0;
      }
    }
  }

  return longest_length;
}

size_t LevenshteinDistance(const wstring& str1, const wstring& str2) {
  const size_t len1 = str1.size();
  const size_t len2 = str2.size();

  vector<size_t> prev_col(len2 + 1);
  for (size_t i = 0; i < prev_col.size(); i++)
    prev_col[i] = i;

  vector<size_t> col(len2 + 1);

  for (size_t i = 0; i < len1; i++) {
    col[0] = i + 1;

    for (size_t j = 0; j < len2; j++)
      col[j + 1] = min(min(1 + col[j], 1 + prev_col[1 + j]),
                       prev_col[j] + (str1[i] == str2[j] ? 0 : 1));

    col.swap(prev_col);
  }

  return prev_col[len2];
}

////////////////////////////////////////////////////////////////////////////////
// Replace

void Replace(wstring& input, wstring find, wstring replace_with,
             bool replace_all, bool case_insensitive) {
  if (find.empty() || find == replace_with || input.length() < find.length())
    return;

  if (!case_insensitive) {
    for (size_t pos = input.find(find); pos != wstring::npos;
         pos = input.find(find, pos)) {
      input.replace(pos, find.length(), replace_with);
      if (!replace_all)
        pos += replace_with.length();
    }
  } else {
    for (size_t i = 0; i < input.length() - find.length() + 1; i++) {
      for (size_t j = 0; j < find.length(); j++) {
        if (input.length() < find.length())
          return;
        if (tolower(input[i + j]) == tolower(find[j])) {
          if (j == find.length() - 1) {
            input.replace(i--, find.length(), replace_with);
            if (!replace_all)
              i += replace_with.length();
            break;
          }
        } else {
          i += j;
          break;
        }
      }
    }
  }
}

void ReplaceChar(wstring& str, const wchar_t c, const wchar_t replace_with) {
  if (c == replace_with)
    return;

  size_t pos = 0;

  do {
    pos = str.find_first_of(c, pos);
    if (pos != wstring::npos)
      str.at(pos) = replace_with;
  } while (pos != wstring::npos);
}

void ReplaceChars(wstring& str, const wchar_t chars[],
                  const wstring replace_with) {
  if (chars == replace_with)
    return;

  size_t pos = 0;

  do {
    pos = str.find_first_of(chars, pos);
    if (pos != wstring::npos)
      str.replace(pos, 1, replace_with);
  } while (pos != wstring::npos);
}

////////////////////////////////////////////////////////////////////////////////
// Split, tokenize

wstring Join(const vector<wstring>& join_vector, const wstring& separator) {
  wstring str;

  for (auto it = join_vector.begin(); it != join_vector.end(); ++it) {
    if (it != join_vector.begin())
      str += separator;
    str += *it;
  }

  return str;
}

void Split(const wstring& str, const wstring& separator,
           std::vector<wstring>& split_vector) {
  if (separator.empty()) {
    split_vector.push_back(str);
    return;
  }

  size_t index_begin = 0, index_end;

  do {
    index_end = str.find(separator, index_begin);
    if (index_end == wstring::npos)
      index_end = str.length();

    split_vector.push_back(str.substr(index_begin, index_end - index_begin));

    index_begin = index_end + separator.length();
  } while (index_begin <= str.length());
}

wstring SubStr(const wstring& str, const wstring& sub_begin,
               const wstring& sub_end) {
  size_t index_begin = str.find(sub_begin, 0);
  if (index_begin == wstring::npos)
    return L"";

  size_t index_end = str.find(sub_end, index_begin);
  if (index_end == wstring::npos)
    index_end = str.length();

  return str.substr(index_begin + 1, index_end - index_begin - 1);
}

size_t Tokenize(const wstring& str, const wstring& delimiters,
                vector<wstring>& tokens) {
  tokens.clear();

  size_t index_begin = str.find_first_not_of(delimiters);

  while (index_begin != wstring::npos) {
    size_t index_end = str.find_first_of(delimiters, index_begin + 1);
    if (index_end == wstring::npos) {
      tokens.push_back(str.substr(index_begin));
      break;
    } else {
      tokens.push_back(str.substr(index_begin, index_end - index_begin));
      index_begin = str.find_first_not_of(delimiters, index_end + 1);
    }
  }

  return tokens.size();
}

////////////////////////////////////////////////////////////////////////////////
// std::string <-> std::wstring conversion

wstring StrToWstr(const string& str, UINT code_page) {
  if (!str.empty()) {
    int length = MultiByteToWideChar(code_page, 0, str.c_str(), -1, nullptr, 0);
    if (length > 0) {
      vector<wchar_t> output(length);
      MultiByteToWideChar(code_page, 0, str.c_str(), -1, &output[0], length);
      return &output[0];
    }
  }

  return wstring();
}

string WstrToStr(const wstring& str, UINT code_page) {
  if (!str.empty()) {
    int length = WideCharToMultiByte(code_page, 0, str.c_str(), -1, nullptr, 0,
                                     nullptr, nullptr);
    if (length > 0) {
      vector<char> output(length);
      WideCharToMultiByte(code_page, 0, str.c_str(), -1, &output[0], length,
                          nullptr, nullptr);
      return &output[0];
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////////////////
// Case conversion

// System-default ANSI code page
std::locale current_locale("");

void ToLower(wstring& str, bool use_locale) {
  if (use_locale) {
    std::transform(str.begin(), str.end(), str.begin(),
                   std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>),
                                current_locale));
  } else {
    std::transform(str.begin(), str.end(), str.begin(), towlower);
  }
}

wstring ToLower_Copy(wstring str, bool use_locale) {
  ToLower(str, use_locale);
  return str;
}

void ToUpper(wstring& str, bool use_locale) {
  if (use_locale) {
    std::transform(str.begin(), str.end(), str.begin(), 
                   std::bind2nd(std::ptr_fun(&std::toupper<wchar_t>),
                                current_locale));
  } else {
    std::transform(str.begin(), str.end(), str.begin(), towupper);
  }
}

wstring ToUpper_Copy(wstring str, bool use_locale) {
  ToUpper(str, use_locale);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// Type conversion

bool ToBool(const wstring& str) {
  if (str.empty())
    return false;

  const wchar_t c = str.front();

  return (c == '1' || c == 't' || c == 'T' || c == 'y' || c == 'Y');
}

double ToDouble(const wstring& str) {
  return _wtof(str.c_str());
}

int ToInt(const wstring& str) {
  return _wtoi(str.c_str());
}

wstring ToWstr(const INT& value) {
  wchar_t buffer[65];
  _ltow_s(value, buffer, 65, 10);
  return wstring(buffer);
}

wstring ToWstr(const ULONG& value) {
  wchar_t buffer[65];
  _ultow_s(value, buffer, 65, 10);
  return wstring(buffer);
}

wstring ToWstr(const INT64& value) {
  wchar_t buffer[65];
  _i64tow_s(value, buffer, 65, 10);
  return wstring(buffer);
}

wstring ToWstr(const UINT64& value) {
  wchar_t buffer[65];
  _ui64tow_s(value, buffer, 65, 10);
  return wstring(buffer);
}

wstring ToWstr(const double& value, int count) {
  std::wostringstream out;
  out << std::fixed << std::setprecision(count) << value;
  return out.str();
}

////////////////////////////////////////////////////////////////////////////////
// Trimming

wstring LimitText(const wstring& str, unsigned int limit, const wstring& tail) {
  if (str.length() > limit) {
    wstring limit_str = str.substr(0, limit);
    if (!tail.empty() && limit_str.length() > tail.length())
      limit_str.replace(limit_str.length() - tail.length(),
                        tail.length(), tail);
    return limit_str;
  } else {
    return str;
  }
}

void Trim(wstring& str, const wchar_t trim_chars[],
          bool trim_left, bool trim_right) {
  if (str.empty())
    return;

  const size_t index_begin =
      trim_left ? str.find_first_not_of(trim_chars) : 0;
  const size_t index_end =
      trim_right ? str.find_last_not_of(trim_chars) : str.length() - 1;

  if (index_begin == wstring::npos || index_end == wstring::npos) {
    str.clear();
    return;
  }

  if (trim_right)
    str.erase(index_end + 1, str.length() - index_end + 1);
  if (trim_left)
    str.erase(0, index_begin);
}

void TrimLeft(wstring& str, const wchar_t trim_chars[]) {
  Trim(str, trim_chars, true, false);
}

void TrimRight(wstring& str, const wchar_t trim_chars[]) {
  Trim(str, trim_chars, false, true);
}

////////////////////////////////////////////////////////////////////////////////
// File and folder related

void AddTrailingSlash(wstring& str) {
  if (str.length() > 0 && str[str.length() - 1] != '\\')
    str += L"\\";
}

wstring AddTrailingSlash(const wstring& str) {
  if (str.length() > 0 && str[str.length() - 1] != '\\') {
    return str + L"\\";
  } else {
    return str;
  }
}

bool CheckFileExtension(wstring extension, const vector<wstring>& extension_list) {
  if (extension.empty() || extension_list.empty())
    return false;

  for (size_t i = 0; i < extension.length(); i++)
    extension[i] = toupper(extension[i]);

  for (size_t i = 0; i < extension_list.size(); i++)
    if (extension == extension_list[i])
      return true;

  return false;
}

wstring GetFileExtension(const wstring& str) {
  return str.substr(str.find_last_of(L".") + 1);
}

wstring GetFileWithoutExtension(wstring str) {
  size_t pos = str.find_last_of(L".");

  if (pos != wstring::npos)
    str.resize(pos);

  return str;
}

wstring GetFileName(const wstring& str) {
  return str.substr(str.find_last_of(L"/\\") + 1);
}

wstring GetPathOnly(const wstring& str) {
  return str.substr(0, str.find_last_of(L"/\\") + 1);
}

bool ValidateFileExtension(const wstring& extension, unsigned int max_length) {
  if (max_length > 0 && extension.length() > max_length)
    return false;

  if (!IsAlphanumeric(extension))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Other

void AppendString(wstring& str0, const wstring& str1, const wstring& str2) {
  if (str1.empty())
    return;

  if (!str0.empty())
    str0.append(str2);

  str0.append(str1);
}

const wstring& EmptyString() {
  static const wstring str;
  return str;
}

wstring PadChar(wstring str, const wchar_t ch, const size_t len) {
  if (len > str.length())
    str.insert(0, len - str.length(), ch);

  return str;
}

wstring PushString(const wstring& str1, const wstring& str2) {
  if (str2.empty()) {
    return L"";
  } else {
    return str1 + str2;
  }
}

void ReadStringFromResource(LPCWSTR name, LPCWSTR type, wstring& output) {
  HRSRC hResInfo = FindResource(nullptr, name,  type);
  HGLOBAL hResHandle = LoadResource(nullptr, hResInfo);
  DWORD dwSize = SizeofResource(nullptr, hResInfo);

  const char* lpData = static_cast<char*>(LockResource(hResHandle));
  string temp(lpData, dwSize);
  output = StrToWstr(temp);

  FreeResource(hResInfo);
}

////////////////////////////////////////////////////////////////////////////////
// Returns the most common non-alphanumeric character in a string that is
// included in the table. Note that characters in the table are listed by their
// precedence.

const wchar_t* common_char_table = L",_ -+;.&|~";

int GetCommonCharIndex(wchar_t c) {
  for (int i = 0; i < 10; i++)
    if (common_char_table[i] == c)
      return i;

  return -1;
}

wchar_t GetMostCommonCharacter(const wstring& str) {
  vector<std::pair<wchar_t, int>> char_map;
  int char_index = -1;

  for (size_t i = 0; i < str.length(); i++) {
    if (!IsAlphanumeric(str[i])) {
      char_index = GetCommonCharIndex(str[i]);
      if (char_index == -1)
        continue;

      for (auto it = char_map.begin(); it != char_map.end(); ++it) {
        if (it->first == str[i]) {
          it->second++;
          char_index = -1;
          break;
        }
      }

      if (char_index > -1) {
        char_map.push_back(std::make_pair(str[i], 1));
      }
    }
  }
  
  char_index = 0;
  for (auto it = char_map.begin(); it != char_map.end(); ++it) {
    if (it->second * 1.8f >= char_map.at(char_index).second &&
        GetCommonCharIndex(it->first) < GetCommonCharIndex(char_map.at(char_index).first)) {
      char_index = it - char_map.begin();
    }
  }

  return char_map.empty() ? '\0' : char_map.at(char_index).first;
}