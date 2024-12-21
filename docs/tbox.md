
#  

## thread controller  

## thread sdo  
异步
readSOD()
writeSDO()

## thread pdo  
可以用一个ontick函数推送pdo发送接收的结果  

class pdo {
  
}

class datapacket {
private:
  int id,
  std::vector<> pdoList;
  std::map<int, pdo> pdoList;
  std::map<int, dout> doutList;
}

ontick(datapacket* rxpacket)
{

}


