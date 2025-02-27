#include "unity.h"
#include "etc_controller.h"
#include "mock_sensor.h"  // Assuming mock functions for sensor inputs

/**
 * Set up a test environment for each test case
 * Function configures the sensor mocks with predefined readings
 * and resets state of the timers and controller before each test
 */
void setUp(void) {
    mock_sensor_reset();  // Reset sensor mocks to default states
    ETCController_reset();  // Reset ETC controller state, including timers
}

/**
 * Tests the `updatePedalTravel` function under normal operating conditions.
 * Ensures that the motor remains enabled and the pedal travel is calculated correctly based on voltage dividers.
 */
void test_updatePedalTravel_NormalOperation(void) {
    // Setting sensors to a normal operating voltage within the expected range after voltage division
    mock_HE1_read_Return(2.25f);  // Midpoint of raw voltage range 0.5-4.5V, divided by 2
    mock_HE2_read_Return(1.53f);  // Midpoint of raw voltage range 0.5-4.5V, adjusted by 330/480

    ETCController_updatePedalTravel();

    TEST_ASSERT_TRUE(ETCController_state.motor_enabled);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, ETCController_state.pedal_travel);
}

/**
 * Tests the `updatePedalTravel` function with sensor voltages out of the acceptable range for their respective dividers.
 * Verifies that the motor is disabled if the voltage remains out of range for more than 100ms.
 */
void test_updatePedalTravel_VoltageOutOfRange(void) {
    mock_HE1_read_Return(0.25f);  // Below minimum threshold after division for HE1
    mock_HE2_read_Return(0.16f);  // Below minimum threshold after division for HE2

    ETCController_updatePedalTravel();
    ETCController_voltage_timer_elapsed_ms_Return(101);  // Simulate timer exceeding 100ms
    ETCController_updatePedalTravel();

    TEST_ASSERT_FALSE(ETCController_state.motor_enabled);
}

/**
 * Tests the implausibility check in `updatePedalTravel`.
 * Ensures that the motor is disabled if the difference in pedal travels exceeds 10%.
 */
void test_updatePedalTravel_ImplausibilityCheck(void) {
    mock_HE1_read_Return(4.5f);  // High voltage edge case for HE1 after division
    mock_HE2_read_Return(0.5f);  // Low voltage edge case for HE2 after division

    ETCController_updatePedalTravel();
    ETCController_implausibility_timer_elapsed_ms_Return(101);  // Simulate timer exceeding 100ms
    ETCController_updatePedalTravel();

    TEST_ASSERT_FALSE(ETCController_state.motor_enabled);
}

/**
 * Main function to run the defined tests.
 */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_updatePedalTravel_NormalOperation);
    RUN_TEST(test_updatePedalTravel_VoltageOutOfRange);
    RUN_TEST(test_updatePedalTravel_ImplausibilityCheck);
    return UNITY_END();
}

