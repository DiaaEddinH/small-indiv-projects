// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <vector>
#include <mpi.h>
using namespace std;

#define PI 3.1415926535

#define  NX		64
#define  NY		64
#define  STEPS  10
#define  MAX	4
#define  MIN	1
#define  BEGIN  1
#define  LTAG	2
#define  RTAG	3
#define  NONE	0
#define  MASTER 0
#define	 DONE	4
#define  comm	MPI_COMM_WORLD

void wdata(float* u, string filename)
{
	ofstream file(filename);
	int ix, iy;

	if (file.is_open())
	{
		for (iy = 0; iy < NY; iy++)
			for (ix = 0; ix < NX; ix++)
			{
				file << *(u + ix * NY + iy);
				if (ix != NX - 1)
					file << ", ";
				else file << endl;
			}
		file.close();
	}
	else
	{
		cout << "Failure to open file!\nExiting...";
		exit(1);
	}
}

void rdata(float* u, string filename)
{
	ifstream file(filename);
	int ix, iy;
	float in;
	string line;
	vector<float> vec;

	if (file.is_open())
	{
		while (getline(file, line, ','))
		{
			stringstream lineStream(line);
			while (lineStream >> in)
				vec.push_back(in);
		}

		for (ix = 0; ix < NX; ix++)
			for (iy = 0; iy < NY; iy++)
				* (u + ix * NY + iy) = vec.at(ix * NY + iy);

		file.close();
	}
	else {
		cout << "Failure to open file!\nExiting...";
		exit(1);
	}
}


void initGrid(float* u)
{
	int ix, iy;

	for (ix = 0;ix < NX;ix++)
		for (iy = 0;iy < NY;iy++)
			* (u + ix * NY + iy) = float(sin(2 * PI * ix / (NX-1)) * sin(2 * PI * iy / (NY-1)));
}


void update(int begin, int end, float* u1, float* u2)
{
	int ix, iy;
	float cx = 0.1;
	float cy = 0.1;

	for (ix = begin; ix <= end; ix++)
		for (iy = 1; iy < NY - 1; iy++)
			* (u2 + ix * NY + iy) = *(u1 + ix * NY + iy)
			+ cx * (*(u1 + (ix + 1) * NY + iy) + *(u1 + (ix - 1) * NY + iy) - 2 * *(u1 + ix * NY + iy))
			+ cy * (*(u1 + ix * NY + iy + 1) + *(u1 + ix * NY + iy - 1) - 2 * *(u1 + ix * NY + iy));
}

