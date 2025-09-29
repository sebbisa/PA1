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
	int p = -1;
	double t = -1;
	int e = -1;
	bool newChannel = false;
	int bufferSize = MAX_MESSAGE;

	vector<FIFORequestChannel*> channels;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
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
			case 'c':
				newChannel = true;
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


    FIFORequestChannel control_chan("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&control_chan);

	if(newChannel){
		MESSAGE_TYPE newChannelMessage = NEWCHANNEL_MSG;
		control_chan.cwrite(&newChannelMessage, sizeof(MESSAGE_TYPE));

		char channelName[MAX_MESSAGE];
		control_chan.cread(&channelName, MAX_MESSAGE);

		FIFORequestChannel* createdChannel = new FIFORequestChannel(channelName, FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(createdChannel);
	}

	FIFORequestChannel chan = *(channels.back());
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256

	if(p != -1 && t != -1 && e != -1){

		datamsg x(p, t, e);

		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		
	}

	else if(p != -1){

		ofstream newfile;
  		newfile.open("./received/x1.csv");

		for(int i = 0; i < 1000; i++){
			newfile << (i * 0.004);
			double reply;

			datamsg e1(p, i * 0.004, 1);
			memcpy(buf, &e1, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			chan.cread(&reply, sizeof(double)); //answer
			newfile << "," << reply;
			
			datamsg e2(p, i * 0.004, 2);
			memcpy(buf, &e2, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			chan.cread(&reply, sizeof(double)); //answer
			newfile << "," << reply << endl;
			
		}

		newfile.close();
	}

	if(filename != ""){

		FILE* newfile;
		string newname = "./received/" + filename;
		newfile = fopen(newname.c_str(), "wb");

		filemsg fm(0, 0);
		int len = sizeof(filemsg) + (filename.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), filename.c_str());
		chan.cwrite(buf2, len);  // I want the file length;

		int64_t filesize = 0;
		chan.cread(&filesize, sizeof(int64_t));

		char* buf3 = new char[bufferSize + 1];
		filemsg* file_req = (filemsg*) buf2;
		for(file_req->offset = 0; file_req->offset < filesize; file_req->offset += bufferSize){
			file_req->length = min(filesize - file_req->offset, (int64_t) bufferSize);
			chan.cwrite(buf2, len);
			chan.cread(buf3, file_req->length);
			buf3[file_req->length] = '\0';

			fwrite(buf3, sizeof(char), file_req->length, newfile);
		}

		delete[] buf2;
		delete[] buf3;

		fclose(newfile);
	}


	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
	for(auto& c : channels)
		c->cwrite(&m, sizeof(MESSAGE_TYPE));
	
	if(newChannel) delete channels[1];
	
	return 0;
    
}
