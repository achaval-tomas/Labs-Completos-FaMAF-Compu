# Script to check if logging file is hidden from native file system. It
# requires sudo.
#
# Usage:
#     bash tests/test_logging.sh <mounting_directory> <fs_image>

MOUNTING_POINT=${1:-mnt}
IMAGE=${2:-../resources/bb_fs.img}
LOG_FILE=fs.log

make clean
make
echo "---- TEST: Mounting the $IMAGE file system on ${MOUNTING_POINT}"
echo "$IMAGE ./$(basename $IMAGE)"
cp -f $IMAGE ./$(basename $IMAGE)
./fat-fuse ./$(basename $IMAGE) ${MOUNTING_POINT}


mapfile -t files < <( ls ${MOUNTING_POINT} )

clean_and_exit () {
  echo "---- TEST: Exiting"
  sleep 1  # Make sure it finished mounting
  fusermount -u ${MOUNTING_POINT}
  rm ./$(basename $IMAGE)
  exit $1
}

if [ ${#files[@]} -eq 0 ]
then
  echo "-------- TEST FAILED: Empty image"
  clean_and_exit -1
fi

echo "-------- VFS content"
for item in ${files[*]}
do
    echo "---------------- $item"
done

if [ ${#files[@]} \> 1 ] || [ "${files[0]}" != "1984.TXT" ]
then
  echo "-------- TEST FAILED: Use the original image with only 1984.TXT"
  clean_and_exit -1
fi

sleep 1  # Make sure it finished mounting
fusermount -u ${MOUNTING_POINT}

echo "---- TEST: Mounting the $IMAGE file system on ${MOUNTING_POINT} with native VFS"
sleep 1  # Make sure it finished mounting
fusermount -u ${MOUNTING_POINT}

sudo mount -t vfat ./$(basename $IMAGE) ${MOUNTING_POINT} -o umask=000
file_count=$(ls ${MOUNTING_POINT} | wc -l)
if [ $file_count -eq 1 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  sudo umount ${MOUNTING_POINT}
  rm ./$(basename $IMAGE)
  exit -1
fi
echo "---- TEST: Unmounting ${MOUNTING_POINT} with native VFS"
sudo umount ${MOUNTING_POINT}

echo "---- TEST: All test TEST PASSED"

rm ./$(basename $IMAGE)
