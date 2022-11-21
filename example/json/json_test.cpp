#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <jsoncpp/json/json.h>
using namespace std;

//序列化:JSON对象 --->  write到流中 (转化为:JSON格式字符串)
void Serialization()
{
  //Json是个命名空间
  //using namespace Json; 
  //测试: 我们手动构建一个JSON对象, 并为其赋予一定的值; 然后把JSON对象转化为 JSON字符串
  Json::Value obj; 
  obj["name"] = "小明";
  obj["age"] = 20;
  obj["scores"].append(95);
  obj["scores"].append(90.5);
  obj["scores"].append(100);

  Json::StreamWriterBuilder swbuilder;
  std::unique_ptr<Json::StreamWriter> sw(swbuilder.newStreamWriter());
  
  std::ostringstream oss;
  sw->write(obj, &oss);
  
  std::cout << oss.str() << std::endl;
}


//反序列化: JSON格式字符串  --->  JSON对象
void Deserialization()
{
  //测试: 我们自己构建一个JSON格式字符串, 然后把这个JSON格式字符串 转化为 JSON对象(存入JSON对象当中)
  string s = 
    R"({
    "name" : "小明",
    "age" : 20,
    "scores" : [90, 90.5, 100]
    })";

  Json::Value obj;
  Json::CharReaderBuilder crBuilder;
  std::unique_ptr<Json::CharReader> cr(crBuilder.newCharReader());
  std::string err;
  bool ret = cr->parse(s.c_str(), s.c_str() + s.size(), &obj, &err);
  if(ret == false){
    std::cerr << "parse Error:" << err << std::endl;
    return;
  }
  
  std::cout << "name: " << obj["name"].asString() << std::endl;
  std::cout << "age: " << obj["age"].asInt() << std::endl;
  
  //int len = obj["scores"].size();
  std::cout << "Array:\n[" << std::endl;
  for(auto& x : obj["scores"])
  {
    std::cout << '\t' << x << std::endl;
  }
  std::cout << "]" << std::endl;
}

int main()
{
  //Serialization();
  Deserialization();
  return 0;
}
