# Script to run checks on the Big Brother logging system.
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

# Test if we can read the file
echo "---- TEST: Reading file"
counts=$(grep "Project Gutenberg" ${MOUNTING_POINT}/1984.TXT | wc -l)
if [ "$counts" == "6" ]
then
  echo "-------- TEST PASSED: Congratulations, you didn't break anything!"
else
  echo "-------- TEST FAILED: Not reading file correctly"
  clean_and_exit -1
fi

echo "---- TEST: Testing log file"

TEST_DESCRIPTION="Reading log file"
cat ${MOUNTING_POINT}/${LOG_FILE}
exit_code=$?
if [ $exit_code -eq 0 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Hidding file from ls"
file_count=$(ls ${MOUNTING_POINT} | grep "${LOG_FILE}" | wc -l)
if [ $file_count -eq 0 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Logging READ operations"
$(head ${MOUNTING_POINT}/1984.TXT)
file_count=$(grep -a "1984.TXT" ${MOUNTING_POINT}/${LOG_FILE} | wc -l)
if [ $file_count -ge 1 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Logging WRITE operations"
$(echo "lala" >> ${MOUNTING_POINT}/1984.TXT)
file_count=$(grep --binary-files=text "1984.TXT" ${MOUNTING_POINT}/${LOG_FILE} | wc -l)
if [ $file_count -ge 2 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

# Unmount and remount
echo "---- TEST: Re-mounting the $IMAGE file system on ${MOUNTING_POINT}"
sleep 1  # Make sure it finished mounting
fusermount -u ${MOUNTING_POINT}

sleep 2  # Make sure it finished mounting
./fat-fuse ./$(basename $IMAGE) ${MOUNTING_POINT}

TEST_DESCRIPTION="Hidding file from ls after re-mount"
file_count=$(ls ${MOUNTING_POINT} | wc -l)
if [ $file_count -eq 1 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Log file exists after re-mount"
file_count=$(grep --binary-files=text "1984.TXT" ${MOUNTING_POINT}/${LOG_FILE} | wc -l)
if [ $file_count -ge 2 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

echo "---- TEST: All test TEST PASSED"

rm ./$(basename $IMAGE)