int main(int argc, char** argv)
{	
	float u[2][NX][NY];
	int taskid, numtasks, numworkers; //Task variables
	int	averow, rows, offset, extra;  //Splitting grid
	int dest, source, left, right;	  //Comm between neighbours
	int	begin, end, abort{};		  //Auxiliary variables
	int	msgtype;					  //Tag for Send/Receive
	int	i, ix, iy, it;				  //Loop variables
	MPI_Status status;

	//Initialize
	MPI_Init(&argc, &argv);
	MPI_Comm_size(comm, &numtasks);
	MPI_Comm_rank(comm, &taskid);
	numworkers = numtasks - 1;

	//Master code
	if (taskid == MASTER)
	{
		if ((numworkers > MAX) || (numworkers < MIN))
		{
			cout << "Error: Allowed number of tasks is between " << (MIN+1); 
			cout << " and " << (MAX+1) << " .\nExiting..." << endl;
			MPI_Abort(comm, abort);
			exit(1);
		}
		//Initialize grid with a function u_0
		initGrid(&u[0][0][0]);
		
		averow = NX / numworkers;
		extra = NX % numworkers;
		offset = 0;
		
		for (i = 1; i <= numworkers; i++)
		{
			//In case grid cannot be split up evenly among tasks
			rows = (i <= extra) ? averow + 1 : averow;

			//Identify you left/right neighbours
			if (i == 1)
				left = NONE;
			else
				left = i - 1;
			if (i == numworkers)
				right = NONE;
			else
				right = i + 1;

			//Send info to each task
			dest = i;
			MPI_Send(&offset, 1, MPI_INT, dest, BEGIN, comm);
			MPI_Send(&rows, 1, MPI_INT, dest, BEGIN, comm);
			MPI_Send(&left, 1, MPI_INT, dest, BEGIN, comm);
			MPI_Send(&right, 1, MPI_INT, dest, BEGIN, comm);
			MPI_Send(&u[0][offset][0], rows*NY, MPI_FLOAT, dest, BEGIN, comm);

			cout << "Sending to task " << dest << ": rows = " << rows << ", offset = " << offset << endl;
			cout << "Left: " << left << " | Right: " << right << endl;

			offset += rows;
		}

		//Receive info from tasks
		for (i = 1; i <= numworkers;i++)
		{
			source = i;
			msgtype = DONE;
			cout << "Passes 1" << endl;
			MPI_Recv(&offset, 1, MPI_INT, source, msgtype, comm, &status);
			cout << "Passes 2" << endl;
			MPI_Recv(&rows, 1, MPI_INT, source, msgtype, comm, &status);
			cout << "Passes 3" << endl;
			MPI_Recv(&u[0][offset][0], rows*NY, MPI_FLOAT, source, msgtype, comm, &status);
			cout << "Passes 4" << endl;
		}

		wdata(&u[0][0][0], "final.dat");
		cout << "Writing data on \"final.dat\" and exiting..." << endl;
		MPI_Finalize();
	}

	// Worker code
	if (taskid != MASTER)
	{
		for (int iz = 0;iz < 2; iz++)
			for (ix = 0;ix < NX; ix++)
				for (iy = 0;iy < NY; iy++)
					u[iz][ix][iy] = 0.0;

		//Receive initial info from main task
		source = MASTER;
		msgtype = BEGIN;
		MPI_Recv(&offset, 1, MPI_INT, source, msgtype, comm, &status);
		MPI_Recv(&rows, 1, MPI_INT, source, msgtype, comm, &status);
		MPI_Recv(&right, 1, MPI_INT, source, msgtype, comm, &status);
		MPI_Recv(&left, 1, MPI_INT, source, msgtype, comm, &status);
		MPI_Recv(&u[0][offset][0], rows * NY, MPI_FLOAT, source, msgtype, comm, &status);

		begin = offset;
		end = offset + rows - 1;

		if (offset == 0)
			begin = 1;
		if ((offset + rows) == NX)
			end--;

		cout << "Task " << taskid << ", begin = " << begin << ", end = " << end << endl;
		cout << "Task " << taskid << " received and begins work..." << endl;

		int c = 0;
		for (it = 0; it < STEPS; it++)
		{
			if (left != NONE)
			{
				MPI_Send(&u[c][offset][0], NY, MPI_FLOAT, left, RTAG, comm);
				source = left;
				msgtype = LTAG;
				MPI_Recv(&u[c][offset - 1][0], NY, MPI_FLOAT, source, msgtype, comm, &status);
			}

			if (left != NONE)
			{
				MPI_Send(&u[c][offset + rows - 1][0], NY, MPI_FLOAT, right, LTAG, comm);
				source = right;
				msgtype = RTAG;
				MPI_Recv(&u[c][offset + rows][0], NY, MPI_FLOAT, source, msgtype, comm, &status);
			}

			update(begin, end, &u[c][0][0], &u[1-c][0][0]);
			c = 1 - c;
		}

		MPI_Send(&offset, 1, MPI_INT, MASTER, DONE, comm);
		MPI_Send(&rows, 1, MPI_INT, MASTER, DONE, comm);
		MPI_Send(&u[c][offset][0], rows*NY, MPI_FLOAT, MASTER, DONE, comm);
		cout << "Sending results to MASTER..." << endl;
		MPI_Finalize();
	}
	return 0;
}

