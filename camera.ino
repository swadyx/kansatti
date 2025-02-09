#include "CanSatNeXT.h"
#include <Arduino.h>
#include <Wire.h>
#include "driver/ledc.h"
#include <SD.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Use your OV7670’s SCCB (I2C) address (often 0x42 >> 1 = 0x21)
#define OV7670_ADDR 0x21

#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO      XCLK_GPIO_NUM
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_DUTY_RES       LEDC_TIMER_1_BIT     // Using proper enumeration
#define LEDC_FREQUENCY      (8000000) // 8 MHz
#define LEDC_DUTY          (1) // 50% duty cycle with 1-bit resolution

// Pin assignments (adjust to your wiring)
#define VSYNC_PIN 33    // VSYNC: frame start/end
#define XCLK_GPIO_NUM 32  // Define the pin for XCLK
#define HREF_PIN 26   // HREF: line valid
#define PCLK_PIN 15    // PCLK: pixel clock
#define DATA0_PIN 25   // Connect OV7670 D6 (lower MSB) here
#define DATA1_PIN 27  // Connect OV7670 D7 (upper MSB) here

// Set your image dimensions (use a small resolution to keep capture time reasonable)
#define WIDTH  160
#define HEIGHT 120

// Our frame buffer: each pixel will be stored as one byte (0–3)
uint8_t frame[HEIGHT][WIDTH];
SPIClass mySpi = SPIClass(VSPI);

bool writePGMFrame(String path, uint8_t frame[][WIDTH], int height, int width) {
  Serial.println("File system");
  printFileSystem();
  // Open file for writing (this will overwrite any existing file with the same name)
  appendFile(path, 0);
  Serial.println("Path: " + path);
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file: " + path);
    return false;
  }
  
  // Write the PGM header.
  // P5 indicates a binary PGM.
  // The header format is:
  //   P5
  //   <width> <height>
  //   255
  file.print("P5\n");
  file.print(width);
  file.print(" ");
  file.print(height);
  file.print("\n255\n");
  
  // For each row in the frame, scale and write the data.
  for (int i = 0; i < height; i++) {
    // Create a temporary buffer for this row.
    uint8_t rowBuffer[WIDTH];
    for (int j = 0; j < width; j++) {
      // Scale the pixel: 0 becomes 0, 1 becomes 85, 2 becomes 170, 3 becomes 255.
      rowBuffer[j] = frame[i][j] * 85;
    }
    // Write the scaled row to the file.
    file.write(rowBuffer, width);
  }
  
  file.close();
  return true;
}

// Write one OV7670 register via I2C
void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(OV7670_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void initOV7670() {
  // Reset the sensor via COM7 (register 0x12).
  writeReg(0x12, 0x80);  // COM7: Reset
  delay(100);

  // Initialization sequence for OV7670:
  // The following register values are chosen based on datasheet data and
  // common initialization arrays for QVGA YUV mode.
  const uint8_t ov7670_init_reg[][2] = {
    {0x11, 0x01},  // CLKRC: Set clock prescaler (0x01 gives a modest division factor)
    {0x12, 0x14},  // COM7: QVGA, YUV output mode (0x14 is a typical value)
    {0x0C, 0x00},  // COM3: Disable scaling features (default)
    // {0x15, 0x02},  // COM10: Configure VSYNC (and related signal) polarity.
    {0x3E, 0x00},  // COM14: Disable extra scaling PCLK division
    {0x70, 0x3A},  // SCALING_XSC: Horizontal scaling control (example value)
    {0x71, 0x35},  // SCALING_YSC: Vertical scaling control (example value)
    {0x72, 0x11},  // SCALING_DCWCTR: Down-sampling control (example value)
    {0x73, 0xF1},  // SCALING_PCLK_DIV: Pixel clock divider (example value)
    {0xA2, 0x02},  // SCALING_PCLK_DELAY: Pixel clock delay (example value)
    // Additional registers can be added here if needed.
  };

  // Write each register value with a small delay between writes.
  const uint8_t numRegs = sizeof(ov7670_init_reg) / sizeof(ov7670_init_reg[0]);
  for (uint8_t i = 0; i < numRegs; i++) {
    writeReg(ov7670_init_reg[i][0], ov7670_init_reg[i][1]);
    delay(10);  // Allow time for each setting to take effect.
  }

  Serial.println("OV7670 initialized: QVGA YUV mode configured.");
}

void setup() {
    // Configure LEDC timer
  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_MODE,
    .duty_resolution = LEDC_DUTY_RES,
    .timer_num = LEDC_TIMER,
    .freq_hz = LEDC_FREQUENCY,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&ledc_timer);

  // Configure LEDC channel
  ledc_channel_config_t ledc_channel = {
    .gpio_num = LEDC_OUTPUT_IO,
    .speed_mode = LEDC_MODE,
    .channel = LEDC_CHANNEL,
    .timer_sel = LEDC_TIMER,
    .duty = LEDC_DUTY,
    .hpoint = 0
  };
  ledc_channel_config(&ledc_channel);
  
  Wire.begin();

  CanSatInit(100);
  Serial.begin(115200);

  // Configure VSYNC pin as input
  pinMode(VSYNC_PIN, INPUT);

  initOV7670();  // configure the OV7670 via I2C
  
  // Set up pins for the signals from the camera
  pinMode(VSYNC_PIN, INPUT);
  pinMode(HREF_PIN, INPUT);
  pinMode(PCLK_PIN, INPUT);
  pinMode(DATA0_PIN, INPUT);
  pinMode(DATA1_PIN, INPUT);

  // mySpi.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SD_CS);
  // Serial.println("Initializing SD card...");
  // if (!SD.begin(SD_CS, mySpi, 80000000)) {
  //   Serial.println("SD card initialization failed!");
  //   while (1) {
  //     Serial.println("SD card initialization failed!");
  //     delay(1000);
  //   }  
  // }
  // Serial.println("SD card initialized.");
}

