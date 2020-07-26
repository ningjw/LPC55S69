#ifndef __BATTERY_DRV_H
#define __BATTERY_DRV_H

// LTC2942 HAL
#define I2C1_MASTER_BASE         LPI2C1_BASE // I2C port where the LTC2942 connected
#define I2C1_MASTER              ((LPI2C_Type *)I2C1_MASTER_BASE)
// LTC2942 address
#define LTC2942_ADDR                    (0x64)


// LTC2942 register definitions
#define LTC2942_REG_STATUS              0x00 // (A) Status
#define LTC2942_REG_CONTROL             0x01 // (B) Control
#define LTC2942_REG_AC_H                0x02 // (C) Accumulated charge MSB
#define LTC2942_REG_AC_L                0x03 // (D) Accumulated charge LSB
#define LTC2942_REG_CTH_H               0x04 // (E) Charge threshold high MSB
#define LTC2942_REG_CTH_L               0x05 // (F) Charge threshold high LSB
#define LTC2942_REG_CTL_H               0x06 // (G) Charge threshold low MSB
#define LTC2942_REG_CTL_L               0x07 // (H) Charge threshold low LSB
#define LTC2942_REG_VOL_H               0x08 // (I) Voltage MSB
#define LTC2942_REG_VOL_L               0x09 // (J) Voltage LSB
#define LTC2942_REG_VOLT_H              0x0A // (K) Voltage threshold high
#define LTC2942_REG_VOLT_L              0x0B // (L) Voltage threshold low
#define LTC2942_REG_TEMP_H              0x0C // (M) Temperature MSB
#define LTC2942_REG_TEMP_L              0x0D // (N) Temperature LSB
#define LTC2942_REG_TEMPT_H             0x0E // (O) Temperature threshold high
#define LTC2942_REG_TEMPT_L             0x0F // (P) Temperature threshold low

// LTC2942 status register bit definitions
#define LTC2942_STATUS_CHIPID           0x80 // A[7] Chip identification (0: LTC2942-1, 0: LTC2941-1)
#define LTC2942_STATUS_AC_OVR           0x20 // A[5] Accumulated charge overflow/underflow
#define LTC2942_STATUS_TEMP_ALRT        0x10 // A[4] Temperature alert
#define LTC2942_STATUS_CHG_ALRT_H       0x08 // A[3] Charge alert high
#define LTC2942_STATUS_CHG_ALRT_L       0x04 // A[2] Charge alert low
#define LTC2942_STATUS_VOL_ALRT         0x02 // A[1] Voltage alert
#define LTC2942_STATUS_UVLO_ALRT        0x01 // A[0] Undervoltage lockout alert

// LTC2942 control register bit definitions
#define LTC2942_CTL_ADC_MSK             0x3F // ADC mode bits [7:6]
#define LTC2942_CTL_PSCM_MSK            0xC7 // Prescaler M bits [5:3]
#define LTC2942_CTL_ALCC_MSK            0xF9 // AL/CC pin control [2:1]
#define LTC2942_CTL_SHUTDOWN            0x01 // B[0] Shutdown


// LTC2942 ADC mode enumeration
enum {
	LTC2942_ADC_AUTO   = 0xC0, // Automatic mode
	LTC2942_ADC_M_VOL  = 0x80, // Manual voltage mode
	LTC2942_ADC_M_TEMP = 0x40, // Manual temperature mode
	LTC2942_ADC_SLEEP  = 0x00  // Sleep
};

// LTC2942 prescaler M enumeration
enum {
	LTC2942_PSCM_1   = 0x00,
	LTC2942_PSCM_2   = 0x08,
	LTC2942_PSCM_4   = 0x10,
	LTC2942_PSCM_8   = 0x18,
	LTC2942_PSCM_16  = 0x20,
	LTC2942_PSCM_32  = 0x28,
	LTC2942_PSCM_64  = 0x30,
	LTC2942_PSCM_128 = 0x38
};

// LTC2942 AL/CC pin mode enumeration
enum {
	LTC2942_ALCC_DISABLED = 0x00, // AL/CC pin disabled
	LTC2942_ALCC_CHG      = 0x02, // Charge complete mode
	LTC2942_ALCC_ALERT    = 0x04  // Alert mode
};

// State of analog section enumeration
enum {
	LTC2942_AN_DISABLED = 0x00,
	LTC2942_AN_ENABLED  = 1,
};


// Function prototypes
uint8_t LTC2942_GetStatus(void);
uint8_t LTC2942_GetControl(void);
uint32_t LTC2942_GetVoltage(void);
int32_t LTC2942_GetTemperature(void);
uint16_t LTC2942_GetAC(void);

void LTC2942_SetAC(uint16_t AC);
void LTC2942_SetADCMode(uint8_t mode);
void LTC2942_SetPrescaler(uint8_t psc);
void LTC2942_SetALCCMode(uint8_t mode);
void LTC2942_SetAnalog(uint8_t state);


#endif
