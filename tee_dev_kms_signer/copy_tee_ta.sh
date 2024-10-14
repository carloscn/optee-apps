#!/bin/bash

export UDISK=/media/${USER}

if [ ! -d "${UDISK}/rootfs/" ]; then
    echo "[ERROR] Directory ${UDISK}/rootfs/ does not exist. Exiting."
    exit 1
fi

echo "[INFO] Copying optee ta..."
sudo cp -v host/kms_signer ${UDISK}/rootfs/usr/bin/ && \
sudo cp -v ta/c7ef55ac-a96c-495d-9c7c-9fd99af847e1.ta ${UDISK}/rootfs/lib/optee_armtz && \
sudo cp -v keys/public_key.pem ${UDISK}/rootfs/home/root/ && \
sudo cp -v test_script_on_device.sh ${UDISK}/rootfs/home/root/
if [ $? -eq 0 ]; then
    echo "[INFO] optee ta copied successfully!"
else
    echo "[ERROR] Failed to copy the optee ta."
    exit 1
fi

sync
