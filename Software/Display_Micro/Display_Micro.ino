/* IFE 2018
   Dashboard Software
   Andrew Smith
*/

#define F_CPU 16000000LU

/* INCLUDES */
#include <SPI.h>
#include "pinout.h"
#include "mcp_can.h"
#include "mcp_can_dfs.h"
#include "ife_can.h"

/* DEFINITIONS */
#define TRUE         0x01
#define FALSE        0x00

#define LED_ON       LOW
#define LED_OFF      HIGH

typedef struct _can_t {
    uint32_t id;
    uint8_t  len;
    uint8_t  data[8];
} can_t;

/* GLOBAL VARS */
volatile uint32_t loop_counter = 0;
volatile uint32_t current = 0;
volatile uint32_t speed = 0;
volatile uint32_t voltage = 0;
MCP_CAN can(CAN_CS_PIN);
can_t rx,tx;

/* FUNCTIONS */
void fpga_write_reg(uint8_t reg, uint8_t val);
void read_can(void);


/* SETUP */
void setup() {
    /* Delay to allow FPGA to self program */
    delay(1000);

    /* Set Pin Directions */
    pinMode(DEBUG, OUTPUT);
    pinMode(DISPLAY_PWM, OUTPUT);
    pinMode(FPGA_D0, OUTPUT);
    pinMode(FPGA_D1, OUTPUT);
    pinMode(FPGA_D2, OUTPUT);
    pinMode(FPGA_D3, OUTPUT);
    pinMode(FPGA_R0, OUTPUT);
    pinMode(FPGA_R1, OUTPUT);
    pinMode(FPGA_R2, OUTPUT);
    pinMode(FPGA_R3, OUTPUT);
    pinMode(FPGA_WRITE, OUTPUT);
 
    /* Init All Registers To 0 */
    for(uint8_t i = 0; i < FPGA_NUM_REGS; i++) {
        fpga_write_reg(i, 0x00);
    }

    /* Set Display Backlight to 75% */
    analogWrite(DISPLAY_PWM, 192);

    /* Init CAN */
    while (CAN_OK != can.begin(CAN_500KBPS)) {
        delay(10);
    }

    delay(100);

    /* Disable Startup Screen */
    fpga_write_reg(0x00, 0x01);
 
    /* Config Done */
    blink_led();
}

void loop()
    if(loop_counter % 20 == 0) {
        read_can();
    }
    if(loop_counter % 200 == 0) {
        write_to_fpga();
        loop_counter = 0;
    }
    loop_counter++;
}

void inline blink_led() {
    digitalWrite(DEBUG, LOW);
    delay(5);
    digitalWrite(DEBUG, HIGH);
}

void read_can() {
    // TODO: Get Message IDs for this data
    if(can.checkReceive() == CAN_MSGAVAIL) { 
        can.ReadMsgBufID(&rx.id, &rx.len, rx.data);
        switch(rx.id) {
            case : // SEVCON RPM
            case : // SEVCON CURRENT
            case : // SEVCON VOLTAGE
        }
    }
}

void write_to_fpga() {
    uint8_t current_100s, current_10s, current_1s;
    uint8_t speed_10s, speed_1s;
    uint8_t voltage_100s, voltage_10s, voltage_1s;

    // Get places:
    current_1s = (uint8_t)(current%10);
    current_10s = (uint8_t)((current%100)/10);
    current_100s = (uint8_t)((current%1000)/100);

    speed_1s = (uint8_t)(speed%10);
    speed_10s = (uint8_t)((speed%100)/10);

    voltage_1s = (uint8_t)(voltage%10);
    voltage_10s = (uint8_t)((voltage%100)/10);
    voltage_100s = (uint8_t)((voltage%1000)/100);

    // Write Voltage:
    fpga_write_reg(FPGA_VOLTAGE_1S, voltage_1s);
    fpga_write_reg(FPGA_VOLTAGE_10S, voltage_10s);
    fpga_write_reg(FPGA_VOLTAGE_100S, voltage_100s);
    
    // Write Speed:
    fpga_write_reg(FPGA_SPEED_1S, speed_1s);
    fpga_write_reg(FPGA_SPEED_10S, speed_10s);

    // Write Current:
    fpga_write_reg(FPGA_CURRENT_1S, current_1s);
    fpga_write_reg(FPGA_CURRENT_10S, current_10s);
    fpga_write_reg(FPGA_CURRENT_100S, current_100s);
}

void fpga_write_reg(uint8_t reg, uint8_t val) {
    // Put address on bus:
    digitalWrite(FPGA_R0, reg & 0x01);
    digitalWrite(FPGA_R1, (reg >> 1) & 0x01);
    digitalWrite(FPGA_R2, (reg >> 2) & 0x01);
    digitalWrite(FPGA_R3, (reg >> 3) & 0x01);

    // Put data on bus:
    digitalWrite(FPGA_D0, val & 0x01);
    digitalWrite(FPGA_D1, (val >> 1) & 0x01);
    digitalWrite(FPGA_D2, (val >> 2) & 0x01);
    digitalWrite(FPGA_D3, (val >> 3) & 0x01);

    delay(1);
    
    // Toggle Write:
    digitalWrite(FPGA_WRITE, HIGH);
    delay(1);
    digitalWrite(FPGA_WRITE, LOW);
}
