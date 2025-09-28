/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Sebastian Silva
	UIN: 434005269
	Date: 9/23/25
*/
#include <stdio.h>
#include <sys/wait.h> 
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	bool newChannel = false;
	int bufferSize = MAX_MESSAGE;

	vector<FIFORequestChannel*> channels;

	
	string filename = "";
	while ((opt = getopt(argc, argv, "c:p:t:e:f:m:")) != -1) {
		switch (opt) {
			case 'c':
				newChannel = true;
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				bufferSize = atoi (optarg);
				break;
		}
	}

	if(fork() == 0){
		char digitBuffer[20];
		sprintf(digitBuffer, "%d", bufferSize);
		char serverExe[20], flag[20];
		strcpy(serverExe, "./server");
		strcpy(flag, "-m");

		char* server_args[] = {serverExe, flag, digitBuffer, NULL}; 
		execvp("./server", server_args);
		return 1;
	}

	wait(nullptr);

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&chan);

	if(newChannel){
		MESSAGE_TYPE newChannelMessage = NEWCHANNEL_MSG;
		chan.cwrite(&newChannelMessage, sizeof(MESSAGE_TYPE));
	}
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256
    datamsg x(p, t, e);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	

	/*
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;

	*/
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
