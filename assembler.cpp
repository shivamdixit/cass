/**
 ************************************************************************
 *						CASS : An Open Source Assembler in C++		v0.1*
 ************************************************************************
 *					  **LICENSED UNDER GNU GENERAL PUBLIC LICENSE**
 *
 *@description CASS is an open source assembler written in C++ for our
 *				ISA which is designed during a project in COA Course
 *@authors 	Shivam Dixit, Ritesh Agrawal, Prasham Gupta, Parag Jain
 *			Anyone is free to contribute to this project
 *
 *
 ************************************************************************
 */


#include<cstdio>
#include<fstream>
#include<cctype>
#include<string>
#include<cstring>
#include<cstdlib>
#include<iomanip>
#include<cstdlib>

/**
 *Macros
 */
#define INPUT_WIDTH 50					//Specifies Max length for an instruction
#define INPUT_HEIGHT 10000				//Specifies Max number of instructions
#define SYMB_TAB_SIZE 1000 				//Specifies size of Symbol Table
#define LABEL_SIZE 15 					//Specifies max-size of a Label
#define MNEUMONIC_SIZE 5 				//Specifies max-size of a Mneumonic
#define NUMBER_OF_REG 28				//Specifies total number of Registers

using namespace std;


/**
 *Global Variables
 */
int currentIndex=0,currentRow=0,instructionLocationCounter=0,symbTableCount=0;
char sourceProgram[INPUT_HEIGHT][INPUT_WIDTH];		//Array to store source
bool isEnd;					//To check if End Of File is reached
int baseAddress=0;			//Base Address of the program after loading into memory


/**
 *Function declarations
 */
void parse(ofstream &);
void eatWhiteSpace(void);
void labelScan(ofstream &,bool);
char * getLabelName();
char * getMemory();
void insertInSymbolTable(char * );
void readMneumonic(ofstream &,bool );
void mneumonicCompare(ofstream &, char * , bool );
void interpretLDR(ofstream &, bool );
void interpretSTR(ofstream & ,bool);
void interpretMAI(ofstream & ,bool );
void interpretJZR(ofstream & ,bool );
void interpretJUM(ofstream &, bool );
void interpretJMC(ofstream &, bool );
void interpretJMZ(ofstream &, bool );
void interpretJMP(ofstream &, bool );
void interpretMVR(ofstream &, bool );
void interpretADD(ofstream &, bool );
void interpretSUB(ofstream &, bool );
void interpretMUL(ofstream &, bool );
void interpretDIV(ofstream &, bool );
void interpretMOD(ofstream &, bool );
void interpretSTI(ofstream &, bool );
void interpretNOT(ofstream &, bool );
void interpretMOI(ofstream &, bool );
void interpretINC(ofstream &, bool );
void interpretDEC(ofstream &, bool );
void interpretLOP(ofstream &, bool );
void interpretELP(ofstream &, bool );
void interpretHLT(ofstream &, bool );
void interpretNOP(ofstream &, bool );
void dataToBinary(ofstream & ,char * );
void regToBinary(ofstream &, char * );
void hexToBinary(ofstream &,char * );
int searchSymbolTable(char * );
unsigned long long int decToBinary(int );


/**
 *Structure to combine Label and ILC count to store in symbol table
 *@char Label Name
 *@int Instruction Location Counter Value
 */
struct symbol {
	char  label[LABEL_SIZE];
	int ILC;
};

typedef struct symbol symbol;

symbol symbolTable[SYMB_TAB_SIZE];			//Global array to store symbol table


/**
 *Accepting command line arguments for input and output filename
 */
int main(int argc, char const *argv[])
{
	int inputNumberOfLines,i,j;
	char const *inputFileName,*outputFileName;
	ifstream fileIn;
	ofstream fileOut;

	if(argc !=3)
	{
		printf("Usage: %s input_file output_file\n",argv[0]);
		return 0;
	}

	inputFileName = argv[1];
	outputFileName = argv[2];

	//Label wali line cannot contain any other instruction

	fileIn.open(inputFileName,ios::in);
	if(!fileIn)
	{
		fprintf(stderr,"Input file not found !!\n");
		return 1;
	}

	inputNumberOfLines=0;
	while(1)
	{
		fileIn.ignore();
		fileIn.getline(sourceProgram[inputNumberOfLines],INPUT_WIDTH,'\r');
		fflush(stdout);
		fflush(stdin);
		if(fileIn.eof())
			break;
		inputNumberOfLines++;
	}

	fileIn.close();
	fileOut.open(outputFileName,ios::out);		//WARNING : This will destroy the previous contents of the file
	parse(fileOut);
	return 0;
}


