#pragma once

//#include "stdafx.h"
//#include <emil/EMIL.h>
#include <queue>
//#include <io.h>
//#include <string>
//#include <iostream>
//#include <GL/freeglut.h>

#include <iostream>
#include <sstream>
//#include <stdexcept>

using namespace std;

class Pest
{
	/*
	NOTE: the code reads last line of a file, the deletes it


	note that i'm keeping the full stepSize array 
	because i need to check backward for the step
	size prior the last reversal.
	Moreover, in eventual successive implementation 
	of a stopping rule I might need that info too.
	*/

private:
	static int openInst;
	static int savedInst;
	static string label;
	
	int instNo;
	
	double upB, loB;// upper and lower bounds
	double w; // wald's constant
	double p; // target p
	bool isLog;
	bool isFixedStep;	// whether the step size is fixed
	
	int hits, trials; // k and n for binomial
	int revPos;
	
	vector <double> step;

	//count of successive steps
	int stpCnt; 

	//number of steps taken (counter)
	int s;

public:

	Pest(double startingLevel, 
		double stepSize, double waldConstant,bool isLogarithmic);
	Pest(double startingLevel, 
		double stepSize, double waldConstant,
		bool isLogarithmic, bool isFixedStepSize);
	Pest(const char* filename);
	Pest() {};
	~Pest();

	static const char* getLabel();
	int saveInstance(const char* filename);
	bool UpdateLevel(int hit);

};