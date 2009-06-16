/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
/*
 *
 *    smsSynth - program for synthesizing an sms file from the command line.
 *
 */
#include "sms.h"


void usage (void)
{
    fprintf (stderr, "\n"
             "Usage: smsSynth [options]  <inputSmsFile> <outputSoundFile>\n"
             "\n"
             "Options:\n"
             "      -v     print out verbose information\n"
             "      -r     sampling rate of output sound (default is original)\n"
             "      -s     synthesis type (0: all (default), 1: deterministic only , 2: stochastic only)\n"
             "      -d     method of deterministic synthesis type (1: IFFT, 2: oscillator bank)\n"
             "      -h     sizeHop (default 128) 128 <= sizeHop <= 8092, rounded to a power of 2 \n"
             "      -t     time factor (default 1): positive value to multiply by overall time \n"
             "      -g     stochastic gain (default 1): positive value to multiply into stochastic gain \n"
             "      -x     transpose factor (default 1): value based on the Equal Tempered Scale to\n"
             "      -f      soundfile output type (default 0): 0 is wav, 1 is aiff"
             "             transpose the frequency \n"
             "\n"
             "synthesize an analysis (.sms) file made with smsAnal."
             "output file format is 32bit floating-point AIFF."
             "\n\n");
    
    exit(1);
}


int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSoundFile = NULL;
	SMS_Header *pSmsHeader = NULL;
	FILE *pSmsFile; /* pointer to sms file to be synthesized */
	SMS_Data smsFrameL, smsFrameR, smsFrame; /* left, right, and interpolated frames */
	float *pFSynthesis; /* waveform synthesis buffer */
	long iSample, i, nSamples, iLeftFrame, iRightFrame;
        int verboseMode = 0;
        float fFrameLoc; /* exact sms frame location, used to interpolate smsFrame */
        float fFsRatio,  fLocIncr;
        int  detSynthType, synthType, sizeHop, iSamplingRate; /*  argument holders */
        int iSoundFileType = 0; /* wav file */
        int doInterp = 1;
        float timeFactor = 1.0;
        float fTranspose = 0.0;
	float stocGain = 1.0;
	SMS_SynthParams synthParams;
	synthParams.iSynthesisType = SMS_STYPE_ALL;
        synthParams.iDetSynthType = SMS_DET_IFFT;
	synthParams.sizeHop = SMS_MIN_SIZE_FRAME;
	synthParams.iSamplingRate = 0; /* if this is not set by an argument, the original value is used */

	if (argc > 3) 
	{
		for (i=1; i<argc-2; i++) 
			if (*(argv[i]++) == '-') 
				switch (*(argv[i]++)) 
				{
                                case 'r':  if (sscanf(argv[i],"%d",&iSamplingRate) < 0 )
                                        {
						printf("error: invalid sampling rate");
                                                exit(1);
                                        }
                                        synthParams.iSamplingRate = iSamplingRate;
                                        break;
                                case 's': sscanf(argv[i], "%d", &synthType);
                                        if(synthType < 0 || synthType > 2)
                                        {
                                                printf("error: detSynthType must be 0, 1, or  2");
                                                exit(1);
                                        }
                                        synthParams.iSynthesisType = synthType;
                                        break;
                                case 'd': sscanf(argv[i], "%d", &detSynthType);
                                        if(detSynthType < 1 || detSynthType > 2)
                                        {
                                                printf("error: detSynthType must be 1 or 2");
                                                exit(1);
                                        }
                                        synthParams.iDetSynthType = detSynthType;
                                        break;
                                case 'h': sscanf(argv[i], "%d", &sizeHop);
                                        if(sizeHop < SMS_MIN_SIZE_FRAME || sizeHop > SMS_MAX_WINDOW) 
                                        {
                                                printf("error: invalid sizeHop");
                                                exit(1);
                                        }
                                        synthParams.sizeHop = sizeHop;
                                        //RTE TODO: round to power of 2 (is it necessary?)
                                        break;
                                case 't':  if (sscanf(argv[i],"%f",&timeFactor) < 0 )
                                        {
						printf("error: invalid time factor");
                                                exit(1);
                                        }
                                        break;
                                case 'g':  if (sscanf(argv[i],"%f",&stocGain) < 0 )
                                        {
						printf("error: invalid stochastic gain");
                                                exit(1);
                                        }
                                        break;
                                case 'x':  sscanf(argv[i],"%f",&fTranspose) ;
                                        break;
                                case 'i':  sscanf(argv[i],"%d",&doInterp) ;
                                        break;
                                case 'v': verboseMode = 1;
                                        break;
                                case 'f': sscanf(argv[i], "%d", &iSoundFileType);
                                        break;
                                default:   usage();
				}
	}
	else if (argc < 2) usage();

	pChInputSmsFile = argv[argc-2];
	pChOutputSoundFile = argv[argc-1];
        
        sms_getHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile);

	if (sms_errorCheck())
	{
                printf("error in sms_getHeader: %s", sms_errorString());
                exit(EXIT_FAILURE);
	}	    

        sms_init();
        sms_initSynth( pSmsHeader, &synthParams );
        /* disabling interpolation for residual resynthesis with original phases (temp) */
        if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
                doInterp = 0;

         /* set modifiers */
        synthParams.fTranspose = TEMPERED_TO_FREQ( fTranspose );
        synthParams.fStocGain = stocGain;

        if(verboseMode)
        {
                printf("__arguments__\n");
                printf("samplingrate: %d \nsynthesis type: ", synthParams.iSamplingRate);
                printf(" do frame interpolation: %d \n", doInterp);
                if(synthParams.iSynthesisType == SMS_STYPE_ALL) printf("all ");
                else if(synthParams.iSynthesisType == SMS_STYPE_DET) printf("deterministic only ");
                else if(synthParams.iSynthesisType == SMS_STYPE_STOC) printf("stochastic only ");
                printf("\ndeteministic synthesis method: ");
                if(synthParams.iDetSynthType == SMS_DET_IFFT) printf("ifft ");
                else if(synthParams.iDetSynthType == SMS_DET_SIN) printf("oscillator bank ");
                printf("\nsizeHop: %d \n", synthParams.sizeHop);
                printf("time factor: %f \n", timeFactor);
                printf("stochastic gain factor: %f \n", synthParams.fStocGain);
                printf("frequency transpose factor: %f \n", synthParams.fTranspose);
                printf("__header info__\n");
                printf("original samplingrate: %d, iFrameRate: %d, origSizeHop: %d\n", pSmsHeader->iSamplingRate,
                       pSmsHeader->iFrameRate, synthParams.origSizeHop);
                printf("original file length: %f seconds \n",
                       pSmsHeader->nFrames / (float) pSmsHeader->iFrameRate);
                if(!iSoundFileType) printf("output soundfile type: wav \n");
                else  printf("output soundfile type: aiff \n");
        }

        /* initialize libsndfile for writing a soundfile */
	sms_createSF ( pChOutputSoundFile, synthParams.iSamplingRate, 0);

	/* setup for synthesis from file */
        if(doInterp)
        {
                sms_allocFrameH (pSmsHeader, &smsFrameL);
                sms_allocFrameH (pSmsHeader, &smsFrameR);
        }
        sms_allocFrameH (pSmsHeader, &smsFrame); /* the actual frame to be handed to synthesizer */

