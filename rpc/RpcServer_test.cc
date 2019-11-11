#include <matelib/InetAddress.h>
#include <matelib/EventLoop.h>
#include "RpcServer.h"
#include <string>
#include <stdio.h>

using namespace std;
using namespace lfp;
using namespace lfp::detail;


int add(int a, int b) {
	return a + b;
}

void print(const string& s) {
	printf("%s\n", s.c_str());
}

class Test {
public:
	void setName(const string& n) {
		name_ = n;
	}
	
	string getName() {
		return name_;
	}

private:
	string name_;
};

int main() {
	EventLoop loop;
	InetAddress listenAddr("127.0.0.1", 8888);
	RpcServer server(&loop, listenAddr, 4);

	Test t;
	server.bind("add", add);
	server.bind("print", print);
	server.bind("setName", &Test::setName, &t);
	server.bind("getName", &Test::getName, &t);

	server.start();
	loop.loop();

	return 0;
}