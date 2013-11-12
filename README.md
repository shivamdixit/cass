cass
====

Assembler for converting our own Assembly Language program into Machine Language. Written in C/C++

Usage: ./a.out [options] input_file out_file
		[options]	-v 	 For verbose output
				--help 	 For help and sample usage

		Input file must be present in same directory
		New line character \r\n
		Mneumonics must begin with space
		Line containing Label should not contain any Mneumonic and must not begin with space
		Address must be specified in 4bit hexadecimal format.
		Immediate data must be in Decimal
		Sample Usage:
		START
		 LDR A,2048H
		 MVR B,A
		 LOP A
		 MUL C,B
		 DEC B
		 HLT
