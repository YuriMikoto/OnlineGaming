#include <winsock2.h>
#include <iostream>
#include "Net.h"

using namespace std;

Net net = Net();

void playerA() {//Receiver.
	net.setupUDP(28000, "127.0.0.1");
	cout << "IP: 127.0.0.1" << endl << "Port: 28000" << endl;
}

void playerB() {//Sender.
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

		while (1) {//A is the receiver.
			net.receiveData("127.0.0.1", 28001, message);
			//cout << message << endl; //Uncomment this to see output. It will do this very rapidly.
		}
	}
	else if (player == 'b') {
		playerB();

		system("PAUSE");
		cout << net.sendData("127.0.0.1", 28000, "test") << endl;
	}

	//net.receiveData("127.0.0.1", 28000, message);

	system("PAUSE");
}

