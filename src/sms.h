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
/*! \file sms.h
 * \brief header file to be included in all SMS application
 */
#ifndef _SMS_H
#define _SMS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <strings.h>
#include <sndfile.h>

#include <sms_export.h>

#define SMS_VERSION 1.15 /*!< \brief version control number */

#define SMS_MAX_NPEAKS 400    /*!< \brief maximum number of peaks */
#define SMS_MAX_FRAME_SIZE 10000 /* maximum size of input frame in samples */
#define SMS_MAX_SPEC 8192  /*! \brief  maximum size for magnitude spectrum */

#ifdef DOUBLE_PRECISION
#define sfloat double
#else
#define sfloat float
#endif

/*! \struct SMS_Header
 *  \brief structure for the header of an SMS file
 *
 *  This header contains all the information necessary to read an SMS
 *  file, prepare memory and synthesizer parameters.
 *
 *  The header also contains variable components for additional information
 *  that may be stored along with the analysis, such as descriptors or text.
 *
 *  The first four members of the Header are necessary in this order to correctly
 *  open the .sms files created by this library.
 *
 *  iSampleRate contains the samplerate of the analysis signal because it is
 *  necessary to know this information to recreate the residual spectrum.
 *
 *  In the first release, the descriptors are not used, but are here because they
 *  were implemented in previous versions of this code (in the 90's).  With time,
 *  the documentation will be updated to reflect which members of the header
 *  are useful in manipulations, and what functions to use for these manipulatinos
 */
typedef struct
{
    int iSmsMagic;           /*!< identification constant */
    int iHeadBSize;          /*!< size in bytes of header */
    int nFrames;	         /*!< number of data frames */
    int iFrameBSize;         /*!< size in bytes of each data frame */
    int iSamplingRate;       /*!< samplerate of analysis signal (necessary to recreate residual spectrum) */
    int iFormat;             /*!< type of data format \see SMS_Format */
    int nTracks;             /*!< number of sinusoidal tracks per frame */
    int iFrameRate;          /*!< rate in Hz of data frames */
    int iStochasticType;     /*!< type stochastic representation */
    int nStochasticCoeff;    /*!< number of stochastic coefficients per frame */
    int iEnvType;            /*!< type of envelope representation */
    int nEnvCoeff;           /*!< number of cepstral coefficients per frame */
    int iMaxFreq;            /*!< maximum frequency of peaks (also corresponds to the last bin of the specEnv) */
    sfloat fResidualPerc;    /*!< percentage of the residual to original */
    int nTextCharacters;     /*!< number of text characters */
    char *pChTextCharacters; /*!< Text string relating to the sound */
} SMS_Header;

/*! \struct SMS_SndHeader
 *  \brief structure including sound header information
 */
typedef struct {
    int nSamples;      /*!< Number of samples in the sound */
    int iSamplingRate; /*!< The sampling rate */
    int channelCount;  /*!< The number of channels */
    int iReadChannel;  /*!< the channel to read from */
    int sizeHeader;	   /*!< size of sound header in bytes */
} SMS_SndHeader;

/*! \struct SMS_Data
 *  \brief structure with SMS data
 *
 * Here is where all the analysis data ends up. Once data is in here, it is ready
 * for synthesis.
 *
 * It is in one contiguous block (pSmsData), the other pointer members point
 * specifically to each component in the block.
 *
 * pFSinPha is optional in the final output, but it is always used to construct the
 * residual signal.
 */
typedef struct
{
    sfloat *pSmsData;     /*!< pointer to all SMS data */
    int sizeData;         /*!< size of all the data */
    sfloat *pFSinFreq;    /*!< frequency of sinusoids */
    sfloat *pFSinAmp;     /*!< magnitude of sinusoids (stored in dB) */
    sfloat *pFSinPha;     /*!< phase of sinusoids */
    int nTracks;          /*!< number of sinusoidal tracks in frame */
    sfloat *pFStocGain;   /*!< gain of stochastic component */
    int nCoeff;           /*!< number of filter coefficients */
    sfloat *pFStocCoeff;  /*!< filter coefficients for stochastic component */
    sfloat *pResPhase;    /*!< residual phase spectrum */
    int nEnvCoeff;        /*!< number of spectral envelope coefficients */
    sfloat *pSpecEnv;
} SMS_Data;


/*! \struct SMS_SndBuffer
 * \brief buffer for sound data
 *
 * This structure is used for holding a buffer of audio data. iMarker is a
 * sample number of the sound source that corresponds to the first sample
 * in the buffer.
 *
 */
