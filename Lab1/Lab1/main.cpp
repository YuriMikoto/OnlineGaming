#include <winsock2.h>
#include <iostream>
#include "Net.h"

using namespace std;

Net net = Net();

void playerA() {
	net.setupUDP(28000, "127.0.0.1");
	cout << "IP: 127.0.0.1" << endl << "Port: 28000" << endl;
}

void playerB() {
	net.setupUDP(28001, "127.0.0.1");
	cout << "IP: 127.0.0.1" << endl << "Port: 28001" << endl;
}

int main() {

	net.initialise();

	char player;
	char message[200] = "";

	std::cin >> player;

	if (player == 'a') {
		playerA();
	}
	else if (player == 'b') {
		playerB();
	}

	system("PAUSE");

	cout << net.sendData("127.0.0.1", 28001, "test") << endl;

	net.receiveData(message);

	cout << message << endl;

	system("PAUSE");
}

