# !/bin/bash

echo "This is a test data file." > test_data_1.bin
dd if=/dev/urandom of=test_data_2.bin bs=1K count=1
dd if=/dev/urandom of=test_data_3.bin bs=1M count=1

kms_signer -f test_data_1.bin -s sha256 -l 2048 -k 0 -o ./output_sign_1.bin
openssl dgst -sha256 -verify public_key.pem -signature output_sign_1.bin test_data_1.bin

kms_signer -f test_data_2.bin -s sha256 -l 2048 -k 0 -o ./output_sign_2.bin
openssl dgst -sha256 -verify public_key.pem -signature output_sign_2.bin test_data_2.bin

kms_signer -f test_data_3.bin -s sha256 -l 2048 -k 0 -o ./output_sign_3.bin
openssl dgst -sha256 -verify public_key.pem -signature output_sign_3.bin test_data_3.bin