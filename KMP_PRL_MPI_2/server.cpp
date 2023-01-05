// Server side C/C++ program to demonstrate Socket
// programming


#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector> 
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>
#include "lines.h"
#include <pthread.h>
#include <mpi.h>
#include <math.h>




#define PORT 8080



void computeLPSArray(char* pat, int M, int* lps);
using namespace std;


// Prints occurrences of txt[] in pat[]
void KMPSearch(char* pat, char* txt,char* result)
{


	int M = strlen(pat);
	int N = strlen(txt);



	// create lps[] that will hold the longest prefix suffix
	// values for pattern
	int lps[M];

	// Preprocess the pattern (calculate lps[] array)
	computeLPSArray(pat, M, lps);

	int i = 0; // index for txt[] 
	int j = 0; // index for pat[]
	while ((N - i) >= (M - j)) {
		if (pat[j] == txt[i]) {
			j++;
			i++;
		}

		if (j == M) {
			char p[100];
			int k = i-j;
			sprintf(p,"%s Found pattern at index %d\n", pat,k);			
			if (sizeof(result) == 0){
				sprintf(result,p);
			}
			else{
				strcat(result,p);
			}
			j = lps[j - 1];
			
		}

		// mismatch after j matches
		else if (i < N && pat[j] != txt[i]) {
			// Do not match lps[0..lps[j-1]] characters,
			// they will match anyway
			if (j != 0)
				j = lps[j - 1];
			else
				i = i + 1;
		}
	}





}

// Fills lps[] for given pattern pat[0..M-1]
void computeLPSArray(char* pat, int M, int* lps)
{

	
	// length of the previous longest prefix suffix
	int len = 0;

	lps[0] = 0; // lps[0] is always 0

	// the loop calculates lps[i] for i = 1 to M-1
	int i = 1;
	while (i < M) {
		if (pat[i] == pat[len]) {
			len++;
			lps[i] = len;
			i++;
		}
		else // (pat[i] != pat[len])
		{
			// This is tricky. Consider the example.
			// AAACAAAA and i = 7. The idea is similar
			// to search step.
			if (len != 0) {
				len = lps[len - 1];

				// Also, note that we do not increment
				// i here
			}
			else // if (len == 0)
			{
				lps[i] = 0;
				i++;
			}
		}
	}
}


void foo() 
{
  system("./client");
}


int main(int argc, char* argv[]) 
{



	int myid,numprocs;

	//MPI_Finalize();

	MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD ,&numprocs); 

	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	char buf[1024]; //window buffer
    //MPI_Win window;

    //comm group contains all proccesses
    MPI_Group comm_group;

    MPI_Comm_group(MPI_COMM_WORLD,&comm_group);

    //printf("Number of proccesses: %d\n",numprocs);
    



	//measure server time execution
	
	auto start = std::chrono::high_resolution_clock::now();
	
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

  	vector<string> strings;

	std::ifstream file("tests/S4.txt");
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			// using printf() in all tests for consistency
			strings.push_back(line.c_str());
		}
    file.close();
}


	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	int n ;

	//MPI_Win_create(buf,1024*numprocs,1024,MPI_INFO_NULL,MPI_COMM_WORLD,&window);
	
	while(1){


	printf("MY ID: %d\n",myid);
	char sharedBuffer[1024]; //shared buffer between processes
	char buffer[1024]; 

	
	if ((new_socket = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen))< 0) {
		printf("accept: %d",new_socket);
		exit(EXIT_FAILURE);
	}

	
	//obtenemos texto
	if ((readLine(new_socket, buffer, 1024)==-1)){printf("Error en el servidor");break;}
	printf("TEXT TO ANALIZE:%s\n", buffer);

	char textToAnalize[1024];
	sprintf(textToAnalize,buffer); //we pass the word to a new variable

		

	if (strcmp(textToAnalize,"FINISH") == 0){
		printf("FINISH\n");
		break;
	}
	

	char** result = new char*[strings.size()];
	auto size = strings.size();
	int stringSize = static_cast<int>(size);

	

	n = stringSize; //number of iterations for each proccess

	
	char bufferProprio[1024];

	//printf("Number of iterations: %d\n",n);

	
		
		
	for (int i=0; i< n; i++){
		result[i] = const_cast<char*>(strings[i].c_str());
		//printf("Pattern: %s\n",result[i]);
		KMPSearch(result[i],textToAnalize,bufferProprio);
		//printf("Result of search: %s\n",bufferProprio);
		strcat(sharedBuffer,bufferProprio);
		memset(bufferProprio,0,1024);

	}

	printf("FOUND PATTERNS:\n%s\n",sharedBuffer);

	if (strcmp(sharedBuffer,"") == 0){
		sprintf(sharedBuffer, "No matches, pattern not found");
	} 
	sprintf(buffer,sharedBuffer);
	

	if ((sendMessage(new_socket, buffer, strlen(buffer)+1) == -1)){printf("Error en envío\n");break;}  
	// Use the result.


	//flush buffer
	memset(sharedBuffer,0,1024);
	memset(buffer,0,1024);
	//printf("Index message sent\n");

	}

	close(new_socket);

	
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);

	//end of time measurement
	auto stop = std::chrono::high_resolution_clock::now();
	
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	cout << "Time taken by function in server: " << duration.count()/1000000.0 << " second" << endl;
	
	
	
	MPI_Finalize();
	return 0;
	


    }
	

	

