#include <unity.h>
#include <modbusCRC.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_ModRTU_CRC_generates_correct_2_byte_checksum() {
    byte data[4] = {0x06,0x02,0x00,0x00};
    std::array<byte, 2> expectedCrc = {0x6c,0xa1};
 
    std::array<byte, 2> actualCRC = ModRTU_CRC(data, 4);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(actualCRC.data(), expectedCrc.data(), 2);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_ModRTU_CRC_generates_correct_2_byte_checksum);

    UNITY_END();
}