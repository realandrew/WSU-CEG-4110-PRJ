#include <iostream>
#include <stdio.h>
#include <string.h>
#include <pocketsphinx.h> // Pocket Spinx is the speech recognizer library
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include "main.h"

#define MODELDIR "model" // Where the speech recognition model (english) is stored

int main(int argc, char* argv[])
{
	printf("Program starting...\n");

	if (!DoSetup()) { // Initialize program
		return -1;
	}

	const char* passwords[] = { // 
		"password",
		"open sesame"
	};
	bool success = PromptForAudioPassword(passwords, 2); // Prompt user for password

	printf("Success? %s\n", success ? "true" : "false");

	DoCleanup(); // Cleanup program before closing

	printf("Program exitting...\n");
	do {
		std::cout << "Press enter when ready to end program...";
	} while (std::cin.get() != '\n');

	return 0; // Successful exit
}

const char* PredictTextFromMicrophone() {

	ad_start_rec(ad); // Start recording on audio input
	ps_start_utt(ps); // Mark the start of utterance (speech)
	utt_started = FALSE; // Clear the utt_started flag

	while (1) { // Loop until speech is detected
		k = ad_read(ad, adbuf, 4096); // Capture the number of frames in the audio buffer
		ps_process_raw(ps, adbuf, k, FALSE, FALSE); // Send the audio buffer to the pocketsphinx decoder

		in_speech = ps_get_in_speech(ps); // Test to see if speech is being detected

		if (in_speech && !utt_started) { // If speech has started and utt_started flag is false                           
			utt_started = TRUE;                      // Then set the flag
		}

		if (!in_speech && utt_started) {             // If speech has ended and the utt_started flag is true
			ps_end_utt(ps);                          // Then mark the end of the utterance
			ad_stop_rec(ad);                         // Stop recording from audio input
			hyp = ps_get_hyp(ps, &score);             // Query pocketsphinx for its "hypothesis" of what the user said
			confidence = logmath_exp(ps_get_logmath(ps), score); // Store it's confidence in it's guess (as a float, multiply by 100 if displaying as percentage)
			return hyp;                              // The function returns the hypothesis
			break;                                   // Exit the while loop and return to main
		}
	}
}

/*
* Function that returns if one string contains another string.
* Used to detect if password was in the sentence said by the user.
*/
bool StringContainsString(const char* w1, const char* w2)
{
	int i = 0;
	int j = 0;

	while (w1[i] != '\0') { // Loop until end of line
		if (w1[i] == w2[j])
		{
			int init = i;
			while (w1[i] == w2[j] && w2[j] != '\0')
			{
				j++;
				i++;
			}
			if (w2[j] == '\0') {
				return true;
			}
			j = 0;
		}
		i++;
	}
	return false;
}

/**
 * Initializes/configures the program. Must be called before prompting for audio password.
 */
bool DoSetup()
{
	// Setup PocketSpinx's configuration
	config = cmd_ln_init(
		NULL, // Updates existing cmd_ln_t*
		ps_args(), // Use standard argument definitions
		TRUE, // Strict argument parsing
		"-hmm", MODELDIR "/en-us/en-us", // Speech recognition model location
		"-lm", MODELDIR "/en-us/en-us.lm.bin", // Speech recognition model file
		"-dict", MODELDIR "/en-us/cmudict-en-us.dict", // Speech recognition dictionary
		"-logfn", "log.txt", // Suppress logging (not needed unless debugging the speech recognition library itself)
		NULL); // Finished supplying arguments

	if (config == NULL) { // Error and exit if config is invalid
		fprintf(stderr, "Failed to create config object, see log for details\n");
		return false;
	}

	ps = ps_init(config); // Initialize decoder
	if (ps == NULL) { // Error and exit if decoder fails to be created
		fprintf(stderr, "Failed to create recognizer, see log for details\n");
		return false;
	}

	ad = ad_open_dev("sysdefault", (int)cmd_ln_float32_r(config, "-samprate")); // Open the system's default microphone at the default system sample-rate

	setupComplete = true;
	return true;
}

/*
* Cleans up the program memory. Must be called if DoSetup was called.
*/
void DoCleanup()
{
	ad_close(ad); // Close the audio input handle
	cmd_ln_free_r(config); // Free up the memory used by the spinx config object
}

/*
* Prompt the user for an audio/voice password.
* Allows three attempts, failing all three results in a system lock.
* Success occurs if the speech recognizer detects the password within the said sentence with a confidence of >= 70%.
* Note: If numPasswords is more than the amount of passwords supplied in the passwords array, an out of bounds exception will occur.
* Returns a boolean - true if user successfully said the password, false if the system should lock the user out.
*/
bool PromptForAudioPassword(const char* passwords[], int numPasswords)
{
	const char* failReason = "";
	if (!setupComplete) { // Make sure setup was called, otherwise we'd encounter errors.
		printf("Must call DoSetup before you call PromptForAudioPassword!");
		return false;
	}

	// Check for password, fails after 3 unsuccessful attempts by the user.
	while (fails < 3 && !passwordSuccess)
	{
		do {
			std::cout << "Press enter when ready to say password...";
		} while (std::cin.get() != '\n');
		printf("Please say the password\n");
		decoded_speech = PredictTextFromMicrophone(); // Capture and decode speech          
		printf("You Said (%.2f%%): %s\n", confidence * 100, decoded_speech); // Print speech prediction to console
		passwordSuccess = false;
		for (int i = 0; i < numPasswords; i++) // Check against each supplied password
		{
			passwordSuccess = StringContainsString(passwords[i], decoded_speech); // Check if password is in said sentence
			printf("Matching against password %d... %s\n", (i+1), passwordSuccess ? "pass" : "fail"); // Log match (best to turn this off in production for security reasons).
			if (passwordSuccess) { // If successful, no need to continue looping through passwords
				break;
			}
		}
		
		if (!passwordSuccess || (confidence * 100) < 70.0f) { // Password must match and the confidence it does must be >= 70.0%
			if (!passwordSuccess) {
				failReason = "Incorrect password.";
			}
			else {
				failReason = "Confidence is too low (< 70%).";
			}
			fails++;
			passwordSuccess = false;
		}
		printf("Password Match: %s\n", passwordSuccess ? "true" : "false");  // Log match (best to turn this off in production for security reasons).
		if (!passwordSuccess) {
			printf("Failed because: %s\n", failReason);
		}
	}

	return passwordSuccess || false; // Return result
}