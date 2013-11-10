/**
 *Assembler for our own Assembly Language
 */

#include<cstdio>
#include<fstream>
#include<cctype>
#include<string>
#include<cstring>
#include<cstdlib>
#include<iomanip>

#define INPUT_WIDTH 50					//Specifies Max length for an instruction
#define INPUT_HEIGHT 10000				//Specifies Max number of instructions
#define SYMB_TAB_SIZE 1000
#define LABEL_SIZE 15
#define MNEUMONIC_SIZE 5
#define NUMBER_OF_REG 28

using namespace std;

//Label wali line cannot contain any other instruction


/**
 *Global Variables
 */
int currentIndex=0,currentRow=0,instructionLocationCounter=0,symbTableCount=0;
int * asLength;
char sourceProgram[INPUT_HEIGHT][INPUT_WIDTH];
bool isEnd;
int baseAddress=0;


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
void interpretHLT(ofstream &, bool );
void interpretJZR(ofstream & ,bool );
void interpretMAI(ofstream & ,bool );
void interpretSTR(ofstream & ,bool);
void regToBinary(ofstream &, char * );
void hexToBinary(ofstream &,char * );
int searchSymbolTable(char * );
unsigned long long int decToBinary(int );

/**
 *Structure for symbol table
 *stores label and ILC
 */
struct symbol {
	char  label[LABEL_SIZE];
	int ILC;
};

typedef struct symbol symbol;

symbol symbolTable[SYMB_TAB_SIZE];			//Global array to store symbol table, which is array of stuctures


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

	fileIn.open(inputFileName,ios::in);
	if(!fileIn)
	{
		printf("Input file not found !!\n");
		return 1;
	}

	inputNumberOfLines=0;
	while(1)
	{
		fileIn.getline(sourceProgram[inputNumberOfLines],INPUT_WIDTH,'\n');
		if(fileIn.eof())
			break;
		inputNumberOfLines++;
	}

	fileIn.close();
	fileOut.open(outputFileName,ios::out);		//WARNING : This will destroy the previous contents of the file
	// for(i=0;i<inputNumberOfLines;i++)
	// {
	// 	for(j=0;sourceProgram[i][j] !='\0';j++)
	// 		fileOut<<sourceProgram[i][j];
	// 	fileOut<<endl;
	// }
	// fileOut.close();
	parse(fileOut);
	return 0;
}


void parse(ofstream & fileOut)
{
	currentRow = 0;
	currentIndex =0;
	//First Pass
	while(!isEnd)
		labelScan(fileOut,true);

	currentRow =0;
	currentIndex =0;
	isEnd = false;
	instructionLocationCounter = 0;
	//Pass Pass
	while(!isEnd)
		labelScan(fileOut,false);
}

/**
 *Function to eat all whitespaces
 */
void eatWhiteSpace(void)
{
	while(sourceProgram[currentRow][currentIndex] == ' ')
		currentIndex++;
}

void labelScan(ofstream & fileOut,bool isFirstPass)
{
	if(sourceProgram[currentRow][currentIndex] != ' ')
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
	eatWhiteSpace();
	readMneumonic(fileOut,isFirstPass);
}

char * getLabelName()
{
	return sourceProgram[currentRow];
}

char * getMemory()
{
	return (char *)malloc(6*sizeof(char));
}



void insertInSymbolTable(char * name)
{
	int i;
	static int index =0;
	if(searchSymbolTable(name) != -1)  				//If symbol already exists in symbol table
	{
		fprintf(stderr, "Error at line number: %d\n Label Already used\n",currentRow+1);
		exit(1);
	}
	symbolTable[index].ILC = instructionLocationCounter;
	for(i=0;name[i] !=':';i++)
	{
		symbolTable[index].label[i] =name[i];
	}
	symbolTable[index].label[i] = '\0';
	index++;
	symbTableCount = index;
}


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
	mneumonic[i] = '\0';
	//Code to compare mnemnonic
	mneumonicCompare(fileOut,mneumonic,isFirstPass);
	currentRow++;
	currentIndex=0;
}

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
	// else if(!strcmp(mnemnonic,"JUM"))
	// 	interpretJUM(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"JMC"))
	// 	interpretJMC(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"JMZ"))
	// 	interpretJMZ(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"JMP"))
	// 	interpretJMP(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MVR"))
	// 	interpretMVR(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"ADD"))
	// 	interpretADD(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"SUB"))
	// 	interpretSUB(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MUL"))
	// 	interpretMUL(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"DIV"))
	// 	interpretDIV(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MOD"))
	// 	interpretMOD(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"OR2"))
	// 	interpretOR2(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"AND"))
	// 	interpretAND(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"XOR"))
	// 	interpretXOR(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"COM"))
	// 	interpretCOM(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"STI"))
	// 	interpretSTI(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"NOT"))
	// 	interpretNOT(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"MOI"))
	// 	interpretMOI(fileOut,isFirstPass);
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
	// else if(!strcmp(mnemnonic,"INC"))
	// 	interpretINC(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"DEC"))
	// 	interpretDEC(fileOut,isFirstPass);
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
	// else if(!strcmp(mnemnonic,"LOP"))
	// 	interpretLOP(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"ELP"))
	// 	interpretELP(fileOut,isFirstPass);
	else if(!strcmp(mnemnonic,"HLT"))
		interpretHLT(fileOut,isFirstPass);
	// else if(!strcmp(mnemnonic,"NOP"))
	// 	interpretNOP(fileOut,isFirstPass);
	else
	{
		printf("Error at line number : %d . Invalid Opcode\n",currentRow+1);
		exit(1);
	}
}

void interpretLDR(ofstream & fileOut, bool isFirstPass)
{
	int i=0;
	char reg[3],addr[6];
	char opcode[] = "00000000000";
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
				printf("Error at line number : %d \n", currentRow+1);
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
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
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
				printf("Error at line number : %d \n", currentRow+1);
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
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
		fileOut<<endl;
	}
}

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
				printf("Error at line number : %d \n", currentRow+1);
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
		fileOut<<opcode;
		regToBinary(fileOut,reg);
		hexToBinary(fileOut,addr);
		fileOut<<endl;
	}
}

void interpretJZR(ofstream & fileOut,bool isFirstPass)
{
	int i=0,ILC=0;
	char reg[3],label[LABEL_SIZE];
	char opcode[] = "00000000011";
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
				printf("Error at line number : %d \n", currentRow+1);
				exit(1);
			}
		}
		reg[i] = '\0';
		currentIndex++;
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

void regToBinary(ofstream & fileOut, char * reg)
{
	const char *registr[] = {
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"ZA",
		"ME"
	};

	const char *opcode[] = {
		"00000",
		"00001",
		"00010",
		"00011",
		"00100",
		"00101",
		"00110",
		"00111",
		"01000",
		"01001",
		"01010",
		"01011",
		"01100",
		"01101",
		"01110",
		"01111",
		"10000",
		"10001",
		"10010",
		"10011",
		"10100",
		"10101",
		"10110",
		"10111",
		"11000",
		"11001",
		"11010",
		"11011"
	};

	for (int i = 0; i < NUMBER_OF_REG ; ++i)
	{
		if(!strcmp(reg,registr[i]))
			fileOut<<opcode[i];
	}
}


void hexToBinary(ofstream & fileOut,char * reg)
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
			default : printf("Error at line number : %d \nInvalid operands.\n",currentRow);
		}
	}
}