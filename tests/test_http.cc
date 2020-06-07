#include <string>
#include "http/httprequest.h"
#include <iostream>
#include <arpa/inet.h>

using namespace std;
using namespace tudou;


int main()

{
	//string s="\r\t";
	//cout << s.size() << endl;
    Buffer buffer;
	string s;

	socklen_t listenfd, connfd, port, clientlen;
    struct sockaddr_in serv;

	serv.sin_port = htons(8080);
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_family = AF_INET;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&opt, static_cast<socklen_t>(sizeof(opt)));

	bind(sockfd, (struct sockaddr*)&serv, sizeof(serv));
	listen(sockfd, 20);

    
    while (1) {
		clientlen = sizeof(serv);
		sockaddr_in client;
		
		connfd = accept(sockfd, (sockaddr *)&client, &clientlen); //line:netp:tiny:accept
	    char buf[1024] = {};                                         //line:netp:tiny:doit
		read(connfd, buf, sizeof(buf));
		cout << buf <<endl;
		Buffer in, out;
		in.append(buf);//line:netp:tiny:close
    	HttpParse pa;
		pa.parse(in);
		
		HttpResponse to(pa.getHttpMessage(),"tudou",out);
		to.append();



		int t = out.find("\r\n\r\n");
		cout << "out.size = " << out.size() << endl;
		string f = out.read(t+4-out.getReadPos()); //请求体之前
		
		cout <<"f.first.size = " <<f.size()<<endl;


		t = out.find("\r\n\r\n");

	
		string  g = out.read(t+4-out.getReadPos());
		f.append(g);
		int ret = write(connfd, f.c_str(), f.size());
		cout << "write: " << ret <<endl;
		if(f.size() < 10000) cout << f<<endl;
		
	}
}
