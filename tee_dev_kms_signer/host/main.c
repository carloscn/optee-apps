/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <tee_kms_signer.h>

#define MAX_BUFFER_SIZE 4096
#if 0
int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_TEE_KMS_SIGNER_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 42;

	/*
	 * TA_TEE_KMS_SIGNER_CMD_INC_VALUE is the actual function in the TA to be
	 * called.
	 */
	printf("Invoking TA to increment %d\n", op.params[0].value.a);
	res = TEEC_InvokeCommand(&sess, TA_TEE_KMS_SIGNER_CMD_INC_VALUE, &op,
				 &err_origin);
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	printf("TA incremented value to %d\n", op.params[0].value.a);

	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}
#endif

// Static function: Executes the signing logic
static int perform_sign(const char *input_file,
						const char *output_file,
						const char *digest,
						size_t key_length,
						int key_id)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TEE_KMS_SIGNER_UUID;
    uint32_t err_origin;
    char data[MAX_BUFFER_SIZE];
    size_t data_size;

    // Read input file data
    FILE *fp = fopen(input_file, "rb");
    if (!fp) {
        perror("Failed to open input file");
        return -1;
    }
    data_size = fread(data, 1, MAX_BUFFER_SIZE, fp);
    fclose(fp);

    // Initialize TEE context and session
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    }

    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_OpenSession failed with code 0x%x origin 0x%x", res, err_origin);
    }

    // Set up the operation parameters
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_VALUE_INPUT, TEEC_NONE);
    op.params[0].tmpref.buffer = data;
    op.params[0].tmpref.size = data_size;

    char signed_data[MAX_BUFFER_SIZE];
    op.params[1].tmpref.buffer = signed_data;
    op.params[1].tmpref.size = sizeof(signed_data);

    op.params[2].value.a = key_id;
    op.params[2].value.b = key_length;

    // Invoke the command to sign the data
    res = TEEC_InvokeCommand(&sess, TA_TEE_KMS_SIGNER_CMD_SIGN, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        fprintf(stderr, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x\n", res, err_origin);
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        return -1;
    }

    // Write the signed data to the output file
    FILE *out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        perror("Failed to open output file");
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        return -1;
    }
    fwrite(signed_data, 1, op.params[1].tmpref.size, out_fp);
    fclose(out_fp);

    printf("Signed data successfully written to %s\n", output_file);

    // Close the TEE session and context
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    return 0;
}

int main(int argc, char *argv[])
{
    char *input_file = NULL;
    char *output_file = NULL;
    int key_id = 0;
    char *digest = "sha256";
    size_t key_length = 2048;

    // Argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            digest = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            key_length = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            key_id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            fprintf(stderr, "Usage: %s -f <input_file> -s <digest> -l <key_length> -k <key_id> -o <output_file>\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Check if all necessary parameters are provided
    if (input_file == NULL || output_file == NULL) {
        fprintf(stderr, "Error: Input file and output file must be specified with -f and -o respectively.\n");
        return EXIT_FAILURE;
    }

    // Call the signing function
    int result = perform_sign(input_file, output_file, digest, key_length, key_id);
    if (result != 0) {
        fprintf(stderr, "Error: Sign operation failed.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
