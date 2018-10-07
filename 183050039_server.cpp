#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <iterator>
#include <vector>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

void fill_map(map<int, string> &temp_map){
	temp_map.insert ( std::pair<int,string>(10,"ten") );
	temp_map.insert ( std::pair<int,string>(11,"eleven") );
}


void print_map(map<int, string> &temp_map){
	map<int,string>::iterator it;
	for (it=temp_map.begin(); it!=temp_map.end(); ++it){
		std::cout << it->first << " => " << it->second << '\n';
	}
}

string insert_key(map<int, string> &temp_map, int key, string val){
	if(temp_map.find(key)==temp_map.end()){
		temp_map.insert ( std::pair<int,string>(key,val) );
		return "key inserted";
	}else{
		return "error! key already present";
	}
}

string update_value_for_key(map<int, string> &temp_map, int key, string val){
	if(temp_map.find(key)==temp_map.end()){
		return "error! key not present to update";
	}else{
		std::map<int, string>::iterator it = temp_map.find(key); 
		it->second = val;
		return "value updated for key";
	}
}

string delete_key(map<int, string> &temp_map, int key){
	if(temp_map.find(key)==temp_map.end()){
		return "error! key not present to delete";
	}else{
		temp_map.erase(key);
		return "key successfully deleted";  
	}
}

string get_val_from_key(map<int, string> &temp_map, int key){
	if(temp_map.find(key)==temp_map.end()){
		return "error! key not present";
	}else{
		return temp_map.find(key)->second;  
	}
}

void error(string msg)
{
    cout<<msg<<endl;
}

int main(int argc, char const *argv[])
{
	/* code */
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[1024] = {0}; 
	map<int,string> KV_pair;
	int status=0;
	cout<<"1"<<endl;
	fill_map(KV_pair);
	
	if (argc < 2) {
         cout<<"ERROR, no port provided\n";
         exit(1);
     }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	portno = atoi(argv[2]);
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    cout<<"2"<<endl;
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
	// bzero(buffer,256);
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
		cout<<"3"<<endl;
		if (newsockfd < 0) 
		  error("ERROR on accept");
		while(1){
			buffer[1024] = {0};
			status = read(newsockfd,buffer,1024);
			if (status < 0) {
				error("ERROR reading from socket");
			}
			cout<<"Here is the message: "<<buffer<<endl;
			string ret_val;
			stringstream ss(buffer);
            vector<string> tokens{istream_iterator<string>{ss},
                      istream_iterator<string>{}};
            if(tokens.front().compare("create")==0){
            	ret_val=insert_key(KV_pair,stoi(tokens[1]),tokens[2]);

            }else if(tokens.front().compare("read")==0){
            	ret_val=get_val_from_key(KV_pair,stoi(tokens[1]));

            }else if(tokens.front().compare("update")==0){
            	ret_val=update_value_for_key(KV_pair,stoi(tokens[1]),tokens[3]);

            }else if(tokens.front().compare("delete")==0){
            	ret_val=delete_key(KV_pair,stoi(tokens[1]));
            }else if(tokens.front().compare("disconnect")==0){
            	close(newsockfd);
            	break;
            }
			
			status = write(newsockfd,ret_val.c_str(),ret_val.length());
			cout<<"trying to write on socket\n";
			if (status < 0) error("ERROR writing to socket");
		}
		cout<<"disconnected2"<<endl;
		
	}
	
	// print_map(KV_pair);
	return 0;
}