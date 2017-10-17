#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>

using namespace std;
#include "header.h"

char blue[] = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
char normal[] = { 0x1b, '[', '0', ';', '3', '9', 'm', 0 };
int terminating =0;
void join (string);
void server(int portNo);
void create(int portNo);
void create_client(const char*,int,string,string,string,int);
void forward_client(const char* ipA,int portA,string ip,string port,string joinid,int msg_type);
string server_join(string ,string ,string);
string check(string key);
void get(string nodeid);
void update_table(string temp);

bool comparator(cell &a,cell &b){
  if (a.nodeid < b.nodeid) return true;
  return false;
}

class Routing_table{
  public:
  string ip,port,nodeid;
  vector<cell> lset;
  vector<cell> nset;
  cell table[8][16];
  map <string,string> mymap;

  void printlset(){
   // sort(this->lset.begin(),this->lset.end(),comparator);
    vector<cell>::iterator it;
    int i=0;
    for(it=this->lset.begin();it!=this->lset.end();it++){
        if(it->nodeid.length()!=0){
          cout<<it->nodeid<<"  ";
          i++;
        }
    }
    while(i<4){
      cout<<"-----"<<"  ";
      i++;
    }
    cout<<endl;
  }
  
  Routing_table(int portNo) {
     ip=getipaddress(); 
     port=to_string(portNo);
     nodeid=hashID(port);
     int i,j;
      for(i=0;i<8;i++){
        for(j=0;j<16;j++){
          table[i][j].ip="";
          table[i][j].port="";
          table[i][j].nodeid="";
        }
      }
      int ind;
      for(i=0;i<8;i++){
      if(nodeid[i] >= 'a' && nodeid[i]<='f'){
        ind=nodeid[i]-'a'+10;
      }else{
        ind=nodeid[i]-'0';
      }
      table[i][ind].ip=ip;
      table[i][ind].port=port;
      table[i][ind].nodeid=nodeid;
     }
  }

  void dump(){
    int i,j;
    for(i=0;i<8;i++){
      for(j=0;j<16;j++){
        if(table[i][j].ip=="" &&   table[i][j].port=="" &&     table[i][j].nodeid==""){
          cout<<"-------- ";
        }else{
           cout<<table[i][j].nodeid<<" ";
        }
      }cout<<endl;
    }
  }

};


Routing_table* node;

void put_routing(string key,string value,string keyid,string port)
{
  int flag=0;
  cell nextcell;
  int size=node->lset.size();
  int i;

  // if(strcmp(node->lset.begin()->nodeid.c_str(),keyid.c_str()) >= 0 && 
  //     strcmp(node->lset.begin()->nodeid.c_str(),keyid.c_str()) <= 0){

  //   for(i=1;i<size;i++){
  //     if(node->lset[i].nodeid == keyid){
  //       flag=1;
  //       nextcell=node->lset[i];
  //       break;
  //     }
  //   }
  // }

  for(i=0;i<size;i++){
    if(node->lset[i].nodeid == keyid){
      flag=1;
      nextcell=node->lset[i];
      break;
    }
  }
  
  string ip=getipaddress();
  //cout<<"flag :"<<flag<<endl;
  if(flag == 0){
    int i;
    for(i=0;i<8;){
      if(node->nodeid[i]==keyid[i])
          i++;
      else
        break;
    }
   // cout<<"i :"<<i<<endl;
    int ind;
    if (keyid[i]>='a' && keyid[i]<='f')
      ind=keyid[i] -'a';
    else
      ind=keyid[i]-'0';
      if(node->table[i][ind].nodeid.length()!= 0 ){
        nextcell = node->table[i][ind];
        flag=1;
      }
  }
  // cout<<"nextcell :"<<nextcell.ip<<endl;
  // cout<<keyid<<endl;
  // cout<<node->nodeid<<endl;
  // cout<<ip<<endl;
  // cout<<port<<endl;
  // cout<<flag<<endl;
  if (flag==0){
    node->mymap[key]=value;
    return;
  }
  else{
  thread t3(create_client,ip.c_str(),atoi(port.c_str()),key,value,keyid,2);
  t3.join(); 
  }      
}


