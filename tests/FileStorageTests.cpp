#include <gtest/gtest.h>
#include <filesystem>
#include <vector>
#include <string>

#include "FileStorage.h"

using namespace S3FileStorage;

class FileStorageTest : public ::testing::Test
{
protected:
    std::string test_storage_path;
    std::unique_ptr<FileStorage> storage;

    void SetUp() override
    {
        test_storage_path = "/tmp/s3_test_storage_" + std::to_string(getpid());
        storage = std::make_unique<FileStorage>(test_storage_path);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_storage_path))
        {
            std::filesystem::remove_all(test_storage_path);
        }
    }
};

TEST_F(FileStorageTest, ConstructorCreatesStoragePath)
{
    EXPECT_TRUE(std::filesystem::exists(test_storage_path));
    EXPECT_TRUE(std::filesystem::is_directory(test_storage_path));
}

TEST_F(FileStorageTest, GetStoragePath)
{
    EXPECT_EQ(storage->s3_storage_path(), test_storage_path);
}

TEST_F(FileStorageTest, SetStoragePath)
{
    std::string new_path = "/tmp/new_test_path";
    storage->s3_storage_path(new_path);
    EXPECT_EQ(storage->s3_storage_path(), new_path);
}

TEST_F(FileStorageTest, CreateFolder_Success)
{
    auto [success, error] = storage->create_folder("test-bucket");

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
    EXPECT_TRUE(std::filesystem::exists(test_storage_path + "/test-bucket"));
}

TEST_F(FileStorageTest, CreateFolder_EmptyName)
{
    auto [success, error] = storage->create_folder("");

    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, CreateFolder_AlreadyExists)
{
    storage->create_folder("existing-bucket");
    auto [success, error] = storage->create_folder("existing-bucket");

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
}

TEST_F(FileStorageTest, WriteFile_BinaryData)
{
    storage->create_folder("test-bucket");

    std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    auto [success, error] = storage->write_file("test-bucket", "test.bin", data);

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
    EXPECT_TRUE(std::filesystem::exists(test_storage_path + "/test-bucket/test.bin"));
}

TEST_F(FileStorageTest, WriteFile_StringData)
{
    storage->create_folder("test-bucket");

    std::string data = "Hello, World!";
    auto [success, error] = storage->write_file("test-bucket", "test.txt", data);

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
}

TEST_F(FileStorageTest, WriteFile_EmptyBucketName)
{
    std::vector<uint8_t> data = {0x01, 0x02};
    auto [success, error] = storage->write_file("", "test.bin", data);

    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, WriteFile_EmptyObjectKey)
{
    std::vector<uint8_t> data = {0x01, 0x02};
    auto [success, error] = storage->write_file("test-bucket", "", data);

    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, WriteFile_CreatesParentDirectory)
{
    std::string data = "test content";
    auto [success, error] = storage->write_file("new-bucket", "path/to/file.txt", data);

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
    EXPECT_TRUE(std::filesystem::exists(test_storage_path + "/new-bucket/path/to/file.txt"));
}

TEST_F(FileStorageTest, ReadFile_Success)
{
    storage->create_folder("test-bucket");

    std::vector<uint8_t> write_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    storage->write_file("test-bucket", "test.bin", write_data);

    auto [read_data, error] = storage->read_file("test-bucket", "test.bin");

    EXPECT_TRUE(read_data.has_value());
    EXPECT_FALSE(error.has_value());
    EXPECT_EQ(read_data.value(), write_data);
}

TEST_F(FileStorageTest, ReadFile_NotExists)
{
    auto [data, error] = storage->read_file("test-bucket", "nonexistent.txt");

    EXPECT_FALSE(data.has_value());
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, ReadFile_EmptyBucketName)
{
    auto [data, error] = storage->read_file("", "test.txt");

    EXPECT_FALSE(data.has_value());
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, ReadFile_EmptyObjectKey)
{
    auto [data, error] = storage->read_file("test-bucket", "");

    EXPECT_FALSE(data.has_value());
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, DeleteFile_Success)
{
    storage->create_folder("test-bucket");
    std::string data = "test content";
    storage->write_file("test-bucket", "test.txt", data);

    auto [success, error] = storage->delete_file("test-bucket", "test.txt");

    EXPECT_TRUE(success);
    EXPECT_FALSE(error.has_value());
    EXPECT_FALSE(std::filesystem::exists(test_storage_path + "/test-bucket/test.txt"));
}