/**
 *Function to parse the input file in two passes
 *@param 	ofstream& fileOut				//Output File stream
 *@return void
 */
void parse(ofstream & fileOut)
{
	currentRow = 0;
	currentIndex =0;
	//First Pass
	while(!isEnd)
		labelScan(fileOut,true);			//Just create symbol table

	currentRow =0;						//Reverting all the counters to zero
	currentIndex =0;
	isEnd = false;
	instructionLocationCounter = 0;

	//Second Pass Pass
	while(!isEnd)
		labelScan(fileOut,false);			//Write output to the file
}



/**
 *Function to skip all whitespaces
 *
 */
void eatWhiteSpace(void)
{
	while(sourceProgram[currentRow][currentIndex] == ' ' || sourceProgram[currentRow][currentIndex] == '\t')
		currentIndex++;
}



/**
 *Function to scan input and detect if it is label or mnemonic
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void labelScan(ofstream & fileOut,bool isFirstPass)
{
	if(sourceProgram[currentRow][currentIndex] != ' ') //Label will not contain any space at the beginning
	{
		//Code to generate symbol table
		if(isFirstPass)
		{
			insertInSymbolTable(getLabelName());
		}
		currentRow++;
		currentIndex=0;
		return;
	}
	eatWhiteSpace();								//Mneumonic will always start with alteast 1 space
	readMneumonic(fileOut,isFirstPass);
}



/**
 *Function to get Name of Label
 *@return char * 		//Pointer to Name of Label
 */
char * getLabelName()
{
	return sourceProgram[currentRow];
}



/**
 *Function to insert Label into Symbol Tabel
 *@param 	char* Name				//Name of Label To be inserted
 *@return void
 */
void insertInSymbolTable(char * name)
{
	int i;
	static int index =0;			//To keep an count of index of array "searchSymbolTable"

	if(searchSymbolTable(name) != -1)  				//If Label already exists in symbol table
	{
		fprintf(stderr, "cass: Error at line number: %d\n Label Already used\n",currentRow+1);
		exit(1);
	}

	symbolTable[index].ILC = instructionLocationCounter;	//Using Global ILC
	for(i=0;name[i] !=':';i++)		//Copying Label Name
	{
		symbolTable[index].label[i] =name[i];
	}
	symbolTable[index].label[i] = '\0'; //Inserting null char at the end
	index++;
	symbTableCount = index;		//Global vairable symbTableCount to keep a count of total number of sym
								//in symbol table
}



