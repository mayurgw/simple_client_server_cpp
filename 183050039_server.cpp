#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <iterator>
#include <vector>

#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define MAX 1000
#define NO_OF_WORKERS 1


int client_buffer[MAX];
int current_count=0,consume_ptr=0,produce_ptr=0;
int sockfd, portno, clilen;
struct sockaddr_in serv_addr, cli_addr;
map<int,string> KV_pair;
pthread_cond_t empty,filled;
pthread_mutex_t mutex;


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


//add request to queue
void *add_client_to_queue(void *data)
{
	

	while(1)
	{
	  // down mutex
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
		cout<<"accepted connection "<<newsockfd<<endl;
		if (newsockfd < 0) 
			error("error! on accept\n");
		pthread_mutex_lock(&mutex);

		while(current_count==MAX-1){
			// sleep master waiting for an empty slot
			printf("master waiting\n");
			pthread_cond_wait(&empty, &mutex);
		}

		produce_ptr=(produce_ptr+1)%MAX;
		client_buffer[produce_ptr]=newsockfd;
		current_count++;
		// up mutex

		pthread_cond_signal(&filled); 
		pthread_mutex_unlock(&mutex);
	}

}

void* serve_request(void* data){
  	
  	while(1){
  		int newsockfd;
  		pthread_mutex_lock(&mutex);
		while(current_count==0){
		// sleep worker waiting for a filled slot
			pthread_cond_wait(&filled, &mutex);
		}
		consume_ptr=(consume_ptr+1)%MAX;
		newsockfd=client_buffer[consume_ptr];
		current_count--;
		// up mutex 
		pthread_mutex_unlock(&mutex);
		while(1){
		// int* newsockfd=(int*) data;
			int status;
			char buffer[1024] = {0};
			status = read(newsockfd,buffer,1024);
			if (status < 0) {
				error("error! reading from socket\n");
			}
			// cout<<"Here is the message: "<<buffer<<endl;
			string ret_val;
			stringstream ss(buffer);
	        vector<string> tokens{istream_iterator<string>{ss},
	                  istream_iterator<string>{}};
	        pthread_mutex_lock(&mutex);
	        if(tokens.front().compare("create")==0){
	        	ret_val=insert_key(KV_pair,stoi(tokens[1]),tokens[3]);

	        }else if(tokens.front().compare("read")==0){
	        	ret_val=get_val_from_key(KV_pair,stoi(tokens[1]));

	        }else if(tokens.front().compare("update")==0){
	        	ret_val=update_value_for_key(KV_pair,stoi(tokens[1]),tokens[3]);

	        }else if(tokens.front().compare("delete")==0){
	        	ret_val=delete_key(KV_pair,stoi(tokens[1]));
	        }else if(tokens.front().compare("disconnect")==0){
	        	close(newsockfd);
	        	pthread_mutex_unlock(&mutex);
	        	break;
	        }
	        pthread_mutex_unlock(&mutex);
			cout<<"writing to socket\n";
			status = write(newsockfd,ret_val.c_str(),ret_val.length());
			
			if (status < 0) error("error! writing to socket\n");
		}
		cout<<"disconnected "<<newsockfd<<endl;
		pthread_cond_signal(&empty); //signaling for the case client gets disconnected
  	}
	
}


int main(int argc, char const *argv[])
{
	/* code */
	
	pthread_t workers[NO_OF_WORKERS],prod_thread;
	
	int status=0;
	cout<<"1"<<endl;
	fill_map(KV_pair);
	
	if (argc < 2) {
         cout<<"ERROR, no port provided\n";
         exit(1);
     }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	portno = atoi(argv[2]);
	string serverip = argv[1];
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serverip.c_str());
    serv_addr.sin_port = htons(portno);
    cout<<"2"<<endl;
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("error! on binding\n");
    listen(sockfd,NO_OF_WORKERS);
    clilen = sizeof(cli_addr);
	// bzero(buffer,256);
    pthread_create(&prod_thread, NULL, add_client_to_queue, NULL);

    // creating worker threads
    for(int threadNo=0;threadNo<NO_OF_WORKERS;threadNo++){
		pthread_create(&workers[threadNo], NULL, serve_request, NULL);				
	}

	//will never be called
	pthread_join(prod_thread, NULL);
	for(int threadNo=0;threadNo<NO_OF_WORKERS;threadNo++){
		pthread_join(threadNo, NULL);		
	}
	
	
	// print_map(KV_pair);
	return 0;
}