int main(){
  string s,sub;
  unsigned int found;
  int found1,portNo=0;
  string ip=""; 

  while(terminating==0){  
    cout<<blue<<"Pastry :"<<normal;
    getline(cin,s);
      found=s.find(' ');
      if (found!=string::npos)
        sub=s.substr(0,found);
      else
        sub=s;

      if(sub == "port"){
        found1=s.find('\n');
        ip=s.substr(found+1,found1);
        portNo=atoi(ip.c_str());
      }
      else if(sub == "create"){
          if(portNo == 0){
                cout<<"Enter port first"<<endl;
                continue;
            }
            create(portNo);
           // hashID(ip);
      }
      else if(sub == "join"){
            join(s);
      }
      else if(sub == "dump"){
            node->dump();
      }
      else if(sub == "lset"){
            node->printlset();
      }
      else if(sub == "get"){
      for(map<string, string>::const_iterator it = node->mymap.begin();
                  it != node->mymap.end(); ++it){
                cout << it->first << " " << it->second  << "\n";
      }
      }
      else if(sub == "put"){
          vector<string> v;
          string temp="";
          int i;
          int len=s.length();

          for(i = 0; i < len; ++i){
            if (s[i]==' '){
              v.push_back(temp);
              temp="";
            }
            else
              temp+=s[i];
          }
          v.push_back(temp);
          if (v.size()!=3)
            cout<<"Command Syntax\n";
          else
            put_routing(v[1],v[2],hashID(v[1]),ip);
      }
      else if(sub == "quit:" || sub == "shutdown:"){
            terminating=1;
        cout<<"Terminating.."<<endl;
        exit(0);
     }
      
  }
  return 0;
}

void join(string s){
    vector<string> v;
    int i=0,len=s.length();
    string temp="";
    for(i = 0; i < len; ++i){
        if (s[i]==' '){
            v.push_back(temp);
            temp="";
            continue;
        }
        temp+=s[i];
    }
    v.push_back(temp);
    if (v.size()!=3)
        cout<<"Command Syntax : join <ip address> <port>\n";
    else{
        int msg_type=1;
        string a="";
        thread t3(create_client,v[1].c_str(),atoi(v[2].c_str()),a,a,a,msg_type);
        t3.join();
    }
}

void create(int portNo){
    thread t1(server,portNo);
    t1.detach();
}

vector<string> extract(string m){
  vector<string> v;
  int i=2;
  string temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);

  return v;
}

vector<string> extract2(string m){
  vector<string> v;
  int i=2;
  string temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);
  i++;
  temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);i++;temp="";
  while(m[i] != '@'){
    temp+=m[i++];
  }
  v.push_back(temp);
  return v;
}

void server(int portNo){
  int listenFd,connFd;
  socklen_t len;
  struct sockaddr_in svrAdd, clntAdd;
  listenFd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenFd < 0){
      cerr << "Cannot open socket" << endl;
      return;
  } 
  bzero((char*) &svrAdd, sizeof(svrAdd)); 
  svrAdd.sin_family = AF_INET;
  svrAdd.sin_addr.s_addr = INADDR_ANY;
  svrAdd.sin_port = htons(portNo);
  if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0){
    cerr << "Cannot bind" << endl;
    return;
  }
    
  listen(listenFd, 1000);
  len = sizeof(clntAdd);
  node =  new Routing_table(portNo);
  //cout<<portNo<<endl;
  while (1){
    connFd = accept(listenFd, (struct sockaddr *)&clntAdd, &len);
    if (connFd < 0){
      cerr << "Cannot accept connection" << endl;
      return;
    }
    char msg[10000];
    
    if(read(connFd,msg,10000)>0){
      int type=msg[0]-'0';
      string m=msg;
      //cout<<"join request from client :"<<m<<endl;
      
      if (type==1){
        vector<string> v = extract(m);
       // cout<<"vector :"<<v[0]<<" "<<v[1]<<" "<<v[2]<<endl;
        thread t3(server_join,v[0],v[1],v[2]);
        t3.detach();
      }
      else if (type==2){
        vector<string> v = extract2(m);
        thread t3(put_routing,v[0],v[1],v[2],v[3]);
        t3.detach();
      }
      else if (type==4){
        //cout<<"message :"<<m<<endl;
        thread t4(update_table,m);
        t4.detach();
      }
    }else{
      cout<<"in else\n";
    }
  }
}

void update_table(string temp){
  temp=temp.substr(2,temp.length());
  string delim = "@",t="";
  int j=0,i=0,k=0;
  char te=temp[0];
  i=te-'0';
 // cout<<"value of t:"<<i<<endl;
  for(j=2;k<16 && temp[j]!='\0';k++){ 
      while(temp[j] != '@'){
        t=t+temp[j];
        j++;
       }
            //cout<<t<<" ";
            if(t.size() != 0){
              node->table[i][k].ip=t;
            }
            t="";j++;
            while(temp[j] != '@'){
              t=t+temp[j];
              j++;
            }
            //cout<<t<<" ";
             if(t.size() != 0)
            node->table[i][k].port=t;
            t="";j++;
            while(temp[j] != '@'){
              t=t+temp[j];
              j++;
            }
            //cout<<t<<endl;
             if(t.size() != 0)
               node->table[i][k].nodeid=t;
            t="";j++;
       cell tempcell;
       tempcell.ip=node->table[i][k].ip;
       tempcell.port=node->table[i][k].port;
       tempcell.nodeid=node->table[i][k].nodeid;
      if(tempcell.nodeid!=node->nodeid && (node->lset.empty() || node->lset.size() < 4)){  
         node->lset.push_back(tempcell);
       }
      else{
      for(vector<cell>::iterator it=node->lset.begin();it!=node->lset.end();it++){
       if(tempcell.nodeid!=node->nodeid && abs(strcmp(it->nodeid.c_str(),node->nodeid.c_str())) > abs(strcmp(tempcell.nodeid.c_str(),node->nodeid.c_str()))){
          //node->lset.erase(node->lset.begin() + 1);
          it = node->lset.erase(it);
          node->lset.push_back(tempcell);
          break;
         }
       }
    }
    sort(node->lset.begin(),node->lset.end(),comparator);
    }     
}

