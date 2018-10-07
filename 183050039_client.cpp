#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <iterator>
#include <vector>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

#include <csignal>

using namespace std;

int is_connected_to_server=0;
int sockfd;

void error(string msg)
{
    cout<<msg;
}



void connect_to_server(string server_ip, int server_port){
    if(!is_connected_to_server){
        struct sockaddr_in serv_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){
            error("error! opening socket\n");
            return;
        } 
            
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
        serv_addr.sin_port = htons(server_port);
        
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
            error("error! connecting\n");
            return;
        }
        cout<<"connected to server\n";
        is_connected_to_server=1; 
            
    }else{
        cout<<"error! already connected to server\n";
        return;
    }
    
}

void disconnect(){
    is_connected_to_server=0;
    int n=close(sockfd);
    // cout<<"closing "<<n<<endl;
}


void perform_actions(string inp_buff){
    stringstream ss(inp_buff);
    vector<string> tokens{istream_iterator<string>{ss},
              istream_iterator<string>{}};
    if(tokens.front().compare("connect")==0){
        connect_to_server(tokens[1],stoi(tokens[2]));
    }else{
        if(is_connected_to_server){
            int inp_buff_len=inp_buff.length();
            string buff_prepend=to_string(inp_buff_len);
            inp_buff_len+=buff_prepend.length();//adding the length of digits also
            inp_buff_len+=1;//for space
            buff_prepend=to_string(inp_buff_len);
            buff_prepend.append(" ");
            inp_buff.insert (0,buff_prepend);
            if(tokens.front().compare("disconnect")==0){
                
                int n = write(sockfd,inp_buff.c_str(),inp_buff.length());
                if (n < 0) 
                     error("error! disconnecting from socket\n");
                cout<<"disconnected\n";
                disconnect();
            }else if(tokens.front().compare("create")==0 || tokens.front().compare("read")==0 ||tokens.front().compare("update")==0||tokens.front().compare("delete")==0){
                int n = write(sockfd,inp_buff.c_str(),inp_buff.length());
                if (n < 0) 
                     error("error! writing from socket\n");
                cout<<"written\n";
                char buffer[256]={0};
                n = read(sockfd,buffer,255);
                if (n < 0) 
                     error("error! reading from socket\n");
                cout<<buffer<<endl;
                cout<<"read\n";
            }else{
                cout<<"invalid command\n";
            }  
        }else{
            cout<<"error! Not connected to server\n";
        } 
    } 
}


void interactive_mode(){
	while(1){
		string inp_buff;
        cout<<"Enter the input: ";
		// cin>>inp_buff;
        getline(cin, inp_buff);
        cout<<inp_buff<<endl;
		if(inp_buff.compare("exit")==0){
			break;
		}else{
            perform_actions(inp_buff);
        }
		
	}
}

void batch_mode(string filename){
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        // cout<<"perform_actions";
        perform_actions(line);
        // process pair (a,b)
    }

}

void signalHandler( int signum ) {
   // cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program 
   if(is_connected_to_server){
        string inp_buff="disconnect";
        int n = write(sockfd,inp_buff.c_str(),inp_buff.length());
        if (n < 0) 
             error("error! disconnecting from socket\n");
        disconnect();
   } 
  
   exit(signum);  
}

int main(int argc, char *argv[])
{

    string filename;
    signal(SIGINT, signalHandler);  
    if (argc < 2) {
       cout<<"usage "<<argv[0]<<" interactive/batch\n";
       exit(0);
    }
    cout<<"1"<<endl;
    string argv1 = argv[1];
    cout<<"2"<<endl;
    if (argv1.compare("interactive") == 0){
    	cout<<"****interactive mode***\n";
        interactive_mode();
    }
    else if(argv1.compare("batch") == 0){
    	if (argc < 3) {
	       cout<<"usage "<<argv[0]<<" batch filename\n";
	       exit(0);
	    }
	    filename=argv[2];
        cout<<"****batch mode***\n";
        batch_mode(filename);
    }else{
    	error("invalid mode");
    }
    
    return 0;
}
