#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "adpcm.h"

#pragma warning(disable: 4996)
#pragma warning(disable: 4703)


void Usage(void)
{
	printf("ADPCM Encoder/Decoder -- usage:\n");
	printf("\tEncoder = pcspeech e infile outfile\n");
	printf("\tDecoder = pcspeech d infile outfile\n");
	exit(1);
}

void adpcm_main(
	int argc,
	char **argv)
{
	int which;
	short sample;
	unsigned char code;
	int n;
	struct ADPCMstate state;
	FILE *fpin;
	FILE *fpout;
	state.prevsample = 0;
	state.previndex = 0;
	/* Determine if this is an encode or decode operation */
	if (argc <= 1)
		Usage();
	else if (strcmp(argv[1], "e") == 0 || strcmp(argv[1], "E") == 0)
		which = 0;
	else if (strcmp(argv[1], "d") == 0 || strcmp(argv[1], "D") == 0)
		which = 1;
	argc--;
	argv++;

	/* Open input file for processing */
	if (argc <= 1)
		Usage();
	else if ((fpin = fopen(argv[1], "r")) == NULL)
	{
		printf("ADPCM Encoder/Decoder\n");
		printf("ERROR: Could not open %s for input\n", argv[1]);
		exit(1);
	}
	argc--;
	argv++;
	/* Open output file */
	if (argc <= 1)
	{
		fclose(fpin);
		Usage();
	}
	else if ((fpout = fopen(argv[1], "w")) == NULL)
	{
		fclose(fpin);
		printf("ADPCM Encoder/Decoder\n");
		printf("ERROR: Could not open %s for output\n", argv[1]);
		exit(1);
	}



	// ADPCM Decoder selected
	if (which)
	{
		printf("ADPCM Decoding in progress\n");
		/* Read and unpack input codes and process them */
		while (1)
		{	
			if (feof(fpin)) break;
			fscanf(fpin, "%d", &code);
			// Send the upper 4-bits of code to decoder
			sample = ADPCMDecoder((code >> 4) & 0x0f, &state);
			// Write sample for upper 4-bits of code
			fprintf(fpout, "%d\n", sample);
			// Send the lower 4-bits of code to decoder
			sample = ADPCMDecoder(code & 0x0f, &state);
			// Write sample for lower 4-bits of code
			fprintf(fpout, "%d\n", sample);
		}
	}

	// ADPCM Encoder selected
	else
	{
		printf("ADPCM Encoding in progress\n");
		/* Read input file and process */
		while (1)
		{	
			if (feof(fpin)) break;
			fscanf(fpin, "%d", &sample);
			// Encode sample into lower 4-bits of code
			code = ADPCMEncoder(sample, &state);
			// Move ADPCM code to upper 4-bits
			code = (code << 4) & 0xf0;
			// Read new sample from file
			if (feof(fpin)) {
				fprintf(fpout, "%d\n", code);
				break;
			}
			
			fscanf(fpin, "%d", &sample);
			
			// Encode sample and save in lower 4-bits of code
			code |= ADPCMEncoder(sample, &state);
			// Write code to file, code contains 2 ADPCM codes
			fprintf(fpout, "%d\n", code);
		}
	}
	fclose(fpin);
	fclose(fpout);
}