typedef struct
{
    sfloat *pFBuffer; /*!< buffer for sound data*/
    int sizeBuffer;   /*!< size of buffer */
    int iMarker;      /*!< sample marker relating to sound source */
    int iFirstGood;   /*!< first sample in buffer that is a good one */
} SMS_SndBuffer;

/*! \struct SMS_Peak
 * \brief structure for sinusodial peak
 * information attached to a spectral peak
 */
typedef struct
{
    sfloat fFreq;  /*!< frequency of peak */
    sfloat fMag;   /*!< magnitude of peak */
    sfloat fPhase; /*!< phase of peak */
} SMS_Peak;

/*! \struct SMS_AnalFrame
 *  \brief structure to hold an analysis frame
 *
 *  This structure has extra information for continuing the analysis,
 *   which can be disregarded once the analysis is complete.
 */
typedef struct
{
    int iFrameSample;         /*!< sample number of the middle of the frame */
    int iFrameSize;           /*!< number of samples used in the frame */
    int iFrameNum;            /*!< frame number */
    SMS_Peak *pSpectralPeaks; /*!< spectral peaks found in frame */
    int nPeaks;               /*!< number of peaks found */
    sfloat fFundamental;      /*!< fundamental frequency in frame */
    SMS_Data deterministic;   /*!< deterministic data */
    int iStatus;              /*!< status of frame enumerated by SMS_FRAME_STATUS \see SMS_FRAME_STATUS */
} SMS_AnalFrame;

/*! \struct SMS_PeakParams
 * \brief structure with useful information for peak detection and continuation
 *
 */
typedef struct
{
    sfloat fLowestFreq;            /*!< the first bin to look for a peak */
    sfloat fHighestFreq;           /*!< the last bin to look for a peak */
    sfloat fMinPeakMag;            /*!< minimum magnitude to consider as a peak */
    int iSamplingRate;             /*!< sampling rate of analysis signal */
    int iMaxPeaks;                 /*!< maximum number of spectral peaks to look for */
    int nPeaksFound;               /*!< the number of peaks found in each analysis */
    sfloat fHighestFundamental;    /*!< highest fundamental frequency in Hz */
    int iRefHarmonic;              /*!< reference harmonic to use in the fundamental detection */
    sfloat fMinRefHarmMag;         /*!< minimum magnitude in dB for reference peak */
    sfloat fRefHarmMagDiffFromMax; /*!< maximum magnitude difference from reference peak to highest peak */
    int iSoundType;                /*!< type of sound to be analyzed \see SMS_SOUND_TYPE */
} SMS_PeakParams;

/*! \struct SMS_SEnvParams;
 * \brief structure information and data for spectral enveloping
 *
 */
typedef struct
{
    int iType;      /*!< envelope type \see SMS_SpecEnvType */
    int iOrder;     /*!< ceptrum order */
    int iMaxFreq;   /*!< maximum frequency covered by the envelope */
    sfloat fLambda; /*!< regularization factor */
    int nCoeff;     /*!< number of coefficients (bins) in the envelope */
    int iAnchor;    /*!< whether to make anchor points at DC / Nyquist or not */
} SMS_SEnvParams;

/*! \struct SMS_Guide
 * \brief information attached to a guide
 *
 * This structure is used to organize the detected peaks into time-varying
 * trajectories, or sinusoidal tracks.  As the analysis progresses, previous
 * guides may be updated according to new information in the peak continuation
 * of new frames (two-way mismatch).
 */
typedef struct
{
    sfloat fFreq;    /*!< frequency of guide */
    sfloat fMag;     /*!< magnitude of guide */
    int iStatus;     /*!< status of guide: DEAD, SLEEPING, ACTIVE */
    int iPeakChosen; /*!< peak number chosen by the guide */
} SMS_Guide;

/*! \struct SMS_AnalParams
 * \brief structure with useful information for analysis functions
 *
 * Each analysis needs one of these, which contains all settings,
 * sound data, deterministic synthesis data, and every other
 * piece of data that needs to be shared between functions.
 *
 * There is an array of already analyzed frames (hardcoded to 50 right now -
 * \todo make it variable) that are accumulated for good harmonic detection
 * and partial tracking. For instance, once the fundamental frequency of a
 * harmonic signal is located (after a few frames), the harmonic analysis
 * and peak detection/continuation process can be re-computed with more accuracy.
 *
 */
