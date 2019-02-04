#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    NginxConfigParser parser;
    NginxConfig out_config;
};

//valid basic config test: 1 simple directive and 1 block directive
TEST_F(NginxConfigParserTest, SimpleConfig)
{
    bool success = parser.Parse("example_config", &out_config);

    EXPECT_TRUE(success);
}

//valid empty config test: no whitespace, no content, just empty file
TEST_F(NginxConfigParserTest, EmptyConfig)
{
    bool success = parser.Parse("empty_config", &out_config);

    EXPECT_TRUE(success);
}

//valid config with only whitespace inside: tabs, newlines, and spaces only
TEST_F(NginxConfigParserTest, WhitespaceOnlyConfig)
{
    bool success = parser.Parse("whitespace_only_config", &out_config);

    EXPECT_TRUE(success);
}

//valid config with 1 context block (nested block directive)
TEST_F(NginxConfigParserTest, SingleContextConfig)
{
    bool success = parser.Parse("single_context_config", &out_config);

    EXPECT_TRUE(success);
}

//invalid config, missing } for block directive
TEST_F(NginxConfigParserTest, OpenBlockConfig)
{
    bool success = parser.Parse("open_block_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, missing } for context
TEST_F(NginxConfigParserTest, OpenContextConfig)
{
    bool success = parser.Parse("open_context_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, extra } at the end of a context
TEST_F(NginxConfigParserTest, ExtraEndBraceConfig)
{
    bool success = parser.Parse("extra_end_brace_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, ' at the end of a config
TEST_F(NginxConfigParserTest, EndsWithSingleQuote)
{
    bool success = parser.Parse("ends_with_single_quote_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, " at the end of a config
TEST_F(NginxConfigParserTest, EndsWithDoubleQuote)
{
    bool success = parser.Parse("ends_with_double_quote_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, { not on own line
TEST_F(NginxConfigParserTest, DoubleOpenBracer)
{
    bool success = parser.Parse("open_bracer_not_on_own_line_config", &out_config);

    EXPECT_FALSE(success);
}

//invalid config, config doesn't exist
TEST_F(NginxConfigParserTest, InvalidConfig)
{
    bool success = parser.Parse("non_existent_config", &out_config);

    EXPECT_FALSE(success);
}