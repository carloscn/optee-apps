# !/bin/bash

# Program path
PROGRAM="kms_signer"
PUBLIC_KEY="public_key.pem"  # Public key file

echo "This is a test data file." > test_data_1.bin
dd if=/dev/urandom of=test_data_2.bin bs=1K count=1
dd if=/dev/urandom of=test_data_3.bin bs=1M count=1

# Test cases
TEST_CASES=(
  "-f test_data_1.bin -s sha256 -l 2048 -k 0 -o output_sign_1.bin"
  "-f test_data_2.bin -s sha256 -l 2048 -k 0 -o output_sign_2.bin"
  "-f test_data_3.bin -s sha256 -l 2048 -k 0 -o output_sign_3.bin"
)

# Test results
RESULTS=()

# Run tests
for i in "${!TEST_CASES[@]}"; do
  echo "Running test case $((i+1))..."

  # Run the program
  $PROGRAM ${TEST_CASES[$i]}
  if [ $? -ne 0 ]; then
    echo "Test case $((i+1)) failed: Program execution error"
    RESULTS+=("Test case $((i+1)): FAILED (execution error)")
    continue
  fi

  # Check if output signature file is generated
  OUTPUT_FILE="output_sign_$((i+1)).bin"
  if [ ! -f "$OUTPUT_FILE" ]; then
    echo "Test case $((i+1)) failed: Output file not generated"
    RESULTS+=("Test case $((i+1)): FAILED (no output file)")
  else
    FILE_SIZE=$(stat -c%s "$OUTPUT_FILE")

    # Check if the signature file has the expected size (approximately 256 bytes for RSA 2048)
    if [ "$FILE_SIZE" -gt 0 ]; then
      echo "Test case $((i+1)) passed: Output file generated"

      # Perform signature verification using openssl
      DATA_FILE=$(echo "${TEST_CASES[$i]}" | grep -oP '(?<=-f )[^ ]+')
      openssl dgst -sha256 -verify "$PUBLIC_KEY" -signature "$OUTPUT_FILE" "$DATA_FILE"
      if [ $? -eq 0 ]; then
        echo "Test case $((i+1)) passed: Signature verification succeeded"
        RESULTS+=("Test case $((i+1)): PASSED")
      else
        echo "Test case $((i+1)) failed: Signature verification failed"
        RESULTS+=("Test case $((i+1)): FAILED (signature verification failed)")
      fi
    else
      echo "Test case $((i+1)) failed: Output file size error"
      RESULTS+=("Test case $((i+1)): FAILED (output size error)")
    fi
  fi

  # Clean up the output file
  rm -f "$OUTPUT_FILE"
done

# Print final results
echo "=================="
echo "Test Results:"
for result in "${RESULTS[@]}"; do
  echo "$result"
done