// Client side C/C++ program to demonstrate Socket programming
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "wav_file_reader.h"

#define PORT 8080
#define BUFFER_SIZE 3200


int sendAudioStream(const int &sock_fd, std::string filename) {

	WavFileReader m_reader(filename);
	uint8_t *data_buffer = new uint8_t[BUFFER_SIZE];
	int bytes_sent = 0;
	int total_bytes_sent = 0;

	while((bytes_sent = m_reader.Read(data_buffer, BUFFER_SIZE)) != 0) {
		total_bytes_sent += bytes_sent;
		std::cout<<"Sending data stream"<<std::endl;
		send(sock_fd, data_buffer, bytes_sent, 0);
	}

	return total_bytes_sent;
}

int main(int argc, char const *argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout<<"Socket creation error"<<std::endl;
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		std::cout<<"Invalid Address / Address not supported"<<std::endl;
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout<<"Connection failed"<<std::endl;
		return -1;
	}

	std::cout<<"Connection established with STT Service"<<std::endl;
	std::cout<<"Sleeping for 10 seconds"<<std::endl;
	sleep(10);
	std::cout<<"Sending Audio Stream Now"<<std::endl;
	// send(sock , hello , strlen(hello) , 0 );
	// std::cout<<"Message Sent"<<std::endl;

	int bytes_sent = sendAudioStream(sock, "whatstheweatherlike.wav");

	std::cout<<"Sent "<<bytes_sent<<" Bytes"<<std::endl;

	return 0;
}