typedef struct
{
    int iDebugMode;                  /*!< debug codes enumerated by SMS_DBG \see SMS_DBG */
    int iFormat;                     /*!< analysis format code defined by SMS_Format \see SMS_Format */
    int iSoundType;                  /*!< type of sound to be analyzed \see SMS_SOUND_TYPE */
    int iStochasticType;             /*!< type of stochastic model defined by SMS_StocSynthType \see SMS_StocSynthType */
    int iFrameRate;                  /*!< rate in Hz of data frames */
    int nStochasticCoeff;            /*!< number of stochastic coefficients per frame */
    sfloat fLowestFundamental;       /*!< lowest fundamental frequency in Hz */
    sfloat fHighestFundamental;      /*!< highest fundamental frequency in Hz */
    sfloat fDefaultFundamental;      /*!< default fundamental in Hz */
    sfloat fPeakContToGuide;         /*!< contribution of previous peak to current guide (between 0 and 1) */
    sfloat fFundContToGuide;         /*!< contribution of current fundamental to current guide (between 0 and 1) */
    sfloat fFreqDeviation;           /*!< maximum deviation from peak to peak */
    int iSamplingRate;               /*! sampling rate of sound to be analyzed */
    int iDefaultSizeWindow;          /*!< default size of analysis window in samples */
    int sizeWindow;
    int sizeHop;                     /*!< hop size of analysis window in samples */
    sfloat fSizeWindow;              /*!< size of analysis window in number of periods */
    int nTracks;                     /*!< number of sinusoidal tracks in frame */
    int nGuides;                     /*!< number of guides used for peak detection and continuation \see SMS_Guide */
    int iCleanTracks;                /*!< whether or not to clean sinusoidal tracks */
    sfloat fMinRefHarmMag;           /*!< minimum magnitude in dB for reference peak */
    sfloat fRefHarmMagDiffFromMax;   /*!< maximum magnitude difference from reference peak to highest peak */
    int iRefHarmonic;	             /*!< reference harmonic to use in the fundamental detection */
    int iMinTrackLength;	         /*!< minimum length in samples of a given track */
    int iMaxSleepingTime;	         /*!< maximum sleeping time for a track */
    sfloat fHighestFreq;             /*!< highest frequency to be searched */
    sfloat fMinPeakMag;              /*!< minimum magnitude in dB for a good peak */
    int iAnalysisDirection;          /*!< analysis direction, direct or reverse */
    int iSizeSound;                  /*!< total size of sound to be analyzed in samples */
    int nFrames;                     /*!< total number of frames that will be analyzed */
    int iWindowType;                 /*!< type of FFT analysis window \see SMS_WINDOWS */
    int iMaxDelayFrames;             /*!< maximum number of frames to delay before peak continuation */
    int minGoodFrames;               /*!< minimum number of stable frames for backward search */
    sfloat maxDeviation;             /*!< maximum deviation allowed */
    int analDelay;                   /*! number of frames in the past to be looked in possible re-analyze */
    sfloat fResidualAccumPerc;       /*!< accumalitive residual percentage */
    int sizeNextRead;                /*!< size of samples to read from sound file next analysis */
    int preEmphasis;                 /*!< whether or not to perform pre-emphasis */
    sfloat preEmphasisLastValue;
    SMS_PeakParams peakParams;       /*!< structure with parameters for spectral peaks */
    SMS_Data prevFrame;              /*!< the previous analysis frame */
    SMS_SEnvParams specEnvParams;    /*!< all data for spectral enveloping */
    SMS_SndBuffer soundBuffer;       /*!< signal to be analyzed */
    SMS_SndBuffer synthBuffer;       /*!< resynthesized signal used to create the residual */
    SMS_AnalFrame *pFrames;          /*!< an array of frames that have already been analyzed */
    SMS_AnalFrame **ppFrames;        /*!< pointers to the frames analyzed (it is circular-shifted once the array is full) */
    sfloat magSpectrum[SMS_MAX_SPEC];
    sfloat phaseSpectrum[SMS_MAX_SPEC];
    sfloat spectrumWindow[SMS_MAX_SPEC];
    sfloat fftBuffer[SMS_MAX_SPEC * 2];
    int sizeResidual;
    sfloat *residual;
    sfloat *residualWindow;
    int *guideStates;
    SMS_Guide* guides;
    sfloat inputBuffer[SMS_MAX_FRAME_SIZE];
    int sizeStocMagSpectrum;
    sfloat *stocMagSpectrum;
    sfloat *approxEnvelope;          /*!< spectral approximation envelope */
} SMS_AnalParams;

/*! \struct SMS_ModifyParams
 *
 * \brief structure with parameters and data that will be used to modify an SMS_Data frame
 */
