#include <iostream>
#include <experimental/filesystem>
using namespace std;
using namespace std::experimental::filesystem;

int main()
{
  create_directory("./a/");
  return 0;
}
