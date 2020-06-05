#include "fileReader.hpp"
#include <fstream>

using namespace std;

string
readFile(const char* file)
{
  string  result;
  fstream fs(file);
  char ch = fs.get();
  while(fs) {
    result += ch;
    ch = fs.get();
  }
  return result;
}