typedef struct
{
    int ready;           /*!< a flag to know if the struct has been initialized */
    int maxFreq;         /*!< maximum frequency component */
    int doResGain;       /*!< whether or not to scale residual gain */
    sfloat resGain;      /*!< residual scale factor */
    int doTranspose;     /*!< whether or not to transpose */
    sfloat transpose;    /*!< transposition factor */
    int doSinEnv;        /*!< whether or not to apply a new spectral envelope to the sin component */
    sfloat sinEnvInterp; /*!< value between 0 (use frame's env) and 1 (use *env). Interpolates inbetween values*/
    int sizeSinEnv;      /*!< size of the envelope pointed to by env */
    sfloat *sinEnv;      /*!< sinusoidal spectral envelope */
    int doResEnv;        /*!< whether or not to apply a new spectral envelope to the residual component */
    sfloat resEnvInterp; /*!< value between 0 (use frame's env) and 1 (use *env). Interpolates inbetween values*/
    int sizeResEnv;      /*!< size of the envelope pointed to by resEnv */
    sfloat *resEnv;      /*!< residual spectral envelope */
} SMS_ModifyParams;

/*! \struct SMS_SynthParams
 * \brief structure with information for synthesis functions
 *
 * This structure contains all the necessary settings for different types of synthesis.
 * It also holds arrays for windows and the inverse-FFT, as well as the previously
 * synthesized frame.
 *
 */
typedef struct
{
    int iStochasticType;        /*!<  type of stochastic model defined by SMS_StocSynthType
                                  \see SMS_StocSynthType */
    int iSynthesisType;         /*!< type of synthesis to perform \see SMS_SynthType */
    int iDetSynthType;          /*!< method for synthesizing deterministic component \see SMS_DetSynthType */
    int iOriginalSRate;         /*!< samplerate of the sound model source (for stochastic synthesis approximation) */
    int iSamplingRate;          /*!< synthesis samplerate */
    int sizeHop;                /*!< number of samples to synthesis for each frame */
    int origSizeHop;            /*!< original number of samples used to create each analysis frame */
    sfloat *pFDetWindow;        /*!< array to hold the window used for deterministic synthesis  \see SMS_WIN_IFFT */
    sfloat *pFStocWindow;       /*!< array to hold the window used for stochastic synthesis (Hanning) */
    sfloat *pSynthBuff;         /*!< an array for keeping samples during overlap-add (2x sizeHop) */
    sfloat *pMagBuff;           /*!< an array for keeping magnitude spectrum for stochastic synthesis */
    sfloat *pPhaseBuff;         /*!< an array for keeping phase spectrum for stochastic synthesis */
    sfloat *pSpectra;           /*!< array for in-place FFT transform */
    SMS_Data prevFrame;         /*!< previous data frame, for interpolation between frames */
    SMS_ModifyParams modParams; /*!< modification parameters */
    int deEmphasis;             /*!< whether or not to perform de-emphasis */
    sfloat deEmphasisLastValue;
    sfloat *approxEnvelope;     /*!< spectral approximation envelope */
} SMS_SynthParams;

/*! \struct SMS_HarmCandidate
 * \brief structure to hold information about a harmonic candidate
 *
 * This structure provides storage for accumimlated statistics when
 * trying to decide which track is the fundamental frequency, during
 * harmonic detection.
 */
typedef struct
{
    sfloat fFreq;       /*!< frequency of harmonic */
    sfloat fMag;        /*!< magnitude of harmonic */
    sfloat fMagPerc;    /*!< percentage of magnitude */
    sfloat fFreqDev;    /*!< deviation from perfect harmonic */
    sfloat fHarmRatio;  /*!< percentage of harmonics found */
} SMS_HarmCandidate;

/*! \struct SMS_ContCandidate
 * \brief structure to hold information about a continuation candidate
 *
 * This structure holds statistics about the guides, which is used to
 * decide the status of the guide
 */
typedef struct
{
    sfloat fFreqDev;  /*!< frequency deviation from guide */
    sfloat fMagDev;   /*!< magnitude deviation from guide */
    int iPeak;        /*!< peak number (organized according to frequency)*/
} SMS_ContCandidate;

/*!  \brief analysis format
 *
 * Is the signal is known to be harmonic, using format harmonic (with out without
 * phase) will give more accuracy to the peak continuation algorithm.  If the signal
 * is known to be inharmonic, then it is best to use one of the inharmonic settings
 * to tell the peak continuation algorithm to just look at the peaks and connect them,
 * instead of trying to look for peaks at specific frequencies (harmonic partials).
 */
enum SMS_Format
{
    SMS_FORMAT_H,   /*!< 0, format harmonic */
    SMS_FORMAT_IH,  /*!< 1, format inharmonic */
    SMS_FORMAT_HP,  /*!< 2, format harmonic with phase */
    SMS_FORMAT_IHP  /*!< 3, format inharmonic with phase */
};

/*! \brief synthesis types
 *
 * These values are used to determine whether to synthesize
 * both deterministic and stochastic components together,
 * the deterministic component alone, or the stochastic
 * component alone.
 */
