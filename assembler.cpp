/**
 *Assembler for our own Assembly Language
 */

#include<cstdio>
#include<fstream>

#define INPUT_WIDTH 50					//Specifies Max length for an instruction
#define INPUT_HEIGHT 10000				//Specifies Max number of instructions

using namespace std;




/**
 *Accepting command line arguments for input and output filename
 */
int main(int argc, char const *argv[])
{
	ifstream fileIn;
	ofstream fileOut;
	int inputNumberOfLines,i,j;
	char input[INPUT_HEIGHT][INPUT_WIDTH];
	char const *inputFileName,*outputFileName;
	if(argc !=3)
	{
		printf("Usage: %s input_file output_file\n",argv[0]);
		return 0;
	}

	inputFileName = argv[1];
	outputFileName = argv[2];

	fileIn.open(inputFileName,ios::in);
	if(!fileIn)
	{
		printf("Input file not found !!\n");
		return 1;
	}

	inputNumberOfLines=0;
	while(1)
	{
		fileIn.getline(input[inputNumberOfLines],INPUT_WIDTH,'\n');
		if(fileIn.eof())
			break;
		inputNumberOfLines++;
	}

	fileIn.close();
	fileOut.open(outputFileName,ios::out);		//WARNING : This will destroy the previous contents of the file
	for(i=0;i<inputNumberOfLines;i++)
	{
		for(j=0;input[i][j] !='\0';j++)
			fileOut<<input[i][j];
		fileOut<<endl;
	}
	fileOut.close();
	return 0;
}