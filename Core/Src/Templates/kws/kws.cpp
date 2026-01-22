#include "kws.h"

/*
	Constructor for the keyword spotting system
*/
KWS::KWS() : network(AI_HANDLE_NULL) {
  ai_error ai_err;
  ai_err = ai_network_create(&network, AI_NETWORK_DATA_CONFIG);
  if (ai_err.type != AI_ERROR_NONE) {
	  printf("E: AI error - type=%d code=%d\r\n", ai_err.type, ai_err.code);
    return;
  }
  printf("NN created...\r\n");

  aiInit();

  ai_input[0] = *AI_NETWORK_IN;
  ai_output[0] = *AI_NETWORK_OUT;

}

/*
	The network need to be initialized once before it can be used for the inference.
*/
int KWS::aiInit() {
	ai_error ai_err;
	printf("Initializing NN...\r\n");
	ai_network_params params = { AI_NETWORK_DATA_WEIGHTS(ai_network_data_weights_get()), AI_NETWORK_DATA_ACTIVATIONS(activations) };
	if (!ai_network_init(network, &params)) {
		ai_err = ai_network_get_error(network);
		printf("E: AI error - type=%d code=%d\r\n", ai_err.type, ai_err.code);
		return 1;
	} else {
		printf("Initialization successful\r\n");
	}
	return 0;
}

/*
	Prints a report of different statistics of the network
*/
void KWS::printReport() {
  // Obtain report
  ai_network_report report;
  ai_network_get_info(network, &report); // returns result -> not used

	printf("-------------NETWORK REPORT---------------------------\r\n");
	printf("\r\nmodel_name:\t\t%s\r\n", report.model_name);
	printf("model_signature:\t%s\r\n", report.model_signature);
	printf("model_datetime:\t\t%s\r\n", report.model_datetime);
	printf("runtime_revision:\t%s\r\n", report.runtime_revision);

	printf("\r\nai_platform_version: \r\n");
	printf("major:\t\t%d\r\n", report.runtime_version.major);
	printf("minor:\t\t%d\r\n", report.runtime_version.minor);
	printf("micro:\t\t%d\r\n", report.runtime_version.micro);
	printf("reserved:\t%d\r\n", report.runtime_version.reserved);

	printf("\r\ntool_revision:\t%s\r\n", report.tool_revision);

	printf("\r\ntool_version: \r\n");
	printf("major:\t\t%d\r\n", report.tool_version.major);
	printf("minor:\t\t%d\r\n", report.tool_version.minor);
	printf("micro:\t\t%d\r\n", report.tool_version.micro);
	printf("reserved:\t%d\r\n", report.tool_version.reserved);

	printf("\r\ntool_api_version: \r\n");
	printf("major:\t\t%d\r\n", report.tool_api_version.major);
	printf("minor:\t\t%d\r\n", report.tool_api_version.minor);
	printf("micro:\t\t%d\r\n", report.tool_api_version.micro);
	printf("reserved:\t%d\r\n", report.tool_api_version.reserved);

	printf("\r\napi_version: \r\n");
	printf("major:\t\t%d\r\n", report.api_version.major);
	printf("minor:\t\t%d\r\n", report.api_version.minor);
	printf("micro:\t\t%d\r\n", report.api_version.micro);
	printf("reserved:\t%d\r\n", report.api_version.reserved);

	printf("\r\ninterface_api_version: \r\n");
	printf("major:\t\t%d\r\n", report.interface_api_version.major);
	printf("minor:\t\t%d\r\n", report.interface_api_version.minor);
	printf("micro:\t\t%d\r\n", report.interface_api_version.micro);
	printf("reserved:\t%d\r\n", report.interface_api_version.reserved);

	printf("\r\nn_macc:\t\t%d\r\n", (int) report.n_macc);

	printf("\r\nn_inputs:\t%d\r\n", report.n_inputs);
	printf("n_inputs:\t%d\r\n", report.n_inputs);

	printf("\r\ninputs: \r\n");
	printf("format:\t\t%d\r\n", (int) report.inputs->format);
	printf("flags:\t\t%d\r\n", (int) report.inputs->flags);
	printf("size:\t\t%d\r\n", (int) report.inputs->size);

	printf("\r\noutputs: \r\n");
	printf("format:\t\t%d\r\n", (int) report.outputs->format);
	printf("flags:\t\t%d\r\n", (int) report.outputs->flags);
	printf("size:\t\t%d\r\n", (int) report.outputs->size);


	printf("n_nodes:\t%d\r\n", (int) report.n_nodes);
	printf("signature:\t%d\r\n", (int) report.signature);
	printf("------------------------------------------------------\r\n");
}

// TODO Vervollständige diese Funktion
/*
	Runs the inference and returns index of detected word. When being called, the batch_size can be chosen as 1
*/
int KWS::runInference(int batch_size) {
  ai_i32 nbatch;
	ai_error ai_err;

	ai_input[0].data = AI_HANDLE_PTR(mMFCC);
	ai_output[0].data = AI_HANDLE_PTR(out_data);

	//////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// TODO füge hier deinen Code ein ////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////




	//////////////////////////////////////////////////////////////////////////////////////

	float max_value = out_data[0];
	uint32_t max_index = 0;
	for (uint32_t i = 0; i < AI_NETWORK_OUT_1_SIZE; i++) {
		//printf("%lu: %f\r\n", (unsigned long) i, out_data[i]);
		// find maximum
		if (i > 0) {
			if (max_value < out_data[i]) {
				max_value = out_data[i];
				max_index = i;
			}
		}
	}
	return max_index;
}

std::string KWS::indexToWord(int index) {
  switch (index) {
    case 0: return "YES";
    case 1: return "NO";
    case 2: return "ON";
    case 3: return "OFF";
    case 4: return "ONE";
    case 5: return "TWO";
    case 6: return "THREE";
    case 7: return "UNKNOWN";
    default: return "_ERROR_";
	}
}