enum SMS_SynthType
{
    SMS_STYPE_ALL,  /*!< both components combined */
    SMS_STYPE_DET,  /*!< deterministic component alone */
    SMS_STYPE_STOC  /*!< stochastic component alone */
};

/*! \brief synthesis method for deterministic component
 *
 * There are two options for deterministic synthesis available to the
 * SMS synthesizer.  The Inverse Fast Fourier Transform method
 * (IFFT) is more efficient for models with lots of partial tracks, but can
 * possibly smear transients.  The Sinusoidal Table Lookup (SIN) can
 * theoretically support faster moving tracks at a higher fidelity, but
 * can consume lots of cpu at varying rates.
 */
enum SMS_DetSynthType
{
    SMS_DET_IFFT,   /*!< Inverse Fast Fourier Transform (IFFT) */
    SMS_DET_SIN     /*!< Sinusoidal Table Lookup (SIN) */
};

/*! \brief synthesis method for stochastic component
 *
 * Currently, Stochastic Approximation is the only reasonable choice
 * for stochastic synthesis: this method approximates the spectrum of
 * the stochastic component by a specified number of coefficients during
 * analyses, and then approximates another set of coefficients during
 * synthesis in order to fit the specified hopsize. The phases of the
 * coefficients are randomly generated, according to the theory that a
 * stochastic spectrum consists of random phases.
 *
 * The Inverse FFT method is not implemented, but is based on the idea of storing
 * the exact spectrum and phases of the residual component to file. Synthesis
 * could then be an exact reconstruction of the original signal, provided
 * interpolation is not necessary.
 *
 * No stochastic component can also be specified in order to skip the this
 * time consuming process altogether.  This is especially useful when
 * performing multiple analyses to fine tune parameters pertaining to the
 * deterministic component; once that is achieved, the stochastic component
 * will be much better as well.
 */
enum SMS_StocSynthType
{
    SMS_STOC_NONE,   /*!< 0, no stochastistic component */
    SMS_STOC_APPROX, /*!< 1, Inverse FFT, magnitude approximation and generated phases */
    SMS_STOC_IFFT    /*!< 2, inverse FFT, interpolated spectrum (not used) */
};

/*! \brief synthesis method for deterministic component
 *
 * There are two options for deterministic synthesis available to the
 * SMS synthesizer.  The Inverse Fast Fourier Transform method
 * (IFFT) is more efficient for models with lots of partial tracks, but can
 * possibly smear transients.  The Sinusoidal Table Lookup (SIN) can
 * theoretically support faster moving tracks at a higher fidelity, but
 * can consume lots of cpu at varying rates.
 */
enum SMS_SpecEnvType
{
    SMS_ENV_NONE,  /*!< none */
    SMS_ENV_CEP,   /*!< cepstral coefficients */
    SMS_ENV_FBINS  /*!< frequency bins */
};

/*! \brief Error codes returned by SMS file functions */
/* \todo remove me */
enum SMS_ERRORS
{
    SMS_OK,      /*!< 0, no error*/
    SMS_NOPEN,   /*!< 1, couldn't open file */
    SMS_NSMS ,   /*!< 2, not a SMS file */
    SMS_MALLOC,  /*!< 3, couldn't allocate memory */
    SMS_RDERR,   /*!< 4, read error */
    SMS_WRERR,   /*!< 5, write error */
    SMS_SNDERR   /*!< 6, sound IO error */
};

/*! \brief debug modes
 *
 * \todo write details about debug files
 */
enum SMS_DBG
{
    SMS_DBG_NONE,        /*!< 0, no debugging */
    SMS_DBG_DET,         /*!< 1, not yet implemented \todo make this show main information to look at for  discovering the correct deterministic parameters*/
    SMS_DBG_PEAK_DET,	 /*!< 2, peak detection function */
    SMS_DBG_HARM_DET,	 /*!< 3, harmonic detection function */
    SMS_DBG_PEAK_CONT,   /*!< 4, peak continuation function */
    SMS_DBG_CLEAN_TRAJ,	 /*!< 5, clean tracks function */
    SMS_DBG_SINE_SYNTH,	 /*!< 6, sine synthesis function */
    SMS_DBG_STOC_ANAL,   /*!< 7, stochastic analysis function */
    SMS_DBG_STOC_SYNTH,  /*!< 8, stochastic synthesis function */
    SMS_DBG_SMS_ANAL,    /*!< 9, top level analysis function */
    SMS_DBG_ALL,         /*!< 10, everything */
    SMS_DBG_RESIDUAL,    /*!< 11, write residual to file */
    SMS_DBG_SYNC,        /*!< 12, write original, synthesis and residual to a text file */
};

