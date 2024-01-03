import unittest
import subprocess
import fs
import re


class TestFatFuse(unittest.TestCase):
    mount_dir = "./mnt"

    @classmethod
    def run_command(self, command):
        """Runs a command in a process."""
        try:
            result = subprocess.run(command.split(" "),
                                    capture_output=True, text=True, check=True)
            return result
        except subprocess.CalledProcessError as e:
            raise e
        except Exception as e:
            raise e

    @classmethod
    def mount(self):
        self.run_command(f"make mount")
        self.run_command(f"ls -laR {self.mount_dir}")


    @classmethod
    def unmount(self):
        self.run_command("make unmount")

    @classmethod
    def make_image(self):
        self.run_command("make image")


class TestFilesystem(TestFatFuse):
    """Test filesystem operations with the mounted filesystem. Also test basic logging functionality."""
    @classmethod
    def setUpClass(self):
        """This method is called only once before any tests in this class runs."""
        try:
            self.run_command("make clean")
            self.run_command("make")
            self.run_command("make image")
            self.run_command("make unmount")
        except Exception as error:
            raise error

    def setUp(self):
        """This method is called before each test. It guarantees that the filesystem is mounted."""
        try:
            self.mount()
        except Exception as e:
            self.fail("Mounting failed.")

    def tearDown(self):
        """This method is called after each test. It guarantees that the filesystem is unmounted. And that the image is fresh."""
        try:
            self.unmount()
        except:
            self.fail("Unmounting failed.")
        try:
            self.make_image()
        except:
            self.fail("make image failed.")

    def test_create_delete_file(self):
        """Testing file creation"""
        try:
            self.run_command(f"touch {self.mount_dir}/test.txt")
        except Exception as e:
            self.fail("Creating file failed.")
        try:
            self.run_command(f"rm {self.mount_dir}/test.txt")
        except Exception as e:
            self.fail("Deleting file failed.")

    def test_create_dir(self):
        """Test directory creation with mkdir and listing with ls"""
        try:
            self.run_command(f"mkdir {self.mount_dir}/test")
        except Exception as e:
            self.fail("Creating directory failed.")
        try:
            self.run_command(f"ls {self.mount_dir}/test")
        except Exception as e:
            self.fail("Listing directory failed.")

    def test_rmdir(self):
        """Test directory deletion with rmdir and listing with ls"""
        try:
            self.run_command(f"rmdir {self.mount_dir}/EMPTYD")
        except Exception as e:
            self.fail("Deleting directory failed.")
        try:
            result = self.run_command(f"ls {self.mount_dir}")
            pattern = r'^(?!.*\bEMPTY\b).*'
            match = re.match(pattern, result.stdout, re.DOTALL)
            self.assertTrue(match)
        except Exception as e:
            self.fail("Listing directory failed.")

    def test_rmdir_non_empty(self):
        """Test that rmdir fails when directory is not empty"""
        try:
            result = self.run_command(f"rmdir {self.mount_dir}/NEMPTYD")
            if result.returncode == 0:
                self.fail("Deleting non-empty directory succeeded.")
        except subprocess.CalledProcessError as e:
            self.assertIn("not empty", e.stderr)
            pass

    def test_rm(self):
        """Test file deletion with rm"""
        try:
            self.run_command(f"rm {self.mount_dir}/HOLA.TXT")
            print('HOLA.TXT deleted successfully\n')
        except Exception as e:
            self.fail("Deleting file failed.")

    def test_rm_no_such_file_or_dir(self):
        """Test that rm fails when target does not exist"""
        try:
            result = self.run_command(f"rm {self.mount_dir}/NOEXIST")
            if result.returncode == 0:
                self.fail("Deleting non-existing file succeeded.")
        except subprocess.CalledProcessError as e:
            self.assertIn("file or directory", e.stderr)
            pass

    def test_rm_not_a_file(self):
        """Test that rm fails when target is not a file"""
        try:
            result = self.run_command(f"rm {self.mount_dir}/EMPTYD")
            if result.returncode == 0:
                self.fail("Deleting non-file succeeded.")
        except subprocess.CalledProcessError as e:
            self.assertIn("a directory", e.stderr)
            pass

    def test_write_to_file(self):
        """Test writing to a file"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        fat_fuse_fs.writetext('test.txt', 'test text')
        read_text = fat_fuse_fs.readtext('test.txt')
        fat_fuse_fs.close()
        self.assertIn('test text', read_text)

    def test_read_log_file(self):
        """Test that log file initiates"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        read_text = fat_fuse_fs.readtext('fs.log')
        fat_fuse_fs.close()
        self.assertIn('INIT', read_text)

    def test_write_operation_logged(self):
        """Test that write operation is logged"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        fat_fuse_fs.writetext('test.txt', 'test text')
        read_text = fat_fuse_fs.readtext('fs.log').split('\n')
        fat_fuse_fs.close()
        self.assertEqual(3, len(read_text))
        self.assertIn('write', read_text[1])

    def test_read_operation_logged(self):
        """Test that read operation is logged"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        fat_fuse_fs.readtext('HOLA.TXT')
        read_text = fat_fuse_fs.readtext('fs.log').split('\n')
        fat_fuse_fs.close()
        self.assertEqual(3, len(read_text))
        self.assertIn('read', read_text[1])

    def test_log_file_not_visible(self):
        """Test that log file is not visible"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        n_visible_files = len(fat_fuse_fs.listdir('/'))
        fat_fuse_fs.close()
        self.assertEqual(n_visible_files, 3)

    def test_log_file_not_visible_with_ls(self):
        """Test that log file is not visible with ls command"""
        result = self.run_command(f"ls -1 {self.mount_dir}")
        self.assertNotIn('fs.log', result.stdout)

    def test_log_file_not_visible_with_find(self):
        """Test that log file is not visible with find command"""
        result = self.run_command(f"find {self.mount_dir}")
        self.assertNotIn('fs.log', result.stdout)

    def test_logfile_persists_after_remount(self):
        """Test that log file persists after remount"""
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        read_text = fat_fuse_fs.readtext('fs.log')
        fat_fuse_fs.close()
        self.assertIn('INIT', read_text)
        self.unmount()

        self.mount()
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        read_text = fat_fuse_fs.readtext('fs.log')
        fat_fuse_fs.close()
        self.assertIn('INIT', read_text)

    def test_logfile_persists_after_remount_with_write(self):
        """Test that log file persists after remount with write"""
        self.mount()
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        fat_fuse_fs.writetext('test.txt', 'test text')
        read_text = fat_fuse_fs.readtext('fs.log')
        fat_fuse_fs.close()
        self.assertIn('write', read_text)
        self.unmount()

        self.mount()
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        fat_fuse_fs.listdir('/')
        read_text = fat_fuse_fs.readtext('fs.log')
        fat_fuse_fs.close()
        self.assertIn('write', read_text)

    def test_logfile_not_visible_when_mounted_with_native_fs(self):
        """Test that log file is not visible when mounted with native fs"""
        self.run_command(f"sudo make unmount")
        self.run_command(f"sudo make mount-native")
        fat_fuse_fs = fs.open_fs('osfs://./mnt')
        files = fat_fuse_fs.listdir('/')
        self.assertNotIn('fs.log', files)
        self.assertEqual(len(files), 3)

class TestCompilation(TestFatFuse):
    """Test compilation of the project with make and make clean"""

    def test_clean_and_make(self):
        try:
            self.run_command("make clean")
            self.run_command("make")
        except Exception as error:
            raise error


if __name__ == '__main__':
    unittest.main()
