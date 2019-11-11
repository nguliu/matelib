#include "RpcClient.h"
#include <string>
#include <stdio.h>

using namespace std;
using namespace lfp;

int main() {
	RpcClient client;

	client.connect("127.0.0.1", 8888);

	// 调用自由函数
	auto val = client.call<int>("add", 10, 100);
	if (val.successful())
		printf("call add result: %d\n", val.value());
	else
		printf("call failed: %s\n", val.message().c_str());

	auto val2 = client.call<void>("print", "你好，RpcServer");
	if (val2.successful())
		printf("call print succeed!\n");
	else
		printf("call failed: %s\n", val2.message().c_str());

	
	// 调用类成员函数
	auto val3 = client.call<string>("getName");
	if (val3.successful())
		printf("getName: %s\n", val3.value().c_str());
	else
		printf("call failed: %s\n", val3.message().c_str());

	auto val4 = client.call<void>("setName", "HelloWord");
	if (val4.successful())
		printf("call setName succeed!\n");
	else
		printf("call failed: %s\n", val4.message().c_str());

	auto val5 = client.call<string>("getName");
	if (val5.successful())
		printf("getName: %s\n", val5.value().c_str());
	else
		printf("call failed: %s\n", val5.message().c_str());

	return 0;
}