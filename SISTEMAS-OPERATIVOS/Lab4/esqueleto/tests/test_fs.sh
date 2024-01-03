# Script to run checks on the read and write operations.
#
# Usage:
#     bash tests/test_write.sh <mounting_directory> <fs_image>


MOUNTING_POINT=${1:-mnt}
IMAGE=${2:-../resources/bb_fs.img}

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

NOW=$(date +%s)
TODAY=$(date '+%Y-%m-%d')
# There can be a time zone difference because date uses UTC
TOMORROW=$(date -d '+1 day' '+%Y-%m-%d')

echo "---- TEST: Writing"

TEST_DESCRIPTION="Read file and update last access date"
access_date=$(stat --format %x mnt/1984.TXT | cut -d' ' -f1)
if [ "$access_date" == "$TODAY" ] || [ "$access_date" == "$TOMORROW" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Create files"
touch ${MOUNTING_POINT}/newfile
exit_code=$?
if [ $exit_code -eq 0 ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Create files and update creation time"
touch ${MOUNTING_POINT}/newfile1
modified_time=$(date +%s -r ${MOUNTING_POINT}/newfile1)
if [ $((modified_time + 0)) -ge $((NOW + 0)) ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo $NOW
  echo $modified_time
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Write files with one cluster"
DATA="Enough data for cluster 1"
echo $DATA > ${MOUNTING_POINT}/newfile2
counts=$(grep "$DATA" ${MOUNTING_POINT}/newfile2 | wc -l)
if [ "$counts" == "1" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Write files with multiple clusters"
yes | head -n 1024 > ${MOUNTING_POINT}/newfile3
counts=$(grep "y" ${MOUNTING_POINT}/newfile3 | wc -l)
if [ "$counts" == "1024" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Write files in sub-directories"
DATA="Writing in sub dirs"
mkdir ${MOUNTING_POINT}/newdir
echo $DATA > ${MOUNTING_POINT}/newdir/newfile2
counts=$(grep "$DATA" ${MOUNTING_POINT}/newdir/newfile2 | wc -l)
if [ "$counts" == "1" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Write existing file that had one cluster"
DATA="Write existing file"
echo "lala" > ${MOUNTING_POINT}/newfile4
echo $DATA > ${MOUNTING_POINT}/newfile4
counts=$(grep "$DATA" ${MOUNTING_POINT}/newfile4 | wc -l)
if [ "$counts" == "1" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Write existing file that had multiple clusters"
yes | head -n 1024 > ${MOUNTING_POINT}/newfile5
DATA="Write existing file"
echo $DATA > ${MOUNTING_POINT}/newfile5
counts=$(grep "$DATA" ${MOUNTING_POINT}/newfile5 | wc -l)
if [ "$counts" == "1" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Truncate file that had multiple clusters"
yes | head -n 1024 > ${MOUNTING_POINT}/newfile6
truncate -s 10 ${MOUNTING_POINT}/newfile6
counts=$(grep "y" ${MOUNTING_POINT}/newfile6 | wc -l)
if [ "$counts" == "5" ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Truncate file and set time correctly"
original_time=$(date +%s -r ${MOUNTING_POINT}/1984.TXT)
truncate -s 10 ${MOUNTING_POINT}/1984.TXT
modified_time=$(date +%s -r ${MOUNTING_POINT}/1984.TXT)
if [ $((modified_time + 0)) -gt $((original_time + 0)) ]
then
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
else
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
fi

TEST_DESCRIPTION="Delete file"
rm ${MOUNTING_POINT}/newfile4
ls -l ${MOUNTING_POINT}/newfile4 &> /dev/null
exit_code=$?
if [ $exit_code -eq 0 ]
then
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
else
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
fi

TEST_DESCRIPTION="File is deleted after re-mounting"
sleep 1  # Make sure it finished mounting
fusermount -u ${MOUNTING_POINT}
ls -l ${MOUNTING_POINT}/newfile4 &> /dev/null
exit_code=$?
if [ $exit_code -eq 0 ]
then
  echo "-------- TEST FAILED: $TEST_DESCRIPTION not working"
  clean_and_exit -1
else
  echo "-------- TEST PASSED: $TEST_DESCRIPTION"
fi

echo "---- TEST: All test TEST PASSED"

clean_and_exit 0
