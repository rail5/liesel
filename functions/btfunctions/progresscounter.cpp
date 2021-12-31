using namespace std;

void progresscounter(int prog, int stage, int numstages, int segcount, int thisseg) {
	int progcounter = (prog / numstages) + ((stage - 1) * (100 / numstages)); // calculate initial value
	
	progcounter = (progcounter / segcount) + ((thisseg - 1) * (100 / segcount)); // divide it based on how many segments we're printing
	
	bool subtractor = progcounter;
	
			/*
			We always want to subtract '1' from the progress counter to accommodate the time it takes to write to a file
			ie, 25% in processing, but we're writing 4 segments, so '25%' should be reserved for when that file
			is done being written, so we decrement to '24%.'
			
			However, if the progress counter is currently at '0%', subtracting '1' can cause errors if the result ('-1%')
			is later processed by another program (for example, the BookThief GUI, which updates a graphical progress bar
			from this output)
			
			So, we feed the progress counter into a boolean type
			
			If you feed any number greater than 0 into a boolean type, it returns 1
			If you feed 0 into a boolean type, it returns 0
			
			We then subtract the result from the progress counter (ie, 0 minus 0 or 1,2,3,etc minus 1)
			*/
	
	cout << progcounter-subtractor << "%" << endl;
}