/**
 *Function to read a mneumonic
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void readMneumonic(ofstream & fileOut,bool isFirstPass)
{
	int i=0;
	char mneumonic[MNEUMONIC_SIZE];
	while(1)
	{
		if(sourceProgram[currentRow][currentIndex] == ' ' || sourceProgram[currentRow][currentIndex] == '\0')
			break;

		mneumonic[i++] = toupper(sourceProgram[currentRow][currentIndex]);
		currentIndex++;
	}
	mneumonic[i] = '\0';			//Storing mneumonic in array "mneumonic"
	//Code to compare mnemnonic
	mneumonicCompare(fileOut,mneumonic,isFirstPass);	//Function to compare given mneumonic
	currentRow++;
	currentIndex=0;
}


/**
 *Function to scan input and detect if it is label or mnemonic
 *@param 	ofstream& fileOut				//Output File stream
 *@patam	char* Mneumonic 				//Actual name of mneumonic
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void mneumonicCompare(ofstream & fileOut, char * mnemnonic, bool isFirstPass)
{
	if(!strcmp(mnemnonic,"LDR"))
		interpretLDR(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"STR"))
		interpretSTR(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"MAI"))
		interpretMAI(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"JZR"))
		interpretJZR(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"JUM"))
		interpretJUM(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"JMC"))
		interpretJMC(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"JMZ"))
		interpretJMZ(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"JMP"))
		interpretJMP(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"MVR"))
		interpretMVR(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"ADD"))
		interpretADD(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"SUB"))
		interpretSUB(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"MUL"))
		interpretMUL(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"DIV"))
		interpretDIV(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"MOD"))
		interpretMOD(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"OR2"))
	// 	interpretOR2(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"AND"))
	// 	interpretAND(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"XOR"))
	// 	interpretXOR(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"COM"))
	// 	interpretCOM(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"STI"))
		interpretSTI(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"NOT"))
		interpretNOT(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"MOI"))
		interpretMOI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"ADI"))
	// 	interpretADI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"SUI"))
	// 	interpretSUI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MUI"))
	// 	interpretMUI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"DVI"))
	// 	interpretDVI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MDI"))
	// 	interpretMDI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"ANI"))
	// 	interpretANI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"ORI"))
	// 	interpretORI(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"INC"))
		interpretINC(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"DEC"))
		interpretDEC(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"LHS"))
	// 	interpretLHS(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"RHS"))
	// 	interpretRHS(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"PSH"))
	// 	interpretPSH(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"POP"))
	// 	interpretPOP(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"OUT"))
	// 	interpretOUT(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"INP"))
	// 	interpretINP(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"LOP"))
		interpretLOP(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"ELP"))
		interpretELP(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"HLT"))
		interpretHLT(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"NOP"))
		interpretNOP(fileOut,isFirstPass);
	else				//Invalid Mnemonic
	{
		fprintf(stderr,"cass: Error at line number : %d\nInvalid mnemnonic!\n",currentRow+1);
		exit(1);
	}
}


/**
 *Function to interpret mneumonic "LDR"
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void interpretLDR(ofstream & fileOut, bool isFirstPass)
{
	int i=0;
	char reg[3],addr[6];
	char opcode[] = "00000000000";			//Opcode of "LDR"
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)						//If second pass
	{
		while(1)							//Loop to read register
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \nInvald Register\n", currentRow+1);
				exit(1);
			}
		}
		reg[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)							//Loop to read address in HEXADECIMAL FORMAT
		{
			if(sourceProgram[currentRow][currentIndex] == 'h' || sourceProgram[currentRow][currentIndex] == 'H' || sourceProgram[currentRow][currentIndex] == '\0')
				break;
			addr[i++] = sourceProgram[currentRow][currentIndex++];
		}
		addr[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
		fileOut<<endl;
	}
}


/**
 *Function to interpret mneumonic "STR"
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void interpretSTR(ofstream & fileOut,bool isFirstPass)
{
	int i=0;
	char reg[3],addr[6];
	char opcode[] = "00000000001";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == 'h' || sourceProgram[currentRow][currentIndex] == 'H' || sourceProgram[currentRow][currentIndex] == '\0')
				break;
			addr[i++] = sourceProgram[currentRow][currentIndex++];
		}
		addr[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
		fileOut<<endl;
	}
}



/**
 *Function to interpret mneumonic "MAI"
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void interpretMAI(ofstream & fileOut,bool isFirstPass)
{
	int i=0;
	char reg[3],addr[6];
	char opcode[] = "00000000010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == 'h' || sourceProgram[currentRow][currentIndex] == 'H' || sourceProgram[currentRow][currentIndex] == '\0')
				break;
			addr[i++] = sourceProgram[currentRow][currentIndex++];
		}
		addr[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
		fileOut<<endl;
	}
}


/**
 *Function to interpret mneumonic "JZR"
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void interpretJZR(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg[3],label[LABEL_SIZE];
	char opcode[] = "00000000011";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)							//Loop to detect register used
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)						//Loop to detect Label Name
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			label[i++] = sourceProgram[currentRow][currentIndex++];
		}
		label[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		ILC = searchSymbolTable(label);
		if(ILC == -1)
		{
			fprintf(stderr,"Error at line number: %d\n Label Not found\n",currentRow+1);
			exit(1);
		}
		fileOut<<setw(16)<<setfill('0')<<decToBinary(ILC+baseAddress);
		fileOut<<endl;
	}
}


/**
 *Function to interpret mneumonic "JUM"
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	bool isFirstPass				//First pass or second pass
 *@return void
 */
void interpretJUM(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char label[LABEL_SIZE];
	char opcode[] = "0000000010000000";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			label[i++] = sourceProgram[currentRow][currentIndex++];
		}
		label[i] = '\0';
		fileOut<<opcode;
		ILC = searchSymbolTable(label);
		if(ILC == -1)
		{
			fprintf(stderr,"Error at line number: %d\n Label Not found\n",currentRow+1);
			exit(1);
		}
		fileOut<<setw(16)<<setfill('0')<<decToBinary(ILC+baseAddress);
		fileOut<<endl;
	}
}

