#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

void test_istringstream()
{
  //string s = "Aster in Deadly Barn";
  //istringstream iss(s);
  //cout << "istringstream: " << iss.str() << endl;
  //
  //string tmp;
  //vector<string> res;
  //while(iss >> tmp)
  //{
  //  res.emplace_back(tmp);
  //}
  //
  //cout << "res:";
  //for(auto& str : res)
  //{
  //  cout << str << " ";
  //}
  //cout << endl;

  istringstream iss("90 92.5");
  cout << "istringstream: " << iss.str() << endl;
  int a(0);
  double b = 0.0f;
  iss >> a >> b;
  cout << "res: ";
  cout << "a=" << a << " " << "b=" << b << endl;
}

void test_ostringstream()
{
  ostringstream oss;
  string s = "Fianl Evl"; 
  oss << s;
  oss << " "; //不加这个的话, 在oss流中, a的值就和前面挨到了一起
  int a = 10;
  oss << a;
  cout << oss.str() << endl;
}

void test_stringstream()
{
  //string s = "This is my Final Fight!";
  //stringstream ss(s);
  //cout << "stringstream: " << ss.str() << endl;
  //vector<string> res;
  //string tmp;
  //while(ss >> tmp)
  //{
  //  res.emplace_back(tmp);
  //}

  //cout << "res: ";
  //for(auto& str : res)
  //{
  //  cout << str << " ";
  //}
  //cout << endl;

  /*  out */
  stringstream ss;
  int a = 100;
  ss << a;
  cout << "stringstream: " << ss.str() << endl;
  int res;
  ss >> res;
  cout << "res: " << res << endl;
}

int main()
{
  //test_istringstream();
  //test_ostringstream();
  test_stringstream();
  return 0;
}