/*         sms_allocFrame (&smsFrame, pSmsHeader->nTracks,  */
/*                         pSmsHeader->nStochasticCoeff, 0, pSmsHeader->iStochasticType); */

	if ((pFSynthesis = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
        {
                printf ("Could not allocate memory for pFSynthesis");
                exit(0);
        }

	iSample = 0;
        /* number of samples is a factor of the ratio of samplerates */
        /* multiply by timeFactor to increase samples to desired file length */
        /* pSmsHeader->iSamplingRate is from original analysis */
        fFsRatio = (float) synthParams.iSamplingRate / pSmsHeader->iSamplingRate;
	nSamples = pSmsHeader->nFrames * synthParams.origSizeHop * timeFactor * fFsRatio;

        /* divide timeFactor out to get the correct frame */
        fLocIncr = pSmsHeader->iSamplingRate / 
                ( synthParams.origSizeHop * synthParams.iSamplingRate * timeFactor); 

	while (iSample < nSamples)
	{
		if(doInterp)
                {////////////////////////////////////////////////
                        fFrameLoc =  iSample *  fLocIncr;
                        // left and right frames around location, gaurding for end of file
                        iLeftFrame = MIN (pSmsHeader->nFrames - 1, floor (fFrameLoc)); 
                        iRightFrame = (iLeftFrame < pSmsHeader->nFrames - 2)
                                ? (1+ iLeftFrame) : iLeftFrame;
                        sms_getFrame (pSmsFile, pSmsHeader, iLeftFrame, &smsFrameL);
                        sms_getFrame (pSmsFile, pSmsHeader, iRightFrame,&smsFrameR);
                        ////////////////////////////////////////
                        sms_interpolateFrames (&smsFrameL, &smsFrameR, &smsFrame,
                                               fFrameLoc - iLeftFrame);
                }
                else
                {
                        sms_getFrame (pSmsFile, pSmsHeader, (int) iSample * fLocIncr, &smsFrame);
                        printf("frame: %d \n",  (int) (iSample * fLocIncr));
                }
                sms_synthesize (&smsFrame, pFSynthesis, &synthParams);
		sms_writeSound (pFSynthesis, synthParams.sizeHop);
    
		iSample += synthParams.sizeHop;

                if(verboseMode)
                {
                        if (iSample % (synthParams.sizeHop * 20) == 0)
                                fprintf(stderr,"%.2f ", iSample / (float) synthParams.iSamplingRate);
                }

	}

        if(verboseMode)
        {
                printf("\nfile length: %f seconds\n", (float) iSample / synthParams.iSamplingRate);
        }
        printf("wrote %ld samples in %s\n",  iSample, pChOutputSoundFile);

	/* close output sound file, free memory and exit */
	sms_writeSF ();
        if(doInterp)
        {
                sms_freeFrame(&smsFrameL);
                sms_freeFrame(&smsFrameR);
        }
        sms_freeFrame(&smsFrame);
	free (pFSynthesis);
        free (pSmsHeader);
        sms_freeSynth(&synthParams);
        sms_free();
	return(1);
}
