#include <iostream>
#include <string>
#include <string.h>

using namespace std;
#include "Packet.h"

void Packet :: build(string srcNodeId,string key,int hopCount,/*message_type type,*/string message){
	this->srcNodeId=srcNodeId;
	this->key=key;
	this->hopCount=hopCount;
//		this->type=type;
	this->message=message;
}

int Packet :: deserialise(char * s){
	/*converts s[] to packet*/
	int len=strlen(s);
	int i=0,count=0;
	for(i = 0; i < len; ++i){
		if (s[i]==';')
		{
			count++;
			continue;
		}
		if (count==0)
		{
			this->srcNodeId+=s[i];
		}else if (count==1)
		{
			this->key+=s[i];
		}else if (count==2)
		{
			this->hopCount+=s[i];
		}else if(count==3){
			//this->type+=s[i];
		}else if (count==4)
		{
			this->message+=s[i];
		}
	}
	if (count==5)
	{
		return 1; //success
	}
	return 0;
}

char* Packet :: serialise(){
	/* convert packet to character stream*/
	string s="";
	s += this->srcNodeId+";";
	s+=this->key+";";
	s+=this->hopCount+";";
//		s+=this->type+";";
	s+=this->message+";";
	int i=0;
	int length = s.length();
	char* temp=(char*)malloc(sizeof(char)*(length+1));
	for(i = 0; i < length; ++i){
		temp[i]=s[i];
	}temp[i]='\0';
	return temp;
}