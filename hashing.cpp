//g++ -Wall -pthread -std=c++11 pastry.cpp -lcrypto
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <iostream>
#include <string>
using namespace std;

#include "header.h"

string md5(string input_str){
	char result[33];
	unsigned char digest[MD5_DIGEST_LENGTH]; 
	MD5((unsigned char *)(input_str.c_str()), input_str.length(), (unsigned char*)&digest);
	
	for(int i = 0; i < 16; i++)
		sprintf(&result[i*2], "%02x", (unsigned int)digest[i]);

	return string(result);
}

string getipaddress(){
	char ip_address[15];
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    memcpy(ifr.ifr_name, "enp2s0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    strcpy(ip_address,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    string str(ip_address);
    return str;
}
 
string hashID(string port){
	string id=port+getipaddress();
	//cout<<id<<endl;
	string s=md5(id);
	s=s.substr(0,8);
	//cout<<s<<endl;
	return s;
}