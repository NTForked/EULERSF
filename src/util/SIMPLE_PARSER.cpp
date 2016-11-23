/*
This file is part of ESFC (Eulerian Solid-FLuid Coupling).

ESFC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ESFC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ESFC.  If not, see <http://www.gnu.org/licenses/>.
*/
//
// simplistic config file parser
//
//////////////////////////////////////////////////////////////////////

#include <util/SIMPLE_PARSER.h>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// whitespace helper
//////////////////////////////////////////////////////////////////////////////
static string removeWhitespace(string in) {
  size_t s = 0;
  size_t e = in.length();
  while (s < in.length() && isspace(in[s])) {
    s++;
  }
  while (e > s && isspace(in[e - 1])) {
    e--;
  }
  return in.substr(s, e - s);
}

//////////////////////////////////////////////////////////////////////////////
// force string to lowercase
//////////////////////////////////////////////////////////////////////////////
static void forceLower(string& input) {
  string::iterator i;
  for (i = input.begin(); i != input.end(); i++) *i = tolower(*i);
}
map<string, string> SIMPLE_PARSER::mVals;
map<string, bool> SIMPLE_PARSER::mUsed;

static void split(const string& s, char delim, vector<string>& elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////////
bool SIMPLE_PARSER::parse(std::string file) {
  if (file.length() == 0) {
    std::cout << "Skipping config file\n";
    return false;
  }

  std::cout << "Using config file " << file << "\n";

  int lineCnt = 1;

  char buffer[512];
  std::ifstream myfile(file.c_str());

  if (!myfile.good()) {
    cout << " Failed to open file " << file << "!!!" << endl;
    return false;
  }
  if (myfile.is_open()) {
    while (!myfile.eof()) {
      myfile.getline(buffer, 512);
      string line(buffer);
      if (line.length() < 1) continue;
      if (line[0] == '#') continue;

      size_t pos = line.find_first_of("=");
      if (pos != string::npos && pos < line.length()) {
        string lhs = removeWhitespace(line.substr(0, pos));
        string rhs =
            removeWhitespace(line.substr(pos + 1, line.length() - pos));

        forceLower(lhs);
        forceLower(rhs);

        // store...
        mVals[lhs] = rhs;
      } else {
        // simple check for errors...
        string check = removeWhitespace(line);
        if (check.length() > 0) {
          std::cerr << "Unable to parse, error in line " << lineCnt << ": '"
                    << line << "' !\n";
          // exit(1);
          return false;
        }
      }
      lineCnt++;
    }
    myfile.close();
    return true;
  } else {
    std::cerr << "Unable to parse!\n";
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////
// See if a parameter was defined
//////////////////////////////////////////////////////////////////////////////
bool SIMPLE_PARSER::defined(string name) {
  map<string, string>::iterator i;
  i = mVals.find(name);

  return (i != mVals.end());
}

//////////////////////////////////////////////////////////////////////////////
// generic scalar retrieval
//////////////////////////////////////////////////////////////////////////////
template <class T>
T SIMPLE_PARSER::getScalarValue(string name, T defaultValue, bool needed) {
  T ret = 0;
  forceLower(name);
  if (mVals.find(name) == mVals.end()) {
    if (needed) {
      std::cerr << "Required value '" << name
                << "' not found in config file!\n";
      exit(1);
    }
    return defaultValue;
  }
  ret = (T)atof(mVals[name].c_str());
  mUsed[name] = true;
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get an VEC3F
//////////////////////////////////////////////////////////////////////////////
VEC3F SIMPLE_PARSER::getVEC3F(string name, VEC3F defaultValue, bool needed) {
  VEC3F ret;
  forceLower(name);
  if (mVals.find(name) == mVals.end()) {
    if (needed) {
      std::cerr << "Required value '" << name
                << "' not found in config file!\n";
      exit(1);
    }
    return defaultValue;
  }
  vector<string> tokens;
  split(mVals[name], ',', tokens);
  for (int v = 0; v < 3; v++) ret[v] = atof(tokens[v].c_str());
  mUsed[name] = true;
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get an VEC2F
//////////////////////////////////////////////////////////////////////////////
VEC2F SIMPLE_PARSER::getVEC2F(string name, VEC2F defaultValue, bool needed) {
  VEC2F ret;
  forceLower(name);
  if (mVals.find(name) == mVals.end()) {
    if (needed) {
      std::cerr << "Required value '" << name
                << "' not found in config file!\n";
      exit(1);
    }
    return defaultValue;
  }
  vector<string> tokens;
  split(mVals[name], ',', tokens);
  for (int v = 0; v < 2; v++) ret[v] = atof(tokens[v].c_str());
  mUsed[name] = true;
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get an integer
//////////////////////////////////////////////////////////////////////////////
int SIMPLE_PARSER::getInt(string name, int defaultValue, bool needed) {
  int ret = getScalarValue<int>(name, defaultValue, needed);
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get a boolean
//////////////////////////////////////////////////////////////////////////////
bool SIMPLE_PARSER::getBool(string name, bool defaultValue, bool needed) {
  bool ret = (getScalarValue<int>(name, defaultValue, needed) != 0);
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get a floating point
//////////////////////////////////////////////////////////////////////////////
double SIMPLE_PARSER::getFloat(string name, double defaultValue, bool needed) {
  double ret = getScalarValue<double>(name, defaultValue, needed);
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// get a string
//////////////////////////////////////////////////////////////////////////////
string SIMPLE_PARSER::getString(string name, string defaultValue, bool needed) {
  string ret("");
  forceLower(name);
  if (mVals.find(name) == mVals.end()) {
    if (needed) {
      std::cerr << "Required value '" << name
                << "' not found in config file!\n";
      exit(1);
    }
    return defaultValue;
  }
  ret = mVals[name];
  mUsed[name] = true;

  // force to lower case
  forceLower(ret);

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// check if there were any unused pairs
//////////////////////////////////////////////////////////////////////////////
bool SIMPLE_PARSER::haveUnusedValues() {
  for (std::map<string, string>::iterator i = mVals.begin(); i != mVals.end();
       i++) {
    if ((*i).second.length() > 0) {
      if (!mUsed[(*i).first]) {
        return true;
      }
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// print unused pairs
//////////////////////////////////////////////////////////////////////////////
string SIMPLE_PARSER::printAllUnused() {
  std::ostringstream out;
  for (std::map<string, string>::iterator i = mVals.begin(); i != mVals.end();
       i++) {
    if ((*i).second.length() > 0) {
      if (!mUsed[(*i).first]) {
        out << "'" << (*i).first << "'='" << (*i).second << "' ";
      }
    }
  }
  return out.str();
}