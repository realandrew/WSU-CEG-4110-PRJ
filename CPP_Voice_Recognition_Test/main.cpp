#include <iostream>
#include <pocketsphinx.h> // Pocket Spinx is the speech recognizer library

#define MODELDIR "model" // Where the speech recognition model (english) is stored

int main(int argc, char* argv[])
{
	std::cout << "App starting..." << std::endl;
	
	ps_decoder_t* ps; // Pocket Spinx speech decoder
	cmd_ln_t* config; // Spinx config
	FILE* fh; // Audio file handle
	char const* hyp; // Recognition hypothesis
	int16 buf[512]; // Current block of audio file
	int rv; // Working result variable
	int32 score; // Hypothesis "score" (how likely it thinks it got it's prediction right).

	config = cmd_ln_init(
		NULL, // Updates existing cmd_ln_t*
		ps_args(), // Use standard argument definitions
		TRUE, // Strict argument parsing
		"-hmm", MODELDIR "/en-us/en-us", // Speech recognition model location
		"-lm", MODELDIR "/en-us/en-us.lm.bin", // Speech recognition model file
		"-dict", MODELDIR "/en-us/cmudict-en-us.dict", // Speech recognition dictionary
		NULL); // Finish supplying arguments

	if (config == NULL) { // Error and exit if config is invalid
		fprintf(stderr, "Failed to create config object, see log for details\n");
		return -1;
	}

	ps = ps_init(config); // Initialize decoder
	if (ps == NULL) { // Error and exit if decoder fails to be created
		fprintf(stderr, "Failed to create recognizer, see log for details\n");
		return -1;
	}

	fh = fopen("goforward.raw", "rb"); // Open audio file. Must be mono-channel, little-endian, headerless (raw), signed 16-bit PCM at 16000 hz
	if (fh == NULL) { // Error and exit if reading the audio file fails
		fprintf(stderr, "Unable to open input file goforward.raw\n");
		return -1;
	}

	rv = ps_start_utt(ps); // Start decoding the "utterance" (speech)

    while (!feof(fh)) { // Read the file 512 blocks at a time
	size_t nsamp; // Current sample
	nsamp = fread(buf, 2, 512, fh); // Read current block
	rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE); // Process current block
    }

    rv = ps_end_utt(ps); // End "utterance" (recognition)
    hyp = ps_get_hyp(ps, &score); // Get hypthosis of what words were said
    printf("Recognized: %s\n", hyp);

    fclose(fh); // Close file
    ps_free(ps); // Free up the memory used by the speech recognizer
    cmd_ln_free_r(config); // Free up the memory used by the spinx config object

	return 0; // Return successful exit
}