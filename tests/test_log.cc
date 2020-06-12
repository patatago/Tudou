#include "log.h"
#include <string>
#include <iostream>
//using namespace std;
using namespace tudou;
using std::to_string;
using std::endl;
int main()
{
	Loglevel(LogLevel::TRACE);
	//og::getInstance().setLogLevel(LogLevel::DEBUG);
	Info<<"first"<<endl;
	Info<<"second"<<endl;
	
	Debug <<  to_string(3.141582600).c_str() << endl;
	//while(1) ;
}
