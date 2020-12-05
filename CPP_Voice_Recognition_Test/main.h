#pragma once

ps_decoder_t* ps; // Pocket Spinx speech decoder
cmd_ln_t* config; // Spinx config
ad_rec_t* ad; // create audio recording structure - for use with ALSA functions

int fails = 0;
bool passwordSuccess = false;
bool setupComplete = false;
int16 adbuf[4096]; // buffer array to hold audio data
uint8 utt_started, in_speech;      // flags for tracking active speech - has speech started? - is speech currently happening?
int32 k;                           // holds the number of frames in the audio buffer
int32 score;
float64 confidence;
char const* hyp; // Stores the hypothesis of what the user said
char const* decoded_speech; // Stores the final prediction of what the user said

bool DoSetup(); // Handles initialization of the program
void DoCleanup(); // Handles memory cleanup after the program is finished
const char* PredictTextFromMicrophone(); // Gets a prediction of what was said in the audio stream coming from the default microphone
bool PromptForAudioPassword(const char* passwords[], int numPasswords); // Writes a prompt to the console asking for user's audio password
bool StringContainsString(const char* w1, const char* w2); // Checks if a string matches another string.