void interpretJMC(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char label[LABEL_SIZE];
	char opcode[] = "0000000010000001";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			label[i++] = sourceProgram[currentRow][currentIndex++];
		}
		label[i] = '\0';
		fileOut<<opcode;
		ILC = searchSymbolTable(label);
		if(ILC == -1)
		{
			fprintf(stderr,"Error at line number: %d\n Label Not found\n",currentRow+1);
			exit(1);
		}
		fileOut<<setw(16)<<setfill('0')<<decToBinary(ILC+baseAddress);
		fileOut<<endl;
	}
}

void interpretJMZ(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char label[LABEL_SIZE];
	char opcode[] = "0000000010000010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			label[i++] = sourceProgram[currentRow][currentIndex++];
		}
		label[i] = '\0';
		fileOut<<opcode;
		ILC = searchSymbolTable(label);
		if(ILC == -1)
		{
			fprintf(stderr,"Error at line number: %d\n Label Not found\n",currentRow+1);
			exit(1);
		}
		fileOut<<setw(16)<<setfill('0')<<decToBinary(ILC+baseAddress);
		fileOut<<endl;
	}
}

void interpretJMP(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char label[LABEL_SIZE];
	char opcode[] = "0000000010000011";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			label[i++] = sourceProgram[currentRow][currentIndex++];
		}
		label[i] = '\0';
		fileOut<<opcode;
		ILC = searchSymbolTable(label);
		if(ILC == -1)
		{
			fprintf(stderr,"Error at line number: %d\n Label Not found\n",currentRow+1);
			exit(1);
		}
		fileOut<<setw(16)<<setfill('0')<<decToBinary(ILC+baseAddress);
		fileOut<<endl;
	}
}

void interpretMVR(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000000000";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretADD(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000000001";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretSUB(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000000010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretMUL(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000000011";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretDIV(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000000100";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretMOD(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "000000001010000000010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}


/**
 *Skipped :
 *OR2
 *AND
 *XOR
 *COM
 */
void interpretSTI(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3],reg2[3];
	char opcode[] = "0000000010100000001010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg2[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg2[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		regToBinary(fileOut,reg2);
		fileOut<<endl;
	}
}

void interpretNOT(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3];
	char opcode[] = "000000001010000001000000000";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		fileOut<<endl;
	}
}


/**
 *Immediate data must be in decimal
 */
void interpretMOI(ofstream & fileOut,bool isFirstPass)
{
	int i=0;
	char reg1[3],data[12];
	char opcode[] = "000000001010000001000000001";
	eatWhiteSpace();
	instructionLocationCounter+=8;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == ',')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		currentIndex++;
		eatWhiteSpace();
		i=0;
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			data[i++] = sourceProgram[currentRow][currentIndex];
			currentIndex++;
			if(i>10)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		data[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		fileOut<<endl;
		dataToBinary(fileOut,data);
		fileOut<<endl;
	}
}

/**
 *Skipped
 *ADI,SUI,MUI,DVI,MDI,ANI,ORI,
 *
 */
void interpretINC(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3];
	char opcode[] = "000000001010000001000001001";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		fileOut<<endl;
	}
}


void interpretDEC(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg1[3];
	char opcode[] = "000000001010000001000001010";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		fileOut<<endl;
	}
}

/**
 *Skipped : LHS,RHS,PSH,POP,OUT,IN
 */