#define SMS_MAX_WINDOW 8190    /*!< \brief maximum size for analysis window */

/* \brief type of sound to be analyzed
 *
 * \todo explain the differences between these two
 */
enum SMS_SOUND_TYPE
{
    SMS_SOUND_TYPE_MELODY, /*!< 0, sound composed of several notes */
    SMS_SOUND_TYPE_NOTE    /*!< 1, sound composed of a single note */
};

/* \brief direction of analysis
 *
 * Sometimes a signal can be clearer at the end than at
 * the beginning.  If the signal is very harmonic at the end then
 * doing the analysis in reverse could provide better results.
 */
enum SMS_DIRECTION
{
    SMS_DIR_FWD,   /*!< analysis from left to right */
    SMS_DIR_REV    /*!< analysis from right to left */
};

/* \brief window selection
*/
enum SMS_WINDOWS
{
    SMS_WIN_HAMMING,  /*!< 0: hamming */
    SMS_WIN_BH_62,    /*!< 1: blackman-harris, 62dB cutoff */
    SMS_WIN_BH_70,    /*!< 2: blackman-harris, 70dB cutoff */
    SMS_WIN_BH_74,    /*!< 3: blackman-harris, 74dB cutoff */
    SMS_WIN_BH_92,    /*!< 4: blackman-harris, 92dB cutoff */
    SMS_WIN_HANNING,  /*!< 5: hanning */
    SMS_WIN_IFFT      /*!< 6: window for deterministic synthesis based on the Inverse-FFT algorithm.
                              This is a combination of an inverse Blackman-Harris 92dB and a triangular window. */
};

/*!
 *  \brief frame status
 */
enum SMS_FRAME_STATUS
{
    SMS_FRAME_EMPTY,
    SMS_FRAME_READY,
    SMS_FRAME_PEAKS_FOUND,
    SMS_FRAME_FUND_FOUND,
    SMS_FRAME_TRAJ_FOUND,
    SMS_FRAME_CLEANED,
    SMS_FRAME_RECOMPUTED,
    SMS_FRAME_DETER_SYNTH,
    SMS_FRAME_STOC_COMPUTED,
    SMS_FRAME_DONE,
    SMS_FRAME_END
};


#define SMS_MIN_SIZE_FRAME  128   /* size of synthesis frame */

/*! \defgroup math_macros Math Macros
 *  \brief mathematical operations and values needed for functions within
 *   this library
 * \{
 */
#define PI 3.141592653589793238462643 /*!< pi */
#define TWO_PI 6.28318530717958647692 /*!< pi * 2 */
#define INV_TWO_PI (1 / TWO_PI)       /*!< 1 / ( pi * 2) */
#define PI_2 1.57079632679489661923   /*!< pi / 2 */
#define LOG2 0.69314718055994529      /*!< natural logarithm of 2 */
#define LOG10 2.3025850929940459      /*!< natural logarithm of 10 */
#define EXP 2.7182818284590451        /*!< Eurler's number */

SMS_EXPORT sfloat sms_magToDB(sfloat x);
SMS_EXPORT sfloat sms_dBToMag(sfloat x);
SMS_EXPORT void sms_arrayMagToDB(int sizeArray, sfloat *pArray);
SMS_EXPORT void sms_arrayDBToMag(int sizeArray, sfloat *pArray);
SMS_EXPORT void sms_setMagThresh(sfloat x);
SMS_EXPORT sfloat sms_rms ( int sizeArray, sfloat *pArray );
SMS_EXPORT sfloat sms_sine (sfloat fTheta);
SMS_EXPORT sfloat sms_sinc (sfloat fTheta);
SMS_EXPORT sfloat sms_random ( void );
SMS_EXPORT int sms_power2(int n);

SMS_EXPORT sfloat sms_scalarTempered( sfloat x);
SMS_EXPORT void sms_arrayScalarTempered( int sizeArray, sfloat *pArray);

#ifndef MAX
/*! \brief returns the maximum of a and b */
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
/*! \brief returns the minimum of a and b */
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif
/*! \} */

