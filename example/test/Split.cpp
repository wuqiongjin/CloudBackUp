#include <iostream>
#include <string>
#include <vector>
using namespace std;

std::vector<std::string> Split(const std::string& s, const std::string& sep) {
	std::vector<std::string> res;
	size_t pos = 0, prev = 0;
	while ((pos = s.find(sep, prev)) != std::string::npos)
	{
		//出现了2个sep连在了一起, 此时跳过
		if (prev == pos) {
			prev = pos + 1;	//别忘了修改prev的值
			continue;
		}
		res.emplace_back(s.substr(prev, pos - prev));
		prev = pos + 1;
	}

	if (prev < s.size())
	{
		res.emplace_back(s.substr(prev));
	}

	return res;
}

int main()
{
  string s = "123///4/5/678";
  auto ret = Split(s, "/");
  for(auto& str : ret)
  {
    cout << str << endl;
  }

  return 0;
}