void loop() {
  static uint16_t frameCounter = 0;
  String filename = "/frame" + String(frameCounter) + ".pgm";

  captureFrame();
  Serial.println("Frame captured!");

  if (writePGMFrame(filename, frame, HEIGHT, WIDTH)) {
    Serial.println("Frame saved successfully: " + filename);
  } else {
    Serial.println("Error writing frame: " + filename);
  }

  // Increment the frame counter for the next loop iteration.
  frameCounter++;
}

// Capture one frame into the global "frame" buffer.
// This routine busy-waits on the VSYNC, HREF, and PCLK signals.
void captureFrame() {
  // Wait for the start of a frame:
  while (digitalRead(VSYNC_PIN) == LOW); 
  // {
  //   Serial.println("vsync is low");
  // }
  while (digitalRead(VSYNC_PIN) == HIGH);
  // {
  //   Serial.println("vsync is high");
  // }
  
  // For each line:
  for (int y = 0; y < HEIGHT; y++) {
    // Wait for HREF to indicate line start:
    while (digitalRead(HREF_PIN) == LOW);
    // {
    //   Serial.println("href is low");
    // }
    
    // For each pixel in the line:
    for (int x = 0; x < WIDTH; x++) {
      // Wait for the pixel clock rising edge:
      while (digitalRead(PCLK_PIN) == LOW);
      // Optionally wait until PCLK falls to ensure a full cycle:
      while (digitalRead(PCLK_PIN) == HIGH);
      
      // Read the two data bits:
      uint8_t bit0 = digitalRead(DATA0_PIN);
      uint8_t bit1 = digitalRead(DATA1_PIN);
      // Combine them into a 2-bit value (0 to 3)
      uint8_t pixel = (bit1 << 1) | bit0;
      frame[y][x] = pixel;
    }
    // Wait for the line to end:
    while (digitalRead(HREF_PIN) == HIGH);
  }
}

// void loop() {
//   byte error, address;
//   int nDevices = 0;

//   for (address = 1; address < 127; address++) {
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();

//     if (error == 0) {
//       Serial.print("I2C device found at address 0x");
//       if (address < 16)
//         Serial.print("0");
//       Serial.println(address, HEX);
//       nDevices++;
//     } else if (error == 4) {
//       Serial.print("Unknown error at address 0x");
//       if (address < 16)
//         Serial.print("0");
//       Serial.println(address, HEX);
//     }
//   }

//   if (nDevices == 0)
//     Serial.println("No I2C devices found\n");
//   else
//     Serial.println("done\n");

//   delay(5000); // Wait 5 seconds before next scan
// }