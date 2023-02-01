#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <openssl/aes.h>
#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

struct terminal
{
	int id;
	string name;
	int socket;
	thread th;
};

vector<terminal> clients;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0;
mutex cout_mtx, clients_mtx;

string color(int code);
void set_name(int id, char name[]);
void shared_print(string str, bool endLine);
int broadcast_message(string message, int sender_id);
int broadcast_message(int num, int sender_id);
void end_connection(int id);
void handle_client(int client_socket, int id);

int main()
{
	int server_socket;
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 0);

	if ((bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
	{
		perror("bind error: ");
		exit(-1);
	}

	if ((listen(server_socket, 8)) == -1)
	{
		perror("listen error: ");
		exit(-1);
	}

	struct sockaddr_in client;
	int client_socket;
	unsigned int len = sizeof(sockaddr_in);

	cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
		 << def_col;

	while (1)
	{
		if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
		{
			perror("accept error: ");
			exit(-1);
		}
		seed++;
		thread t(handle_client, client_socket, seed);
		lock_guard<mutex> guard(clients_mtx);
		clients.push_back({seed, string("Anonymous"), client_socket, (move(t))});
	}

	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].th.joinable())
			clients[i].th.join();
	}

	close(server_socket);
	return 0;
}

string color(int code)
{
	return colors[code % NUM_COLORS];
}

// Set name of client
void set_name(int id, char name[])
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].id == id)
		{
			clients[i].name = string(name);
		}
	}
}

// For synchronisation of cout statements
void shared_print(string str, bool endLine = true)
{
	lock_guard<mutex> guard(cout_mtx);
	cout << str;
	if (endLine)
		cout << endl;
}

string encrypt_message(string message)
{
	char ciphertext[MAX_LEN];
	static const unsigned char key[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	const unsigned char ivec[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
			0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	AES_KEY enc_key;

	AES_set_encrypt_key(key, 128, &enc_key);
	// AES_encrypt((unsigned char *)message.c_str(),(unsigned char *)ciphertext,&enc_key);
	AES_cbc_encrypt((unsigned char *)message.c_str(), (unsigned char *)ciphertext, strlen(message.c_str()), &enc_key, (unsigned char *)ivec, AES_ENCRYPT);

	return ciphertext;
}

// Broadcast message to all clients except the sender
int broadcast_message(string message, int sender_id)
{
	char temp[MAX_LEN];
	strcpy(temp, message.c_str());
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].id != sender_id)
		{
			send(clients[i].socket, temp, sizeof(temp), 0);
		}
	}
}

int first_message(string message, int sender_id, string f_name)
{
	char temp[MAX_LEN];
	strcpy(temp, message.c_str());
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].name == f_name)
		{
			send(clients[i].socket, temp, sizeof(temp), 0);
		}
	}
}

// Send num to Friend
int first_message(int num, int sender_id, string f_name)
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].name == f_name)
		{
			send(clients[i].socket, &num, sizeof(num), 0);
		}
	}
}

// Send message to Friend
int send_message(string message, int sender_id, string f_name)
{
	char temp[MAX_LEN];
	strcpy(temp, message.c_str());
	bool found = false;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].name == f_name)
		{
			found = true;
			send(clients[i].socket, temp, sizeof(temp), 0);
		}
	}

	// friend is not online
	if (found == false)
	{
		string message = "Friend is not online!!";
		char temp[MAX_LEN];
		strcpy(temp, message.c_str());
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i].id == sender_id)
			{
				string msg = "#NULL";
				char tmp[MAX_LEN];
				strcpy(tmp, msg.c_str());
				found = true;
				send(clients[i].socket, tmp, sizeof(tmp), 0);
				send(clients[i].socket, &sender_id, sizeof(sender_id), 0);
				string x = encrypt_message(temp);
				send(clients[i].socket, x.c_str(), sizeof(x), 0);
			}
		}
	}
}

// Send num to Friend
int send_message(int num, int sender_id, string f_name)
{
	bool found = false;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].name == f_name)
		{
			found = true;
			send(clients[i].socket, &num, sizeof(num), 0);
		}
	}
	// friend is not online
	if (found == false)
	{
		string message = "Friend is not online!!";
		char temp[MAX_LEN];
		strcpy(temp, message.c_str());
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i].id == sender_id)
			{
				string msg = "#NULL";
				char tmp[MAX_LEN];
				strcpy(tmp, msg.c_str());
				found = true;
				send(clients[i].socket, tmp, sizeof(tmp), 0);
				send(clients[i].socket, &sender_id, sizeof(sender_id), 0);
				string x = encrypt_message(temp);
				send(clients[i].socket, x.c_str(), sizeof(x), 0);
			}
		}
	}
}

// Broadcast a number to all clients except the sender
int broadcast_message(int num, int sender_id)
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].id != sender_id)
		{
			send(clients[i].socket, &num, sizeof(num), 0);
		}
	}
}

void end_connection(int id)
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].id == id)
		{
			lock_guard<mutex> guard(clients_mtx);
			clients[i].th.detach();
			clients.erase(clients.begin() + i);
			close(clients[i].socket);
			break;
		}
	}
}

void handle_client(int client_socket, int id)
{
	char name[MAX_LEN], str[MAX_LEN], f_name[MAX_LEN];
	recv(client_socket, name, sizeof(name), 0);
	set_name(id, name);
	recv(client_socket, f_name, sizeof(f_name), 0);

	// Display welcome message
	string welcome_message = string(name) + string(" has joined");
	// broadcast_message("#NULL", id);
	// broadcast_message(id, id);
	// broadcast_message(welcome_message, id);
	first_message("#NULL", id, f_name);
	first_message(id, id, f_name);
	first_message(welcome_message, id, f_name);
	shared_print(color(id) + welcome_message + def_col);

	bool found = false;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].name == f_name)
		{
			found = true;
		}
	}

	if (found == false)
	{
		string n = f_name;
		string message = "Friend is not online!!";
		char temp[MAX_LEN];
		strcpy(temp, message.c_str());
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i].id == id)
			{
				send_message("#NULL", id, clients[i].name);
				send_message(id, id, clients[i].name);
				string tmp = encrypt_message(temp);
				send_message(tmp, id, clients[i].name);
				// shared_print(color(id) + welcome_message + def_col);
			}
		}
	}

	while (1)
	{
		int bytes_received = recv(client_socket, str, sizeof(str), 0);
		if (bytes_received <= 0)
			return;
		if (strcmp(str, "#exit") == 0)
		{
			// Display leaving message
			string message = string(name) + string(" has left");
			// broadcast_message("#NULL", id);
			// broadcast_message(id, id);
			// broadcast_message(message, id);
			send_message("#NULL", id, f_name);
			send_message(id, id, f_name);

			// encrypting the meaasge
			send_message(message, id, f_name);
			shared_print(color(id) + message + def_col);
			end_connection(id);
			return;
		}

		// chat messages
		//  broadcast_message(string(name), id);
		//  broadcast_message(id, id);
		//  broadcast_message(string(str), id);
		send_message(string(name), id, f_name);
		first_message(id, id, f_name);
		first_message(string(str), id, f_name);
		shared_print(color(id) + name + " : " + def_col + str);
	}
}