void interpretLOP(ofstream & fileOut,bool isFirstPass)
{
	int i=0;
	char reg1[3];
	char opcode[] = "000000001010000001000010001";
	eatWhiteSpace();
	instructionLocationCounter+=4;
	if(!isFirstPass)
	{
		while(1)
		{
			if(sourceProgram[currentRow][currentIndex] == '\0')
				break;
			reg1[i++] = toupper(sourceProgram[currentRow][currentIndex]);
			currentIndex++;
			if(i>2)					//Implement exception handling
			{
				fprintf(stderr,"Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg1[i] = '\0';
		fileOut<<opcode;
		regToBinary(fileOut,reg1);
		fileOut<<endl;
	}
}

void interpretELP(ofstream & fileOut, bool isFirstPass)
{
	char opcode[] = "00000000101000000100001010000000";
	eatWhiteSpace();
	instructionLocationCounter+=4;

	if(!isFirstPass)
	{
		fileOut<<opcode;
		fileOut<<endl;
	}
}

void interpretHLT(ofstream & fileOut, bool isFirstPass)
{
	char opcode[] = "00000000101000000100001010000001";
	eatWhiteSpace();
	instructionLocationCounter+=4;

	if(!isFirstPass)
	{
		fileOut<<opcode;
		fileOut<<endl;
	}
	isEnd =true;
}

void interpretNOP(ofstream & fileOut, bool isFirstPass)
{
	char opcode[] = "00000000101000000100001010000010";
	eatWhiteSpace();
	instructionLocationCounter+=4;

	if(!isFirstPass)
	{
		fileOut<<opcode;
		fileOut<<endl;
	}
}

/**
 *Function to convert "immediate" DECIMAL data(in char form) into 32 bit binary data
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	char* data						//Array of data
 *@return void
 */
void dataToBinary(ofstream & fileOut,char * data)
{
	int integer;
	unsigned long long int bin;
	integer = atoi(data);
	bin = decToBinary(integer);
	fileOut<<setw(32)<<setfill('0')<<bin;
}


/**
 *Function to Search symbol table for a label
 *@param 	char* element				//Label to be searched for
 *@return ILC of label, -1 if label not found
 */
int searchSymbolTable(char * element)
{
	int i;
	for(i=0;i<=symbTableCount;i++)
	{
		if(!strcmp(symbolTable[i].label,element))
			return symbolTable[i].ILC;
	}
	return -1;
}


/**
 *Function to convert decimal number to Binary
 *@param  int								//Decimal number
 *@return unsigned long long 				//Binary equivalent of the given data
 */
unsigned long long int decToBinary(int num)
{
	unsigned long long bin=0,t=1,bit;
	while(num)
	{
		bit = num % 2;
		num = num / 2;
		bin += bit*t;
		t *= 10;
	}
	return bin;
}

/**
 *Function to convert register to a binary and write the output in file
 *@param 	ofstream& fileOut				//Output File stream
 *@param 	char* reg 						//Name of register
 *@return void
 */
void regToBinary(ofstream & fileOut, char * reg)
{
	const char *registr[] = {
		"A",		"B",		"C",		"D",
		"E",		"F",		"G",		"H",
		"I",		"J",		"K",		"L",
		"M",		"N",		"O",		"P",
		"Q",		"R",		"S",		"T",
		"U",		"V",		"W",		"X",
		"Y",		"Z",		"ZA",		"ME"
	};

	const char *opcode[] = {
		"00000",		"00001",		"00010",		"00011",
		"00100",		"00101",		"00110",		"00111",
		"01000",		"01001",		"01010",		"01011",
		"01100",		"01101",		"01110",		"01111",
		"10000",		"10001",		"10010",		"10011",
		"10100",		"10101",		"10110",		"10111",
		"11000",		"11001",		"11010",		"11011"
	};

	for (int i = 0; i < NUMBER_OF_REG ; ++i)
	{
		if(!strcmp(reg,registr[i]))
			fileOut<<opcode[i];
	}
}

//Modify the code so that if entered hex is of only 1 or 2 or 3 characters then output must be in 16 bit format only
/**
 *Function to convert 4bit hexadecimal address into 16 bit binary
 *@param 	ofstream& fileOut				//Output File stream
 *@param  	char* reg 						//Address pointer
 *@return void
 */
void hexToBinary(ofstream & fileOut,char * reg)			//Its not register, but pointer to address
{
	for (int i = 0; reg[i] != '\0' ; ++i)
	{
		switch(reg[i])
		{
			case '0' : fileOut<<"0000";
						break;
			case '1' : fileOut<<"0001";
						break;
			case '2' : fileOut<<"0010";
						break;
			case '3' : fileOut<<"0011";
						break;
			case '4' : fileOut<<"0100";
						break;
			case '5' : fileOut<<"0101";
						break;
			case '6' : fileOut<<"0110";
						break;
			case '7' : fileOut<<"0111";
						break;
			case '8' : fileOut<<"1000";
						break;
			case '9' : fileOut<<"1001";
						break;
			case 'A' : fileOut<<"1010";
						break;
			case 'B' : fileOut<<"1011";
						break;
			case 'C' : fileOut<<"1100";
						break;
			case 'D' : fileOut<<"1101";
						break;
			case 'E' : fileOut<<"1110";
						break;
			case 'F' : fileOut<<"1111";
						break;
			default : fprintf(stderr,"Error at line number : %d \nInvalid operands.\n",currentRow+1);
		}
	}
}