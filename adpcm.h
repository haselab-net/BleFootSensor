#pragma once

struct ADPCMstate {
	short prevsample;	/* Predicted sample */
	int previndex;		/* Index into step size table */
};

/* Function prototype for the ADPCM Encoder routine */
char ADPCMEncoder(short, struct ADPCMstate *);

/* Function prototype for the ADPCM Decoder routine */
int ADPCMDecoder(char, struct ADPCMstate *);