int createConnectionWithX(string ip,string port){
  int sockfd = 0;
  struct sockaddr_in serv_addr;
  int portNo = atoi(port.c_str());
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      cout<<"\n Error : Could not create socket \n";
  } 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portNo); 
  if(inet_pton(AF_INET,ip.c_str(), &serv_addr.sin_addr)<0){
    cout<<"\n inet_pton error occured\n"<<endl;
  } 

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    cout<<"Error : Connect Failed \n"<<endl;
  } 
  return sockfd;
}

string server_join(string ip, string port, string joinid){
  int i,j;
  for(i=0;i<8;){
    if(node->nodeid[i] == joinid[i])
      i++;
    else
      break;
  }
  
  string t="@";
  string s="4"+t+to_string(i)+t;
  int bytecount;

  for(j=0;j<16;j++){
    s=s+node->table[i][j].ip+"@"+node->table[i][j].port+"@"+node->table[i][j].nodeid+"@";
  }
 // cout<<"ip :"<<ip<<"port :"<<port<<endl;
  int XFd = createConnectionWithX(ip,port);
  if((bytecount=write(XFd,s.c_str(),s.length())) < 0){
    cout<<"Error in write\n";
  }
      cell tempcell;
       tempcell.ip=ip;
       tempcell.port=port;
       tempcell.nodeid=joinid;
  if(node->lset.empty() || node->lset.size() < 4){ 
       node->lset.push_back(tempcell);
      // sort(node->lset.begin(),node->lset.end());
  }
  else{
      for(vector<cell>::iterator it=node->lset.begin();it!=node->lset.end();it++){
       if(abs(strcmp(it->nodeid.c_str(),node->nodeid.c_str())) > abs(strcmp(joinid.c_str(),node->nodeid.c_str()))){
          //node->lset.erase(node->lset.begin() + 1);
          it = node->lset.erase(it);
          node->lset.push_back(tempcell);
          break;
       }
     }
  }
  j=0;
  if(i < 7){
    if(joinid[i+1] >='a' && joinid[i+1]<='e')
      j=joinid[i+1]-'a'+10;
    else
      j=joinid[i+1];
   // cout<<"value of j:"<<endl;
    if(node->table[i][j].nodeid != ""){
      string forwardip=node->table[i][j].ip;
      string forwardport=node->table[i][j].port;
      thread t10(forward_client,forwardip.c_str(),atoi(forwardport.c_str()),ip,port,joinid,1);
      t10.detach();
      }
      node->table[i][j].ip=ip;
      node->table[i][j].port=port;
      node->table[i][j].nodeid=joinid;
  }
  return s;
}

void forward_client(const char* ipA,int portA,string ip,string port,string joinid,int msg_type){
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout<<"\n Error : Could not create socket \n";
    } 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portA); 
    if(inet_pton(AF_INET,ipA, &serv_addr.sin_addr)<0){
      cout<<"\n inet_pton error occured\n"<<endl;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    cout<<"Error : Connect Failed \n"<<endl;
    } 

     switch(msg_type){
      case 1:{
        /*routing*/
        int bytes;
        string t="@";
        string msg="1"+t+ip+t+port+t+joinid+"@"+"\n";
        if((bytes=write(sockfd,msg.c_str(),msg.length())) > 0){
        }
        break;
        sleep(100);
       // node->dump();
      }
      case 2 :
              /*get*/
      break;
      case 3:
            /*put*/
      break;
     }
}

void create_client(const char* ipA,int portA,string key,string value,string keyid,int msg_type){
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout<<"\n Error : Could not create socket \n";
    } 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portA); 
    if(inet_pton(AF_INET,ipA, &serv_addr.sin_addr)<0){
      cout<<"\n inet_pton error occured\n"<<endl;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    cout<<"Error : Connect Failed \n"<<endl;
    } 

     switch(msg_type){
      case 1:{
        /*routing*/
        int bytes;
        string t="@";
        string msg="1"+t+node->ip+t+node->port+t+node->nodeid+"@"+"\n";
        if((bytes=write(sockfd,msg.c_str(),msg.length())) > 0){
        }
        break;
      }
      case 2 :{ 
        string t="@";
        string msg="2"+t+ipA+t+to_string(portA)+t+node->nodeid+t+key+t+value+t+keyid+"@\n";
        write(sockfd,msg.c_str(),msg.length());
        break;
      }
      case 3:{
              
              break;
      }
     }
}