#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#include "rc4.h"

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);

int main()
{
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_port = htons(10000); // Port no. of server
	client.sin_addr.s_addr = INADDR_ANY;
	// client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Provide IP address of server
	bzero(&client.sin_zero, 0);

	if ((connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in))) == -1)
	{
		perror("connect: ");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c);
	char name[MAX_LEN];
	cout << "Enter your name : ";
	cin.getline(name, MAX_LEN);
	send(client_socket, name, sizeof(name), 0);

	char f_name[MAX_LEN];
	cout << "Enter your friend's name : ";
	cin.getline(f_name, MAX_LEN);
	send(client_socket, f_name, sizeof(f_name), 0);

	// static const unsigned char key[] = {
	// 	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	// 	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	// 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	// 	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	string key = "secret key";
	cout<< "key is:" << key<< endl;

	cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
		 << def_col;

	thread t1(send_message, client_socket);
	thread t2(recv_message, client_socket);

	t_send = move(t1);
	t_recv = move(t2);

	if (t_send.joinable())
		t_send.join();
	if (t_recv.joinable())
		t_recv.join();

	return 0;
}

// Handler for "Ctrl + C"
void catch_ctrl_c(int signal)
{
	char str[MAX_LEN] = "#exit";
	send(client_socket, str, sizeof(str), 0);
	exit_flag = true;
	t_send.detach();
	t_recv.detach();
	close(client_socket);
	exit(signal);
}

string color(int code)
{
	return colors[code % NUM_COLORS];
}

// Erase text from terminal
int eraseText(int cnt)
{
	char back_space = 8;
	for (int i = 0; i < cnt; i++)
	{
		cout << back_space;
	}
}

// Send message to everyone
void send_message(int client_socket)
{
	while (1)
	{
		cout << colors[1] << "You : " << def_col;
		char str[MAX_LEN];
		cin.getline(str, MAX_LEN);

		// RC4 Encryption
		string key = "secret key";
		string ciphertext = encrypt(key, str);

		send(client_socket, ciphertext.c_str(), sizeof(ciphertext), 0);
		if (strcmp(str, "#exit") == 0)
		{
			exit_flag = true;
			t_recv.detach();
			close(client_socket);
			return;
		}
	}
}

// Receive message
void recv_message(int client_socket)
{
	while (1)
	{
		if (exit_flag)
			return;
		char name[MAX_LEN], str[MAX_LEN];
		int color_code;
		int bytes_received = recv(client_socket, name, sizeof(name), 0);
		if (bytes_received <= 0)
			continue;
		recv(client_socket, &color_code, sizeof(color_code), 0);
		recv(client_socket, str, sizeof(str), 0);
		//string ciph(str);

		// RC4 decryption
		string key = "secret key";
		string plaintext = decrypt(key, str, strlen(str));
		string cipher(str);

		eraseText(6);
		if (strcmp(name, "#NULL") != 0)
		{

			cout << color(color_code) << name << " : " << def_col << str << endl;
			// // Convert the ciphertext string to a vector of uint8_t
			// vector<uint8_t> ciphertextVec(cipher.begin(), cipher.end());

			// // Print the ciphertext in hexadecimal format
			// cout << color(color_code) << name << " : " << def_col;
			// for (int i = 0; i < ciphertextVec.size(); i++)
			// {
			// 	printf("%02x", ciphertextVec[i]);
			// }
			// cout << endl;

			cout << color(color_code) << name << " : " << def_col << plaintext << endl;
		}
		else
		{
			cout << color(color_code) << str << endl;
			// cout << color(color_code);

			// // Convert the ciphertext string to a vector of uint8_t
			// vector<uint8_t> ciphertextVec(cipher.begin(), cipher.end());

			// // Print the ciphertext in hexadecimal format
			// for (int i = 0; i < ciphertextVec.size(); i++)
			// {
			// 	printf("%02x", ciphertextVec[i]);
			// }
			// cout << endl;


			// for (int i = 0; i < strlen((char *)str); i++)
			// {
			//     std::cout << std::hex << (int)str[i];
			// }

			cout << color(color_code) << plaintext << endl;
		}
		cout << colors[1] << "You : " << def_col;
		fflush(stdout);
	}
}