/* function declarations */
SMS_EXPORT int sms_analyze(int sizeWaveform, const sfloat *pWaveform, SMS_Data *pSmsFrame, SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_analyzeFrame(int iCurrentFrame, SMS_AnalParams *pAnalParams, sfloat fRefFundamental);

SMS_EXPORT int sms_init(void);

SMS_EXPORT void sms_free(void);

SMS_EXPORT int sms_initAnalysis( SMS_AnalParams *pAnalParams, const SMS_SndHeader *pSoundHeader);

SMS_EXPORT void sms_initAnalParams( SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_initSynthParams( SMS_SynthParams *synthParams);

SMS_EXPORT int sms_initSynth( const SMS_Header *pSmsHeader, SMS_SynthParams *pSynthParams);

SMS_EXPORT int sms_changeSynthHop( SMS_SynthParams *pSynthParams, int sizeHop);

SMS_EXPORT void sms_freeAnalysis( SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_freeSynth( SMS_SynthParams *pSynthParams);

SMS_EXPORT void sms_fillSoundBuffer(int sizeWaveform, const sfloat *pWaveform, SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_windowCentered(int sizeWindow, const sfloat *pWaveform, const sfloat *pWindow, int sizeFft, sfloat *pFftBuffer);

SMS_EXPORT void sms_getWindow(int sizeWindow, sfloat *pWindow, int iWindowType);

SMS_EXPORT void sms_scaleWindow(int sizeWindow, sfloat *pWindow);

SMS_EXPORT int sms_spectrum(int sizeWindow, const sfloat *pWaveform, const sfloat *pWindow, int sizeMag, sfloat *pMag, sfloat *pPhase, sfloat *pFftBuffer);

SMS_EXPORT int sms_invSpectrum(int sizeWaveform, sfloat *pWaveform, const sfloat *pWindow, int sizeMag, const sfloat *pMag, const sfloat *pPhase, sfloat *pFftBuffer);

SMS_EXPORT /* \todo remove this once invSpectrum is completely implemented */
SMS_EXPORT int sms_invQuickSpectrumW( const sfloat *pFMagSpectrum, const sfloat *pFPhaseSpectrum, int sizeFft, sfloat *pFWaveform, int sizeWave, const sfloat *pFWindow, sfloat *pFftBuffer);

SMS_EXPORT int sms_spectralApprox( const sfloat *pSpec1, int sizeSpec1, int sizeSpec1Used, sfloat *pSpec2, int sizeSpec2, int nCoefficients, sfloat *envelope);

SMS_EXPORT int sms_spectrumMag(int sizeWindow, const sfloat *pWaveform, const sfloat *pWindow, int sizeMag, sfloat *pMag, sfloat *pFftBuffer);

SMS_EXPORT void sms_dCepstrum(int sizeCepstrum, sfloat *pCepstrum, int sizeFreq, const sfloat *pFreq, const sfloat *pMag, sfloat fLambda, int iSamplingRate);

SMS_EXPORT void sms_dCepstrumEnvelope(int sizeCepstrum, const sfloat *pCepstrum, int sizeEnv, sfloat *pEnv);

SMS_EXPORT void sms_spectralEnvelope( SMS_Data *pSmsData, const SMS_SEnvParams *pSpecEnvParams);

SMS_EXPORT int sms_sizeNextWindow(int iCurrentFrame, const SMS_AnalParams *pAnalParams);

SMS_EXPORT sfloat sms_fundDeviation( const SMS_AnalParams *pAnalParams, int iCurrentFrame);

SMS_EXPORT int sms_detectPeaks(int sizeSpec, const sfloat *pFMag, const sfloat *pPhase, SMS_Peak *pSpectralPeaks, const SMS_PeakParams *pPeakParams);

SMS_EXPORT void sms_harmDetection( SMS_AnalFrame *pFrame, sfloat fRefFundamental, const SMS_PeakParams *pPeakParams);

SMS_EXPORT int sms_peakContinuation(int iFrame, const SMS_AnalParams *pAnalParams);

SMS_EXPORT sfloat sms_preEmphasis(sfloat fInput, SMS_AnalParams *pAnalParams);

SMS_EXPORT sfloat sms_deEmphasis(sfloat fInput, SMS_SynthParams *pSynthParams);

SMS_EXPORT void sms_cleanTracks(int iCurrentFrame, SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_scaleDet( const sfloat *pSynthBuffer, const sfloat *pOriginalBuffer, sfloat *pSinAmp, const SMS_AnalParams *pAnalParams, int nTracks);

SMS_EXPORT int sms_prepSine(int nTableSize);

SMS_EXPORT int sms_prepSinc(int nTableSize);

SMS_EXPORT void sms_clearSine(void);

SMS_EXPORT void sms_clearSinc(void);

SMS_EXPORT void sms_synthesize( SMS_Data *pSmsFrame, sfloat *pSynthesis, SMS_SynthParams *pSynthParams);

SMS_EXPORT void sms_sineSynthFrame( const SMS_Data *pSmsFrame, sfloat *pBuffer, int sizeBuffer, SMS_Data *pLastFrame, int iSamplingRate);

SMS_EXPORT void sms_initHeader( SMS_Header *pSmsHeader);

SMS_EXPORT int sms_getHeader( const char *pChFileName, SMS_Header **ppSmsHeader, FILE **ppInputFile);

SMS_EXPORT void sms_fillHeader( SMS_Header *pSmsHeader, const SMS_AnalParams *pAnalParams, const char *pProgramString);

SMS_EXPORT int sms_writeHeader( const char *pFileName, SMS_Header *pSmsHeader, FILE **ppOutSmsFile);

SMS_EXPORT int sms_writeFile( FILE *pSmsFile, SMS_Header *pSmsHeader);

SMS_EXPORT int sms_initFrame(int iCurrentFrame, SMS_AnalParams *pAnalParams, int sizeWindow);

SMS_EXPORT int sms_clearAnalysisFrame(int iCurrentFrame, SMS_AnalParams *pAnalParams);

SMS_EXPORT int sms_allocFrame( SMS_Data *pSmsFrame, int nTracks, int nCoeff, int iPhase, int stochType, int nEnvCoeff);

SMS_EXPORT int sms_allocFrameH( const SMS_Header *pSmsHeader, SMS_Data *pSmsFrame);

SMS_EXPORT int sms_getFrame( FILE *pInputFile, SMS_Header *pSmsHeader, int iFrame, SMS_Data *pSmsFrame);

SMS_EXPORT int sms_writeFrame( FILE *pSmsFile, const SMS_Header *pSmsHeader, const SMS_Data *pSmsFrame);

SMS_EXPORT void sms_freeFrame( SMS_Data *pSmsFrame);

SMS_EXPORT void sms_clearFrame( SMS_Data *pSmsFrame);

SMS_EXPORT void sms_copyFrame( SMS_Data *pCopySmsFrame, const SMS_Data *pOriginalSmsFrame);

SMS_EXPORT int sms_frameSizeB( const SMS_Header *pSmsHeader);

SMS_EXPORT int sms_residual(int sizeWindow, const sfloat *pSynthesis, const sfloat *pOriginal, sfloat *pResidual, const sfloat *pWindow);

SMS_EXPORT void sms_filterHighPass(int sizeResidual, sfloat *pResidual, int iSamplingRate);

SMS_EXPORT int sms_stocAnalysis(int sizeWindow, const sfloat *pResidual, const sfloat *pWindow, SMS_Data *pSmsFrame, SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_interpolateFrames( const SMS_Data *pSmsFrame1, const SMS_Data *pSmsFrame2, SMS_Data *pSmsFrameOut, sfloat fInterpFactor);

SMS_EXPORT int sms_openSF( const char *pChInputSoundFile, SMS_SndHeader *pSoundHeader);

SMS_EXPORT void sms_closeSF();

SMS_EXPORT int sms_getSound( const SMS_SndHeader *pSoundHeader, long sizeSound, sfloat *pSound, long offset, SMS_AnalParams *pAnalParams);

SMS_EXPORT int sms_createSF( const char *pChOutputSoundFile, int iSamplingRate, int iType);

SMS_EXPORT void sms_writeSound( const sfloat *pBuffer, int sizeBuffer);

SMS_EXPORT void sms_writeSF(void);

SMS_EXPORT void sms_fft(int sizeFft, sfloat *pArray);

SMS_EXPORT void sms_ifft(int sizeFft, sfloat *pArray);

SMS_EXPORT void sms_RectToPolar(int sizeSpec, const sfloat *pReal, sfloat *pMag, sfloat *pPhase);

SMS_EXPORT void sms_PolarToRect(int sizeSpec, sfloat *pReal, const sfloat *pMag, const sfloat *pPhase);

SMS_EXPORT void sms_spectrumRMS(int sizeMag, const sfloat *pInRect, sfloat *pOutMag);

SMS_EXPORT void sms_initModify( const SMS_Header *header, SMS_ModifyParams *params);

SMS_EXPORT void sms_initModifyParams( SMS_ModifyParams *params);

SMS_EXPORT void sms_freeModify( SMS_ModifyParams *params);

SMS_EXPORT void sms_modify( SMS_Data *frame, const SMS_ModifyParams *params);

SMS_EXPORT /***********************************************************************************/
SMS_EXPORT /************* debug functions: ******************************************************/

SMS_EXPORT int sms_createDebugFile ( const SMS_AnalParams *pAnalParams);

SMS_EXPORT void sms_writeDebugData ( const sfloat *pBuffer1, const sfloat *pBuffer2, const sfloat *pBuffer3, int sizeBuffer);

SMS_EXPORT void sms_writeDebugFile ( void );

SMS_EXPORT void sms_error( const char *pErrorMessage );

SMS_EXPORT int sms_errorCheck( void );

SMS_EXPORT char* sms_errorString( void );

#endif /* _SMS_H */
