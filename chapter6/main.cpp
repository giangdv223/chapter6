#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#define TIMEOUT 5.0
#define RESPONSE_SIZE 8192

void parse_url(char* url, char** hostname,const char** port, char** path)
{
	char* p;
	p = strstr(url, "://");

	char* protocol = 0;
	if (p)
	{
		protocol = url;
		*p = 0;
		p += 3;
	}
	else
	{
		p = url;
	}

	if (protocol)
	{
		if (strcmp(protocol, "http"))
		{
			std::cout << "Only 'http' is supported." << std::endl;
			exit(1);
		}
	}

	*hostname = p;
	while (*p && *p != ':' && *p != '/' && *p != '80')
		++p;

	char m_port[] = "80";
	*port = "80";
	if (*p == ':')
	{
		*p++ = 0;
		*port = p;
	}
	while (*p && *p != '/' && *p != '#') ++p;

	*path = p;
	if (*p == '/')
	{
		*path = p + 1;
	}
	*p = 0;

	while (*p && *p != '#') ++p;
	if (*p == '#') *p = 0;

	std::cout << url << std::endl;
	printf("hostname: %s\n", *hostname);
	printf("port: %s\n", *port);
	printf("path: %s\n", *path);
}

void send_request(SOCKET s, char* hostname, char* port, char* path)
{
	char buffer[2048];
	sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
	sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
	sprintf(buffer + strlen(buffer), "Connection: close\r\n");
	sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
	sprintf(buffer + strlen(buffer), "\r\n");
	send(s, buffer, sizeof(buffer), 0);
	printf("Send Header: \n%s", buffer);
}

SOCKET connect_to_host(char* hostname, char* port)
{
	std::cout << "Configuring remote address..." << std::endl;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* peer_address;
	if(getaddrinfo(hostname, port, &hints, &peer_address))
	{
	std::cout << "getaddrinfo() failed. " << GETSOCKETERRNO() << std::endl;
	exit(1);
	}

	std::cout << "Remote address is: " << std::endl;
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer),
		service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	std::cout << address_buffer << std::endl << service_buffer << std::endl;
	std::cout << "Creating socket..." << std::endl;
	SOCKET server;
	server = socket(peer_address->ai_family,
		peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISVALIDSOCKET(server)) {
		std::cout << "socket() failed. " << GETSOCKETERRNO() << std::endl;
		exit(1);
	}
	printf("Connecting...\n");
	if (connect(server,
		peer_address->ai_addr, peer_address->ai_addrlen)) {
		std::cout << "Connect() failed. " << GETSOCKETERRNO() << std::endl;
	}
	freeaddrinfo(peer_address);
	std::cout << "Connected. \n\n" << std::endl;
	return server;

}

int main(int argc, char*argv[])
{
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		std::cout << "Failed to initialize." << std::endl;
		return 1;
	}

	if (argc < 2)
	{
		std::cout << "usage: web_get url\n";
		return 1;
	}

	char* url = argv[1];
	char* hostname, * port, * path;
	parse_url(url, &hostname, &port, &path);

	SOCKET server = connect_to_host(hostname, port);
	send_request(server, hostname, port, path);

	const clock_t start_time = clock();
	char response[RESPONSE_SIZE + 1];
	char* p = response, * q;
}