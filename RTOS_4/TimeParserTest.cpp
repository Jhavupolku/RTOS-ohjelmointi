#include <gtest/gtest.h>
#include "TimeParser.h"

TEST(TimeParserTest, ChecksStringLength)
{
    // Six characters valid
    char valid[] = "000005";
    EXPECT_EQ(time_parse(valid), 5);

    // Too short invalid
    char invalid[] = "00005";
    EXPECT_EQ(time_parse(invalid), TIME_LEN_ERROR);
}

TEST(TimeParserTest, ChecksNumericCharacters)
{
    // Only numbers
    char valid[] = "000010";
    EXPECT_EQ(time_parse(valid), 10);

    // invalid with letters
    char invalid[] = "000A10";
    EXPECT_EQ(time_parse(invalid), TIME_ARRAY_ERROR);
}

TEST(TimeParserTest, RejectsZeroSeconds)
{
    // Positive valid
    char valid[] = "000001";
    EXPECT_EQ(time_parse(valid), 1);

    // Invalid zero seconds
    char invalid[] = "000000";
    EXPECT_EQ(time_parse(invalid), TIME_ZERO_ERROR);
}

TEST(TimeParserTest, ChecksNullAndEmptyInput)
{
    // Valid string
    char valid[] = "000005";
    EXPECT_EQ(time_parse(valid), 5);

    // Invalid empty
    char empty[] = "";
    EXPECT_EQ(time_parse(empty), TIME_ARRAY_ERROR);

    // Invalid NULL
    EXPECT_EQ(time_parse(NULL), TIME_ARRAY_ERROR);
}