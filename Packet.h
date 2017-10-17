#include <iostream>
#include <string>
#include <string.h>

using namespace std;

class Packet{
	string srcNodeId,key;
	int hopCount;
//	message_type type;
	string message;

public:

	void build(string srcNodeId,string key,int hopCount/*,message_type type*/,string message);
	int deserialise(char * s);
	char* serialise();
};