TEST_F(FileStorageTest, DeleteFile_NotExists)
{
    auto [success, error] = storage->delete_file("test-bucket", "nonexistent.txt");

    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, DeleteFile_EmptyBucketName)
{
    auto [success, error] = storage->delete_file("", "test.txt");

    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, FileExists_True)
{
    storage->create_folder("test-bucket");
    std::string data = "test content";
    storage->write_file("test-bucket", "test.txt", data);

    auto [exists, error] = storage->file_exists("test-bucket", "test.txt");

    EXPECT_TRUE(exists);
    EXPECT_FALSE(error.has_value());
}

TEST_F(FileStorageTest, FileExists_False)
{
    auto [exists, error] = storage->file_exists("test-bucket", "nonexistent.txt");

    EXPECT_FALSE(exists);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, FileExists_EmptyParams)
{
    auto [exists, error] = storage->file_exists("", "test.txt");

    EXPECT_FALSE(exists);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, GetFilePath)
{
    auto path = storage->get_file_path("my-bucket", "my-object.txt");

    EXPECT_EQ(path, test_storage_path + "/my-bucket/my-object.txt");
}

TEST_F(FileStorageTest, GetFileSize_Success)
{
    storage->create_folder("test-bucket");
    std::string data = "Hello, World!";
    storage->write_file("test-bucket", "test.txt", data);

    auto [size, error] = storage->get_file_size("test-bucket", "test.txt");

    EXPECT_TRUE(size.has_value());
    EXPECT_EQ(size.value(), data.size());
    EXPECT_FALSE(error.has_value());
}

TEST_F(FileStorageTest, GetFileSize_NotExists)
{
    auto [size, error] = storage->get_file_size("test-bucket", "nonexistent.txt");

    EXPECT_FALSE(size.has_value());
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, WriteAndReadLargeFile)
{
    storage->create_folder("test-bucket");

    std::vector<uint8_t> large_data(1024 * 1024); // 1MB
    for (size_t i = 0; i < large_data.size(); ++i)
    {
        large_data[i] = static_cast<uint8_t>(i % 256);
    }

    auto [write_success, write_error] = storage->write_file("test-bucket", "large.bin", large_data);
    EXPECT_TRUE(write_success);

    auto [read_data, read_error] = storage->read_file("test-bucket", "large.bin");
    EXPECT_TRUE(read_data.has_value());
    EXPECT_EQ(read_data.value(), large_data);
}

TEST_F(FileStorageTest, OverwriteExistingFile)
{
    storage->create_folder("test-bucket");

    std::string data1 = "First content";
    storage->write_file("test-bucket", "test.txt", data1);

    std::string data2 = "Second content - different!";
    auto [success, error] = storage->write_file("test-bucket", "test.txt", data2);

    EXPECT_TRUE(success);

    auto [read_data, read_error] = storage->read_file("test-bucket", "test.txt");
    EXPECT_TRUE(read_data.has_value());

    std::string read_str(read_data.value().begin(), read_data.value().end());
    EXPECT_EQ(read_str, data2);
}

TEST_F(FileStorageTest, EmptyStoragePath)
{
    FileStorage empty_storage("");

    auto [success, error] = empty_storage.create_folder("bucket");
    EXPECT_FALSE(success);
    EXPECT_TRUE(error.has_value());
}

TEST_F(FileStorageTest, NestedObjectKey)
{
    std::string data = "nested file content";
    auto [success, error] = storage->write_file("bucket", "path/to/nested/file.txt", data);

    EXPECT_TRUE(success);
    EXPECT_TRUE(std::filesystem::exists(test_storage_path + "/bucket/path/to/nested/file.txt"));
}
