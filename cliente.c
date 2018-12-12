/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 2.0
Fecha: 12/2018
Descripción:
	Cliente sencillo TCP.

Autor: Eduardo Fúnez Fernández y Miguel Ángel Moral Pérez

*******************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr *server_in;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	int ipversion=AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16]="127.0.0.1";//loopback IPv4
	char default_ip6[64]="::1";//loopback IPv6
	
	unsigned long ipdestl;
	struct in_addr address;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   
	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets
	
	printf("*******************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n******************\r\n");
	

	do{

		printf("CLIENTE> Que version de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;//eleccion IPv6

		}
		else { //Distinto de 6 se elige la versión 4
			ipversion = AF_INET;//eleccion IPv4
		}

		sockfd=socket(ipversion,SOCK_STREAM,0);
		if(sockfd==INVALID_SOCKET){
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else{
			printf("CLIENTE> Introduzca la IP del dominio (pulsar enter para IP por defecto):\r\n ");
			gets_s(ipdest,sizeof(ipdest));

			//Para el cambio de dominio a dirección IP:
			ipdestl = inet_addr(ipdest);
			if (ipdestl == INADDR_NONE) {
				//Cuando la direccion introducida es un dominio en texto
				struct hostent *host;
				host = gethostbyname(ipdest);//obtiene la dirección ip del dominio
				if (host != NULL) {
					memcpy(&address, host->h_addr_list[0], 4);
					printf("\nDireccion %s\n", inet_ntoa(address));
					strcpy(ipdest, inet_ntoa(address));
				}
			}

			//Dirección por defecto según la familia
			if(strcmp(ipdest,"")==0 && ipversion==AF_INET)
				strcpy_s(ipdest,sizeof(ipdest),default_ip4);

			if(strcmp(ipdest,"")==0 && ipversion==AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest),default_ip6);

			if(ipversion==AF_INET){//IPv4
				memset(&server_in4, 0, sizeof(server_in4));
				server_in4.sin_family=AF_INET;
				server_in4.sin_port=htons(TCP_SERVICE_PORT);
				server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(AF_INET,ipdest,&server_in4.sin_addr.s_addr);
				server_in=(struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}
			if(ipversion==AF_INET6){//IPv6
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family=AF_INET6;
				server_in6.sin6_port=htons(TCP_SERVICE_PORT);
				inet_pton(ipversion,ipdest,&server_in6.sin6_addr);
				server_in=(struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado=S_WELC;

			if(connect(sockfd, server_in, address_size)==0){//Socket
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
				//Inicio de la máquina de estados
				do{
					switch(estado){
					
					case S_WELC:
						//Saludo server.
						break;

					case S_HELO:
						//Establecimiento conexion
						printf("CLIENTE> Introduzca 'helo' (enter para salir): \r\n");
						gets_s(input,sizeof(input));
						if(strlen(input) == 0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							estado=S_QUIT;
						}
						else 
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s",input, CRLF);
							
						break;   


					case S_MAIL:
						//Usuario introduce remitente
						printf("CLIENTE> Introduzca el remitente (enter para salir):\r\n mail from: ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s",input, CRLF);
							estado = S_QUIT;
						}
						else
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MF, input, CRLF);
						
						break;

					case S_RCPT:
						//Usuario introduce destinatario
						printf("CLIENTE> Introduzca el usuario destinatario (enter para salir):\r\n rcpt to: ");
						gets_s(input,sizeof(input));
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",RCPT,input,CRLF);
						  
						break;

					case S_DATA: 
						//Usuario comprueba datos
						printf("CLIENTE> Si los datos estan bien pulse 'S'. En caso contrario ENTER. \r\n");
						gets_s(input, sizeof(input));
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							estado=S_MAIL;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",DATA,CRLF);
						break;

					case S_MENS:
						printf("CLIENTE> Contenido del mensaje 'data'(utilice '.' y enter para enviar):\r\n");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", FMENS, CRLF);
							estado = S_FINM;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
						break;
					
					}

					if(estado!=S_WELC){ // y distinto del data
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
						if(enviados==SOCKET_ERROR){
							 estado=S_QUIT;
							 continue;
						}
					}  
					if (estado != S_MENS) {	// si estado es distinto del data
						recibidos = recv(sockfd, buffer_in, 512, 0);
						if (recibidos <= 0) {
							DWORD error = GetLastError();
							if (recibidos < 0) {
								printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
								estado = S_QUIT;
							}
							else {
								printf("CLIENTE> Conexión con el servidor cerrada\r\n");
								estado = S_QUIT;
							}
						}else {
						
							switch (estado) {
								//Maquina de estados para recibir los códigos del servidor
							case S_WELC:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OK, 3) == 0) {
									estado++;
								}
								else estado = S_WELC;

								break;

							case S_HELO:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OKW, 3) == 0) {
									estado++;
								}
								else estado = S_HELO;

								break;

							case S_MAIL:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OKW, 3) == 0)
								{
									estado++;
								}
								else estado = S_MAIL;

								break;
							case S_RCPT:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OKW, 3) == 0)
								{
									estado++;
								}
								else estado = S_RCPT;


								break;
							case S_DATA:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OKDATA, 3) == 0)
								{
									estado++;
								}
								else estado = S_MENS;
								break;

							case S_FINM:
								buffer_in[recibidos] = 0x00;
								printf(buffer_in);
								if (strncmp(buffer_in, OKW, 3) == 0)
								{
									estado = S_MAIL;
								}
								break;
							}
                        }
						// fin maquina de estados
						
					}

				}while(estado!=S_QUIT);		
			}
			else{
				int error_code=GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	WSACleanup();
	return(0);
}
